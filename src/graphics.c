/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "graphics.h"
#include "kerneltypes.h"
#include "font.h"
#include "kernelcore.h"
#include "math.h"
#include "mouse.h"
#include "process.h"
#include "window.h"

#define ARC_DT 0.01

void graphics_draw_cross_mouse();
void graphics_draw_pointer_mouse();

int graphics_width() {
    return video_xres;
}

int graphics_height() {
    return video_yres;
}

static inline void plot_pixel(int x, int y, struct graphics_color c) {
    // check graphics res bounds
    if (x < 0 || x > video_xres - 1 || y < 0 || y > video_yres - 1) {
        return;
    }

    // if there is a window, check that drawing doesn't exceed bounds
    // TODO: return if no window. only implement when nothing else does a raw graphics draw call
    // Check to make sure that we are not drawing out of bounds
    if (current->window) {
        struct window *w = current->window;
        if (x < w->bounds_x_1 || x > w->bounds_x_2 || y < w->bounds_y_1 || y > w->bounds_y_2) {
            return;
        }
    }

    uint8_t *v = video_buffer + video_xbytes * y + x * 3;
    v[2] = c.r;
    v[1] = c.g;
    v[0] = c.b;

    // Check to make sure that we are not drawing in mouse region
    int diff_x = x - mouse_x;
    int diff_y = y - mouse_y;
    // if the point is in the mouse region, draw to the mouse buffer, copy into the video region, and draw the mouse
    if (mouse_inited && diff_x < MOUSE_SIDE_2 && diff_x > -MOUSE_SIDE_2 && diff_y < MOUSE_SIDE_2 && diff_y > -MOUSE_SIDE_2) {
        // wraps to [0, 28]
        int x_index = diff_x + MOUSE_SIDE_2 - 1;
        int y_index = diff_y + MOUSE_SIDE_2 - 1;
        int index = y_index * (MOUSE_SIDE - 1) + x_index;
        mouse_draw_buffer[index].r = c.r;
        mouse_draw_buffer[index].g = c.g;
        mouse_draw_buffer[index].b = c.b;
        graphics_draw_pointer_mouse();
    }
}

// like plot_pixel, but does not do mouse erasing and redrawing or bounds checking
static inline void plot_pixel_no_check(int x, int y, struct graphics_color c) {
    if (x < 0 || x > video_xres - 1 || y < 0 || y > video_yres - 1) {
        return;
    }
    uint8_t *v = video_buffer + video_xbytes * y + x * 3;
    v[2] = c.r;
    v[1] = c.g;
    v[0] = c.b;
}

void graphics_arc(int x, int y, double r, double start_theta, double end_theta, struct graphics_color c) {
    for (; start_theta < end_theta; start_theta += ARC_DT) {
        int nx = r * cos(start_theta) + x;
        int ny = r * sin(start_theta) + y;

        plot_pixel(nx, ny, c);
    }
}

void graphics_circle(int x, int y, double r, struct graphics_color c) {
    double t;
    for (t = 0.0; t < 2 * M_PI; t += ARC_DT) {
        int nx = r * cos(t) + x;
        int ny = r * sin(t) + y;

        plot_pixel(nx, ny, c);
    }
}

void graphics_rect(int x, int y, int w, int h, struct graphics_color c) {
    int i, j;
    for (j = 0; j < h; j++) {
        for (i = 0; i < w; i++) {
            plot_pixel(x + i, y + j, c);
        }
    }
}

void graphics_line(int x1, int y1, int x2, int y2, struct graphics_color c, bool mouse_draw) {

    int tmp;
    // flip points if x2 is less than x1
    if (x2 < x1) {
        tmp = x2;
        x2 = x1;
        x1 = tmp;
        tmp = y2;
        y2 = y1;
        y1 = tmp;
    }
    double deltax = x2 - x1;
    double deltay = y2 - y1;
    double error = 0;

    // vertical line
    if (deltax == 0) {
        // flip vertical line where y2 < y1
        if (y2 < y1) {
            tmp = y2;
            y2 = y1;
            y1 = tmp;
        }
        int i;
        for (i = y1; i <= y2; i++) {
            if (mouse_draw) {
                plot_pixel_no_check(x1, i, c);
            }
            else {
                plot_pixel(x1, i, c);
            }
        }
    }
    else {
        int sign;
        double deltaerr = deltay / deltax;
        deltaerr = deltaerr < 0 ? -deltaerr : deltaerr;
        int y = y1;
        int x;
        int dy = y2 - y1;

        if (dy < 0) {
            sign = -1;
        }
        else {
            sign = 1;
        }

        for (x = x1; x <= x2; x++) {
            // check to see if drawing mouse or not
            if (mouse_draw) {
                plot_pixel_no_check(x, y, c);
            }
            else {
                plot_pixel(x, y, c);
            }

            error = error + deltaerr;
            while (error >= 0.5) {
                if (mouse_draw) {
                    plot_pixel_no_check(x, y, c);
                }
                else {
                    plot_pixel(x, y, c);
                }

                y = y + sign;
                error = error - 1.0;
            }
        }
    }
}

void graphics_clear(struct graphics_color c) {
    graphics_rect(0, 0, video_xres, video_yres, c);
}

void graphics_bitmap(int x, int y, int width, int height, uint8_t * data,
                     struct graphics_color fgcolor,
                     struct graphics_color bgcolor) {
    int i, j, b;
    int value;

    b = 0;

    for (j = 0; j < height; j++) {
        for (i = 0; i < width; i++) {
            value = ((*data) << b) & 0x80;
            if (value) {
                plot_pixel(x + i, y + j, fgcolor);
            } else {
                plot_pixel(x + i, y + j, bgcolor);
            }
            b++;
            if (b == 8) {
                data++;
                b = 0;
            }
        }
    }
}

void graphics_char(int x, int y, char ch, struct graphics_color fgcolor,
                   struct graphics_color bgcolor) {
    int u = ((int)ch) * FONT_WIDTH * FONT_HEIGHT / 8;
    return graphics_bitmap(x, y, FONT_WIDTH, FONT_HEIGHT, &fontdata[u],
                           fgcolor, bgcolor);
}

void graphics_copy_from_color_buffer(int x, int y, int width, int height, struct graphics_color *buffer, int buf_size) {
    int i, j;
    int buf_ix = 0;
    // copy old buffer into video buffer
    for (j = y; j < y + height; j++) {
        for (i = x; i < x + width; i++) {
            if (i >= 0 && i < video_xres && j >= 0 && j < video_yres ) {
                //draw pixel does bounds checking
                plot_pixel_no_check(i, j, buffer[buf_ix]);
            }

            buf_ix++;
            if (buf_ix >= buf_size) {
                return;
            }
        }
    }
}

void graphics_copy_to_color_buffer(int x, int y, int width, int height, struct graphics_color *buffer, int buf_size) {
    int i, j;
    int buf_ix = 0;
    // copy video buffer into color buffer
    for (j = y; j < y + height; j++) {
        for (i = x; i < x + width; i++) {
            if (i >= 0 && i < video_xres && j >= 0 && j < video_yres ) {
                uint8_t *v = video_buffer + video_xbytes * j + i * 3;
                buffer[buf_ix].r = v[2];
                buffer[buf_ix].g = v[1];
                buffer[buf_ix].b = v[0];
            }

            buf_ix++;
            if (buf_ix >= buf_size) {
                return;
            }
        }
    }
}

// Can't use graphics_line since it uses plot pixel.
// It does mouse region checking, which draws into the mouse buffer.
// It would then just draw into the mouse buffer, defeating the purpose of the mouse buffer.
void graphics_draw_cross_mouse() {
    // horizontal line
    int start_x = (mouse_x - MOUSE_SIDE_2 + 1) < 0 ? 0 : mouse_x - MOUSE_SIDE_2 + 1;
    int end_x = (mouse_x + MOUSE_SIDE_2) > video_xres ? video_xres : mouse_x + MOUSE_SIDE_2;
    graphics_line(start_x, mouse_y, end_x - 1, mouse_y, mouse_fg_color, 1);

    // vertical line
    int start_y = (mouse_y - MOUSE_SIDE_2 + 1) < 0 ? 0 : mouse_y - MOUSE_SIDE_2 + 1;
    int end_y = (mouse_y + MOUSE_SIDE_2) > video_yres ? video_yres : mouse_y + MOUSE_SIDE_2;
    graphics_line(mouse_x, start_y, mouse_x, end_y - 1, mouse_fg_color, 1);
}

// draws a standard pointer mouse
void graphics_draw_pointer_mouse() {
    // long pointer lines
    // TODO: why does MOUSE_SIDE_2 - 3 work but not MOUSE_SIDE_2 - 1 not work (only draw line issue)
    graphics_line(mouse_x, mouse_y, mouse_x + (MOUSE_SIDE_2 / 3), mouse_y + MOUSE_SIDE_2 - 3, mouse_fg_color, 1);
    graphics_line(mouse_x, mouse_y, mouse_x + MOUSE_SIDE_2 - 1, mouse_y + (MOUSE_SIDE_2 / 3), mouse_fg_color, 1);

    // base of pointer
    graphics_line(mouse_x + (MOUSE_SIDE_2 / 3), mouse_y + MOUSE_SIDE_2 - 1, mouse_x + (MOUSE_SIDE_2 / 2), mouse_y + (2 * MOUSE_SIDE_2 / 3), mouse_fg_color, 1);
    graphics_line(mouse_x + MOUSE_SIDE_2 - 1, mouse_y + (MOUSE_SIDE_2 / 3), mouse_x + (2 * MOUSE_SIDE_2 / 3), mouse_y + (MOUSE_SIDE_2 / 2), mouse_fg_color, 1);

    // tail of pointer
    graphics_line(mouse_x + (MOUSE_SIDE_2 / 2), mouse_y + (2 * MOUSE_SIDE_2 / 3), mouse_x + (2 * MOUSE_SIDE_2 / 3), mouse_y + MOUSE_SIDE_2 - 1, mouse_fg_color, 1);
    graphics_line(mouse_x + (2 * MOUSE_SIDE_2 / 3), mouse_y + (MOUSE_SIDE_2 / 2), mouse_x + MOUSE_SIDE_2 - 1, mouse_y + (2 * MOUSE_SIDE_2 / 3), mouse_fg_color, 1);
    // connecting line
    graphics_line(mouse_x + (2 * MOUSE_SIDE_2 / 3), mouse_y + MOUSE_SIDE_2 - 1, mouse_x + MOUSE_SIDE_2 - 1, mouse_y + (2 * MOUSE_SIDE_2 / 3), mouse_fg_color, 1);
}

void graphics_mouse() {
    // copy mouse buffer into video buf
    graphics_copy_from_color_buffer(old_mouse_x - MOUSE_SIDE_2 + 1, old_mouse_y - MOUSE_SIDE_2 + 1, MOUSE_SIDE - 1, MOUSE_SIDE - 1, mouse_draw_buffer, (MOUSE_SIDE - 1) * (MOUSE_SIDE - 1));
    // copy mouse region of video buf into mouse buffer
    graphics_copy_to_color_buffer(mouse_x - MOUSE_SIDE_2 + 1, mouse_y - MOUSE_SIDE_2 + 1, MOUSE_SIDE - 1, MOUSE_SIDE - 1, mouse_draw_buffer, (MOUSE_SIDE - 1) * (MOUSE_SIDE - 1));
    // actually draw mouse
    graphics_draw_pointer_mouse();
}

