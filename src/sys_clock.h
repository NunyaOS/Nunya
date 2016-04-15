/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef SYS_CLOCK_H
#define SYS_CLOCK_H

#include "kerneltypes.h"
#include "clock.h"

static inline int32_t read_clock(clock_t *clock) {
    return syscall(SYSCALL_clock_read, (uint32_t)clock, 0, 0, 0, 0);
}

static inline int32_t sleep(uint32_t millis) {
    return syscall(SYSCALL_sleep, millis, 0, 0, 0, 0);
}

#endif

