/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef SYS_MEMORY_H
#define SYS_MEMORY_H

#include "kerneltypes.h"

/**
 * @brief   Returns the number of pages the current process is using.
 * @details Use this function to query how much of the current process'
 *          memory allocation has been used. Compare to the process'
 *          maximum memory to see how much memory is left for use.
 *
 * @return  The number of pages the process is using.
 */
static inline int32_t get_current_memory_usage() {
    return syscall(SYSCALL_memory_current_usage, 0, 0, 0, 0, 0);
}

/**
 * @brief   Returns the maximum number of pages the current process can use.
 * @details Use this function to see how many pages the process is allowed to use.
 *          Compare to the process' current usage to see how many pages are left to use.
 *
 * @return  The maximum number of pages the process can use.
 */
static inline int32_t get_max_memory() {
    return syscall(SYSCALL_memory_max, 0, 0, 0, 0, 0);
}

/**
 * @brief   Sets the maximum number of pages allowed by a process capability.
 * @details Use this function to customize a process capability with a specified
 *          maximum number of pages. If the desired number of pages exceeds the current
 *          process' allocation, the function will fail. If the capability is not owned
 *          by the process, the function will fail.

 * @param   identifier The identifier of the capability to be altered.
 * @param   pages The maximum number of pages to be allowed by the capability.
 *
 * @return  0 on success, -1 on failure.
 */
static inline int32_t capability_set_max_memory(uint32_t identifier, uint32_t pages) {
    return syscall(SYSCALL_capability_set_max_memory, identifier, pages, 0, 0, 0);
}


#endif
