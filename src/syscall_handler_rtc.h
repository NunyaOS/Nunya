/*
Copyright (C) 2016 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef SYSCALL_HANDLER_CLOCK_H
#define SYSCALL_HANDLER_CLOCK_H

#include "kerneltypes.h"
#include "rtc.h"

int32_t sys_rtc_read(struct rtc_time *t);

#endif

