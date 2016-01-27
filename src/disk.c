/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

/*
 * disk.c
 */

#include "disk.h"
#include "ata.h"
#include "string.h"
#include "console.h"

#define DEFAULT_ATA_UNIT 0


int disk_read(char *destination, int start_block_index, int offset,
              int num_bytes) {
    int blocks_needed = ((offset + num_bytes) / ATA_BLOCKSIZE) + 1;
    if (((offset + num_bytes) % ATA_BLOCKSIZE) == 0) {
        blocks_needed--;
    }


    int blocks_read_in = 0;
    char block_buffer[ATA_BLOCKSIZE];
    int memcpy_offset;
    int bytes_in_destination = 0;
    int bytes_after_block_offset;       // Usually ATA_BLOCKSIZE except on first block with offset

    //Take one block at a time for memory and performance and such
    for (blocks_read_in = 0; blocks_read_in < blocks_needed; blocks_read_in++) {
        ata_read(DEFAULT_ATA_UNIT, block_buffer, 1,
                 start_block_index + blocks_read_in);
        if (blocks_read_in == 0) {
            memcpy_offset = offset;
            bytes_after_block_offset = ATA_BLOCKSIZE - offset;
        } else {
            memcpy_offset = 0;
            bytes_after_block_offset = ATA_BLOCKSIZE;
        }
        int bytes_to_copy =
            ((bytes_in_destination + bytes_after_block_offset) >
             num_bytes) ? num_bytes -
            bytes_in_destination : bytes_after_block_offset;

        memcpy(destination, block_buffer + memcpy_offset, bytes_to_copy);
        bytes_in_destination += bytes_to_copy;
    }

    return bytes_in_destination;
}
