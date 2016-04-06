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

/**
 * @brief   The main syscall function. All syscalls go through this.
 * @details Use this function to implement the calling of the other syscalls
 *          declared in syscall.h. Do not call it when the dedicated function
 *          call for a specific syscall is available.
 *
 * @param   n The number of the syscall, declared in syscall.h
 *
 * @param   a The first of 5 optional parameters. Pass 0 if not needed.
 *
 * @param   b The second of 5 optional parameters. Pass 0 if not needed.
 *
 * @param   c The third of 5 optional parameters. Pass 0 if not needed.
 *
 * @param   d The fourth of 5 optional parameters. Pass 0 if not needed.
 *
 * @param   e The fifth of 5 optional parameters. Pass 0 if not needed.
 *
 * @return  The code indicating success or failure of the syscall.
 */
uint32_t syscall(uint32_t n, uint32_t a, uint32_t b, uint32_t c, uint32_t d,
                 uint32_t e);


// TEMPORARY, FOR DEBUGGING
static inline int32_t debug_print(uint32_t input) {
    return syscall(SYSCALL_debug_print, input, 0, 0, 0, 0);
}


// PROCESS LIFECYCLE ----------------------------------------

/**
 * @brief   Exits the current process.
 * @details Kills the process by halting its executing, removing it from
 *          queues, and reclaiming assets. This should also kill any
            children of the process.
 *
 * @param   status The exit status of the program.
 *
 * @return  The code indicating success or failure of the syscall.
 */
static inline int32_t exit(uint32_t status) {
    return syscall(SYSCALL_exit, status, 0, 0, 0, 0);
}

/**
 * @brief   Yields the current process to others in the queue.
 * @details Removes and then replaces the process at the end of
 *          ready queue, allowing other processes to run. The process
 *          will resume execution when it reaches the front of the queue.
 *
 * @return  The code indicating success or failure of the syscall.
 */
static inline int32_t yield() {
    return syscall(SYSCALL_yield, 0, 0, 0, 0, 0);
}

/**
 * @brief   Creates and begins execution of a new process.
 * @details Loads a new process into memory as a child of the current process,
 *          and then begins execution of that process. The current process
 *          yields and is placed at the end of the ready queue.
 *
 * @param   process_path The filesystem path of the process, which should be
 *          a .nun executable.
 *
 * @param   child_permissions The set of permissions to be passed to the child,
 *          including memory limits, filesystem access, and window management.
 *          These permissions cannot exceed that of the parent and count toward
 *          the parent's allocation. They also cannot be null.
 *
 * @return  The code indicating success or failure of the syscall.
 */
static inline int32_t run(char *process_path, struct process_permissions *child_permissions) {
    return syscall(SYSCALL_run, (uint32_t)process_path, (uint32_t)child_permissions, 0, 0, 0);
}

// END PROCESS LIFECYCLE ----------------------------------------

#endif
