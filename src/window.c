/*
Copyright (C) 2016 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "window.h"
#include "memory.h"
#include "window_manager.h"

#define CHARACTER_W 8
#define CHARACTER_H 8

// a special number to see if the offset calculation has been done
// TODO: change b/c we want to allow offsets to be negative (when moving window out of frame)
#define OFFSET_INIT -1
// this can be -1 as bounds will always be [0, graphics_width/height)
#define BOUNDS_INIT -1

static inline void window_clear_bounds(struct window *win) {
    win->bounds_x_1 = 0;
    win->bounds_x_2 = 0;
    win->bounds_y_1 = 0;
    win->bounds_y_2 = 0;
}

static inline void window_set_bounds(struct window *win, int x, int y, int w, int h) {
    // check if bounds_x_1 ... y_2 are -1.
    // if so, do calculation. otherwise, do nothing
    win->bounds_x_1 = x < 0 ? 0 : x;
    win->bounds_x_2 = x + w >= graphics_width() ? graphics_width() - 1 : x + w;
    win->bounds_y_1 = y < 0 ? 0 : y;
    win->bounds_y_2 = y + h >= graphics_height() ? graphics_height() - 1 : y + h;
}

// This function prepares to draw in a window by setting up its boundary
// restrictions.
// THIS FUNCTION SHOULD ALWAYS BE CALLED WHEN MAKING A WINDOW
// OR MOVING A WINDOW
static inline void window_init_draw(struct window *w) {
	// if one bounds number has been set, have already calculated all bounds and offsets
	if (w->bounds_x_1 != BOUNDS_INIT && w->x_offset != OFFSET_INIT) {
		return;
	}
    int x = w->x;
    int y = w->y;
    struct window *init = w;
    int width = w->width;
    int height = w->height;
    while (w->parent) {
        w = w->parent;
        x += w->x;
        y += w->y;
        if(w->width - x < width)
            width = w->width - x;
        if(w->height - y < height)
            height = w->height - y;
    }

    // Account for the offset of the top-level window
    height += w->y;
    width += w->x;

    init->x_offset = x;
    init->y_offset = y;
    window_set_bounds(init, x, y, width, height);
}

struct window *window_create(int x, int y, int width, int height, struct window *parent) {
    struct window *w = kmalloc(sizeof(*w));

    w->x = x;
    w->y = y;
    w->width = width;
    w->height = height;
    w->parent = parent;
    struct list l = {0, 0};
    w->event_queue = l;
    w->x_offset = OFFSET_INIT;
    w->y_offset = OFFSET_INIT;
    w->bounds_x_1 = BOUNDS_INIT;
    w->bounds_x_2 = BOUNDS_INIT;
    w->bounds_y_1 = BOUNDS_INIT;
    w->bounds_y_2 = BOUNDS_INIT;
	window_init_draw(w);

    // Draw the border
    struct graphics_color bc = {128,128,128};
    window_set_border_color(w, bc);

    active_window = w;
    return w;
}

void window_set_border_color(struct window *w, struct graphics_color border_color) {
    w->border_color = border_color;

    // Draw the new border
    window_draw_line(w, 0, 0, 0, w->height, border_color);
    window_draw_line(w, 0, 0, w->width, 0, border_color);
    window_draw_line(w, 0, w->height, w->width, w->height, border_color);
    window_draw_line(w, w->width, 0, w->width, w->height, border_color);
}

void window_draw_line(struct window *w, int x1, int y1, int x2, int y2, struct graphics_color color) {
    x1 += w->x_offset;
    y1 += w->y_offset;
    x2 += w->x_offset;
    y2 += w->y_offset;
    graphics_line(x1, y1, x2, y2, color, 0);
}

void window_draw_arc(struct window *w, int x, int y, double r, double start_theta, double end_theta, struct graphics_color color) {
    graphics_arc(x + w->x, y + w->y, r, start_theta, end_theta, color);
}

void window_draw_circle(struct window *w, int x, int y, double r, struct graphics_color color) {
    graphics_circle(x + w->x, y + w->y, r, color);
}

void window_draw_char(struct window *w, int x, int y, char ch, struct graphics_color fgcolor,
                      struct graphics_color bgcolor) {
    x += w->x_offset;
    y += w->y_offset;
    graphics_char(x, y, ch, fgcolor, bgcolor);
}

void window_draw_string(struct window *w, int x, int y, const char *str, struct graphics_color fgcolor,
                        struct graphics_color bgcolor) {
    /*x += w->x_offset;*/
    /*y += w->y_offset*/
    int pos_h = 0;
    int pos_w = 0;
    while (*str) {
        if (*str == '\n') {
            pos_h += CHARACTER_H;
            pos_w = 0;
            str++;
            continue;
        }
        window_draw_char(w, x + pos_w, y + pos_h, *str, fgcolor, bgcolor);
        pos_w += CHARACTER_W;
        str++;
    }
}

void window_draw_bitmap(struct window *w, int x, int y, int width, int height, uint8_t * data,
                     struct graphics_color fgcolor,
                     struct graphics_color bgcolor) {
    graphics_bitmap(x + w->x, y + w->y, width, height, data, fgcolor, bgcolor);
}


void window_hierarchy_test() {
    struct graphics_color text_color = {0,255,0};
    struct graphics_color text_bg = {0,0,0};

    struct window *top = window_create(30, 30, 500, 400, 0);
    struct window *child = window_create(100, 200, 500, 300, top);
    window_draw_string(child, 250, 0, "Hello world!", text_color, text_bg);
    kfree(top);
    kfree(child);
}

