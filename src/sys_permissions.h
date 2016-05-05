/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef SYS_PERMISSIONS_H
#define SYS_PERMISSIONS_H

#include "kerneltypes.h"

/**
 * @brief   Creates a new permissions capability for a process to use.
 * @details Allocates an identifier for a process permissions capability,
 *          to be adjusted as needed before passed as a parameter to the
 *          run() syscall. The identifier will need to be freed by the process
 *          when no longer needed so as to stop counting toward the process'
 *          memory allocation.
 *
 * @return  The identifier of the newly created process capability.
 */
static inline int32_t permissions_capability_create() {
    return syscall(SYSCALL_capability_create, 0, 0, 0, 0, 0);
}

/**
 * @brief   Deletes the capability indicated by the identifier
 * @details Checks whether the current process owns the capability
 *          associated with the identifier, and if so, deletes it.
 *          This returns the memory associated with the capability to
 *          the process' memory allowance.
 *
 * @param   identifier The identifier of the capability to delete.
 */
static inline void permissions_capability_delete(uint32_t identifier) {
    syscall(SYSCALL_capability_delete, identifier, 0, 0, 0, 0);
}

/**
 * @brief Adds to list of fs allowances in permission capability
 * @details Ensures the path is only duplicating an existing allowance at a
 * lower location in the file tree, then creates a new node in the list of 
 * allowances based on arguments
 *
 * @param identifier The identifier of the permissions capability to be edited
 * @param path The path of the allowance being added
 * @param do_allow_below Whether or not to allow access to the subtree with path as root
 * @return 1 on success, 0 on failure
 */
static inline int32_t permissions_capability_add_allowance(uint32_t identifier, const char *path, bool do_allow_below) {
    return syscall(SYSCALL_capability_fs_add_allowance, identifier, (uint32_t)path, (uint32_t)do_allow_below, 0, 0);
}

/**
 * @brief Removes from the list of fs allowances in permission capability
 * @details Searches list of allowances for the path and removes it if found
 *
 * @param identifier The identifier of the permissions capability to be edited
 * @param path The path of the allowance being removed
 * @return 1 on success, 0 on failure or allowance not found in list
 */
static inline int32_t permissions_capability_remove_allowance(uint32_t identifier, const char *path) {
    return syscall(SYSCALL_capability_fs_remove_allowance, identifier, (uint32_t)path, 0, 0, 0);
}

#endif
