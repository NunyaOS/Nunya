/*
Copyright (C) 2016 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef SYS_CLOCK_H
#define SYS_CLOCK_H

#include "kerneltypes.h"
#include "sys_clock_struct.h"

/**
 * @brief Reads clock info
 * @details Reads clock info into a user allocated clock_t struct.
 *
 * @param clock The returned clock info.
 * @return 0
 */
static inline int32_t read_clock(clock_t *clock) {
    return syscall(SYSCALL_clock_read, (uint32_t)clock, 0, 0, 0, 0);
}

/**
 * @brief Sleeps.
 * @details Sleeps for the number of half milliseconds specified in half_millis. It is a known bug that the clock only waits in half milliseconds intervals. [NUN-39]
 *
 * @param half_millis Units of half milliseconds.
 * @return 0
 */
static inline int32_t sleep(uint32_t half_millis) {
    return syscall(SYSCALL_sleep, half_millis, 0, 0, 0, 0);
}

#endif

