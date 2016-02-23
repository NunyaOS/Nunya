/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/


#include "console.h"
#include "string.h"
#include "testing.h"

#define KEYBOARD_BUFFER_SIZE 256

//Privates
void cmd_line_echo(const char *arg_line) {
    console_printf("%s\n", arg_line);
    return;
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
    } else if(strcmp("test", first_word) == 0) {
        run_all_tests();
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
