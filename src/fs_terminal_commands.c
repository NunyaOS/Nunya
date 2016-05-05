/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "console.h"
#include "string.h"
#include "iso.h"
#include "fs_terminal_commands.h"

void cat_file(const char *fname);
void get_abs_path(const char *path, char *abs_path);
void ls_dir(const char *dname);
void move_into_directory(const char *dname, char *abs_path);
void move_up_directory(char *abs_path);
void process_path_part(const char *dname, char *abs_path);

char cur_path[WORKING_DIRECTORY_PATH_BUFFER_SIZE]; // The current working directory for the command line environment

/**
 * @brief Prints contents of a single file
 * @param fname Name of file to be printed
 */
void cat_file(const char *fname) {
    char abs_path[WORKING_DIRECTORY_PATH_BUFFER_SIZE];
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
 * @brief Lists contents of a single directory
 * @param dname Name of directory to list
 */
void ls_dir(const char *dname) {
    char abs_path[WORKING_DIRECTORY_PATH_BUFFER_SIZE];
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
 * @brief Move into a directory below yourself
 * @details Move into the directory below yourself specified by dname
 * @param dname Name of directory into which you attempt to move
 */
void move_into_directory(const char *dname, char *abs_path) {
    if (strcmp(abs_path, ROOT) != 0) {   // Not at root, so we need to add a '/' to the path
        int abs_path_len = strlen(abs_path);

        // Check if next part of path will overflow buffer
        if ((abs_path_len + strlen(dname) + 1) >= (WORKING_DIRECTORY_PATH_BUFFER_SIZE - 1)) {
            abs_path[0] = '\0';
            return;
        }
        memcpy(abs_path + abs_path_len, "/", 1);
        memcpy(abs_path + abs_path_len + 1, dname, strlen(dname) + 1);
    } else {
        int abs_path_len = strlen(abs_path);

        // Check if next part of path will overflow buffer
        if ((abs_path_len + strlen(dname)) >= (WORKING_DIRECTORY_PATH_BUFFER_SIZE - 1)) {
            abs_path[0] = '\0';
            return;
        }
        memcpy(abs_path + strlen(abs_path), dname, strlen(dname) + 1);
    }
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

void cmd_line_cat(const char *arg_line) {
    if(strcmp(arg_line, "--HELP") == 0) {
        console_printf("Print contents of a file to screen\nusage: cat <file1, file2, ...>\n");
        return;
    }
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

void cmd_line_cd(const char *arg_line) {
    if(strcmp(arg_line, "--HELP") == 0) {
        console_printf("Change the current working directory\nusage: cd <directory path>\n");
        return;
    }
    char line_copy[strlen(arg_line) + 1];
    strcpy(line_copy, arg_line);
    char *first_word = strtok(line_copy, " ");

    // If you give no arguments to cd or the argument is "/", you are sent to root
    if (strlen(first_word) == 0 || strcmp(first_word, ROOT) == 0) {
        set_cur_path(ROOT);
        return;
    }

    char abs_path[WORKING_DIRECTORY_PATH_BUFFER_SIZE];
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

void cmd_line_echo(const char *arg_line) {
    if(strcmp(arg_line, "--HELP") == 0) {
        console_printf("Print contents of argument to screen\nusage: echo <buffer to print>\n");
        return;
    }
    console_printf("%s\n", arg_line);
    return;
}

void cmd_line_ls(const char *arg_line) {
    if(strcmp(arg_line, "--HELP") == 0) {
        console_printf("List the contents of a directory (default is current working directory\n");
        console_printf("usage: ls <directory path>\n");
        return;
    }
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

void cmd_line_pwd(const char *arg_line) {
    if(strcmp(arg_line, "--HELP") == 0) {
        console_printf("Print the current working directory\nusage: pwd\n");
        return;
    }
    console_printf("%s\n", cur_path);
}

const char *get_cur_path() {
    return cur_path;
}

void set_cur_path(const char *dpath) {
    if (strlen(dpath) < WORKING_DIRECTORY_PATH_BUFFER_SIZE) {
        strcpy(cur_path, dpath);
    } else {
        console_printf("provided path exceeds bounds [size %d]\n", WORKING_DIRECTORY_PATH_BUFFER_SIZE);
    }
}
