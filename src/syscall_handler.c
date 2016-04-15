/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "syscall.h"
#include "console.h"
#include "process.h"
#include "syscall_handler_window.h"


uint32_t sys_exit(uint32_t code) {
    process_exit(code);
    return 0;
}

uint32_t sys_yield() {
    process_yield();
    return 0;
}

uint32_t sys_testcall(uint32_t code) {
    console_printf("testing: %d\n", code);
    return 0;
}

int32_t syscall_handler(uint32_t n, uint32_t a, uint32_t b, uint32_t c,
                        uint32_t d, uint32_t e) {
    switch (n) {
        case SYSCALL_exit:
            return sys_exit(a);
        case SYSCALL_testcall:
            return sys_testcall(a);
        case SYSCALL_yield:
            return sys_yield();
        case SYSCALL_window_create:
            return sys_window_create(a, b, c, d);
        case SYSCALL_window_set_border_color:
            return sys_set_border_color((const struct graphics_color *)a);
        case SYSCALL_window_draw_line:
            return sys_draw_line(a, b, c, d, (const struct graphics_color *)e);
        case SYSCALL_window_draw_arc:
            return sys_draw_arc(a, b, (const struct arc_info *)c, (const struct graphics_color *)d);
        case SYSCALL_window_draw_circle:
            return sys_draw_circle(a, b, (const double *)c, (const struct graphics_color *)d);
        case SYSCALL_window_draw_char:
            return sys_draw_char(a, b, (char)c, (const struct graphics_color *)d, (const struct graphics_color *)e);
        case SYSCALL_window_draw_string:
            return sys_draw_string(a, b, (const char *)c, (const struct graphics_color *)d, (const struct graphics_color *)e);
        default:
            return -1;
    }
}
