/*
 *	(c) 2020-21 Stuart Hunter
 *
 *	TODO:
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	See <http://www.gnu.org/licenses/> to get a copy of the GNU General
 *	Public License.
 *
 */

#include "fan_control.h"
#include "icons.h"
#include "mem_info.h"
#include "oled_fonts.h"
#include "ssd1306_i2c.h"
#include "sysinfoled.h"
#include "timer.h"
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/vfs.h>
#include <time.h>
#include <unistd.h>

void dodelay(uint16_t d) { usleep(d * 1000); }

bool goser = false;
bool freed = false;
// free and cleanup here
void before_exit(void) {
    printf("\nCleanup and shutdown\n");
    if (!freed) { // ??? race condition
        freed = true;
        ssd1306_clearDisplay();
        //ssd1306_stopscroll();
        ssd1306_displayOff();
    }
    printf("All Done\nBye Bye.\n");
}

void sigint_handler(int sig) {
    before_exit();
    exit(0);
}

void attach_signal_handler(void) {

    struct sigaction new_action, old_action;

    new_action.sa_handler = sigint_handler;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = 0;

    sigaction(SIGINT, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN)
        sigaction(SIGINT, &new_action, NULL);
    sigaction(SIGHUP, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN)
        sigaction(SIGHUP, &new_action, NULL);
    sigaction(SIGTERM, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN)
        sigaction(SIGTERM, &new_action, NULL);
    sigaction(SIGQUIT, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN)
        sigaction(SIGQUIT, &new_action, NULL);
    sigaction(SIGSTOP, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN)
        sigaction(SIGSTOP, &new_action, NULL);

    new_action.sa_flags = SA_NODEFER;
    sigaction(SIGSEGV, &new_action,
              NULL); /* catch seg fault - and hopefully backtrace */
}

int elementLength(int szh, int szw) { return szh * (int)((szw + 7) / 8); }

void putIcon(DrawAttr *disp) {
    int szw = 28;
    int szh = 28;
    int w = elementLength(szh, szw);
    uint8_t dest[w];
    int start = (int)disp->icon * w;
    memcpy(dest, deskpi28x28() + start, sizeof dest);
    ssd1306_fillRect(2, 2, szw, szh, BLACK);
    ssd1306_drawBitmap(2, 2, dest, szw, szh, WHITE);
}

void drawIP(DrawAttr *disp) {
    int szw = 8;
    int szh = 24;
    int el = elementLength(szh, szw);
    uint8_t dest[el];
    int x = disp->pos.x;
    int start = 0;
    for (size_t i = 0; i < strlen(disp->value); i++) {
        int wa = szw;
        uint8_t cc = disp->value[i];
        switch (cc) {
            case '.':
                start = 11 * el;
                wa = 6;
                break;
            case ':':
                start = 12 * el;
                wa = 6;
                break;
            case ' ':
                start = 13 * el;
                wa = 6;
                break;
            case '?': start = 14 * el; break;
            default: start = (cc - 48) * el;
        }
        memcpy(dest, thinman24x8() + start, sizeof dest);
        ssd1306_drawBitmap(x - ((6 == wa) ? 2 : 0), disp->pos.y, dest, szw, szh,
                           WHITE);
        x += wa;
    }
}

void drawTime(DrawAttr *disp, DrawTime *dt) {

    int x = dt->pos.x;
    size_t ll = strlen(disp->lastval);
    const int sepWidth = 6;
    const int offset = 6;
    const int tsize = 3;

    for (size_t i = 0; i < strlen(disp->value); i++) {
        // selective updates, less "blink"
        int w = dt->charWidth;
        if ((i > ll) || (disp->value[i] != disp->lastval[i])) {
            if ((i > ll) || ('X' == disp->lastval[i])) {
                w = ((disp->lastval[i] == ':') || (disp->lastval[i] == ' '))
                        ? sepWidth
                        : dt->charWidth;
                ssd1306_fillRect((w == sepWidth) ? x - offset : x,
                                 dt->pos.y - 1, w, dt->charHeight + 2, BLACK);
            } else {
                w = ((disp->lastval[i] == ':') || (disp->lastval[i] == ' '))
                        ? sepWidth
                        : dt->charWidth;
                ssd1306_drawChar((w == sepWidth) ? x - offset : x, dt->pos.y,
                                 disp->lastval[i], BLACK, tsize); // soft erase
            }
            w = ((disp->value[i] == ':') || (disp->value[i] == ' '))
                    ? sepWidth
                    : dt->charWidth;
            ssd1306_drawChar((w == sepWidth) ? x - offset : x, dt->pos.y,
                             disp->value[i], WHITE, tsize);
        }
        w = ((disp->value[i] == ':') || (disp->value[i] == ' '))
                ? sepWidth
                : dt->charWidth;
        x += w;
    }
    strcpy(disp->lastval, disp->value);
}

// read CPU temperature and execute fan controller
void fanTimer(size_t timer_id, void *user_data) {

    DrawAttr *attr = (DrawAttr *)user_data;
    double temp = 0.00;
    char buf[MAX_SIZE];

    // Read CPU temperature
    int fh = open(TEMP_FILE, O_RDONLY);
    if (fh < 0) {
        temp = 0.00;
        printf("failed to open %s\n", TEMP_FILE);
    } else {
        if (read(fh, buf, sizeof(buf)) < 0) {
            temp = 0.00;
            printf("failed to read temp\n");
        } else {
            // Convert to floating point printing
            temp = atoi(buf) / 1000.0;
            bool fan = fanControl(temp, &goser);
            // should use fan icon - fix this
            sprintf(attr->value, " %.1fC %s", temp, ((fan) ? "On" : "Off"));
        }
    }
    close(fh);
}

int main(void) {

    attach_signal_handler();

    struct DrawAttr attrs[6] = {{true,
                                 {30, 8},
                                 128,
                                 24,
                                 IT_CPU_INFO,
                                 true,
                                 {0},
                                 INIT_ATTR_VALUE}, // AT_CPU_INFO
                                {true,
                                 {30, 8},
                                 128,
                                 24,
                                 IT_CPU_TEMP,
                                 true,
                                 {0},
                                 INIT_ATTR_VALUE}, // AT_CPU_TEMP
                                {false,
                                 {30, 8},
                                 128,
                                 24,
                                 IT_MEM_INFO,
                                 true,
                                 {0},
                                 INIT_ATTR_VALUE}, // AT_MEM_INFO
                                {false,
                                 {30, 8},
                                 128,
                                 24,
                                 IT_DISK_INFO,
                                 true,
                                 {0},
                                 INIT_ATTR_VALUE}, // AT_DISK_INFO
                                {true,
                                 {1, 8},
                                 128,
                                 24,
                                 IT_NET_ETHER,
                                 true,
                                 {0},
                                 INIT_ATTR_VALUE}, // AT_NET_INFO
                                {true,
                                 {2, 2},
                                 128,
                                 26,
                                 IT_NULL_ICON,
                                 false,
                                 {0},
                                 INIT_ATTR_VALUE}}; // AT_CLOCK_INFO

    loadFanConfiguration();
    goser = init_serial("/dev/ttyUSB0");
    size_t fTimer;
    fTimer = timer_start(1000, fanTimer, TIMER_PERIODIC, (void *)&attrs[1]);

    time_t time_now;
    struct tm *loctm;
    bool paint = false;
    int trick = 0;
    int frame = 0;
    int frames = 4; // 0..4
    int fd_temp;
    char buf[MAX_SIZE];

    // get system usage / info
    struct sysinfo sys_info;
    struct statfs disk_info;
    struct memInfo mem_info;

    struct ifaddrs *ifAddrStruct = NULL;
    void *tmpAddrPtr = NULL;
    getifaddrs(&ifAddrStruct);

    DrawTime dt = {.charWidth = 18,
                   .charHeight = 26,
                   .bufferLen = 0,
                   .pos = {5, 4},
                   .font = 0};

    ssd1306_begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS);
    ssd1306_clearDisplay();

    while (true) {

        trick = 5 * frames; // 30 SECONDS

        while (--trick) {

            // Read the system information
            if (sysinfo(&sys_info) != 0) {
                printf("sysinfo-Error\n");
                ssd1306_clearDisplay();
                char *text = "S Y S I N F O - E r r o r";
                ssd1306_drawString(text);
                ssd1306_display();
                dodelay(500);

                // prime for reset
                for (int initr = 0; initr < 4; initr++)
                    if (0 == attrs[initr].lastval[0])
                        continue;
                    else
                        attrs[initr].lastval[0] = 0;

                continue;

            } else {

                ssd1306_setTextSize(2);
                strcpy(attrs[AT_CLOCK_INFO].lastval, INIT_ATTR_VALUE);

                paint = false;
                int lastframe = frame;
                frame = (int)trick / frames;

                // CPU usage
                unsigned long avgCPULoad = sys_info.loads[0] / 1000;
                sprintf(attrs[AT_CPU_INFO].value, " CPU%% %ld", avgCPULoad);

                // Running memory usage, remaining/total memory
                char totalBuff[12];
                char usedBuff[12];

                scanMemoryInfo(&mem_info);
                int z =
                    readableUnits(usedBuff, mem_info.usedMem, sizeof(usedBuff));
                z = readableUnits(totalBuff, mem_info.totalMem,
                                  sizeof(totalBuff));
                sprintf(attrs[AT_MEM_INFO].value, " %s/%s", usedBuff,
                        totalBuff);

                // Get IP address
                while (ifAddrStruct != NULL) {
                    if (ifAddrStruct->ifa_addr != NULL &&
                        ifAddrStruct->ifa_addr->sa_family == AF_INET) {
                        // check it is IP4 is a valid IP4 Address
                        tmpAddrPtr =
                            &((struct sockaddr_in *)ifAddrStruct->ifa_addr)
                                 ->sin_addr;
                        char addressBuffer[INET_ADDRSTRLEN];
                        inet_ntop(AF_INET, tmpAddrPtr, addressBuffer,
                                  INET_ADDRSTRLEN);
                        if (strcmp(ifAddrStruct->ifa_name, "eth0") == 0) {
                            sprintf(attrs[AT_NET_INFO].value, "%s",
                                    addressBuffer);
                            attrs[AT_NET_INFO].icon = IT_NULL_ICON;
                            break;
                        } else if (strcmp(ifAddrStruct->ifa_name, "wlan0") ==
                                   0) {
                            sprintf(attrs[AT_NET_INFO].value, "%s",
                                    addressBuffer);
                            attrs[AT_NET_INFO].icon = IT_NULL_ICON;
                            break;
                        } else {
                            sprintf(attrs[AT_NET_INFO].value,
                                    "???.???.???.???");
                        }
                    }
                    ifAddrStruct = ifAddrStruct->ifa_next;
                }

                // Read disk space, remaining/total space
                statfs("/", &disk_info);

                long totalBlocks = disk_info.f_bsize;
                long totalSize = totalBlocks * disk_info.f_blocks;
                long freeDisk = disk_info.f_bfree * totalBlocks;

                z = readableUnits(usedBuff, freeDisk, sizeof(usedBuff));
                z = readableUnits(totalBuff, totalSize, sizeof(totalBuff));
                sprintf(attrs[AT_DISK_INFO].value, " %s/%s", usedBuff,
                        totalBuff);

// less of a helper with the idx based impl. clean enough so a keeper
#define dispAttr(disp)                                                         \
    if (strcmp(disp.value, disp.lastval) != 0) {                               \
        if (strcmp(disp.lastval, INIT_ATTR_VALUE) != 0)                        \
            paint = true;                                                      \
        if (disp.forceClear) {                                                 \
            ssd1306_clearDisplay();                                            \
            paint = true;                                                      \
        }                                                                      \
        if (IT_NULL_ICON != disp.icon)                                         \
            putIcon(&disp);                                                    \
        ssd1306_fillRect(disp.pos.x, disp.pos.y, disp.w, disp.h, BLACK);       \
        ssd1306_drawText(disp.pos.x, disp.pos.y, disp.value);                  \
    }

                if (attrs[frame].visible) {
                    if (frame == AT_NET_INFO) {
                        drawIP(
                            &attrs
                                [frame]); // display IP - specialized tall-skinny font
                    } else {
                        dispAttr(attrs[frame]); // display icon and data
                        if (frame < AT_NET_INFO)
                            strcpy(attrs[frame].lastval, attrs[frame].value);
                    }
                    // Refresh display
                    if (paint) {
                        ssd1306_display();
                    }
                }

#undef dispAttr
            }
            dodelay(1000);
        }

        trick = 10 * 60 * 1.5; // 1.5 MINUTES
        paint = true;

        while (--trick) {

            if (paint) {
                paint = false;
                ssd1306_setTextSize(3);
                ssd1306_clearDisplay();
                ssd1306_drawRect(0, 0, 128, 32, WHITE);
            }
            time(&time_now);
            loctm = localtime(&time_now);
            char timeStr[32];
            sprintf(timeStr, "%s", asctime(loctm));
            strncpy(attrs[AT_CLOCK_INFO].value, timeStr + 11, 8);
            attrs[AT_CLOCK_INFO].value[8] = 0;
            if (loctm->tm_sec % 2)
                attrs[AT_CLOCK_INFO].value[2] = 32;

            drawTime(&attrs[AT_CLOCK_INFO], &dt);
            ssd1306_display();
            dodelay(100);

            // prime for repainting
            for (int initr = 0; initr < 5; initr++)
                if (0 == attrs[initr].lastval[0])
                    continue;
                else
                    attrs[initr].lastval[0] = 0;
        }
    }

    return 0;
}
