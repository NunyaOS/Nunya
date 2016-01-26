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
 * @brief Draws a line on the display from (x1, y1) to (x2, y2)
 * 
 * @details Order of points does not matter. Points outside of display will be rounded to the closest edge
 * 
 * @param x1
 * @param y1
 * @param x2
 * @param y2
 * @param c: color of line
 * 
 * @return
 */
void graphics_line( int x1, int y1, int x2, int y2, struct graphics_color c );
void graphics_rect( int x, int y, int w, int h, struct graphics_color c );
void graphics_clear( struct graphics_color c );
void graphics_char( int x, int y, char ch, struct graphics_color fgcolor, struct graphics_color bgcolor );
void graphics_bitmap( int x, int y, int width, int height, uint8_t *data, struct graphics_color fgcolor, struct graphics_color bgcolor );

#endif

