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

/**
 * @brief whether second channel is enabled
 * @details This function returns (default no) whether the second PS2 channel is enabled.
 * @return boolean on if second channel enabled
 */
int ps2_second_channel_enabled();

/**
 * @brief checks to see if the data port is ready to be read
 * @details This checks the 0th bit of the status register for a 0 (full) to see if the controller data port is ready to be read. There is a timeout of 1s for this function.
 * @return if the controller is ready to be read
 */
int ps2_controller_read_ready();

/**
 * @brief checks to see if the data port is ready to be written to
 * @details This checks the 1st bit of the status register for a 1 (empty) to see if the controller data port is ready to be written to. There is a timeout of 1s for this function.
 * @return if the controller is ready to be written to
 */
int ps2_controller_write_ready();

/**
 * @brief read the controller configuration byte
 * @details This is a wrapper around sending the read command to the command register, waiting for read ready, and reading the byte from the data port.
 * @return controller configuration byte
 */
uint8_t ps2_read_controller_config_byte();

/**
 * @brief write controller configuration byte
 * @details This is a wrapper around command_write with the write controller configuration byte command and the byte itself as arguments.
 * @param cont_config_byte controller configuration byte
 */
void ps2_write_controller_config_byte(uint8_t cont_config_byte);

/**
 * @brief writes a data byte after sending a command
 * @details This handles some commands that have a data byte that is sent afterwards. It sends the command, waits for write ready, and sends the data to the data port.
 * @param command command to send to command register
 * @param data data to send to data port after command sent
 */
void ps2_command_write(uint8_t command, uint8_t data);

#endif

