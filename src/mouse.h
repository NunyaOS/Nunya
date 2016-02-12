/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef MOUSE_H
#define MOUSE_H
// IRQ12: mouse
// 44: interrupt number

char keyboard_read();
const char *keyboard_read_str();

void keyboard_init();

#endif

