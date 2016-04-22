/*
Copyright (C) 2016 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include "sys_window_struct.h"

void send_event_mouse_click();

void send_event_mouse_move();

void send_event_keyboard_press(char c);

extern struct window *active_window;

#endif
