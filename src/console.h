/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef CONSOLE_H
#define CONSOLE_H

#include "kerneltypes.h"
#include "string.h"

void console_init();
void console_putchar(char c);
int console_write(int unit, const void *buffer, int nblocks, int offset);
void console_heartbeat();

/**
* @brief sets the consoles fg color to given RGB
* @details The purpose of this function is to change the color of text displayed on the screen shown when booting to an RGB tuple.
* @param r The value for red between 0, 255 inclusive
* @param g The value for green between 0, 255 inclusive
* @param b The value for blue between 0, 255 inclusive
* @return void
*/
void console_set_fgcolor(uint8_t r, uint8_t g, uint8_t b);

/**
* @brief sets the consoles bg color to given RGB
* @details The purpose of this function is to change the color of highlighting behind text displayed on the screen shown when booting to an RGB tuple.
* @param r The value for red between 0, 255 inclusive
* @param g The value for green between 0, 255 inclusive
* @param b The value for blue between 0, 255 inclusive
* @return void
*/
void console_set_bgcolor(uint8_t r, uint8_t g, uint8_t b);

#define console_printf printf

#endif
