/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
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
 * @return number of bytes read, -1 on error
 */
int disk_read(char *destination, int block, int offset, int num_bytes);

/**
 * @brief Write data to a particular disk block
 * @details Writes exactly num_bytes characters to a specified block at a given offset
 *
 * @param source Character buffer to copy to disk
 * @param start_block_index The block to which to write
 * @param offset The byte number of the block to start writing the source to
 * @param num_bytes The number of bytes to write to the disk
 *
 * @return number of bytes successfully written to the disk, -1 on error
 */
int disk_write(char *source, int start_block_index, int offset, int num_bytes);


#endif /* DISK_H */
