/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "kerneltypes.h"
#include "permissions_capabilities.h"
#include "syscall_handler_permissions.h"
#include "string.h"
#include "console.h"
#include "fs.h"

int32_t sys_capability_create() {
    return create_permissions_capability();
}

void sys_capability_delete(uint32_t identifier) {
    if (capability_owned_by_process(identifier, current)) {
        delete_permissions_capability(identifier);
    }
}

int32_t sys_capability_fs_add_allowance(uint32_t identifier, const char *path, bool do_allow_below) {
    struct permissions_capability *pc = capability_for_identifier(identifier);
    if (!pc) {
        return 0;
    }

    //must be under current paths with is_just_dir = 0;
    if (fs_security_check(path)) {
        struct list_node *iterator;
        for (iterator = pc->fs_allowances.head; iterator != 0; iterator = iterator->next) {
            if (strcmp(((struct fs_allowance *)iterator)->path, path) == 0) {
                //duplicate entry, do not actually add.
                return 1;
            }
        }

        struct fs_allowance *new = kmalloc(sizeof(*new));
        //set other needed values
        strcpy(new->path, path);
        new->do_allow_below = do_allow_below;

        //insert at head of list
        list_push_head(&(pc->fs_allowances), (struct list_node *) new);

        return 1;
    }
    return 0;
}

int32_t sys_capability_fs_remove_allowance(uint32_t identifier, const char *path) {
    struct permissions_capability *pc = capability_for_identifier(identifier);
    if (!pc) {
        return 0;
    }

    //does not recursively remove allowances from fs
    //should recursively remove allowances from processes,
    //but we can't yet, because we don't have pointers to our children
    struct list_node *iterator;

    for(iterator = pc->fs_allowances.head; iterator != 0; iterator = iterator->next) {
        if (strcmp(((struct fs_allowance *)iterator)->path, path) == 0) {
            list_remove(iterator);
            kfree(iterator);
            return 1;
        }
    }
    return 0;
}
