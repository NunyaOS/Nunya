/*
Copyright (C) 2016 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "kerneltypes.h"
#include "sys_rtc_struct.h"

/**
 * @brief Reads from the RTC.
 * @details Reads RTC info into a user allocated rtc_time struct
 *
 * @param t The returned RTC info.
 * @return 0
 */
static inline int32_t read_rtc(struct rtc_time *t) {
    return syscall(SYSCALL_rtc_read, (uint32_t)t, 0, 0, 0, 0);
}

