/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef FS_TERMINAL_COMMANDS_H
#define FS_TERMINAL_COMMANDS_H

#define ROOT "/"

#define WORKING_DIRECTORY_PATH_BUFFER_SIZE 256


/**
 * @brief Prints the contents of multiple files
 * @details Prints the contents of multiple files
 * listed in the argument arg_line separated by space.
 * @param arg_line Names of files to be printed
 */
void cmd_line_cat(const char *arg_line);

/**
 * @brief Change current working directory
 * @details Change current working directory to that specified by arg_line
 * @param arg_line All arguments given after command 'cd'
 */
void cmd_line_cd(const char *arg_line);

/**
 * @brief Echo the argument supplied on a new line
 * @param arg_line The string to be printed out
 */
void cmd_line_echo(const char *arg_line);

/**
 * @brief List contents for multiple directories
 * @details List all files and directories for the directories
 * listed in arg_line. If no argument is given, it assumes the
 * self directory. Otherwise, it prints out the contents of each
 * directory supplied in arg_line
 * @param arg_line Paths to directories to list. If empty, assumes self.
 */
void cmd_line_ls(const char *arg_line);

/**
 * @brief Print the current working directory
 */
void cmd_line_pwd();

/**
 * @brief Get the current working directory path
 * @return Current working directory path
 */
const char *get_cur_path();

/**
 * @brief Set the current working directory
 * @param dpath The path to which to set the current working directory
 */
void set_cur_path(const char *dpath);

#endif
