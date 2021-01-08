/*
 *  mem_info.c
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
#include <string.h>
#include <fcntl.h>

#include "mem_info.h"

void scanMemoryInfo(memInfo* this) {

    unsigned long swapFree = 0;
    unsigned long shmem = 0;
    unsigned long sreclaimable = 0;

/*
const char *
getramu(void)
{
	long total, free, buffers, cached;

	return (pscanf("/proc/meminfo",
	               "MemTotal: %ld kB\n"
	               "MemFree: %ld kB\n"
	               "MemAvailable: %ld kB\nBuffers: %ld kB\n"
	               "Cached: %ld kB\n",
	               &total, &free, &buffers, &buffers, &cached) == 5) ?
	       bprintf("%f", (float)(total - free - buffers - cached) / 1024 / 1024) :
	       NULL;
}
*/

    FILE* file = fopen("/proc/meminfo", "r");
    if (file == NULL) {
        printf("Cannot open /proc/meminfo\n");
        this->totalMem = 0;
    } else {

        char buffer[128];
        while (fgets(buffer, 128, file)) {

            #define readAttr(label, metric)                          \
             if (String_startsWith(buffer, label)) {                 \
                 sscanf(buffer + strlen(label), " %ld kB", &metric); \
                 break;                                              \
             }

            switch(buffer[0]) {
                case 'M':
                    readAttr("MemTotal:", this->totalMem);
                    readAttr("MemFree:", this->freeMem);
                    break;
                case 'B':
                    readAttr("Buffers:", this->buffersMem);
                    break;
                case 'C':
                    readAttr("Cached:", this->cachedMem);
                    break;
                case 'S':
                    switch (buffer[1]) {
                        case 'w':
                            readAttr("SwapTotal:", this->totalSwap);
                            readAttr("SwapFree:", swapFree);
                            break;
                        case 'h':
                            readAttr("Shmem:", shmem);
                            break;
                        case 'R':
                            readAttr("SReclaimable:", sreclaimable);
                            break;
                    }
                break;
            }
            #undef readAttr
        }
    }

    fclose(file);

    this->usedMem = (unsigned long*)this->totalMem - (unsigned long*)this->freeMem;
    this->cachedMem = this->cachedMem + sreclaimable - shmem;
    this->usedSwap = this->totalSwap - swapFree;

}

int readableUnits(char* buffer, unsigned long value, size_t size) {

    const char* prefix = "KMGTPEZY";
    long powi = 1;
    long powj = 1, precision = 2;

    for (;;) {
        if (value / 1024 < powi)
            break;
        if (prefix[1] == '\0')
            break;
        powi *= 1024;
        ++prefix;
    }

    if (*prefix == 'K')
        precision = 0;

    for (; precision > 0; precision--) {
        powj *= 10;
        if (value / powi < powj)
            break;
    }
    return snprintf(buffer, size, "%.*f%c", precision, (double) value / powi, *prefix);

}
