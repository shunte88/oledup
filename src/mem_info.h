/*
 *  mem_info.h
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

#ifndef DISPMEM_INFO_H
#define DISPMEM_INFO_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

typedef struct memInfo {
   unsigned long totalMem;
   unsigned long freeMem;
   unsigned long buffersMem;
   unsigned long usedMem;
   unsigned long cachedMem;
   unsigned long totalSwap;
   unsigned long usedSwap;
}memInfo;

static inline bool String_startsWith(const char* s, const char* match) {
   return strncmp(s, match, strlen(match)) == 0;
}

void scanMemoryInfo(memInfo* this);
int readableUnits(char* buffer, unsigned long value, size_t size);

#endif

