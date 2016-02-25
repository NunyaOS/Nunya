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

int mouse_x;
int mouse_y;
uint8_t mouse_button;

void mouse_init();
int mouse_request_packet();
int mouse_scan();

#endif

