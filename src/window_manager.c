/*
Copyright (C) 2016 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "window_manager.h"
#include "kmalloc.h"
#include "window.h"

struct window *active_window = 0;

static void send_event(struct event *e) {
	if (!active_window || !e) {
		return;
	}

	struct list *l = &(active_window->event_queue);
	list_push_head(l, (struct list_node *)e);
}

void send_event_mouse_click() {
	struct event *e = kmalloc(sizeof(struct event));
	e->type = EVENT_MOUSE_CLICK;
	e->character = '\0';
	send_event(e);
}

void send_event_mouse_move() {
	struct event *e = kmalloc(sizeof(struct event));
	e->type = EVENT_MOUSE_MOVE;
	e->character = '\0';
	send_event(e);
}

void send_event_keyboard_press(char c) {
	struct event *e = kmalloc(sizeof(struct event));
	e->type = EVENT_KEYBOARD_PRESS;
	e->character = c;
	send_event(e);
}