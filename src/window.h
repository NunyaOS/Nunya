/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef WINDOW_H
#define WINDOW_H

#include "graphics.h"

struct window {
    int x;
    int y;
    int width;
    int height;
    struct graphics_color border_color;
    struct graphics_color background_color;
};

/**
 * @brief Creates a window for given dimens
 * @details Creates a window with the given location and size
 *
 * @param x The x position of the window, relative to the screen
 * @param y The y position of the window, relative to the screen
 * @param width The width of the window in pixels
 * @param height The height of the window in pixels
 * @return A pointer to the window
 */
struct window *window_create(int x, int y, int width, int height);

/**
 * @brief Draws a line in the window
 * @details Draws a line at given position in the window
 * It will be clipped if it extends outside the window
 * All positions are realtive to the window
 * @param window The window to draw in
 * @param x1 The x position of the first point in the line
 * @param y1 The position of the first point in the line
 * @param x2 The x position of the second point in the line
 * @param y2 The y position of the second point in the line
 * @param color The color of the line
 */
void window_draw_line(struct window *w, int x1, int y1, int x2, int y2, struct graphics_color color);

/**
 * @brief Draws an arc in the window
 * @details Draws an arc with given size and center inside the window
 * Curves existing outside the window bounds will be clipped. All points
 * relative to the window
 *
 * @param window The window to draw in
 * @param x The x position of the center of the arc
 * @param y The y position of the center of the arc
 * @param r The radius of the arc
 * @param start_theta The starting angle of the arc
 * @param end_theta The ending angle of the arc
 * @param color The color to draw in
 */
void window_draw_arc(struct window *w, int x, int y, double r, double start_theta, double end_theta, struct graphics_color c);

/**
 * @brief Draws a circle in the window
 * @details Draws a circle of given size and location in the window.
 * Curves outside the windows bounds will be clipped. All locations
 * relative to the window
 *
 * @param window The window to draw in
 * @param x The x position of the center of the window
 * @param y The y position of the center of the window
 * @param r The radius of the circle
 * @param color The color to draw in
 */
void window_draw_circle(struct window *w, int x, int y, double r, struct graphics_color color);

/**
 * @brief Draws a character in the window
 * @details Draws given character data in the window. Draws outside the bounds
 * of the window will be clipped. All locations relative to the window
 *
 * @param window The window to draw in
 * @param x The x position of the top left corner of the char
 * @param y The y position of the top left corner of the char
 * @param ch The character to draw
 * @param fgcolor The color to draw the character
 * @param bgcolor The color to draw the negative space
 */
void window_draw_char(struct window *w, int x, int y, char ch, struct graphics_color fgcolor,
                      struct graphics_color bgcolor);

/**
 * @brief Draws data to window
 * @details Draws the given graphics data to the screen within the given window
 * Dawing outside the bounds of the window will result in clipping. All locations
 * relative to the window
 *
 * @param window The window to draw in
 * @param x The x position of the top left corner
 * @param y The y position of the top left corner
 * @param width The width of the data to draw
 * @param height The height of the data to draw
 * @param data The data to draw
 * @param fgcolor The color to draw in
 * @param bgcolor The color of negative space
 */
void window_draw_bitmap(struct window *w, int x, int y, int width, int height, uint8_t * data,
                     struct graphics_color fgcolor,
                     struct graphics_color bgcolor); 
#endif