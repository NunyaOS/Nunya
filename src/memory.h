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


void	memory_init();
void *	memory_alloc_page( bool zeroit );
void	memory_free_page( void *addr );

/**
* @brief Kernel allocation of the parameter requested size of memory
* @details Looks goes through a linked list of pages for a large enough gap of contiguous free slots of fixed size . If no page for kmalloc has a large enough gap, a new gap is asked from memory.
*
* @param
* @return
*/
void *  kmalloc(unsigned int size);
void    kfree(void * to_free);
#endif
