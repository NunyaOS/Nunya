/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef SYSCALL_H
#define SYSCALL_H

#include "kerneltypes.h"
#include "global_structs.h"

#include "sys_process_lifecycle.h"

#define SYSCALL_exit     1
#define SYSCALL_yield    2
#define SYSCALL_run      3
#define SYSCALL_debug_print 9000 // for debugging

// The main syscall function. All syscalls go through this
uint32_t syscall(uint32_t n, uint32_t a, uint32_t b, uint32_t c, uint32_t d,
                 uint32_t e);


// TEMPORARY, FOR DEBUGGING
static inline int32_t debug_print(uint32_t input) {
    return syscall(SYSCALL_debug_print, input, 0, 0, 0, 0);
}


// PROCESS LIFECYCLE

static inline int32_t exit(uint32_t status) {
    return syscall(SYSCALL_exit, status, 0, 0, 0, 0);
}

static inline int32_t yield() {
    return syscall(SYSCALL_yield, 0, 0, 0, 0, 0);
}

static inline int32_t run(char *process_path, struct process_permissions *child_permissions) {
    return syscall(SYSCALL_run, (uint32_t)process_path, (uint32_t)child_permissions, 0, 0, 0);
}

// END PROCESS LIFECYCLE

#endif
