/*
Copyright (C) 2016 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef WINDOW_H
#define WINDOW_H

#include "graphics.h"
#include "list.h"

struct window {
    struct window *parent;
    int x;
    int y;
    int width;
    int height;
    struct graphics_color border_color;
    struct graphics_color background_color;
    struct list event_queue;
};

/**
 * @brief Creates a window for given dimens
 * @details Creates a window with the given location and size
 *
 * @param x The x position of the window, relative to the screen
 * @param y The y position of the window, relative to the screen
 * @param width The width of the window in pixels
 * @param height The height of the window in pixels
 * @param parent The parent window, or NULL if none
 * @return A pointer to the window
 */
struct window *window_create(int x, int y, int width, int height, struct window *parent);

/**
 * @brief Sets the border color of the window
 * @details Sets the border color of the window and re-draws with the new color
 * @param window The window whose border color to change
 * @param graphics_color The new border color
 */
void window_set_border_color(struct window *w, struct graphics_color border_color);
/**
 * @brief Draws a line in the window
 * @details Draws a line at given position in the window
 * It will be clipped if it extends outside the window
 * All positions are realtive to the window
 * @param w The window to draw in
 * @param x1 The x position of the first point in the line
 * @param y1 The y position of the first point in the line
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
 * @param w The window to draw in
 * @param x The x position of the center of the arc
 * @param y The y position of the center of the arc
 * @param r The radius of the arc
 * @param start_theta The starting angle of the arc, between 0 and 2*PI. See sin/cos
 * @param end_theta The ending angle of the arc, between 0 and 2*PI. See sin/cos
 * @param color The color to draw in
 */
void window_draw_arc(struct window *w, int x, int y, double r, double start_theta, double end_theta, struct graphics_color c);

/**
 * @brief Draws a circle in the window
 * @details Draws a circle of given size and location in the window.
 * Curves outside the windows bounds will be clipped. All locations
 * relative to the window
 *
 * @param w The window to draw in
 * @param x The x position of the center of the circle, relative to the window
 * @param y The y position of the center of the circle, relative to the window
 * @param r The radius of the circle
 * @param color The color to draw in
 */
void window_draw_circle(struct window *w, int x, int y, double r, struct graphics_color color);

/**
 * @brief Draws a character in the window
 * @details Draws given character data in the window. Draws outside the bounds
 * of the window will be clipped. All locations relative to the window
 *
 * @param w The window to draw in
 * @param x The x position of the top left corner of the char
 * @param y The y position of the top left corner of the char
 * @param ch The character to draw
 * @param fgcolor The color to draw the character
 * @param bgcolor The color to draw the negative space
 */
void window_draw_char(struct window *w, int x, int y, char ch, struct graphics_color fgcolor,
                      struct graphics_color bgcolor);

/**
 * @brief Draws a string in the window
 * @details Draws the given string in the window. If the string extends beyond
 * the bounds of the window, it will be clipped.
 *
 * @param window The window to draw in
 * @param x The x position of the top left corner of the first character
 * @param y The y position of the top right corner of the first character
 * @param str The text to be drawn, null terminated
 * @param graphics_color The color to draw the characters
 * @param graphics_color The color to draw the negative space
 */
void window_draw_string(struct window *w, int x, int y, const char *str, struct graphics_color fgcolor,
                        struct graphics_color bgcolor);

/**
 * @brief Draws data to window
 * @details Draws the given graphics data to the screen within the given window
 * Drawing outside the bounds of the window will result in clipping. All locations
 * relative to the window
 *
 * @param w The window to draw in
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

/**
 * @brief Tests drawing in nested windows
 * @details This creates nested windows and draws to them, to ensure
 * that child windows are being correctly clipped
 */
void window_hierarchy_test();
#endif