/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef MOUSE_H
#define MOUSE_H

#include "graphics.h"

#define MOUSE_ACK 0xFA
// TODO: to support hotplugging, then every so often, need to query if mouse is alive (send 0xEB and recv and ack)
extern struct graphics_color mouse_fg_color;

/**
 * @brief Get current mouse x position
 * @return mouse x position
 */
int get_mouse_x();

/**
 * @brief Get current mouse y position
 * @return mouse y position
 */
int get_mouse_y();

/**
 * @brief Intializes PS2 mosue
 * @details This sends appropriate commands and configurations for a standard functioning PS2 mouse.  It initializes mouse pointer location and interrupt handler for mouse interrupts.
 */
void mouse_init();

/**
 * @brief Request a packet from the mouse
 * @details This function is for polling mouse to see if it is still alive (hot plugging). It checks for the appropritae ACK as well.
 * @return byte sent by mouse
 */
int mouse_request_packet();

/**
 * @brief Poll mouse for bytes
 * @details This is for polling the mouse for movement bytes (as opposed to interrupts). It waits
 * for the byte (until a timeout) to read from the PS2 data port.
 * @return mouse data
 */
int mouse_scan();

#endif

