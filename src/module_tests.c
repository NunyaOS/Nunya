/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "console.h"
#include "pagetable.h"
#include "process.h"

void walk_memory() {
    // allocate some new memory
    // 0x80000000 is the begin of memory space in user mode
    int new_page = pagetable_map(current->pagetable, 0x80000000, 0,
        PAGE_FLAG_READWRITE | PAGE_FLAG_USER | PAGE_FLAG_ALLOC);
    if (new_page) {
        console_printf("[sl] allocation successful\n");
    } else {
        console_printf("[sl] allocation UNSUCCESSFUL\n");
    }

    uint32_t vaddr = 0x80000000 & 0xfffff000;
    console_printf("[sl] start walking memory from vaddr %x\n", vaddr);

    while (1) {
        uint32_t test = *(uint32_t *)vaddr;

        uint32_t paddr;
        pagetable_getmap(current->pagetable, vaddr, &paddr);
        ++test;
        console_printf("[sl] changed memory at %x (physical %x)\n",
            vaddr, paddr);

        vaddr += PAGE_SIZE;
    }
}
