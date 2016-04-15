/*
Copyright (C) 2016 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "syscall_handler_clock.h"
#include "kerneltypes.h"
#include "clock.h"

int32_t sys_clock_read(clock_t *clock) {
    *clock = clock_read();
    return 0;
}

int32_t sys_sleep(uint32_t millis) {
    clock_wait(millis);
    return 0;
}

