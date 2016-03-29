/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "memory_raw.h"
#include "process.h"
#include "console.h"
#include "kerneltypes.h"
#include "string.h"
#include "memorylayout.h"
#include "kernelcore.h"
#include "pagetable.h"

static uint32_t pages_free = 0;
static uint32_t pages_total = 0;

static uint32_t *freemap = 0;
static uint32_t freemap_bits = 0;
static uint32_t freemap_bytes = 0;
static uint32_t freemap_cells = 0;
static uint32_t freemap_pages = 0;

static void *alloc_memory_start = (void *)ALLOC_MEMORY_START;

#define CELL_BITS (8*sizeof(*freemap))

// Translate between physical address and memory page number
static void addr_from_cell_num_offset(uint32_t *addr, int cell_num, int offset);
static void cell_num_offset_from_addr(uint32_t addr, int *cell_num, int *offset);

// Detect memory map; set freemap accordingly
static void memory_detect_map();
static int memory_is_range_type_available(int type);

void memory_init() {
    int i;

    memory_detect_map();

    pages_total = (total_memory * 1024) / (PAGE_SIZE / 1024);
    pages_free = pages_total;
    console_printf("memory: %d MB (%d KB) total\n",
                   (pages_free * PAGE_SIZE) / MEGA,
                   (pages_free * PAGE_SIZE) / KILO);

    // Freemap is organized as follows
    // Each page is represented as one bit
    // Each cell in the freemap is 32 bits, representing 32 pages
    freemap = alloc_memory_start;
    freemap_bits = pages_total;
    freemap_bytes = 1 + freemap_bits / 8;
    freemap_cells = 1 + freemap_bits / CELL_BITS;
    freemap_pages = 1 + freemap_bytes / PAGE_SIZE;

    console_printf("memory: %d bits %d bytes %d cells %d pages\n",
                   freemap_bits, freemap_bytes, freemap_cells, freemap_pages);

    memset(freemap, 0xff, freemap_bytes);
    for (i = 0; i < freemap_pages; i++) {
        memory_alloc_page(0);
    }

    // This is ahack that I don't understand yet.
    // vmware doesn't like the use of a particular page
    // close to 1MB, but what it is used for I don't know.

    freemap[0] = 0x0;

    // VirtualBox doesn't like memory address 0x1A0000 through 0x1C0000
    // so block it off
    freemap[5] = 0x0;

    console_printf("memory: %d MB (%d KB) available\n",
                   (pages_free * PAGE_SIZE) / MEGA,
                   (pages_free * PAGE_SIZE) / KILO);
}

uint32_t memory_pages_free() {
    return pages_free;
}

uint32_t memory_pages_total() {
    return pages_total;
}

void *memory_alloc_page(bool zeroit) {
    uint32_t i, j;
    uint32_t cellmask;
    void *pageaddr;

    if (!freemap) {
        console_printf("memory: not initialized yet!\n");
        return 0;
    }

    for (i = 0; i < freemap_cells; i++) {
        if (freemap[i] == 0) {
            // pages represented in the cell are fully allocated
            continue;
        }

        for (j = 0; j < CELL_BITS; j++) {
            cellmask = (1 << j);
            if (freemap[i] & cellmask) {
                freemap[i] &= ~cellmask;
                addr_from_cell_num_offset((uint32_t *)&pageaddr, i, j);
                if (zeroit) {
                    memset(pageaddr, 0, PAGE_SIZE);
                }
                pages_free--;
                return pageaddr;
            }
        }
    }

    console_printf("memory: WARNING: everything allocated\n");
    halt();

    return 0;
}

void memory_free_page(void *pageaddr) {
    uint32_t pagenumber = (pageaddr - alloc_memory_start) >> PAGE_BITS;
    uint32_t cellnumber = pagenumber / CELL_BITS;
    uint32_t celloffset = pagenumber % CELL_BITS;
    uint32_t cellmask = (1 << celloffset);
    freemap[cellnumber] |= cellmask;
    pages_free++;
}

static void addr_from_cell_num_offset(uint32_t *addr, int cell_num, int offset) {
    int pagenumber = cell_num * CELL_BITS + offset;
    *addr = (pagenumber << PAGE_BITS) + (uint32_t)alloc_memory_start;
}

static void cell_num_offset_from_addr(uint32_t addr, int *cell_num, int *offset) {
    addr &= 0xfffff000;     // align address to page boundary
    int pagenumber = (addr - (uint32_t)alloc_memory_start) >> PAGE_BITS;
    *cell_num = pagenumber / CELL_BITS;
    *offset = pagenumber % CELL_BITS;
}

static void memory_detect_map() {
    int i;
    for (i = 0; i < mem_descriptor_arr_max_length; ++i) {
        if ((mem_descriptor[i].length_high
            | mem_descriptor[i].length_low) == 0) {
            // skip empty entries
            continue;
        }

        // modify freemap if necessary
        int range_length = mem_descriptor[i].length_low;
        int type = mem_descriptor[i].address_range_type;
        if (!memory_is_range_type_available(type)) {
            // if the range is reserved, remove all pages from freemap
            int length = 0;
            uint32_t range_base_addr = mem_descriptor[i].base_address_low;

            // instead of starting in the middle of a page (which may free too
            // few pages), we move the base_addr to start from the beginning of
            // a page
            range_length += range_base_addr & 0x00000fff;
            range_base_addr &= 0xfffff000;

            while (length < range_length) {
                uint32_t addr = range_base_addr + length;
                int cell_num = 0;
                int page_offset = 0;
                cell_num_offset_from_addr(addr, &cell_num, &page_offset);

                // remove from freemap
                uint32_t cellmask = 1 << page_offset;
                if (freemap[cell_num] & cellmask) {
                    freemap[cell_num] &= ~cellmask;
                }

                // check next page
                length += PAGE_SIZE;
            }
        }
    } // for each mem_descriptor in mem_descriptor_arr
}

static int memory_is_range_type_available(int type) {
    switch (type) {
    case 1:     // AddressRangeMemory
    case 3:     // AddressRangeACPI
        return 1;
    case 2:     // AddressRangeReserved
    case 4:     // AddressRangeNVS
    default:    // AddressRangeOtherUnvailable
        return 0;
    }
}
