/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef PS2_H
#define PS2_H

#define PS2_DATA_PORT 0x60
#define PS2_STATUS_REGISTER 0x64
#define PS2_COMMAND_REGISTER 0x64
#define PS2_TIMEOUT 1

#include "ioports.h"

void ps2_init();
int ps2_second_channel_enabled();
int ps2_controller_read_ready();
int ps2_controller_write_ready();
uint8_t ps2_read_controller_config_byte();
void ps2_write_controller_config_byte(uint8_t cont_config_byte);
void ps2_command_write(uint8_t command, uint8_t data);

#endif

