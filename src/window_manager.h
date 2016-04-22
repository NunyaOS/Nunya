/*
Copyright (C) 2016 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include "sys_window_struct.h"

/**
 * @brief Posts a mouse click event
 * @details Adds a mouse click event to the active window's
 * event queue
 */
void send_event_mouse_click();

/**
 * @brief Post a mouse move event
 * @details Adds a mouse move event to the active window's
 * event queue
 */
void send_event_mouse_move();

/**
 * @brief Post a keyboard press event
 * @details Adds a keyboard press event to the active window's
 * event queue
 * 
 * @param c The character that was pressed
 */
void send_event_keyboard_press(char c);

extern struct window *active_window;

#endif
