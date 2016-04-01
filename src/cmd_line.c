/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "cmd_line.h"
#include "console.h"
#include "string.h"
#include "testing.h"
#include "fs_terminal_commands.h"
#include "window.h"
#include "graphics.h"

#include "iso.h"            // iso
#include "memorylayout.h"   // PROCESS_ENTRY_POINT
#include "process.h"        // current, pagetable_getmap
#include "kernelcore.h"     // halt

#define KEYBOARD_BUFFER_SIZE 256

void cmd_line_init() {
    set_cur_path(ROOT);
}

void cmd_line_show_prompt() {
    console_set_fgcolor(255, 100, 100);
    console_printf("%s > ", get_cur_path());
    console_set_fgcolor(255, 255, 255);
}

void cmd_line_attempt(const char *line) {
    char line_copy[KEYBOARD_BUFFER_SIZE];
    strcpy(line_copy, line);
    strip_backspace(line_copy); // Get rid of backspace characters

    char *first_word = strtok(line_copy, " ");
    const char *the_rest_mixed_case = line_copy + (strlen(first_word) + 1);
    char the_rest[strlen(the_rest_mixed_case) + 1];
    to_upper(the_rest, the_rest_mixed_case);

    //Ugly case statement alternative
    if (strcmp("echo", first_word) == 0) {
        cmd_line_echo(the_rest);
    } else if (strcmp("test", first_word) == 0) {
        run_all_tests();
    } else if (strcmp("cd", first_word) == 0) {
        cmd_line_cd(the_rest);
    } else if (strcmp("pwd", first_word) == 0) {
        cmd_line_pwd();
    } else if (strcmp("ls", first_word) == 0) {
        cmd_line_ls(the_rest);
    } else if (strcmp("cat", first_word) == 0) {
        cmd_line_cat(the_rest);
    } else if (strcmp("runproc", first_word) == 0) {
        cmd_line_run_userproc();
    } else if (strcmp("window_test", first_word) == 0) {
        console_printf("\f");
        window_hierarchy_test();
    }
    /*else if () {
     *...
     *}
     */
    else {
        console_printf("%s: command not found\n", first_word);
    }
    memset(line_copy, '\0', KEYBOARD_BUFFER_SIZE);
    return;
}

void cmd_line_run_userproc() {
    // Load process data
    struct iso_dir *root_dir = iso_dopen("/", 3);
    struct iso_file *proc_file = iso_fopen("/USERPROC.", root_dir->ata_unit);
    uint8_t *process_data = kmalloc(proc_file->data_length);
    int num_read = iso_fread(process_data, proc_file->data_length, 1, proc_file);

    // Create a new process(page, page)
    struct process *new_proc = process_create(PAGE_SIZE, PAGE_SIZE);

    // Load the code into the proper page
    uint32_t real_addr;
    if (!pagetable_getmap(new_proc->pagetable, PROCESS_ENTRY_POINT, &real_addr)) {
        console_printf("Unable to get physical address of 0x80000000\n");
        halt();
    }
    // Copy data
    memcpy((void *)real_addr, (void *)process_data, proc_file->data_length);
    kfree(process_data);

    // Push the new process onto the ready list
    __process_set_initial_process_ready(new_proc);

    // Exit the current process, enter user mode
    console_printf("Enter user mode\n");
    process_exit(0);
}
