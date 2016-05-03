/*
Copyright (C) 2016 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef SYSCALL_HANDLER_CLOCK_H
#define SYSCALL_HANDLER_CLOCK_H

#include "kerneltypes.h"
#include "clock.h"

int32_t sys_clock_read(clock_t *clock);

int32_t sys_sleep(uint32_t millis);

#endif

