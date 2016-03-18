/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef CMD_LINE_H
#define CMD_LINE_H

/**
* @brief Attempt to run the command given with its arguments
* @detail Splits the line on whitespace, taking the first as the command and the rest as arguments. Will print "command not found" message if command is not found.
*
* @param line The string typed in at the prompt.
* @return void
*/
void cmd_line_attempt(const char * line);

/**
 * @brief Initialize the command line
 */
void cmd_line_init();

/**
 * @brief Display terminal prompt to user
 * @details Prints in light red the current working directory
 * to the terminal screen
 */
void cmd_line_show_prompt();

/**
 * @brief Run a designated test user process
 * @details Load a designated user process from test_iso.iso and execute it.
 */
void cmd_line_run_userproc();

#endif
