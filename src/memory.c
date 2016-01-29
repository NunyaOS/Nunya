/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "process.h"
#include "console.h"
#include "kerneltypes.h"
#include "memory.h"
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

// globals necessary for kmalloc
struct kmalloc_page_info *kmalloc_head = 0;
unsigned kmalloc_next_vaddr = ALLOC_MEMORY_START;

#define CELL_BITS (8*sizeof(*freemap))

void memory_init() {
    int i;

    pages_total = (total_memory * 1024) / (PAGE_SIZE / 1024);
    pages_free = pages_total;
    console_printf("memory: %d MB (%d KB) total\n",
                   (pages_free * PAGE_SIZE) / MEGA,
                   (pages_free * PAGE_SIZE) / KILO);

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
    uint32_t pagenumber;
    void *pageaddr;

    if (!freemap) {
        console_printf("memory: not initialized yet!\n");
        return 0;
    }

    for (i = 0; i < freemap_cells; i++) {
        if (freemap[i] != 0) {
            for (j = 0; j < CELL_BITS; j++) {
                cellmask = (1 << j);
                if (freemap[i] & cellmask) {
                    freemap[i] &= ~cellmask;
                    pagenumber = i * CELL_BITS + j;
                    pageaddr = (pagenumber << PAGE_BITS) + alloc_memory_start;
                    if (zeroit) {
                        memset(pageaddr, 0, PAGE_SIZE);
                    }
                    pages_free--;
                    return pageaddr;
                }
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

struct kmalloc_page_info *kmalloc_create_page_info(unsigned paddr) {
    struct kmalloc_page_info *n = (struct kmalloc_page_info *)paddr;
    n->max_free_gap = KMALLOC_NUM_SLOTS;
    memset(n->free, 1, KMALLOC_SLOT_SIZE * KMALLOC_NUM_SLOTS);
    return n;
}

/**
* @brief Put a new page at the head of the kmalloc linked list of pages in use.
* @details When kmalloc does not have sufficient space on any of the allocated pages on its linked list, this function helps it by using a new page from the pagetable, putting a struct kmalloc_page_info at the start of it, and inserting said page at the beginning of the linked list of pages.
*/
void kmalloc_get_page() {
    //get current process's pagetable
    struct pagetable *pt = current->pagetable;
    unsigned phys_addr;

    if (kmalloc_next_vaddr >= PROCESS_ENTRY_POINT) {
        console_printf("kmalloc is out of space... uh oh\n");
        //TODO: figure out a system interrupt of some sort here
        return;
    }
 
    if (pagetable_getmap(pt, kmalloc_next_vaddr, &phys_addr) != 1) {
        console_printf("kmalloc: Unable to get mapping to next vmem page\n");
        return;
    }

    //prepare for the next page requested by updating the global variable for it.
    kmalloc_next_vaddr += PAGE_SIZE;

    // build the page struct
    struct kmalloc_page_info *pg_info = kmalloc_create_page_info(phys_addr);

    //rearrange the linked list of pages with the new page at the beginning.
    pg_info->next = kmalloc_head;
    kmalloc_head = pg_info;
}

/**
* @brief Find the slot index at the start of a sufficiently large gap in memory for kmalloc
* @details Examines the free vector of the pointer to page_info and gives the index of the first slot of the first sufficiently large gap in the page.
*
* @param page_info Pointer to the kmalloc_page_info which maintains the details regarding the page we're looking for the largest gap in.
* @param num_slots Assumedly positive, integer number of required contiguous free slots
* @return The index of a free slot on the page that begins a string of at least num_slots consecutive free slots
*/
int kmalloc_locate_sufficient_gap(struct kmalloc_page_info *page_info, int num_slots) {
    int gaps_first_slot = 0;
    int consecutive_safe = 0;

    int i;
    for (i = 0; i < KMALLOC_NUM_SLOTS; i++) {
        if (page_info->free[i] == 1) {
            consecutive_safe++;
            if (consecutive_safe == num_slots) {
                return gaps_first_slot;
            }
        } else {
            consecutive_safe = 0;
            gaps_first_slot = i + 1;
        }
    }
    return -1; 
}

/**
* @brief Returns the largest gap in terms of the number of slots in that gap.
* @details Looks through the arugments free array, looking for the longest consecutive string of free slots
*
* @param page_info Pointer to the kmalloc_page_info which maintains the details regarding the page we're looking for the largest gap in.
* @return The number of slots in the largest continuous, free gap on the page.
*/
int kmalloc_get_largest_gap_size(struct kmalloc_page_info *page_info) {
    int i;
    int biggest_gap_in_slots = 0;
    int current_gap_in_slots = 0;
    for (i = 0; i < KMALLOC_NUM_SLOTS; i++) {
        if (page_info->free[i]) {
            current_gap_in_slots++;
            if (current_gap_in_slots > biggest_gap_in_slots) {
                biggest_gap_in_slots = current_gap_in_slots;
            }
        } else {
            current_gap_in_slots = 0;
        }
    }

    //Return available number of slots in the largest gap.
    //Everything regarding the space in the front to track the number of slots
    //is taken care of in kmalloc proper.
    return biggest_gap_in_slots;
}

void *kmalloc(unsigned int size) {
    uint16_t slots_needed = (size + sizeof(uint16_t)) / KMALLOC_SLOT_SIZE;
    //addresses integer division truncation
    if (size % 8 != 0) {
        slots_needed++;
    }
    
    //start at head, iterate through looking for a page with a large enough gap
    struct kmalloc_page_info *page_info = kmalloc_head;
    while (page_info && page_info->max_free_gap < slots_needed) {
        page_info = page_info->next;
    }

    //If no such page exists, grab another!
    if (!page_info) {
        kmalloc_get_page();
        page_info = kmalloc_head;
    }

    //find some large enough gap
    int slots_start_index = kmalloc_locate_sufficient_gap(page_info, slots_needed);
     
    //fill in the front of the first slot with number of slots info
    //phys addr is addr of page info plus size of page info plus slot offset * slot size
    //needs to fit a memory address of 4 bytes
    uint32_t first_slot_phys_addr = (uint32_t)page_info + sizeof(struct kmalloc_page_info) + (KMALLOC_SLOT_SIZE * slots_start_index);

    //cast first_slot_phys_addr to uint16_t *, so that we can dereference it and fill it with slots needed, a 16 bit integer
    *((uint16_t *)first_slot_phys_addr) = (uint16_t)slots_needed;

    //mark these as malloc'd in kmalloc_page
    int i;
    for (i = 0; i < slots_needed; i++) {
        page_info->free[slots_start_index + i] = 0;
    }
    
    //find largest remaining gap
    page_info->max_free_gap = kmalloc_get_largest_gap_size(page_info);

    //return a pointer to the useful memory area, which is sizeof(uint16_t) into the first slot.
    return (void *)(first_slot_phys_addr + sizeof(uint16_t));
}

/**
* @brief Mark the memory being kfree'd as free in the kmalloc_page_info
* @details Identifies the number of slots consumed by the pointer given as the mem_loc argument, and marks these as free in the page_info given.
*
* @param page_info Pointer to the kmalloc_page_info which maintains the details regarding the page we're freeing memory in.
* @param mem_loc The pointer to the memory block to be freed.
*/
void kfree_mark_free(struct kmalloc_page_info *page_info, void *mem_loc) {
    //the number of consecutive slots consumed is stored just ahead of the pointer given
    uint16_t slots_consumed = *((uint16_t *)(mem_loc - sizeof(uint16_t)));

    //find offset for how far into the page memloc is in units of slots
    int first_slot_index = ((uint32_t)mem_loc - (uint32_t)page_info - sizeof(uint16_t) - sizeof(struct kmalloc_page_info)) / KMALLOC_SLOT_SIZE;

    //mark the slots which were consumed all free, starting with first_slot_index
    int i;
    for (i = 0; i < slots_consumed; i++) {
        page_info->free[first_slot_index + i] = 1;
    }
}

void kfree(void *to_free) {
    if (!to_free) {
        return;
    }

    struct kmalloc_page_info *page_info = kmalloc_head;
    int is_freed = 0;
    while (page_info && is_freed == 0) {
        uint32_t page_start = (uint32_t)page_info;

        //not <= and >= because pointers should never be on first or last byte of page
        if (page_start < (uint32_t)to_free && page_start + PAGE_SIZE > (uint32_t)to_free) {
            //liberate pointer from page
            kfree_mark_free(page_info, to_free);

            is_freed = 1;
        } else {
            page_info = page_info->next;
        }
    }

    if (is_freed == 0) {
        console_printf("kfree(%x) failed as %x was not kmalloc'd\n");
        //TODO: raise error?
    } else {
        page_info->max_free_gap = kmalloc_get_largest_gap_size(page_info);
    }

    return;
}
