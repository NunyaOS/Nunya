/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "kerneltypes.h"

struct graphics_color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

int graphics_width();
int graphics_height();

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
void graphics_rect(int x, int y, int w, int h, struct graphics_color c);
void graphics_clear(struct graphics_color c);
void graphics_char(int x, int y, char ch, struct graphics_color fgcolor,
                   struct graphics_color bgcolor);
void graphics_bitmap(int x, int y, int width, int height, uint8_t * data,
                     struct graphics_color fgcolor,
                     struct graphics_color bgcolor);

#endif
