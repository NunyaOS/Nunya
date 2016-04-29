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

static int x_offset = 0;
static int y_offset = 0;

// This function prepares to draw in a window by setting up its boundary
// restrictions.
// THIS FUNCTION SHOULD ALWAYS BE CALLED BEFORE MAKING A GRAPHICS DRAW
// CALL IN THE WINDOW SYSTEM
static inline void window_begin_draw(struct window *w) {
    int x = w->x;
    int y = w->y;
    int width = w->width;
    int height = w->height;
    while(w->parent) {
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

    x_offset = x;
    y_offset = y;

    graphics_set_bounds(x, y, width, height);
}

// This function removes the restrictions used to enforce window clipping
// on the graphics system
// THIS FUNCTION SHOULD ALWAYS BE CALLED IF window_begin_draw WAS CALLED
// TO FINISH THE DRAW
static inline void window_end_draw() {
    graphics_clear_bounds();
    x_offset = 0;
    y_offset = 0;
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

    // Draw the border
    struct graphics_color border_color = {128,128,128};
    struct graphics_color background_color = {0,0,0};
    w->background_color = background_color;
    window_set_border_color(w, border_color);

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
    window_begin_draw(w);
    x1 += x_offset;
    y1 += y_offset;
    x2 += x_offset;
    y2 += y_offset;
    graphics_line(x1, y1, x2, y2, color, 0);
    window_end_draw();
}

void window_draw_arc(struct window *w, int x, int y, double r, double start_theta, double end_theta, struct graphics_color color) {
    window_begin_draw(w);
    graphics_arc(x + w->x, y + w->y, r, start_theta, end_theta, color);
    window_end_draw();
}

void window_draw_circle(struct window *w, int x, int y, double r, struct graphics_color color) {
    window_begin_draw(w);
    graphics_circle(x + w->x, y + w->y, r, color);
    window_end_draw();
}

void window_draw_char(struct window *w, int x, int y, char ch, struct graphics_color fgcolor,
                      struct graphics_color bgcolor) {
    window_begin_draw(w);
    x += x_offset;
    y += y_offset;
    graphics_char(x, y, ch, fgcolor, bgcolor);
    window_end_draw();
}

void window_draw_string(struct window *w, int x, int y, const char *str, struct graphics_color fgcolor,
                        struct graphics_color bgcolor) {
    x += x_offset;
    y += y_offset;
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
    window_begin_draw(w);
    graphics_bitmap(x + w->x, y + w->y, width, height, data, fgcolor, bgcolor);
    window_end_draw();
}

void window_clear(struct window *w) {
    // Don't want to erase the border
    graphics_rect(w->x + 1, w->y + 1, w->width - 1, w->height - 1, w->background_color);
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