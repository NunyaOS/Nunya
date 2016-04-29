/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef RTC_H
#define RTC_H

#include "kerneltypes.h"
#include "sys_rtc_struct.h"

void rtc_init();
void rtc_read(struct rtc_time *t);

#endif
