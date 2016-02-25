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
#define ROOT "/"

char cur_path[256];

void move_up_directory();
void move_into_directory(const char *dname);
void try_cd(const char *dpath, const char *dname);
void process_cd_part(const char *dname);
void set_cur_path(const char *dpath);

/**
 * @brief Initialize the command line
 */
void cmd_line_init() {
    set_cur_path(ROOT);
}

/**
 * @brief Set the current working directory
 * @param dpath The path to which to set the current working directory
 */
void set_cur_path(const char *dpath) {
    strcpy(cur_path, dpath);
}

/**
 * @brief Moves up one directory
 * @details Changes working directory to the parent of current directory
 */
void move_up_directory() {
    if (strcmp(cur_path, "/") == 0) {
        console_printf("cd: root has no parent directory\n");
        return;
    }
    int i;
    // Move up one directory by finding the last occurrence of '/'
    for (i = strlen(cur_path); i >= 0; i--) {
        if (i == 0) {
            set_cur_path(ROOT);
            return;
        }
        else if (cur_path[i] == '/') {
            cur_path[i] = '\0';
            return;
        }
    }
}

/**
 * @brief Move into a directory below yourself
 * @details Move into the directory below yourself specified by dname
 * @param dname Name of directory into which you attempt to move
 */
void move_into_directory(const char *dname) {
    char dpath[256];
    strcpy(dpath, cur_path);
    if (strcmp(cur_path, ROOT) != 0) {   // Not at root, so we need to add a '/' to the path
        memcpy(dpath + strlen(cur_path), "/", 1);
        memcpy(dpath + strlen(cur_path) + 1, dname, strlen(dname) + 1);
    } else {
        memcpy(dpath + strlen(cur_path), dname, strlen(dname) + 1);
    }
    try_cd(dpath, dname);
}

/**
 * @brief Moves up or down one directory
 * @details Helper function of cmd_line_cd
 * Moves up or down one directory, specified by dname
 * @param dname Next directory in which to move
 */
void process_cd_part(const char *dname) {
    if (strcmp(dname, "..") == 0) {     // Move up one directory
        move_up_directory();
        return;
    } else {
        move_into_directory(dname);
        return;
    }
}

/**
 * @brief Try to move into a new directory
 * @details Try to move into the directory at dpath.
 * If not possible, doesn't update cur_path.
 * @param dpath Full path to new directory location
 * @param dname Name of new directory location
 */
void try_cd(const char *dpath, const char *dname) {
    struct iso_dir *dir = iso_dopen(dpath, 3);
    if (!dir) {
        console_printf("cd: %s is not a directory\n", dname);
        return;
    }
    iso_dclose(dir);
    set_cur_path(dpath);
}

/**
 * @brief Change current working directory
 * @details Change current working directory to that specified by arg_line
 * @param arg_line All arguments given after command 'cd'
 */
void cmd_line_cd(const char *arg_line) {
    char line_copy[strlen(arg_line) + 1];
    strcpy(line_copy, arg_line);
    char *first_word = strtok(line_copy, " ");

    // If you give no arguments to cd or the argument is "/", you are sent to root
    if (strlen(first_word) == 0 || strcmp(first_word, ROOT) == 0) {
        set_cur_path(ROOT);
        return;
    }

    // Process path piece-wise separated by '/'
    char *next_dir = strtok(first_word, "/");
    while (next_dir) {
        process_cd_part(next_dir);
        next_dir = strtok(0, "/");
    }
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
