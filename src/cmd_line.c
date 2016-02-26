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

char cur_path[256]; // The current working directory for the command line environment

void move_up_directory(char *abs_path);
void move_into_directory(const char *dname, char *abs_path);
void process_path_part(const char *dname, char *abs_path);
void set_cur_path(const char *dpath);
void get_abs_path(const char *path, char *abs_path);
void cat_file(const char *fname);
void ls_dir(const char *dname);

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
void move_up_directory(char *abs_path) {
    if (strcmp(abs_path, "/") == 0) {
        abs_path[0] = '\0';
        return;
    }
    int i;
    // Move up one directory by finding the last occurrence of '/'
    for (i = strlen(abs_path); i >= 0; i--) {
        if (i == 0) {
            strcpy(abs_path, ROOT);
            return;
        }
        else if (abs_path[i] == '/') {
            abs_path[i] = '\0';
            return;
        }
    }
}

/**
 * @brief Move into a directory below yourself
 * @details Move into the directory below yourself specified by dname
 * @param dname Name of directory into which you attempt to move
 */
void move_into_directory(const char *dname, char *abs_path) {
    if (strcmp(abs_path, ROOT) != 0) {   // Not at root, so we need to add a '/' to the path
        int abs_path_len = strlen(abs_path);
        memcpy(abs_path + abs_path_len, "/", 1);
        memcpy(abs_path + abs_path_len + 1, dname, strlen(dname) + 1);
    } else {
        memcpy(abs_path + strlen(abs_path), dname, strlen(dname) + 1);
    }
}

/**
 * @brief Moves up or down one directory
 * @details Helper function of cmd_line_cd
 * Moves up or down one directory, specified by dname
 * @param dname Next directory in which to move
 */
void process_path_part(const char *dname, char *abs_path) {
    if (strcmp(dname, ".") == 0) {
        return;
    } else if (strcmp(dname, "..") == 0) {     // Move up one directory
        move_up_directory(abs_path);
        return;
    } else {
        move_into_directory(dname, abs_path);
        return;
    }
}

/**
 * @brief Translate a path argument into an absolute path
 * @details Takes filepath path and translates it into an
 * absolute path that is copied into abs_path. If the path
 * is relative, it builds off of the cur_path. Otherwise, it
 * treats path as an absolute path.
 * @param path Argument that points to the desired file/directory
 * @param abs_path Buffer into which resultant absolute path is placed
 */
void get_abs_path(const char *path, char *abs_path) {
    if (strlen(path) == 0) {
        abs_path[0] = '\0';
        return;
    }
    if (strcmp(path, ROOT) == 0) {
        strcpy(abs_path, ROOT);
        return;
    }
    char path_copy[256];
    strcpy(path_copy, path);
    char *next_dir;

    if (path[0] == '/') {   // Absolute path
        strcpy(abs_path, "/");
        next_dir = strtok(path_copy + 1, "/");    // Move up one character to look like a relative path in root directory
    } else {    // Relative path
        strcpy(abs_path, cur_path);
        next_dir = strtok(path_copy, "/");
    }

    while (next_dir) {
        process_path_part(next_dir, abs_path);
        if (!abs_path[0]) {     // An error occurred on this part of the path
            return;
        }
        next_dir = strtok(0, "/");
    }
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

    char abs_path[256];
    get_abs_path(first_word, abs_path);
    if (!abs_path[0]) {
        console_printf("cd: root has no parent\n");
        return;
    }
    struct iso_dir *dir = iso_dopen(abs_path, 3);
    if (!dir) {
        console_printf("cd: no such path %s\n", abs_path);
    } else {
        set_cur_path(abs_path);
    }
    iso_dclose(dir);
    return;
}

/**
 * @brief List contents for multiple directories
 * @details List all files and directories for the directories
 * listed in arg_line. If no argument it given, it assumes the
 * self directory. Otherwise, it prints out the contents of each
 * directory supplied in arg_line
 * @param arg_line Paths to directories to list. If empty, assumes self.
 */
void cmd_line_ls(const char *arg_line) {
    if (strlen(arg_line) > 0) {
        char arg_line_copy[256];
        strcpy(arg_line_copy, arg_line);
        char *dname = arg_line_copy;
        int i;
        for(i = 0; i < strlen(arg_line); i++) {
            if (arg_line_copy[i] == ' ') {
                arg_line_copy[i] = '\0';
                console_printf("%s:\n", dname);
                ls_dir(dname);
                console_printf("\n");
                dname = arg_line_copy + i + 1;
            }
        }
        ls_dir(dname);
    } else {
        ls_dir(cur_path);
    }
}

/**
 * @brief Lists contents of a single directory
 * @param dname Name of directory to list
 */
void ls_dir(const char *dname) {
    char abs_path[256];
    get_abs_path(dname, abs_path);
    struct iso_dir *dir = iso_dopen(abs_path, 3);
    if (dir) {
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
    } else {
        console_printf("ls: no directory %s\n", dname);
    }
    iso_dclose(dir);
}

/**
 * @brief Print the current working directory
 */
void cmd_line_pwd() {
    console_printf("%s\n", cur_path);
}

/**
 * @brief Echo the argument supplied on a new line
 * @param arg_line The string to be printed out
 */
void cmd_line_echo(const char *arg_line) {
    console_printf("%s\n", arg_line);
    return;
}

/**
 * @brief Prints the contents of multiple files
 * @details Prints the contents of multiple files
 * listed in the argument arg_line separated by space.
 * @param arg_line Names of files to be printed
 */
void cmd_line_cat(const char *arg_line) {
    char arg_line_copy[256];
    strcpy(arg_line_copy, arg_line);
    char *fname = arg_line_copy;
    int i;
    for(i = 0; i < strlen(arg_line); i++) {
        if (arg_line_copy[i] == ' ') {
            arg_line_copy[i] = '\0';
            cat_file(fname);
            fname = arg_line_copy + i + 1;
        }
    }
    cat_file(fname);
}

/**
 * @brief Prints contents of a single file
 * @param fname Name of file to be printed
 */
void cat_file(const char *fname) {
    char abs_path[256];
    char c;
    get_abs_path(fname, abs_path);
    struct iso_file *file = iso_fopen(abs_path, 3);
    if (!file) {
        console_printf("cat: %s does not exist\n", fname);
        iso_fclose(file);
        return;
    }
    while (!file->at_EOF) {
        iso_fread(&c, 1, 1, file);
        console_putchar(c);
    }
    iso_fclose(file);
}

//Exposed
void cmd_line_attempt(const char * line) {
    char line_copy[KEYBOARD_BUFFER_SIZE];
    strcpy(line_copy, line);
    char *first_word = strtok(line_copy, " ");
    const char *the_rest = line_copy + (strlen(first_word) + 1);

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
    memset(line_copy, '\0', KEYBOARD_BUFFER_SIZE);
    return;
}
