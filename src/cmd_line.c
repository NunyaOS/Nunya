/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/


#include "console.h"
#include "string.h"

#define KEYBOARD_BUFFER_SIZE 256
//Privates
void cmd_line_run_echo(const char * arg_line)
{
    console_printf("%s\n", arg_line);
    return;
}

void cmd_line_run_date(const char * arg_line)
{
    console_printf("date: command not found\n");
    return;
}

//Exposed
void cmd_line_attempt(const char * line)
{
    char line_copy[KEYBOARD_BUFFER_SIZE];
    strcpy(line_copy, line);
    char *first_word = strtok(line_copy, " ");
    const char *the_rest = line + (strlen(first_word) + 1);

    //Ugly case statement alternative
    if(!strcmp("echo", first_word))
        cmd_line_run_echo(the_rest);
    else if(!strcmp("date", first_word))
        cmd_line_run_date(the_rest);
    else
        console_printf("%s: command not found\n", first_word);
    return;
}
