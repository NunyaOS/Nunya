/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef MOUSE_H
#define MOUSE_H

#define MOUSE_ACK 0xFA
// TODO: to support hotplugging, then every so often, need to query if mouse if alive (send 0xEB and recv and ack)

// used to check if data on port 0x60 is for the mouse or the keyboard
void mouse_init();
void mouse_init_2();
int mouse_scan();

#endif

