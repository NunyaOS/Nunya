/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef ISO_H_
#define ISO_H_

struct iso_file {
	int cur_offset;	// offset within file data
	int extent_offset;	// offset to actual location on disk of start of file
	char pname[256];
};

struct iso_file *iso_open(const char *pname);
int iso_close(struct iso_file *file);
int iso_read(char *dest, int elem_size, int num_elem, struct iso_file *file);

#endif /* ISO_H_ */
