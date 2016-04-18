/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef SYSCALL_H
#define SYSCALL_H

#include "kerneltypes.h"

#define SYSCALL_exit     1
#define SYSCALL_testcall 2
#define SYSCALL_yield    3
// TODO: change these syscall numbers
#define SYSCALL_clock_read 4
#define SYSCALL_sleep 5
#define SYSCALL_rtc_read 6

#define SYSCALL_open     601
#define SYSCALL_close    602
#define SYSCALL_read     603
#define SYSCALL_write    604

#define SYSCALL_window_create 200
#define SYSCALL_window_set_border_color 201
#define SYSCALL_window_draw_line 202
#define SYSCALL_window_draw_arc 203
#define SYSCALL_window_draw_circle 204
#define SYSCALL_window_draw_char 205
#define SYSCALL_window_draw_string 206

#define SYSCALL_open     601
#define SYSCALL_close    602
#define SYSCALL_read     603
#define SYSCALL_write    604

uint32_t syscall(uint32_t n, uint32_t a, uint32_t b, uint32_t c, uint32_t d,
                 uint32_t e);

// MARK Module System call includes

#include "sys_fs.h"
#include "sys_window.h"

#endif
