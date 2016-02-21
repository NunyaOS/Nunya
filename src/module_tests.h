/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

/**
 * @brief   Test memory allocation by walking all available user memory
 * @details Test memory allocation by walking all available user memory. We
 *          start accessing memory from 0x80000000, and keep walking until we
 *          finish allocating all available memory frames.
 */
void walk_memory();
