/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "debug_kernel.h"
#include "console.h"

void debug_dump_mem_from_addr(uint32_t *addr, int len) {
#ifdef NUNYA_KDEBUG
    int i;

    console_printf("[DEBUG] Dumping stack starting from %x\n", addr);
    console_printf("  addr   |   value\n");

    if (len < 0) {
        len = -len;
        addr = addr - len + 1;
    }

    for (i = 0; i < len; ++i) {
        printf("%x | %x\n", (addr + i), *(addr + i));
    }
#endif
}
