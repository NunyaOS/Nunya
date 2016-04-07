/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "syscall.h"
#include "sys_process_lifecycle.h"
#include "kerneltypes.h"

#include "console.h" // for debugging



uint32_t sys_debug_print(uint32_t a) {
    console_printf(" testing: %d\n", a);
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
            return sys_run((char *)a, (struct process_permissions *)b, current);
        case SYSCALL_debug_print:
            return sys_debug_print(a); // for debugging
        default:
            return -1;
    }
}
