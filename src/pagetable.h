/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef PAGETABLE_H
#define PAGETABLE_H

#define PAGE_SIZE 4096

#define PAGE_FLAG_USER        0
#define PAGE_FLAG_KERNEL      1
#define PAGE_FLAG_EXISTS      0
#define PAGE_FLAG_ALLOC       2
#define PAGE_FLAG_READONLY    0
#define PAGE_FLAG_READWRITE   4
#define PAGE_FLAG_NOCLEAR     0
#define PAGE_FLAG_CLEAR       8

/**
 * @brief   Create a pagetable
 * @details Allocates a pagetable in memory. Each pagetable is one memory page
 *          large, in this case, 4096 bytes. The pagetable needs to be manually
 *          deleted using pagetable_delete.
 *
 * @return  A pointer to the newly created pagetable.
 */
struct pagetable *pagetable_create();

/**
 * @brief   Initialize a direct-mapped pagetable
 * @details Initialize a given pagetable by direct mapping kernel space memory
 *          up until 2GB, and direct mapping video memory for the video system.
 *          TODO (SL): we should ensure video memory is correctly mapped into
 *          kernel space even without vram in the future.
 *
 * @param   p A pointer to the pagetable to be initialized
 */
void pagetable_init(struct pagetable *p);

/**
 * @brief   Map a virtual address to a physical address in a page directory
 * @details Given a virtual address, the pagetable maps its page to a given
 *          physical frame  address. If PAGE_FLAG_ALLOC is passed in, paddr is
 *          ignored, and a new physical frame is allocated to be mapped.
 *
 * @param   p       A pointer to the page directory to be modified
 * @param   vaddr   Virtual address to be mapped
 * @param   paddr   Physical address to be mapped. If PAGE_FLAG_ALLOC is passed
                    in, paddr is ignored
 * @param   flags   Flags of the new pages
 * @return  1 if the mapping is successful, and 0 otherwise
 */
int pagetable_map(struct pagetable *p, unsigned vaddr, unsigned paddr,
                  int flags);

/**
 * @brief   Get the physical address of a virtual address in a page directory
 * @details Given a virtual address, the pagetable gets its corresponding
 *          physical address. The lookup is successful if the vaddr is mapped
 *          to a paddr, and the page is currently in memory.
 *
 * @param   p       A pointer to the page directory to be looked up
 * @param   vaddr   Virtual address to be looked up
 * @param   paddr   A pointer to store the looked up physical address
 * @return  1 if the lookup is successful; 0 if the virtual address is not
 *          mapped to any physical address, or if the page isn't currently in
 *          memory
 */
int pagetable_getmap(struct pagetable *p, unsigned vaddr, unsigned *paddr);

/**
 * @brief   Unmap a page from a given pagetable
 * @details Given a virtual address, the pagetable marks the corresponding page
 *          as "not in memory" in the given pagetable.
 *
 * @param   p       A pointer to the pagetable to be modified
 * @param   vaddr   A virtual address in the page
 */
void pagetable_unmap(struct pagetable *p, unsigned vaddr);

/**
 * @brief   Allocate pagetables and map a given virtual address and length
 * @details Given a virtual address, the pagetable allocates physical memory and
 *          maps it for a given range of virtual memory.
 *
 * @param   p       A pointer to the page directory to be modified
 * @param   vaddr   Address to the beginning of the first page
 * @param   length  Total amount of memory to be allocated and mapped (in bytes)
 * @param   flags   Flags of the new pages
 */
void pagetable_alloc(struct pagetable *p, unsigned vaddr, unsigned length,
                     int flags);

/**
 * @brief   Delete (free) a given pagetable
 * @details Given a page directory, delete all its contents and free the
 *          physical memory mapped to the virtual addresses contained in the
 *          page directory.
 *
 * @param   p   A pointer to the pagetable to be deleted
 */
void pagetable_delete(struct pagetable *p);

/**
 * @brief   Load a given pagetable
 * @details Load a pagetable into %cr3, so when we swap processes, we can make
 *          sure the system is using the correct pagetable.
 *
 * @param   p   A pointer to the pagetable to be loaded into %cr3
 * @return  A pointer to the swapped out pagetable.
 */
struct pagetable *pagetable_load(struct pagetable *p);

/**
 * @brief   Enable paging for the processor
 * @details Enable paging by setting the paging bit of %cr0 to 1.
 */
void pagetable_enable();

/**
 * @brief   Refresh pagetable
 * @details Reload the currently-enabled pagetable.
 */
void pagetable_refresh();

/**
 * @brief   Handle a pagefault exception
 * @details Handle a pagefault exception originated by the CPU. This function
 *          should *NEVER* be called directly.
 */
void exception_handle_pagefault(int intr, int code);

#endif
