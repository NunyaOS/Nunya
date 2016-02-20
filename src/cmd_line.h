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

#endif
