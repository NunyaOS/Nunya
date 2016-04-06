/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

// This file contains all structs that are needed on both the kernel level and the user level
// Define eligible structs here instead of importing a .h file into syscall.h

#ifndef GLOBAL_STRUCTS_H
#define GLOBAL_STRUCTS_H

#include "list.h"
#include "kerneltypes.h"

struct process_permissions {
    struct list *directories;
    int max_number_of_pages;
    // todo: window stuff
};

struct process {
    struct list_node node;
    int state;
    int exitcode;
    struct pagetable *pagetable;
    char *kstack;
    char *kstack_top;
    char *stack_ptr;
    uint32_t entry;
    int number_of_pages_using;
    struct process_permissions *permissions;
};

#endif