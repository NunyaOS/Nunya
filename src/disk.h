/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

/*
 * disk.h
 */

#ifndef DISK_H
#define DISK_H

/**
 * @brief Read in data from a particular disk block
 * @details Read exactly num_bytes characters from a specified block and offset
 *
 * @param destination
 * @param start_block_index
 * @param offset
 * @param num_bytes
 *
 * @return number of bytes read
 */
int disk_read(char *destination, int block, int offset, int num_bytes);

#endif /* DISK_H */
