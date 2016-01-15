/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef CMD_LINE_H
#define CMD_LINE_H

//Publics
void cmd_line_attempt(const char * line);

//Privates
void cmd_line_run_echo(const char * arg_line);
void cmd_line_run_date(const char * arg_line);

#endif
