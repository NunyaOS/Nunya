/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "kerneltypes.h"
#include "permissions_capabilities.h"
#include "syscall_handler_permissions.h"

int32_t sys_capability_create() {
    return create_permissions_capability();
}

void sys_capability_delete(uint32_t identifier) {
    if (capability_owned_by_process(identifier, current)) {
        delete_permissions_capability(identifier);
    }
}

