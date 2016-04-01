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

#define ARC_DT 0.01

static int bounds_x_1, bounds_x_2, bounds_y_1, bounds_y_2;

void graphics_draw_mouse();

int graphics_width() {
    return video_xres;
}

int graphics_height() {
    return video_yres;
}

void graphics_init() {
    graphics_clear_bounds();
}

void graphics_set_bounds(int x, int y, int w, int h) {
    bounds_x_1 = x < 0 ? 0 : x;
    bounds_x_2 = x + w >= video_xres ? video_xres - 1 : x + w;
    bounds_y_1 = y < 0 ? 0 : y;
    bounds_y_2 = y + h >= video_yres ? video_yres - 1 : y + h;
}

void graphics_clear_bounds() {
    // Clearing bounds actually just sets the bounds to be the edges of
    // the screen. This allows us to ensure we don't draw outside of screen memory
    bounds_x_1 = 0;
    bounds_x_2 = video_xres - 1;
    bounds_y_1 = 0;
    bounds_y_2 = video_yres - 1;
}

static inline void plot_pixel(int x, int y, struct graphics_color c) {
    // Check to make sure that we are not drawing out of bounds
    if (x < bounds_x_1 || x > bounds_x_2 || y < bounds_y_1 || y > bounds_y_2) {
        return;
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
        graphics_draw_mouse();
    }
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

void graphics_line(int x1, int y1, int x2, int y2, struct graphics_color c) {

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
            plot_pixel(x1, i, c);
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
            plot_pixel(x, y, c);
            error = error + deltaerr;
            while (error >= 0.5) {
                plot_pixel(x, y, c);
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
            if (i < 0 || i > video_xres - 1 || j < 0 || j > video_yres - 1) {
                continue;
            }
            uint8_t *v = video_buffer + video_xbytes * j + i * 3;
            v[2] = buffer[buf_ix].r;
            v[1] = buffer[buf_ix].g;
            v[0] = buffer[buf_ix].b;
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
            if (i < 0 || i > video_xres - 1 || j < 0 || j > video_yres - 1) {
                continue;
            }
            uint8_t *v = video_buffer + video_xbytes * j + i * 3;
            buffer[buf_ix].r = v[2];
            buffer[buf_ix].g = v[1];
            buffer[buf_ix].b = v[0];
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
void graphics_draw_mouse() {
    int i;
    uint8_t *v;
    int start_x = (mouse_x - MOUSE_SIDE_2 + 1) < 0 ? 0 : mouse_x - MOUSE_SIDE_2 + 1;
    int end_x = (mouse_x + MOUSE_SIDE_2) > video_xres ? video_xres : mouse_x + MOUSE_SIDE_2;
    // horizontal line
    for (i = start_x; i < end_x; i++) {
        v = video_buffer + video_xbytes * mouse_y + i * 3;
        v[2] = mouse_fg_color.r;
        v[1] = mouse_fg_color.g;
        v[0] = mouse_fg_color.b;
    }

    int start_y = (mouse_y - MOUSE_SIDE_2 + 1) < 0 ? 0 : mouse_y - MOUSE_SIDE_2 + 1;
    int end_y = (mouse_y + MOUSE_SIDE_2) > video_yres ? video_yres : mouse_y + MOUSE_SIDE_2;
    // vertical line
    for (i = start_y; i < end_y; i++) {
        v = video_buffer + video_xbytes * i + mouse_x * 3;
        v[2] = mouse_fg_color.r;
        v[1] = mouse_fg_color.g;
        v[0] = mouse_fg_color.b;
    }
}

void graphics_mouse() {
    // copy mouse buffer into video buf
    graphics_copy_from_color_buffer(old_mouse_x - MOUSE_SIDE_2 + 1, old_mouse_y - MOUSE_SIDE_2 + 1, MOUSE_SIDE - 1, MOUSE_SIDE - 1, mouse_draw_buffer, (MOUSE_SIDE - 1) * (MOUSE_SIDE - 1));
    // copy mouse region of video buf into mouse buffer
    graphics_copy_to_color_buffer(mouse_x - MOUSE_SIDE_2 + 1, mouse_y - MOUSE_SIDE_2 + 1, MOUSE_SIDE - 1, MOUSE_SIDE - 1, mouse_draw_buffer, (MOUSE_SIDE - 1) * (MOUSE_SIDE - 1));
    // actually draw mouse
    graphics_draw_mouse();
}

