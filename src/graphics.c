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

int graphics_width()
{
	return video_xres;
}

int graphics_height()
{
	return video_yres;
}

static inline void plot_pixel( int x, int y, struct graphics_color c )
{
	uint8_t *v = video_buffer + video_xbytes*y+x*3;
	v[2] = c.r;
	v[1] = c.g;
	v[0] = c.b;
}

void graphics_rect( int x, int y, int w, int h, struct graphics_color c )
{
	int i, j;

	for(j=0;j<h;j++) {
		for(i=0;i<w;i++) {
			plot_pixel(x+i,y+j,c);
		}
	}
}

/** Draw a line on the display
 *
 *  Based on Bresenham's algorithm
 *
 */
void graphics_line(int x1, int y1, int x2, int y2, struct graphics_color c) {
    // check for bounds
    x1 = x1 >= video_xres ? video_xres - 1 : x1;
    y1 = y1 >= video_yres ? video_yres - 1 : y1;
    x2 = x2 >= video_xres ? video_xres - 1 : x2;
    y2 = y2 >= video_yres ? video_yres - 1 : y2;
    x1 = x1 < 0 ? 0 : x1;
    y1 = y1 < 0 ? 0 : y1;
    x2 = x2 < 0 ? 0 : x2;
    y2 = y2 < 0 ? 0 : y2;

    int tmp;
    // flip points if x2 is less than x1
    if(x2 < x1) {
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
    if(deltax == 0) {
        // flip vertical line where y2 < y1
        if(y2 < y1) {
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

        if(dy < 0) {
            sign = -1;
        }
        else {
            sign = 1;
        }

        for(x = x1; x <= x2; x++) {
            plot_pixel(x, y, c);
            error = error + deltaerr;
            while(error >= 0.5) {
                plot_pixel(x, y, c);
                y = y + sign;
                error = error - 1.0;
            }
        }
    }
}

void graphics_clear( struct graphics_color c )
{
	graphics_rect(0,0,video_xres,video_yres,c);
}

void graphics_bitmap( int x, int y, int width, int height, uint8_t *data, struct graphics_color fgcolor, struct graphics_color bgcolor )
{
	int i,j,b;
	int value;

	b=0;

	for(j=0;j<height;j++) {
		for(i=0;i<width;i++) {
			value = ((*data)<<b)&0x80;
			if(value) {
				plot_pixel(x+i,y+j,fgcolor);
			} else {
				plot_pixel(x+i,y+j,bgcolor);
			}
			b++;
			if(b==8) {
				data++;
				b=0;
			}
		}
	}
}

void graphics_char( int x, int y, char ch, struct graphics_color fgcolor, struct graphics_color bgcolor )
{
	int u = ((int)ch)*FONT_WIDTH*FONT_HEIGHT/8;
	return graphics_bitmap(x,y,FONT_WIDTH,FONT_HEIGHT,&fontdata[u],fgcolor,bgcolor);
}
