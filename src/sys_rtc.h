/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "kerneltypes.h"
#include "rtc.h"

static inline int32_t read_rtc(struct rtc_time *t) {
    return syscall(SYSCALL_rtc_read, (uint32_t)t, 0, 0, 0, 0);
}

