/*
Copyright (C) 2016 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "window.h"
#include "memory.h"

#define CHARACTER_W 8
#define CHARACTER_H 8

// This function prepares to draw in a window by setting up its boundary
// restrictions.
// THIS FUNCTION SHOULD ALWAYS BE CALLED BEFORE MAKING A GRAPHICS DRAW
// CALL IN THE WINDOW SYSTEM
static inline void window_begin_draw(struct window *w) {
    graphics_set_bounds(w->x, w->y, w->width, w->height);
}

// This function removes the restrictions used to enforce window clipping
// on the graphics system
// THIS FUNCTION SHOULD ALWAYS BE CALLED IF window_begin_draw WAS CALLED
// TO FINISH THE DRAW
static inline void window_end_draw() {
    graphics_clear_bounds();
}

struct window *window_create(int x, int y, int width, int height) {
    struct window *w = kmalloc(sizeof(*w));

    w->x = x;
    w->y = y;
    w->width = width;
    w->height = height;

    // Draw the border
    struct graphics_color bc = {128,128,128};
    window_set_border_color(w, bc);

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
    graphics_line(x1 + w->x, y1 + w->y, x2 + w->x, y2 + w->y, color);
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
    graphics_char(x + w->x, y + w->y, ch, fgcolor, bgcolor);
    window_end_draw();
}

void window_draw_string(struct window *w, int x, int y, const char *str, struct graphics_color fgcolor,
                        struct graphics_color bgcolor) {
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