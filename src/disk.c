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

int disk_read(char *destination, int start_block_index, int offset, int num_bytes) {
    int blocks_needed = ((offset + num_bytes) / ATA_BLOCKSIZE);
    if(((offset + num_bytes) % ATA_BLOCKSIZE) != 0) {
        blocks_needed++;
    }

    int blocks_read_in;
    char block_buffer[ATA_BLOCKSIZE];
    int memcpy_offset;
    int bytes_in_destination = 0;

    //Take one block at a time for memory and performance and such
    for (blocks_read_in = 0; blocks_read_in < blocks_needed; blocks_read_in++) {
        ata_read(DEFAULT_ATA_UNIT, block_buffer, 1, start_block_index + blocks_read_in);
        memcpy_offset = blocks_read_in == 0 ? offset : 0;

        int bytes_to_copy = ((num_bytes - bytes_in_destination) + memcpy_offset) > ATA_BLOCKSIZE ? ATA_BLOCKSIZE - memcpy_offset : num_bytes - bytes_in_destination;

        memcpy(destination + bytes_in_destination, block_buffer + memcpy_offset, bytes_to_copy);
        bytes_in_destination += bytes_to_copy;
    }

    return bytes_in_destination;
}

int disk_write(char *source, int start_block_index, int offset, int num_bytes) {
    int blocks_needed = ((offset + num_bytes) / ATA_BLOCKSIZE);
    if (((offset + num_bytes) % ATA_BLOCKSIZE) != 0) {
        blocks_needed++;
    }

    char block_buffer[ATA_BLOCKSIZE];
    int blocks_read_in;
    int memcpy_offset;
    int bytes_written = 0;
    int bytes_to_write;

    for (blocks_read_in = 0; blocks_read_in < blocks_needed; blocks_read_in++) {
        ata_read(DEFAULT_ATA_UNIT, block_buffer, 1, start_block_index);
        memcpy_offset = blocks_read_in == 0 ? offset : 0;

        bytes_to_write = ((num_bytes - bytes_written) + memcpy_offset) > ATA_BLOCKSIZE ? ATA_BLOCKSIZE - memcpy_offset : num_bytes - bytes_written;
        memcpy(block_buffer + memcpy_offset, source + bytes_written, bytes_to_write);
        bytes_written += bytes_to_write;

        ata_write(DEFAULT_ATA_UNIT, block_buffer, 1, start_block_index + blocks_read_in);
    }

    return bytes_written;
}
