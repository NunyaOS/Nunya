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

extern uint16_t total_memory;
extern uint32_t kernel_size;

struct address_range_descriptor {
    uint32_t base_address_low;
    uint32_t base_address_high;
    uint32_t length_low;
    uint32_t length_high;
    uint32_t address_range_type;
    uint32_t acpi_ext_attr;
};
extern uint32_t mem_descriptor_arr_max_length;
// extern uint32_t mem_descriptor_arr_length;
extern struct address_range_descriptor mem_descriptor[20];

extern void halt();
extern void reboot();

extern void intr_return();

extern void *interrupt_stack_pointer;

#endif
