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

#define SYSCALL_open     601
#define SYSCALL_close    602
#define SYSCALL_read     603
#define SYSCALL_write    604
#define SYSCALL_add_fs_allow 605
#define SYSCALL_remove_fs_allow 606

uint32_t syscall(uint32_t n, uint32_t a, uint32_t b, uint32_t c, uint32_t d,
                 uint32_t e);

static inline int32_t exit(uint32_t status) {
    return syscall(SYSCALL_exit, status, 0, 0, 0, 0);
}

static inline int32_t testcall(int x) {
    return syscall(SYSCALL_testcall, x, 0, 0, 0, 0);
}

static inline int32_t yield() {
    return syscall(SYSCALL_yield, 0, 0, 0, 0, 0);
}

static inline int32_t open(const char *path, const char *mode) {
    return syscall(SYSCALL_open, (uint32_t)path, (uint32_t)mode, 0, 0, 0);
}

static inline int32_t close(uint32_t fd) {
    return syscall(SYSCALL_close, fd, 0, 0, 0, 0);
}

static inline int32_t read(char *dest, uint32_t bytes, uint32_t fd) {
    return syscall(SYSCALL_read, (uint32_t)dest, bytes, fd, 0, 0);
}

static inline int32_t write(char *src, uint32_t bytes, uint32_t fd) {
    return syscall(SYSCALL_write, (uint32_t)src, bytes, fd, 0, 0);
}

static inline int32_t add_fs_allow(const char *path, bool do_allow_below) {
    return syscall(SYSCALL_add_fs_allow, (uint32_t)path, do_allow_below, 0, 0, 0);
}

static inline int32_t remove_fs_allow(const char *path) {
    return syscall(SYSCALL_remove_fs_allow, (uint32_t)path, 0, 0, 0, 0);
}
#endif
