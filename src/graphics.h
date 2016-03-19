/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "kerneltypes.h"
#include "mouse.h"

struct graphics_color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

int graphics_width();
int graphics_height();

/**
 * @brief Initializes the graphics system
 * @details Sets up the initial drawing bounds of the system
 * and prepares to begin drawing
 */
void graphics_init();

/**
 * @brief Sets bounds for restricted drawing
 * @details This will restrict all drawing to the given area of the screen.
 * Drawing outside this area will have no effect
 *
 * @param x The x position of the top left corner of the bounding box
 * @param y The y position of the top left corner of the bounding box
 * @param width The width of the bounding box
 * @param height The height of the bounding box
 */
void graphics_set_bounds(int x, int y, int width, int height);

/**
 * @brief Clears the graphics drawing restrictions
 * @details This removes any restriction on drawing area that was added
 * by graphics_set_bounds
 */
void graphics_clear_bounds();

/**
 * @brief Draws a line on the display from (x1, y1) to (x2, y2)
 * @details Order of points does not matter. Points outside of display will be rounded to the closest edge
 *
 * @param x1
 * @param y1
 * @param x2
 * @param y2
 * @param c: color of line
 * @return
 */
void graphics_line(int x1, int y1, int x2, int y2, struct graphics_color c);
/**
 * @brief Draws a curve with given radius and range
 * @details This draws a curve from start_t to end_t with
 * a given radius. The curve will be circular as opposed to elliptical.
 * Angles should be given on a unit circle ranging from 0 to 2PI
 *
 * @param x The x coordinate of the center of the arc
 * @param y The y coordinate of the center of the arc
 * @param r The radius of the arc
 * @param start_theta The starting angle of the arc, in radians
 * @param end_theta The ending angle of the arc, in radians
 * @param graphics_color The color to draw in
 */
void graphics_arc(int x, int y, double r, double start_theta, double end_theta,
                  struct graphics_color c);

/**
 * @brief Draws a circle centered at given point
 * @details Draws an arc from 0 to 2PI centered at x, y in the given color
 * with a radius of r
 *
 * @param x The x coordinate of the center
 * @param y The y coordinate of the center
 * @param r The radius of the circle
 * @param graphics_color The color to draw in
 */
void graphics_circle(int x, int y, double r, struct graphics_color c);

/**
 * @brief Draw the mouse
 * @details This reads the mouse_x and mouse_y variables and draws a point on the screen that corresponds to that point.
 */
void graphics_mouse();

void graphics_rect(int x, int y, int w, int h, struct graphics_color c);
void graphics_clear(struct graphics_color c);
void graphics_char(int x, int y, char ch, struct graphics_color fgcolor,
                   struct graphics_color bgcolor);
void graphics_bitmap(int x, int y, int width, int height, uint8_t * data,
                     struct graphics_color fgcolor,
                     struct graphics_color bgcolor);

static inline void graphics_copy_from_color_buffer(int x, int y, int width, int height, struct graphics_color *buffer) {
    int i, j;
    int buf_ix;
    // copy old buffer into video buffer
    for (j = y; i < height; i++) {
        for (i = x; i < width; i++) {
            if (i < 0 || i > video_xres - 1 || j < 0 || j > video_yres - 1) {
                continue;
            }
            uint8_t *v = video_buffer + video_xbytes * j + i * 3;
            v[2] = buffer[buf_ix].r;
            v[1] = buffer[buf_ix].g;
            v[0] = buffer[buf_ix].b;
			buf_ix++;
        }
    }
}

static inline void graphics_copy_from_color_buffer(int x, int y, int width, int height, struct graphics_color *buffer) {
    int i, j;
    int buf_ix;
    // copy video buf into a color buffer
    for (j = mouse_y - MOUSE_SIDE_2; i < MOUSE_SIDE_2; i++) {
        for (j = -MOUSE_SIDE_2 + 1; i < MOUSE_SIDE_2; i++) {
            if (x < 0 || x > video_xres - 1 || y < 0 || y > video_yres - 1) {
                continue;
            }
			buffer[buf_ix].r = ;
			buffer[buf_ix].g = ;
			buffer[buf_ix].b = ;

            uint8_t *v = video_buffer + video_xbytes * j + i * 3;
            v[2] = c.r;
            v[1] = c.g;
            v[0] = c.b;
        }
    }
    uint8_t *v = video_buffer + video_xbytes * y + x * 3;
    v[2] = mouse_fg_color.r;
    v[1] = mouse_fg_color.g;
    v[0] = mouse_fg_color.b;
}
}

/**
 * @brief Draws mouse on the vid buf
 * @brief This draws mouse on the vid buf. This does not check erase the mouse in the sense that it just 
 */
static inline void graphics_draw_mouse() {

}

/**
 * @brief Draws the mouse on the screen
 * @details This copies the old mouse_draw_buffer back into the video buffer and then copies the new mouse region from the video buffer into mouse_draw_buffer. It then draws the mouse (a cross) on the screen. graphics_mouse does not use plot_pixel because of mouse_buffer checking
 */
static inline void graphics_mouse() {
    // copy mouse buffer into video_buf
    graphics_copy_from_color_buffer(old_mouse_x - MOUSE_SIDE_2 + 1, old_mouse_y - MOUSE_SIDE_2 + 1, MOUSE_SIDE - 1, MOUSE_SIDE - 1, mouse_draw_buffer);
    // copy mouse region of video buf into mouse_buffer
}

#endif
