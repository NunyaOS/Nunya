/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef MEMORY_H
#define MEMORY_H

#include "kerneltypes.h"

#define KMALLOC_SLOT_SIZE 8
#define KMALLOC_NUM_SLOTS (PAGE_SIZE - 0)/KMALLOC_SLOT_SIZE

struct kmalloc_page_info{
    struct kmalloc_page_info *next;  // next page pointer
    int max_free_gap;   // number of slots in the largest continguous free gap
    uint8_t free[KMALLOC_NUM_SLOTS]; // bit vector of free slots
};


void	memory_init();
void *	memory_alloc_page( bool zeroit );
void	memory_free_page( void *addr );
void *  kmalloc(unsigned int size);
void    kfree(void * to_free);
#endif
