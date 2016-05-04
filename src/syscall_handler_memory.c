/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "syscall_handler_memory.h"
#include "process.h"
#include "permissions_capabilities.h"

int32_t sys_current_memory_usage() {
    return current->number_of_pages_using;
}

int32_t sys_max_memory() {
    return current->permissions->max_number_of_pages;
}

int32_t sys_capability_set_max_memory(uint32_t identifier, uint32_t pages) {
    // check if identifier is owned by the process
    if (!capability_owned_by_process(identifier, current)) {
        return -1;
    }

    // check if page request is valid given current usage
    int pages_left = sys_max_memory() - sys_current_memory_usage();
    if (pages > pages_left) {
        return -1;
    }

    // apply the desired change to capability
    struct permissions_capability *c = capability_for_identifier(identifier);
    if (c == 0) {
        return -1;
    }

    c->max_number_of_pages = pages;
    return 0;
}