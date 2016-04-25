/*
Copyright (C) 2016 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef SYS_WINDOW_STRUCT_H
#define SYS_WINDOW_STRUCT_H

#include "kerneltypes.h"
#include "list.h"

struct graphics_color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

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

#endif