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
void console_putchar( char c );
int  console_write( int unit, const void *buffer, int nblocks, int offset );
void console_heartbeat();

/*! console_set_fgcolor
* The purpose of this function is to change the color of text
* displayed on the screen shown when booting to an RGB tuple.
* \arg \c r value for red
* \arg \c g - value for green
* \arg \c b - value for blue
* return type \c void
*/
void console_set_fgcolor(uint8_t r, uint8_t g, uint8_t b);


/*! console_set_bgcolor
* The purpose of this function is to change the color of the highlighted text
* displayed on the screen shown when booting to an RGB tuple.
* \arg \c r value for red
* \arg \c g - value for green
* \arg \c b - value for blue
* return type \c void
*/
void console_set_bgcolor(uint8_t r, uint8_t g, uint8_t b);

#define console_printf printf

#endif
