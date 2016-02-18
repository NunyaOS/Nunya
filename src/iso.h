/*
Copyright (C) 2016 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef ISO_H_
#define ISO_H_

struct iso_file {
    int ata_unit;
    int cur_offset;    // offset within file data
    int extent_offset;    // offset to actual location on disk of start of file
    int data_length;
    char pname[256];
};

struct iso_file *iso_fopen(const char *pname, int ata_unit);
int iso_fclose(struct iso_file *file);
int iso_fread(void *dest, int elem_size, int num_elem, struct iso_file *file);

#endif /* ISO_H_ */
