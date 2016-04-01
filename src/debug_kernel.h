/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef DEBUG_KERNEL_H
#define DEBUG_KERNEL_H

#include "kerneltypes.h"

// This is a set of functions we can call in kernel mode to facilitate
// debugging in kernel mode. To enable them, uncomment the following definition:

enum register_name_t {
    reg_esi = 0,
    reg_edi,
    reg_ebp,
    reg_esp
};

/**
 * @brief   Dump memory content of a given length at a given address
 * @detail  Dump memory content in hex, starting or ending at a given address
 *          depending on the given length.
 *
 * @param   addr    The start or end address of memory to be dumped
 * @param   len     The length of memory to be dumped in 4 bytes (32-bit
 *                  integers). If len is positive, then we dump memory contents
 *                  starting from addr and reach (addr + len); if len is
 *                  negative, then we dump memory contents starting from (addr
 *                  - |len|) and reach addr
 */
void debug_dump_mem_from_addr(uint32_t *addr, int len);

#endif
