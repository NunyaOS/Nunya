/*
Copyright (C) 2016 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef SYSCALL_HANDLER_WINDOW_H
#define SYSCALL_HANDLER_WINDOW_H

#include "window.h"
#include "syscall.h"

int32_t sys_window_create(int x, int y, int width, int height);

int32_t sys_set_border_color(const struct graphics_color *color);

int32_t sys_draw_line(int x1, int y1, int x2, int y2, const struct graphics_color *color);

int32_t sys_draw_arc(int x, int y, const struct arc_info *arc, const struct graphics_color *color);

int32_t sys_draw_circle(int x, int y, const double *r, const struct graphics_color *color);

int32_t sys_draw_char(int x, int y, char c, const struct graphics_color *fgcolor, const struct graphics_color *bgcolor);

int32_t sys_draw_string(int x, int y, const char *str, const struct graphics_color *fgcolor,
    const struct graphics_color *bgcolor);

#endif
