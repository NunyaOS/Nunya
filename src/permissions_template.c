/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/


#include "kerneltypes.h"
#include "kmalloc.h"
#include "permissions_template.h"
#include "console.h"

#define MAX_NUMBER_OF_PERMISSIONS 169 // temporary, for now they'll be statically allocated
#define DEFAULT_FRACTION_OF_MEMORY 3

static struct permissions_template* permissions_table[MAX_NUMBER_OF_PERMISSIONS]; // statically allocated array of templates

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

struct process_permissions *permissions_from_template(struct permissions_template *template) {

    if (template == 0) {
        console_printf("Error: invalid template\n");
        return 0; // error case, incomplete permissions. maybe use default permissions?
    }
    struct process_permissions *new_permissions = kmalloc(sizeof(struct process_permissions));
    if (new_permissions == 0) {
        console_printf("Error: could not allocate for permissions\n");
        return new_permissions;
    }
    // apply information
    new_permissions->max_number_of_pages = template->max_number_of_pages;
    new_permissions->max_width = template->max_width;
    new_permissions->max_height = template->max_height;
    new_permissions->offset_x = template->offset_x;
    new_permissions->offset_y = template->offset_y;

    // todo: the rest of the permissions

    return new_permissions;
}

struct permissions_template *template_for_identifier(uint32_t identifier) {
    if (identifier >= MAX_NUMBER_OF_PERMISSIONS) {
        console_printf("Error: invalid identifier\n");
        return 0; // error case, todo
    }
    return permissions_table[identifier];
}

struct process_permissions *permissions_from_identifier(uint32_t identifier) {
    struct permissions_template *template = template_for_identifier(identifier);
    struct process_permissions *permissions = permissions_from_template(template);
    return permissions;
}

int template_owned_by_process(uint32_t identifier, struct process *p) {
    struct permissions_template *requested = template_for_identifier(identifier);
    if (requested <= 0) {
        console_printf("Error: invalid template identifier\n");
        return 0; // error
    }
    if (requested->owner == p) {
        return 1; // true
    } else {
        return 0; // false
    }

}

uint32_t create_permissions_template() {
    // create the template
    // counts toward current process' memory allocation
    struct permissions_template *new_template = kmalloc(sizeof(struct permissions_template));
    if (new_template <= 0) {
        console_printf("Error: could not create template\n");
        return 0; //error
    }
    // set its owner
    new_template->owner = current;
    // log it in the table
    uint32_t identifier = generate_new_identifier();
    permissions_table[identifier] = new_template;

    // set default values to be related to parents
    new_template->directories = current->permissions->directories;
    new_template->max_number_of_pages = current->permissions->max_number_of_pages / DEFAULT_FRACTION_OF_MEMORY;
    new_template->max_width = current->permissions->max_width;
    new_template->max_height = current->permissions->max_height;
    new_template->offset_x = 0;
    new_template->offset_y = 0;

    // todo: the rest of the permissions

    return identifier;
}

void delete_templates_owned_by_process(struct process *p) {
    int i;
    for (i = 1; i < MAX_NUMBER_OF_PERMISSIONS; i++) {
        struct permissions_template *t = template_for_identifier(i);
        if (t != 0 && t->owner == p) {
            delete_permissions_template(i);
        }
    }
}

void delete_permissions_template(uint32_t identifier) {
    struct permissions_template *template = template_for_identifier(identifier);
    permissions_table[identifier] = 0; // clear out of the table
    kfree(template);
}