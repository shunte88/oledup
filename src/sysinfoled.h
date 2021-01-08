/*
 *  sysinfoled.h
 *
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

#ifndef DISPSYS_INFO_H
#define DISPSYS_INFO_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define MAX_SIZE 32
#define INIT_ATTR_VALUE "XXXXXXXX"

enum iconType {
    IT_CPU_TEMP = 0,
    IT_NET_WIFI,
    IT_NET_ETHER,
    IT_CPU_INFO,
    IT_MEM_INFO,
    IT_DISK_INFO,
    IT_NULL_ICON,
};

enum attrType {
    AT_CPU_INFO = 0,
    AT_CPU_TEMP,
    AT_MEM_INFO,
    AT_DISK_INFO,
    AT_NET_INFO,
    AT_CLOCK_INFO,
};

typedef struct point_t {
    int16_t x;
    int16_t y;
} point_t;

typedef struct DrawAttr {
    bool visible;
    point_t pos;
    int16_t w;
    int16_t h;
    enum iconType icon;
    bool forceClear;
    char value[MAX_SIZE];
    char lastval[MAX_SIZE];
} DrawAttr;

typedef struct DrawTime {
    int charWidth;
    int charHeight;
    int bufferLen;
    point_t pos;
    int font;
    bool fmt12;
} DrawTime;

#endif

