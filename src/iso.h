/*
Copyright (C) 2016 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef ISO_H_
#define ISO_H_

#include "kerneltypes.h"

struct iso_file {
    int ata_unit;
    int cur_offset;    // offset within file data
    int extent_offset;    // offset to actual location on disk of start of file
    int data_length;
    char pname[256];
};

struct iso_dir {
	int ata_unit;
	int extent_offset;  //location of the directory's extent
	int cur_offset;  //offset to the front of the next directory record to read
	int data_length;
};

struct directory_record {
    uint8_t length_of_record;
    uint8_t length_of_ext_record;
    char loc_of_ext[8];
    char data_length[8];
    uint8_t rec_date_time[7];
    char file_flags[1];
    char file_flags_interleaved[1];
    uint8_t interleave_gap_size;
    char vol_seq_num[4];
    uint8_t len_identifier;
    char file_identifier[31];  //max file id with extent is 30, +1 for null
};

struct iso_file *iso_fopen(const char *pname, int ata_unit);
int iso_fclose(struct iso_file *file);
int iso_fread(void *dest, int elem_size, int num_elem, struct iso_file *file);

struct iso_dir *iso_dopen(const char *pname, int ata_unit);
void iso_dread(struct directory_record **dest_ptr, struct iso_dir *dir);
int iso_dclose(struct iso_dir *dir);

#endif /* ISO_H_ */
