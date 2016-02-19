/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef MOUSE_H
#define MOUSE_H

#define MOUSE_DATA_PORT 0x60
// used to check if data on port 0x60 is for the mouse or the keyboard
#define MOUSE_CHECK_PORT 0x64

char keyboard_read();
const char *keyboard_read_str();

void keyboard_init();

#endif

