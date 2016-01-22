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

int disk_write(char *source, int block, int offset, int num_bytes);

int disk_read(char *destination, int block, int offset, int num_bytes);

#endif /* DISK_H */
