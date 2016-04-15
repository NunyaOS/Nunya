/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef CLOCK_H
#define CLOCK_H

#include "kerneltypes.h"

typedef struct {
    uint32_t seconds;
    uint32_t millis;
} clock_t;

void clock_init();
clock_t clock_read();
clock_t clock_diff(clock_t start, clock_t stop);
void clock_wait(uint32_t millis);

/**
 * @brief Compares two times
 * @details This is a comparison function to the relation between two times
 *
 * @param a The first time
 * @param b The second time
 *
 * @return Returns -1 if a is smaller, 0 if they are the same, and 1 if a is bigger
 */
int clock_compare(clock_t a, clock_t b);

#endif
