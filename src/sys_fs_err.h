/*
Copyright (C) 2016 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

/* These will all be returned in syscalls which have a return type of unit32_t
 * This means that they will all be very high constants because of two's
 * complement on low absolute value negatives being nearest to the maximum
 * value of 2^32. Hopefully this is far out of conflict with bytes_read
*/

#define ERR_FD_OOR -1   // file descriptor is out of range
#define ERR_NO_ALLOWANCE -2    // process does not have an allowance to use this file
#define ERR_OPEN_CONFLICT -3   // opening of file refused due to another process having the file open
#define ERR_FDS_EXCEEDED -4    // process has too many open files and cannot open more
#define ERR_BAD_MODE -5    // mode specified is not a legal mode string
#define ERR_BAD_PATH -6    // path does not start with /X/, where X is in the inclusive range 0-3, which is a requisite for Nunya OS
#define ERR_KERNEL_OPEN_FAIL -7   // the kernel could not create the structures it needed to track the file if opened
#define ERR_WAS_NOT_OPEN -8   // attempted to close a file that was not on the open files table
#define ERR_BAD_ACCESS_MODE -9    // attempted to access a file for a mode it was not open in, i.e. read from file opened in 'w' mode
#define ERR_NOT_OWNER -10    // owner of process does not own the file
#define ERR_BAD_ATA_KIND -11  // ata_type is not within allowable enum ata_kind values
