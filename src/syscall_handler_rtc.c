/*
Copyright (C) 2016 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "kerneltypes.h"
#include "rtc.h"
#include "syscall_handler_rtc.h"

int32_t sys_rtc_read(struct rtc_time *t) {
   rtc_read(t);
   return 0;
}

