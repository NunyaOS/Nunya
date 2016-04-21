/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef SYS_PROCESS_H
#define SYS_PROCESS_H

#include "kerneltypes.h"

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
static inline int32_t exit(int32_t status) {
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
 * @param   process_path The filesystem path of the process executable, which should be
 *          a .nun executable.
 *
 * @param   permissions_identifier The identifier of the permissions capability containing
 *          the desired permissions for the process. Create a capability using the permissions
 *          syscalls and pass its identifier here.
 *
 * @return  The code indicating success or failure of the syscall.
 */
static inline int32_t run(char *process_path, uint32_t permissions_identifier) {
    return syscall(SYSCALL_run, (uint32_t)process_path, permissions_identifier, 0, 0, 0);
}

#endif
