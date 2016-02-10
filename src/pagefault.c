/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "kerneltypes.h"    // uint32_t
#include "pagetable.h"      // pagetable_getmap, pagetable_map, PAGE_SIZE
#include "kernelcore.h"     // halt
#include "console.h"        // console_printf
#include "process.h"        // current, process_dump, process_exit
#include "interrupt.h"      // interrupt_dump_process

void exception_handle_pagefault(int intr, int code) {
    // vector index should be 14; otherwise something really wrong happened
    if (intr != 14) {
        console_printf("interrupt: incorrect exception handler called\n");
        console_printf("expect vector 14; received vector %d\n", intr);
        halt();
    }

    uint32_t vaddr, paddr;
    asm("mov %%cr2, %0":"=r"(vaddr));
    // When a page fault exception is thrown, test if the vaddr is mapped
    if (pagetable_getmap(current->pagetable, vaddr, &paddr)) {
        // If it's mapped, then it means we can't access the vaddr.
        // Dump the process
        console_printf("interrupt: illegal page access at vaddr %x\n",
                       vaddr);
        process_dump(current);
        process_exit(0);
    } else {
        // Otherwise, we know we have a legit page fault
        // printf("interrupt: page fault at %x\n", vaddr);

        // Currently we give it as much memory as we could
        pagetable_alloc(current->pagetable, vaddr, PAGE_SIZE,
            // TODO(SL): figure out if these flags are correct
            PAGE_FLAG_READWRITE | PAGE_FLAG_USER | PAGE_FLAG_ALLOC);

        // If we can't allocate additional memory, kill the process
        if (0) {
            interrupt_dump_process();
        }
    }
}
