/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef FS_H
#define FS_H

#include "kerneltypes.h"
#include "process.h"
#include "sys_fs_structs.h"

enum ata_kind {
    ISO = 1,
};

struct fs_agnostic_file {
    uint8_t ata_unit;
    uint8_t mode;  //out of 7, r=4, w=2, a=1
    enum ata_kind ata_type;
    bool at_EOF;
    void *filep;
    char path[256];
};

/**
 * @brief Closes a file
 * @details Closes and frees the resources associated with the given file
 * descriptor
 *
 * @param fd The file descriptor of the file to be closed.
 * return 1 if the file was open and is now closed, otherwise an integer code
 * matching a descriptive error in an enumeration in sys_fs_err.h
 */
int32_t fs_close(uint32_t fd);

/**
 * @brief Read from a file into a buffer
 * @details Reads up to the specified number of bytes from the specified file
 * descriptor into the specified buffer
 *
 * @param dest The buffer to be filled with what is read from the file
 * @param bytes The maximum number of bytes to be read from the file
 * @param fd The file descriptor for the file which is to be read from
 * @return On success the number of bytes read, otherwise an integer code
 * matching a descriptive error in enumeration in sys_fs_err.h
 */
int32_t fs_read(char *dest, uint32_t bytes, uint32_t fd);

/**
 * @brief Opens a file
 * @details Opens a the path at the specified file in the specified mode if it
 * exists and the process is able to open it
 *
 * @param path The path of the file which is to be opened
 * @param mode The mode the file is to be opened in. Acceptable modes are "a",
 * "r", "w", "ar", "ra", "rw", and "wr" with 'a' meaning append, 'r' meaning
 * read, and 'w' meaning write.
 * @return An integer file descriptor on success. Otherwise, a very high
 * integer code matching a descriptive error in an enumeration in sys_fs_err.h.
 */
int32_t fs_open(const char *path, const char *mode);

/**
 * @brief Writes to a file
 * @details Writes the specified number of bytes from the src to the file.
 *
 * @param src Pointer to the buffer to be written to the file.
 * @param bytes The number of bytes from the start of the buffer to be written
 * to the file.
 * @param fd The file descriptor which the buffer should be written to
 * @return On success the number of bytes written. Otherwise, a very high
 * integer code matching a descriptive error in an enumeration in sys_fs_err.h.
 */
int32_t fs_write(const char *src, uint32_t bytes,  uint32_t fd);

/**
 * @brief Initializes security aspects for file system regarding a process
 * @details Creates a process's list of security allowances (currently default
 * root access) and an empty open files table.
 *
 * @param p A process which the initialization should occur on.
 */
void fs_init_security(struct process *p);

/**
 * @brief Initialize the open file table for the kernel
 * @details Initialize each fs_agnostic file to reflect an unopened file
 */
void fs_sys_init_open_file_table();

#endif
