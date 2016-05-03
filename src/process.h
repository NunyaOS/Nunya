/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef PROCESS_H
#define PROCESS_H

#include "kerneltypes.h"
#include "list.h"
#include "pagetable.h"
#include "x86.h"
#include "memory.h"
#include "sys_fs_structs.h"

#define PROCESS_STATE_CRADLE  0
#define PROCESS_STATE_READY   1
#define PROCESS_STATE_RUNNING 2
#define PROCESS_STATE_BLOCKED 3
#define PROCESS_STATE_GRAVE   4

struct process_permissions {
    // Memory permissions
    int max_number_of_pages;

    // Window permissions, to override parents'
    int max_width;
    int max_height;
    int offset_x;
    int offset_y;
    struct list fs_allowances;
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
    struct fd fd_table[PROCESS_MAX_OPEN_FILES];
    struct process *parent;
    int number_of_pages_using;
    struct process_permissions *permissions;
    struct process_files *files;
    struct list fs_allowances_list;
    struct window *window;
    uint32_t pid;
};

void process_init();

struct process *process_create(unsigned code_size, unsigned stack_size);
void process_yield();
void process_preempt();
void process_exit(int code);
void process_dump(struct process *p);
void process_cleanup(struct process *p);

void process_wait(struct list *q);
void process_wakeup(struct list *q);
void process_wakeup_all(struct list *q);

void add_process_to_ready_queue(struct process *p);

extern struct process *current;

#endif
