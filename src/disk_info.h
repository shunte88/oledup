/*
 *  disk_info.h
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

#ifndef OUDISK_INFO_H
#define OUDISK_INFO_H

#include <stdint.h>
#include <sys/time.h>
#include <time.h>

struct io_long {
    uint32_t in;
    uint32_t out;
};

struct io_total_double {
    long double in;
    long double out;
    long double total;
};

struct double_types {
    struct io_total_double bytes;
    struct io_total_double packets;
    struct io_total_double errors;
};

struct iface_speed_stats {
    struct io_long bytes;
    struct io_long packets;
    struct io_long errors;
};

typedef struct iface_speed_stats iface_speed_stats_t;

struct double_list {
    struct double_types data;
    struct double_list *next;
};

struct avg_t {
    struct double_list *first;
    struct double_list *last;
    uint16_t items;
    struct double_types item_sum;
};

struct iface_stats {
    char *if_name;
    iface_speed_stats_t data;
    struct double_types max;
    struct iface_speed_stats sum;
    struct avg_t avg;
    struct timeval time;
};

typedef struct iface_stats iface_stats_t;
typedef struct double_types double_types_t;

void getDiskInfo(char verbose);

#endif