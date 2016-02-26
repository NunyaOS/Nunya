/*
Copyright (C) 2016 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef ISO_H
#define ISO_H

#include "kerneltypes.h"

struct iso_file {
    int ata_unit;
    int cur_offset;    // offset within file data
    int extent_offset;    // offset to actual location on disk of start of file
    uint32_t data_length;
    int at_EOF;
    char pname[256];
};

struct iso_dir {
    int ata_unit;    // On which ATA unit this directory record exists
    int extent_offset;  //location of the directory's extent
    int cur_offset;  //offset to the front of the next directory record to read
    uint32_t data_length;    // Length of data in directory record
};


struct directory_record {
    uint8_t length_of_record;
    uint8_t length_of_ext_record;
    unsigned char loc_of_ext[8];   //Characters of hex for location of extent, first 4 are little endian, second 4 are big endian
    unsigned char data_length[8];  //Characters of hex for the length of the entity, first 4 are little endian, second 4 are big endian
    uint8_t rec_date_time[7];   // time stamp of last modification
    char file_flags[1];
    char file_flags_interleaved[1];
    uint8_t interleave_gap_size;
    char vol_seq_num[4];
    uint8_t len_identifier;
    char file_identifier[32];  //max file id with extension is 30, max directory name is 31, +1 for null
};

/**
 * @brief Opens the file specified
 * @details Attempts to find and open the file specified by the pname on the given ata_unit,
 * calls kmalloc to create return value
 *
 * @param pname The absolute path name to search for the file at
 * @param ata_unit The ata unit to search for the file on
 * @return Pointer to freshly allocated iso_file at byte 0, null if not found or on error
 */
struct iso_file *iso_fopen(const char *pname, int ata_unit);

/**
 * @brief Closes the file
 * @details Calls free on the file if not null.
 *
 * @param file Pointer to the file to be closed
 * @return 0 if successfully closed, -1 if not.
 */
int iso_fclose(struct iso_file *file);

/**
 * @brief Reads specified number of bytes from a file into dest
 * @details Reads num_elem number of objects of size elem_size into
 * the buffer dest from file.
 *
 * @param dest Buffer into which data is copied
 * @param elem_size Size of each element to be copied
 * @param num_elem Number of elements from file to be copied
 * @param file File from which data is copied
 * @return Number of elements successfully read, -1 on error
 */
int iso_fread(void *dest, int elem_size, int num_elem, struct iso_file *file);

/**
 * @brief Opens the directory specified
 * @details Attempts to find and open the directory specified by the absolute pname on the given ata_unit,
 * calls kmalloc to create return value. No relative paths allowed.
 *
 * @param pname The absolute path name to search for the directory at
 * @param ata_unit The ata unit to search for the directory on
 * @return Pointer to freshly allocated iso_dir at first directory record, null if not found or on error
 */
struct iso_dir *iso_dopen(const char *pname, int ata_unit);

/**
 * @brief Closes the directory
 * @details Calls free on the directory if not null.
 *
 * @param dir Pointer to the directory to be closed
 * @return 0 if successfully closed, -1 if not.
 */
int iso_dclose(struct iso_dir *dir);

/**
 * @brief Fetches the next directory record pointed to by dir
 *
 * @param dir Stream of directory records inside of a directory extent
 * @return The next directory record in the stream, null on error or
 * on end of stream
 */
struct directory_record *iso_dread(struct iso_dir *dir);
#endif /* ISO_H */
