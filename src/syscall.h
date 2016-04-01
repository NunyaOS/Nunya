/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef SYSCALL_H
#define SYSCALL_H

#include "kerneltypes.h"
#include "graphics.h"

#define SYSCALL_exit     1
#define SYSCALL_testcall 2
#define SYSCALL_yield    3

#define SYSCALL_window_create 200
#define SYSCALL_window_set_border_color 201
#define SYSCALL_window_draw_line 202
#define SYSCALL_window_draw_arc 203
#define SYSCALL_window_draw_circle 204
#define SYSCALL_window_draw_char 205
#define SYSCALL_window_draw_string 206

uint32_t syscall(uint32_t n, uint32_t a, uint32_t b, uint32_t c, uint32_t d,
                 uint32_t e);

/**
 * @brief A struct to hold information needed to draw an arc
 * @details This struct holds data that can be passed to draw_arc
 * to indicate the radius and start/end angles. This information
 * is collected here as alone, draw_arc would need more parameters
 * than are available to system calls
 */
struct arc_info {
	double r;
	double start_theta;
	double end_theta;
};

static inline int32_t exit(uint32_t status) {
    return syscall(SYSCALL_exit, status, 0, 0, 0, 0);
}

static inline int32_t testcall(int x) {
    return syscall(SYSCALL_testcall, x, 0, 0, 0, 0);
}

static inline int32_t yield() {
    return syscall(SYSCALL_yield, 0, 0, 0, 0, 0);
}

// Window System Calls

/**
 * @brief Creates a window for the process
 * @details Invokes the window_create function with given parameters and assigns
 * the resulting window to calling process
 * 
 * @param x The x position of the window, relative to its parent
 * @param y The y position of the window, relative to its parent
 * @param width The width of the window
 * @param height The height of the window
 * @return 0 if a window was created, otherwise error
 */
static inline int32_t create_window(int x, int y, uint32_t width, uint32_t height) {
	return syscall(SYSCALL_window_create, x, y, width, height, 0);
}

/**
 * @brief Sets the border color of the process' window
 * @details Invokes the window_set_border_color function to change window color
 * 
 * @param graphics_color The color to set the border to
 * @return 0 on success, or -1 if the calling process has no window
 */
static inline int32_t set_border_color(const struct graphics_color *border_color) {
	return syscall(SYSCALL_window_set_border_color, (int)border_color, 0, 0, 0, 0);
}

/**
 * @brief Draws a line in the current process' window
 * @details Invokes the window_draw_line function to draw into the current window
 * 
 * @param x1 The x position of the first point in the line
 * @param y1 The y position of the first point in the line
 * @param x2 The x position of the second point in the line
 * @param y2 The y position of the second point in the line
 * @param color The color of the line
 * @return 0 on success, or -1 if the calling process has no window
 */
static inline int32_t draw_line(int x1, int y1, int x2, int y2, const struct graphics_color *color) {
	return syscall(SYSCALL_window_draw_line, x1, y1, x2, y2, (int)color);
}

/**
 * @brief Draws an arc in the current process' window
 * @details Invokes the window_draw_arc function to draw into the current window
 * 
 * @param x The x position of the center of the arc
 * @param y The y position of the center of the arc
 * @param r The radius of the arc
 * @param start_theta The starting angle of the arc, between 0 and 2*PI. See sin/cos
 * @param end_theta The ending angle of the arc, between 0 and 2*PI. See sin/cos
 * @param color The color to draw in
 * @return 0 on success, or -1 if the calling process has no window
 */
static inline int32_t draw_arc(int x, int y, const struct arc_info *arc, const struct graphics_color *c) {
	return syscall(SYSCALL_window_draw_arc, x, y, (int)arc, (int)c, 0);
}

/**
 * @brief Draws a circle in the current process' window
 * @details Invokes the window_draw_circle function to draw into the current window
 * 
 * @param x The x position of the center of the circle, relative to the window
 * @param y The y position of the center of the circle, relative to the window
 * @param r The radius of the circle
 * @param color The color to draw in
 * @return 0 on success, or -1 if the calling process has no window
 */
static inline int32_t draw_circle(int x, int y, const double *r, const struct graphics_color *c) {
	return syscall(SYSCALL_window_draw_circle, x, y, (int)r, (int)c, 0);
}

/**
 * @brief Draws a character in the current process' window
 * @details Invokes the window_draw_char function to draw into the current window
 * 
 * @param x The x position of the top left corner of the char
 * @param y The y position of the top left corner of the char
 * @param ch The character to draw
 * @param fgcolor The color to draw the character
 * @param bgcolor The color to draw the negative space
 * @return 0 on success, or -1 if the calling process has no window
 */
static inline int32_t draw_char(int x, int y, char c, const struct graphics_color *fgcolor, const struct graphics_color *bgcolor) {
	return syscall(SYSCALL_window_draw_char, x, y, c, (int)fgcolor, (int)bgcolor);
}

/**
 * @brief Draws a string in the current process' window
 * @details Invokes the window_draw_string function to draw into the current window
 * 
 * @param x The x position of the top left corner of the first character
 * @param y The y position of the top right corner of the first character
 * @param str The text to be drawn, null terminated
 * @param graphics_color The color to draw the characters
 * @param graphics_color The color to draw the negative space
 * @return 0 on success, or -1 if the calling process has no window
 */
static inline int32_t draw_string(int x, int y, const char *str, const struct graphics_color *fgcolor,
	const struct graphics_color *bgcolor) {
	return syscall(SYSCALL_window_draw_string, x, y, (int)str, (int)fgcolor, (int)bgcolor);
}

#endif