/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "syscall.h"
#include "console.h"
#include "process.h"
#include "sys_fs.h"

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

uint32_t sys_open(uint32_t path_intptr, uint32_t mode_intptr) {
    return sys_fs_open((const char *)path_intptr, (const char *)mode_intptr);
}

uint32_t sys_close(uint32_t fd) {
    sys_fs_close(fd);
    return 0;
}

uint32_t sys_read(uint32_t dest_intptr, uint32_t bytes, uint32_t fd) {
    return sys_fs_read((char *)dest_intptr, bytes, fd);
}

uint32_t sys_write(uint32_t src_intptr, uint32_t bytes, uint32_t fd) {
    return sys_fs_write((const char *)src_intptr, bytes, fd);
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
        case SYSCALL_open:
            return sys_open(a, b);
        case SYSCALL_close:
            return sys_close(a);
        case SYSCALL_read:
            return sys_read(a, b, c);
        case SYSCALL_write:
            return sys_write(a, b, c);
        default:
            return -1;
    }
}
