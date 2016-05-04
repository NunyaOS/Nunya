/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "syscall.h"
#include "syscall_handler_process.h"
#include "syscall_handler_permissions.h"
#include "syscall_handler_memory.h"
#include "kerneltypes.h"

#include "console.h" // for debugging

#include "syscall_handler_fs.h"
#include "process.h"
#include "syscall_handler_window.h"
#include "syscall_handler_clock.h"
#include "syscall_handler_rtc.h"

int32_t sys_debug_print(uint32_t a) {
    console_printf("PID %d, testing: %d\n", current->pid, a);
    return 0;
}

int32_t syscall_handler(uint32_t n, uint32_t a, uint32_t b, uint32_t c,
                        uint32_t d, uint32_t e) {
    switch (n) {
        case SYSCALL_exit:
            return sys_exit(a);
        case SYSCALL_yield:
            return sys_yield();
        case SYSCALL_run:
            return sys_run((char *)a, b, current);
        case SYSCALL_clock_read:
            return sys_clock_read((clock_t *)a);
        case SYSCALL_sleep:
            return sys_sleep(a);
        case SYSCALL_rtc_read:
            return sys_rtc_read((struct rtc_time *)a);
        case SYSCALL_capability_create:
            return sys_capability_create();
        case SYSCALL_capability_delete:
            sys_capability_delete(a);
            return 0;
        case SYSCALL_capability_fs_add_allowance:
            return sys_capability_fs_add_allowance(a, (const char *)b, (bool)c);
        case SYSCALL_capability_fs_remove_allowance:
            return sys_capability_fs_remove_allowance(a, (const char *)b);
        case SYSCALL_memory_current_usage:
            return sys_current_memory_usage();
        case SYSCALL_capability_set_max_memory:
            return sys_capability_set_max_memory(a, b);
        case SYSCALL_memory_max:
            return sys_max_memory();
        case SYSCALL_open:
            return sys_fs_open((const char *)a, (const char *)b);
        case SYSCALL_close:
            return sys_fs_close(a);
        case SYSCALL_read:
            return sys_fs_read((char *)a, b, c);
        case SYSCALL_write:
            return sys_fs_write((const char *)a, b, c);
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
        case SYSCALL_window_clear:
            return sys_clear();
        case SYSCALL_window_get_event:
            return sys_get_event((struct event *)a);
        case SYSCALL_debug_print:
            return sys_debug_print(a); // for debugging
        default:
            return -1;
    }
}

