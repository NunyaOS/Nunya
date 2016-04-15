/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef SYSCALL_H
#define SYSCALL_H

#include "kerneltypes.h"

#define SYSCALL_exit     1
#define SYSCALL_yield    2
#define SYSCALL_run      3

#define SYSCALL_template_create 50
#define SYSCALL_template_delete 51
// #define SYSCALL_template_lookup 52

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

#define SYSCALL_debug_print 9000 // for debugging

/**
 * @brief   The main syscall function. All syscalls go through this.
 * @details Use this function to implement the calling of the other syscalls
 *          declared in syscall.h. Do not call it when the dedicated function
 *          call for a specific syscall is available.
 *
 * @param   n The number of the syscall, declared in syscall.h
 * @param   a The first of 5 optional parameters. Pass 0 if not needed.
 * @param   b The second of 5 optional parameters. Pass 0 if not needed.
 * @param   c The third of 5 optional parameters. Pass 0 if not needed.
 * @param   d The fourth of 5 optional parameters. Pass 0 if not needed.
 * @param   e The fifth of 5 optional parameters. Pass 0 if not needed.
 *
 * @return  The code indicating success or failure of the syscall.
 */
int32_t syscall(uint32_t n, uint32_t a, uint32_t b, uint32_t c, uint32_t d,
                 uint32_t e);

// TEMPORARY FOR DEBUGGING
static inline int32_t debug_print(uint32_t input) {
     return syscall(SYSCALL_debug_print, input, 0, 0, 0, 0);
}

// ------- Include module-level headers here -------

#include "sys_process.h"
#include "sys_permissions.h"
#include "sys_fs.h"
#include "sys_window.h"

#endif
