/*
 *  fan_control.h
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

#ifndef PWM_FANCTRL_H
#define PWM_FANCTRL_H

#include <stdbool.h>

#define TEMP_FILE "/sys/class/thermal/thermal_zone0/temp"

bool loadFanConfiguration(void);
bool fanControl(double temperature, bool *goser);
// "construct" serial comms
bool init_serial(const char *serial_name);
int send_serial(const char *data);
// "destroy" serial comms
int __init_serial(void);

#endif
