/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/


#include "kerneltypes.h"
#include "kmalloc.h"
#include "permissions_capabilities.h"
#include "console.h"
#include "fs.h"

#define MAX_NUMBER_OF_PERMISSIONS 169 // temporary, for now they'll be statically allocated
#define DEFAULT_FRACTION_OF_MEMORY 3

static struct permissions_capability* permissions_table[MAX_NUMBER_OF_PERMISSIONS]; // statically allocated array of capabilities

uint32_t generate_new_identifier() {
    int i;
    for (i = 1; i < MAX_NUMBER_OF_PERMISSIONS; i++) {
        if (permissions_table[i] == 0) {
            return i;
        }
    }
    console_printf("Error: no free identifier\n");
    return 0;
}

struct process_permissions *permissions_from_capability(struct permissions_capability *capability) {

    if (capability == 0) {
        console_printf("Error: invalid capability\n");
        return 0; // error case, incomplete permissions. maybe use default permissions?
    }
    struct process_permissions *new_permissions = kmalloc(sizeof(struct process_permissions));
    if (new_permissions == 0) {
        console_printf("Error: could not allocate for permissions\n");
        return new_permissions;
    }
    // apply information
    new_permissions->max_number_of_pages = capability->max_number_of_pages;
    new_permissions->max_width = capability->max_width;
    new_permissions->max_height = capability->max_height;
    new_permissions->offset_x = capability->offset_x;
    new_permissions->offset_y = capability->offset_y;

    struct list l = LIST_INIT;
    new_permissions->fs_allowances = l;
    fs_copy_allowances_list(&(new_permissions->fs_allowances), &(capability->fs_allowances));

    return new_permissions;
}

struct permissions_capability *capability_for_identifier(uint32_t identifier) {
    if (identifier >= MAX_NUMBER_OF_PERMISSIONS) {
        console_printf("Error: invalid identifier\n");
        return 0; // error case, todo
    }
    return permissions_table[identifier];
}

struct process_permissions *permissions_from_identifier(uint32_t identifier) {
    struct permissions_capability *capability = capability_for_identifier(identifier);
    struct process_permissions *permissions = permissions_from_capability(capability);
    return permissions;
}

int capability_owned_by_process(uint32_t identifier, struct process *p) {
    struct permissions_capability *requested = capability_for_identifier(identifier);
    if (requested <= 0) {
        console_printf("Error: invalid capability identifier\n");
        return 0; // error
    }
    if (requested->owner == p) {
        return 1; // true
    } else {
        return 0; // false
    }

}

uint32_t create_permissions_capability() {
    // create the capability
    // counts toward current process' memory allocation
    struct permissions_capability *new_capability = kmalloc(sizeof(struct permissions_capability));
    if (new_capability <= 0) {
        console_printf("Error: could not create capability\n");
        return 0; //error
    }
    // set its owner
    new_capability->owner = current;
    // log it in the table
    uint32_t identifier = generate_new_identifier();
    permissions_table[identifier] = new_capability;

    // set default values to be related to parents
    new_capability->max_number_of_pages = current->permissions->max_number_of_pages / DEFAULT_FRACTION_OF_MEMORY;
    new_capability->max_width = current->permissions->max_width;
    new_capability->max_height = current->permissions->max_height;
    new_capability->offset_x = 0;
    new_capability->offset_y = 0;

    struct list l = LIST_INIT;
    new_capability->fs_allowances = l;
    fs_copy_allowances_list(&(new_capability->fs_allowances), &(current->fs_allowances_list));

    return identifier;
}

void delete_capabilities_owned_by_process(struct process *p) {
    int i;
    for (i = 1; i < MAX_NUMBER_OF_PERMISSIONS; i++) {
        struct permissions_capability *t = capability_for_identifier(i);
        if (t != 0 && t->owner == p) {
            delete_permissions_capability(i);
        }
    }
}

void delete_permissions_capability(uint32_t identifier) {
    struct permissions_capability *capability = capability_for_identifier(identifier);
    permissions_table[identifier] = 0; // clear out of the table

    // don't leak here with fs_allowances
    fs_free_allowances_list(&(capability->fs_allowances));

    kfree(capability);
}
