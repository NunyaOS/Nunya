/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "console.h"
#include "process.h"
#include "iso.h"
#include "memorylayout.h" // PROCESS_ENTRY_POINT
#include "sys_process_lifecycle.h"

uint32_t sys_exit(uint32_t code) {
    process_exit(code);
    return 0;
}

uint32_t sys_yield() {
    process_yield();
    return 0;
}

uint32_t sys_run(char *process_path, struct process_permissions *child_permissions) {
    if (child_permissions == 0) {
        return -1;
    }

    // Load process data
    struct iso_dir *root_dir = iso_dopen("/", 3);
    if (root_dir == 0) {
        return -1;
    }

    struct iso_file *proc_file = iso_fopen(process_path, root_dir->ata_unit);
    if (proc_file == 0) {
        return -1;
    }

    uint8_t *process_data = kmalloc(proc_file->data_length);
    if (process_data == 0) {
        return -1;
    }

    int num_read = iso_fread(process_data, proc_file->data_length, 1, proc_file);
    if (num_read == 0) {
        return -1;
    }

    // store the current number of pages used, so we can see how many the child uses
    int page_count_before_child = current->number_of_pages_using;

    // Create a new process(page, page)
    struct process *child_proc = process_create(PAGE_SIZE, PAGE_SIZE);
    child_proc->permissions = *child_permissions; // store as value, but we needed to pass as reference because syscall
    child_proc->parent = current; // store the child's parent
    list_push_tail(&current->children, &child_proc->node); // add child to parent's list

    // Load the code into the proper page
    uint32_t real_addr;
    if (!pagetable_getmap(child_proc->pagetable, PROCESS_ENTRY_POINT, &real_addr)) {
        console_printf("Unable to get physical address of 0x80000000\n");
        sys_exit(-1); // todo: end more cleanly
    }

    // Copy data
    memcpy((void *)real_addr, (void *)process_data, proc_file->data_length);

    // transfer pages used count to child
    int child_pages_used = current->number_of_pages_using - page_count_before_child;
    child_proc->number_of_pages_using += child_pages_used;
    current->number_of_pages_using = page_count_before_child;

    // add the max number of pages child could use to parent's usage
    current->number_of_pages_using += child_proc->permissions.max_number_of_pages;

    // check if we've exceeded the parent's allocation
    if (current->number_of_pages_using > current->permissions.max_number_of_pages) {
        console_printf("current process exceeded limit: %d > %d\n", current->number_of_pages_using, current->permissions.max_number_of_pages);
        sys_exit(-1);
    }

    // check if we've exceeded the child's allocation
    if (child_proc->number_of_pages_using > child_proc->permissions.max_number_of_pages) {
        console_printf("child process exceeded limit\n");
        sys_exit(-1);
    }

    // free the intermediary memory we used
    kfree(process_data);

    // Push the new process onto the ready list
    add_process_to_ready_queue(child_proc);

    // yield the current process to the new one
    process_yield();
    return 0;
}