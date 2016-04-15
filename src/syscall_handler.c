/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "syscall.h"
#include "syscall_handler_fs.h"
#include "process.h"

uint32_t sys_exit(uint32_t code) {
    process_exit(code);
    return 0;
}

uint32_t sys_yield() {
    process_yield();
    return 0;
}

int32_t syscall_handler(uint32_t n, uint32_t a, uint32_t b, uint32_t c,
                        uint32_t d, uint32_t e) {
    switch (n) {
        case SYSCALL_yield:
             return sys_yield();
        case SYSCALL_exit:
             return sys_exit(a);
        case SYSCALL_open:
            return sys_fs_open((const char *)a, (const char *)b);
        case SYSCALL_close:
            return sys_fs_close(a);
        case SYSCALL_read:
            return sys_fs_read((char *)a, b, c);
        case SYSCALL_write:
            return sys_fs_write((const char *)a, b, c);
        default:
            return -1;
    }
}
