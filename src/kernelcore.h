/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef KERNELCORE_H
#define KERNELCORE_H

#include "kerneltypes.h"

extern uint16_t video_xbytes;
extern uint16_t video_xres;
extern uint16_t video_yres;
extern uint8_t *video_buffer;

/**
 * @brief   The total amount of memory above 1MB
 * @details The amount of memory above 1MB. E.g. if the system has 256MB of
 *          memory, total_memory will be 255.
 */
extern uint16_t total_memory;
extern uint32_t kernel_size;

/**
 * @brief   Halt the machine
 */
extern void halt();

/**
 * @brief   Reboot the machine
 */
extern void reboot();

extern void intr_return();

extern void *interrupt_stack_pointer;

#endif
