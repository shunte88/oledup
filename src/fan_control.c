/*
 *  fan_control.c
 *
 *      (c) 2020-21 Stuart Hunter
 *
 *      TODO:
 *
 *      This program is free software: you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation, either version 3 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      See <http://www.gnu.org/licenses/> to get a copy of the GNU General
 *      Public License.
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

static int serial_port = 0;
uint8_t cpu_temp_conf[8]={40, 25, 50, 50, 65, 75, 75, 100};;

// initialized the serial port
bool init_serial(const char *serial_name) {

    bool ret = true;
    serial_port = open(serial_name, O_RDWR);
    if (serial_port < 0){
        printf("Cannot open /dev/ttyUSB0 serial port ErrorCode: %s\n", strerror(errno));
        printf("Please check the /boot/config.txt file and add dtoverlay=dwc2, dr_mode=host and reboot your Pi\n");
        ret = false;
    }
    if(ret) {

        struct termios tty;

        if(0!=tcgetattr(serial_port, &tty)) {
            printf("Please check serial port over OTG\n");
            ret = false;
        }

        if(ret) {
            tty.c_cflag &= ~PARENB;
            tty.c_cflag |= PARENB;
            tty.c_cflag &= ~CSTOPB; 
            tty.c_cflag |= CSTOPB;

            tty.c_cflag |= CS5;
            tty.c_cflag |= CS6;
            tty.c_cflag |= CS7;
            tty.c_cflag |= CS8;

            tty.c_cflag &= ~CRTSCTS;
            tty.c_cflag |= CRTSCTS;

            tty.c_cflag |= CREAD | CLOCAL;

            tty.c_lflag &= ~ICANON;
            tty.c_lflag &= ~ECHO;
            tty.c_lflag &= ~ECHOE;
            tty.c_lflag &= ~ECHONL;
            tty.c_lflag &= ~ISIG;

            tty.c_iflag &= ~(IXON | IXOFF | IXANY);
            tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);

            tty.c_oflag &= ~OPOST;
            tty.c_oflag &= ~ONLCR;

            tty.c_cc[VTIME] = 10;
            tty.c_cc[VMIN] = 0;

            cfsetispeed(&tty, B9600);
            cfsetospeed(&tty, B9600);

            if (tcsetattr(serial_port, TCSANOW, &tty) !=0){
                printf("---Serial port cannot be detected---\n");
                ret = false;
            }
        }
    }

    return ret;

}

// send data to serial port
int send_serial(const char* data) {
    return write(serial_port, data, sizeof data);
}

// close the serial port
int __init_serial() {
    return close(serial_port);
}

bool loadFanConfiguration(void) {

    bool ret = true;

    FILE* file = fopen("/etc/deskpi.conf", "r");
    if(file != NULL) {
        int i = 0;
        char buffer[128] = {0};
        while ((i<8)&&(fgets(buffer, sizeof(buffer), file))) {
            if('#'!=buffer[0]) {
                cpu_temp_conf[i] = atoi(buffer);
            } else {
                continue;
            }
            i++;
        }
    } else {
        ret = false;
    }
    return ret;

}

bool fanControl(double temperature, bool* goser) {

    bool ret = true;
    char data[8] = {0};

    if(!goser)goser = (bool*)init_serial("/dev/ttyUSB0");

    if(temperature < cpu_temp_conf[0]) {
        sprintf(data, "pwm_%03d", 0);
        ret = false;
    } else if(temperature >= cpu_temp_conf[0] && temperature < cpu_temp_conf[2]) {
        sprintf(data, "pwm_%03d", cpu_temp_conf[1]);
    } else if(temperature >= cpu_temp_conf[2] && temperature < cpu_temp_conf[4]) {
        sprintf(data, "pwm_%03d", cpu_temp_conf[3]);
    } else if(temperature >= cpu_temp_conf[4] && temperature < cpu_temp_conf[6]) {
        sprintf(data, "pwm_%03d", cpu_temp_conf[5]);
    } else if(temperature >= cpu_temp_conf[6]) {
        sprintf(data, "pwm_%03d", cpu_temp_conf[7]);
    }

    if(goser) ret = (0!=send_serial((const char*)&data));
    return ret; // this indicates fan on/off, mixing in send status is confusing!!!

}
