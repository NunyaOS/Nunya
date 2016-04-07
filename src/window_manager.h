/*
Copyright (C) 2016 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include "list.h"

/**
 * Defines possible types of events that the system
 * can recieve
 */
typedef enum event_type {
	EVENT_MOUSE_MOVE,
	EVENT_MOUSE_CLICK,
	EVENT_KEYBOARD_PRESS
} event_type_t;

/**
 * @brief A window event
 * @details This represents a single user input event,
 * and tracks what type of event occured and the character
 * that was typed, if it was a keyboard event
 * 
 * If the event was not a keyboard event, character will be
 * the null terminator '\0' 
 * 
 */
struct event {
	struct list_node node;
	event_type_t type;
	char character;
};

void send_event_mouse_click();

void send_event_mouse_move();

void send_event_keyboard_press(char c);

extern struct window *active_window;

#endif
