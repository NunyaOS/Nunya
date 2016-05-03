/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "syscall_handler_process.h"
#include "console.h"
#include "iso.h"
#include "memorylayout.h" // PROCESS_ENTRY_POINT
#include "permissions_capabilities.h"

int32_t sys_exit(uint32_t code) {
    process_exit((int32_t)code);
    return 0;
}

int32_t sys_yield() {
    process_yield();
    return 0;
}

int32_t sys_run(const char *process_path, const uint32_t permissions_identifier, struct process *parent) {

    // Get the capability
    if (capability_owned_by_process(permissions_identifier, current) != 1) {
        console_printf("Error: could not get permissions capability\n");
        return -1;
    }

    // Load process data
    struct iso_dir *root_dir = iso_dopen("/", 3);
    if (root_dir == 0) {
        console_printf("Error accessing binary\n");
        return -1;
    }

    struct iso_file *proc_file = iso_fopen(process_path, root_dir->ata_unit);
    if (proc_file == 0) {
        console_printf("Error accessing binary\n");
        return -1;
    }

    uint8_t *process_data = kmalloc(proc_file->data_length);
    if (process_data == 0) {
        // free the intermediary memory we used
        kfree(process_data);
        console_printf("Error accessing binary\n");
        return -1;
    }

    int num_read = iso_fread(process_data, proc_file->data_length, 1, proc_file);
    if (num_read == 0) {
        // free the intermediary memory we used
        kfree(process_data);
        console_printf("Error accessing binary\n");
        return -1;
    }

    // store the current number of pages used, so we can see how many the child uses
    int page_count_before_child = parent->number_of_pages_using;

    // Create a new process(page, page)
    struct process *child_proc = process_create(PAGE_SIZE, PAGE_SIZE);

    if (child_proc <= 0) {
        // free the intermediary memory we used
        kfree(process_data);
        process_cleanup(child_proc);
        console_printf("Error creating process\n");
        return -1;
    }

    // incorporate the permissions
    struct process_permissions *child_permissions = permissions_from_identifier(permissions_identifier);
    child_proc->permissions = child_permissions;


    child_proc->parent = parent; // store the child's parent
    list_push_tail(&parent->children, &child_proc->node); // add child to parent's list



    // Load the code into the proper page
    uint32_t real_addr;
    if (!pagetable_getmap(child_proc->pagetable, PROCESS_ENTRY_POINT, &real_addr)) {
        console_printf("Unable to get physical address of 0x80000000\n");
        // free the intermediary memory we used
        kfree(process_data);
        process_cleanup(child_proc);
        return -1;
    }

    // Copy data
    memcpy((void *)real_addr, (void *)process_data, proc_file->data_length);

    // transfer pages used count to child
    int child_pages_used = parent->number_of_pages_using - page_count_before_child;
    child_proc->number_of_pages_using += child_pages_used;
    parent->number_of_pages_using = page_count_before_child;

    // add the max number of pages child could use to parent's usage
    parent->number_of_pages_using += child_proc->permissions->max_number_of_pages;

    // free the intermediary memory we used
    kfree(process_data);

    // check if we've exceeded the parent's allocation
    if (parent->number_of_pages_using > parent->permissions->max_number_of_pages) {
        console_printf("Error: process %d attempted to create a process %d without available memory: %d > %d\n", parent->pid, child_proc->pid, parent->number_of_pages_using, parent->permissions->max_number_of_pages);
        process_cleanup(child_proc);
        // memcpy((void *)real_addr, 0, proc_file->data_length);
        return -1;
    }

    // check if we've exceeded the child's allocation
    if (child_proc->number_of_pages_using > child_proc->permissions->max_number_of_pages) {
        console_printf("Error: child process %d exceeded its limit\n", child_proc->pid);
        process_cleanup(child_proc);
        return -1;
    }

    // Push the new process onto the ready list
    add_process_to_ready_queue(child_proc);

    // yield the current process to the new one
    process_yield();
    return 0;
}