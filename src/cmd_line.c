/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/


#include "console.h"
#include "string.h"
#include "testing.h"
#include "iso.h"

#define KEYBOARD_BUFFER_SIZE 256

char cur_path[256];

void move_up_directory();

void cmd_line_init() {
    strcpy(cur_path, "/");
}

void move_up_directory() {
    if (strcmp(cur_path, "/") == 0) {
        console_printf("cd: root has no parent directory\n");
        return;
    }
    int i;
    // Move up one directory by finding the last occurrence of '/'
    for (i = strlen(cur_path); i >= 0; i--) {
        if (i == 0) {
            strcpy(cur_path, "/");
            return;
        }
        else if (cur_path[i] == '/') {
            cur_path[i] = '\0';
            return;
        }
    }
}

void cmd_line_cd(const char *arg_line) {
    char line_copy[strlen(arg_line) + 1];
    strcpy(line_copy, arg_line);
    char *first_word = strtok(line_copy, " ");

    // If you give no arguments to cd, you are sent to root
    if (strlen(first_word) == 0) {
        strcpy(cur_path, "/");
        return;
    }

    struct iso_dir *dir;
    char dir_arg[256];
    if (strcmp(first_word, ".") == 0) {
        // Do nothing, just going to self directory
        return;
    } else if (strcmp(first_word, "..") == 0) {     // Move up a directory
        move_up_directory();
        return;
    } else if (first_word[0] == '/') {   // Argument is an absolute path
        strcpy(dir_arg, first_word);
    } else {    // Argument is a relative path
        if (first_word[0] == '.' && first_word[1] == '/') {
            first_word = first_word + 2;    // Forget about the ./ and move it along
        }
        strcpy(dir_arg, cur_path);
        if (strcmp(cur_path, "/") != 0) {
            memcpy(dir_arg + strlen(cur_path), "/", 1);
            memcpy(dir_arg + strlen(cur_path) + 1, first_word, strlen(first_word) + 1);
        } else {
            memcpy(dir_arg + strlen(cur_path), first_word, strlen(first_word) + 1);
        }
    }

    dir = iso_dopen(dir_arg, 3);
    if (!dir) {
        console_printf("cd: %s is not a directory\n", first_word);
        return;
    }
    iso_dclose(dir);
    strcpy(cur_path, dir_arg);
}

// Currently does not use arg_line at all
void cmd_line_ls(const char *arg_line) {
    struct iso_dir *dir = iso_dopen(cur_path, 3);
    struct directory_record *dr = iso_dread(dir);
    while (dr) {
        if (is_dir(dr->file_flags[0])) {
            console_set_fgcolor(100,100,255);
        } else {
            console_set_fgcolor(0,255,0);
        }
        console_printf("%s\n", dr->file_identifier);
        dr = iso_dread(dir);
    }
    console_set_fgcolor(255,255,255);
    iso_dclose(dir);
}

void cmd_line_pwd() {
    console_printf("%s\n", cur_path);
}

//Privates
void cmd_line_echo(const char *arg_line) {
    console_printf("%s\n", arg_line);
    return;
}

void cmd_line_cat(const char *arg_line) {
    char arg_line_copy[256];
    strcpy(arg_line_copy, arg_line);
    char *fname = strtok(arg_line_copy, " ");
    char c;
    char fpath[256];
    while (fname) {
        strcpy(fpath, cur_path);
        if (strcmp(cur_path, "/") != 0) {
            memcpy(fpath + strlen(cur_path), "/", 1);
            memcpy(fpath + strlen(cur_path) + 1, fname, strlen(fname) + 1);
        } else {
            memcpy(fpath + strlen(cur_path), fname, strlen(fname) + 1);
        }
        struct iso_file *file = iso_fopen(fpath, 3);
        while (!file->at_EOF) {
            iso_fread(&c, 1, 1, file);
            console_putchar(c);
        }
        fname = strtok(0, " ");
    }
}

//Exposed
void cmd_line_attempt(const char * line) {
    char line_copy[KEYBOARD_BUFFER_SIZE];
    strcpy(line_copy, line);
    char *first_word = strtok(line_copy, " ");
    const char *the_rest = line + (strlen(first_word) + 1);

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
    }
    /*else if () {
     *...
     *}
     */
    else {
        console_printf("%s: command not found\n", first_word);
    }
    return;
}
