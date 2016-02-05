/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef MEMORY_H
#define MEMORY_H

#include "kerneltypes.h"

#define KMALLOC_SLOT_SIZE 8

//462 is the size of a struct kmalloc_page_info.
//It comes from x = 8 + 454, as pointer and max_free_gap sum to 8 bytes, then 4094 = (8 * x) + (8 + x)
//where the first term is the space for the slots and the second is the sizeof(struct kmalloc_page_info)
#define KMALLOC_NUM_SLOTS (PAGE_SIZE - 462)/KMALLOC_SLOT_SIZE 

struct __attribute__((__packed__)) kmalloc_page_info {
    struct kmalloc_page_info *next;  // next page pointer
    int max_free_gap;   // number of slots in the largest continguous free gap
    uint8_t free[KMALLOC_NUM_SLOTS]; // bit vector of free slots
};

void memory_init();
void *memory_alloc_page(bool zeroit);
void memory_free_page(void *addr);

/**
* @brief Kernel allocation of the parameter requested size of memory
* @details Iterates through a linked list of pages for a large enough gap of contiguous free slots of fixed size. If no page for kmalloc has a large enough gap, a new gap is asked from memory.
*
* @param size The size in bytes of the chunk of memory requested from kmalloc, not to exceed 3632
* @return A pointer to the allocated memory that needs to be kfree()'d to be released.
*/
void *kmalloc(unsigned int size);

/**
* @brief Frees previously allocated memory in the kernel's portion of memory
* @detail Takes the given pointer, and finding the appropriate page in memory, marks that region as free on that page's kmalloc_page_info.
*
* @param to_free The pointer to the segment of memory to be free'd. This should have been obtained from a call to kmalloc.
* @return void
*/
void kfree(void* to_free);

#endif
