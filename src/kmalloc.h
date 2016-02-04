/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef KMALLOC_H
#define KMALLOC_H

/**
 * @brief   Kernel allocation of the parameter requested size of memory
 * @details Iterates through a linked list of pages for a large enough gap of
 *          contiguous free slots of fixed size. If no page for kmalloc has a
 *          large enough gap, a new gap is asked from memory.
 *
 * @param   size The size in bytes of the chunk of memory requested from
 *          kmalloc, not to exceed 3632
 * @return  A pointer to the allocated memory that needs to be kfree()'d to be
 *          released.
 */
void *kmalloc(unsigned int size);

/**
 * @brief   Frees previously allocated memory in the kernel's portion of memory
 * @detail  Takes the given pointer, and finding the appropriate page in
 *          memory, marks that region as free on that page's kmalloc_page_info.
 *
 * @param   to_free The pointer to the segment of memory to be free'd. This
 *          should have been obtained from a call to kmalloc.
 */
void kfree(void* to_free);

#endif
