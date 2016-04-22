/*
Copyright (C) 2016 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/
#include "syscall_handler_window.h"
#include "process.h"
#include "string.h"

#define CHECK_PROC_WINDOW() if(current->window == 0) return -1

int32_t sys_window_create(int x, int y, int width, int height) {
    if (current->window != 0) {
        return -1;
    }
    struct window *win = window_create(x, y, width, height, 0);
    current->window = win;
    return win != 0;
}

int32_t sys_set_border_color(const struct graphics_color *color) {
    CHECK_PROC_WINDOW();
    window_set_border_color(current->window, *color);
    return 0;
}

int32_t sys_draw_line(int x1, int y1, int x2, int y2, const struct graphics_color *color) {
    CHECK_PROC_WINDOW();
    window_draw_line(current->window, x1, y1, x2, y2, *color);
    return 0;
}

int32_t sys_draw_arc(int x, int y, const struct arc_info *arc, const struct graphics_color *color) {
    CHECK_PROC_WINDOW();
    window_draw_arc(current->window, x, y, arc->r, arc->start_theta, arc->end_theta, *color);
    return 0;
}

int32_t sys_draw_circle(int x, int y, const double *r, const struct graphics_color *color) {
    CHECK_PROC_WINDOW();
    window_draw_circle(current->window, x, y, *r, *color);
    return 0;
}

int32_t sys_draw_char(int x, int y, char c, const struct graphics_color *fgcolor, const struct graphics_color *bgcolor) {
    CHECK_PROC_WINDOW();
    window_draw_char(current->window, x, y, c, *fgcolor, *bgcolor);
    return 0;
}

int32_t sys_draw_string(int x, int y, const char *str, const struct graphics_color *fgcolor,
    const struct graphics_color *bgcolor) {
    CHECK_PROC_WINDOW();
    window_draw_string(current->window, x, y, str, *fgcolor, *bgcolor);
    return 0;
}

int32_t sys_get_event(struct event *e) {
    CHECK_PROC_WINDOW();
    struct list *list = &(current->window->event_queue);
    struct event *last_event = (struct event *)list_pop_tail(list);
    if (last_event == 0) {
        return 2;
    }

    memcpy(e, last_event, sizeof(struct event));

    return 0;
}
