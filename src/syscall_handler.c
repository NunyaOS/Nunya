/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "interrupt.h"
#include "syscall.h"
#include "console.h"
#include "process.h"
#include "fs_sys.h"

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
    return fs_sys_open((const char *)path_intptr, (const char *)mode_intptr);
}

uint32_t sys_close(uint32_t fd) {
    fs_sys_close(fd);
    return 0;
}

uint32_t sys_read(uint32_t dest_intptr, uint32_t bytes, uint32_t fd) {
    return fs_sys_read((char *)dest_intptr, bytes, fd);
}

uint32_t sys_write(uint32_t src_intptr, uint32_t bytes, uint32_t fd) {
    return fs_sys_write((const char *)src_intptr, bytes, fd);
}

uint32_t sys_add_fs_allow(uint32_t path_intptr, uint32_t do_allow_below) {
    return fs_sys_add_allowance((const char *)path_intptr,  (bool)do_allow_below);
}

uint32_t sys_remove_fs_allow(uint32_t path_intptr) {
    return fs_sys_remove_allowance((const char *)path_intptr);
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
        case SYSCALL_add_fs_allow:
            return sys_add_fs_allow(a, b);
        case SYSCALL_remove_fs_allow:
            return sys_remove_fs_allow(a);
        default:
            return -1;
    }
}
