/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "pagetable.h"
#include "memory_raw.h"
#include "string.h"
#include "kerneltypes.h"    // uint32_t
#include "kernelcore.h"     // halt
#include "console.h"        // console_printf
#include "process.h"        // current, process_dump, process_exit
#include "interrupt.h"      // interrupt_dump_process

#define ENTRIES_PER_TABLE (PAGE_SIZE/4)

struct pageentry {
    unsigned present:1;         // 1 = present
    unsigned readwrite:1;       // 1 = writable
    unsigned user:1;            // 1 = user mode
    unsigned writethrough:1;    // 1 = write through

    unsigned nocache:1;         // 1 = no caching
    unsigned accessed:1;        // 1 = accessed
    unsigned dirty:1;           // 1 = dirty
    unsigned pagesize:1;        // leave to zero

    unsigned globalpage:1;      // 1 if not to be flushed
    unsigned avail:3;

    unsigned addr:20;
};  // 32 bits = 4 bytes

struct pagetable {
    struct pageentry entry[ENTRIES_PER_TABLE];
};

struct pagetable *pagetable_create() {
    // Each pagetable is one page (4096 bytes) with 2^10 entries
    // Each process has a 4 MB large pagetable
    return (struct pagetable *)memory_alloc_page(1);
}

void pagetable_init(struct pagetable *p) {
    uint32_t lower_half_memory_top = (2048 - 1) * 1024 * 1024;
    uint32_t stop = total_memory * 1024 * 1024;
    if (stop > lower_half_memory_top) {
        // Cap stop at the 2GB boundary, to avoid direct-mapping the higher
        // user space memory
        stop = lower_half_memory_top;
    }

    uint32_t i;
    for (i = 0; i < stop; i += PAGE_SIZE) {
        pagetable_map(p, i, i, PAGE_FLAG_KERNEL | PAGE_FLAG_READWRITE);
    }

    // SL: in VirtualBox, vram is separate from ram, and no matter how much
    // physical memory it has, video_buffer is always 0xe0000000.
    // TODO (SL): [NUN-15] Ensure video buffer is mapped into superviser mode
    // without vram present
    stop = (uint32_t)video_buffer + video_xres * video_yres * 3;
    for (i = (uint32_t)video_buffer; i <= stop; i += PAGE_SIZE) {
        pagetable_map(p, i, i, PAGE_FLAG_KERNEL | PAGE_FLAG_READWRITE);
    }
}

int pagetable_getmap(struct pagetable *p, unsigned vaddr, unsigned *paddr) {
    struct pagetable *q;
    struct pageentry *e;

    unsigned a = vaddr >> 22;           // page directory index: top 10 bits
    unsigned b = (vaddr >> 12) & 0x3ff; // page table index: middle 10 bits

    e = &p->entry[a];                   // e: page table in directory
    if (!e->present) {
        return 0;
    }

    q = (struct pagetable *)(e->addr << 12);    // q: page table address

    e = &q->entry[b];                   // e: physical page address
    if (!e->present) {
        return 0;
    }

    *paddr = e->addr << 12;

    return 1;
}

int pagetable_map(struct pagetable *p, unsigned vaddr, unsigned paddr,
                  int flags) {
    struct pagetable *q;
    struct pageentry *e;

    unsigned a = vaddr >> 22;
    unsigned b = (vaddr >> 12) & 0x3ff;

    // If we need to allocate the page, allocate first
    // TODO (SL): if the virtual address is already mapped in the page table,
    // what should we do?
    if (flags & PAGE_FLAG_ALLOC) {
        paddr = (unsigned)memory_alloc_page(0);
        if (!paddr) {
            return 0;
        }
    }

    e = &p->entry[a];

    if (!e->present) {
        // Create page directory entry
        q = pagetable_create();
        if (!q) {
            return 0;
        }
        e->present = 1;
        e->readwrite = 1;
        e->user = (flags & PAGE_FLAG_KERNEL) ? 0 : 1;
        e->writethrough = 0;
        e->nocache = 0;
        e->accessed = 0;
        e->dirty = 0;
        e->pagesize = 0;
        e->globalpage = (flags & PAGE_FLAG_KERNEL) ? 1 : 0;
        e->avail = 0;
        e->addr = (((unsigned)q) >> 12);
    } else {
        q = (struct pagetable *)(((unsigned)e->addr) << 12);
    }

    e = &q->entry[b];

    // Create page table entry
    e->present = 1;
    e->readwrite = (flags & PAGE_FLAG_READWRITE) ? 1 : 0;
    e->user = (flags & PAGE_FLAG_KERNEL) ? 0 : 1;
    e->writethrough = 0;
    e->nocache = 0;
    e->accessed = 0;
    e->dirty = 0;
    e->pagesize = 0;
    e->globalpage = !e->user;
    e->avail = (flags & PAGE_FLAG_ALLOC) ? 1 : 0;
    e->addr = (paddr >> 12);

    return 1;
}

void pagetable_unmap(struct pagetable *p, unsigned vaddr) {
    struct pagetable *q;
    struct pageentry *e;

    unsigned a = vaddr >> 22;
    unsigned b = vaddr >> 12 & 0x3ff;

    e = &p->entry[a];
    if (e->present) {
        q = (struct pagetable *)(e->addr << 12);
        e = &q->entry[b];
        e->present = 0;
    }
}

void pagetable_delete(struct pagetable *p) {
    unsigned i, j;

    struct pageentry *e;
    struct pagetable *q;

    for (i = 0; i < ENTRIES_PER_TABLE; i++) {
        e = &p->entry[i];
        if (e->present) {
            q = (struct pagetable *)(e->addr << 12);
            for (j = 0; j < ENTRIES_PER_TABLE; j++) {
                e = &q->entry[i];
                if (e->present && e->avail) {
                    void *paddr = (void *)(e->addr << 12);
                    memory_free_page(paddr);
                }
            }
            memory_free_page(q);
        }
    }
}

void pagetable_alloc(struct pagetable *p, unsigned vaddr, unsigned length,
                     int flags) {
    unsigned npages = length / PAGE_SIZE;

    if (length % PAGE_SIZE) {
        npages++;
    }

    vaddr &= 0xfffff000;

    while (npages > 0) {
        unsigned paddr;
        if (!pagetable_getmap(p, vaddr, &paddr)) {
            pagetable_map(p, vaddr, 0, flags | PAGE_FLAG_ALLOC);
        }
        vaddr += PAGE_SIZE;
        npages--;
    }
}

struct pagetable *pagetable_load(struct pagetable *p) {
    struct pagetable *oldp;
    asm("mov %%cr3, %0":"=r"(oldp));
    asm("mov %0, %%cr3"::"r"(p));
    return oldp;
}

void pagetable_refresh() {
    asm("mov %cr3, %eax");
    asm("mov %eax, %cr3");
}

void pagetable_enable() {
    asm("movl %cr0, %eax");
    asm("orl $0x80000000, %eax");
    asm("movl %eax, %cr0");
}

void pagetable_copy(struct pagetable *sp, unsigned saddr,
                    struct pagetable *tp, unsigned taddr, unsigned length);

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
        // If we can't allocate additional memory, kill the process
        if (0) {
            interrupt_dump_process();
        }

        // Currently we give it as much memory as we could
        pagetable_alloc(current->pagetable, vaddr, PAGE_SIZE,
            // TODO(SL): figure out if these flags are correct
            PAGE_FLAG_READWRITE | PAGE_FLAG_USER | PAGE_FLAG_ALLOC);
    }
}

