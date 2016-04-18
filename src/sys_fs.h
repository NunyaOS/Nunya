/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef SYS_FS_H
#define SYS_FS_H

#include "kerneltypes.h"

/**
 * @brief Closes a file
 * @details Closes and frees the resources associated with the given file
 * descriptor
 *
 * @param fd The file descriptor of the file to be closed.
 * return 0 if the file was open and is now closed, otherwise an integer code
 * matching a descriptive error in an enumeration in sys_fs_err.h
 */
static inline int32_t close(uint32_t fd) {
    return syscall(SYSCALL_close, fd, 0, 0, 0, 0);
}

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
static inline int32_t read(char *dest, uint32_t bytes, uint32_t fd) {
    return syscall(SYSCALL_read, (uint32_t)dest, bytes, fd, 0, 0);
}

/**
 * @brief Opens a file
 * @details Opens a the path at the specified file in the specified mode if it
 * exists and the process is able to open it
 *
 * @param path The path of the file which is to be opened
 * @param mode The mode the file is to be opened in. Acceptable modes are "a",
 * "r", "w", "ar", "ra", "rw", and "wr" with 'a' meaning append, 'r' meaning
 * read, and 'w' meaning write.
 * @return An integer file descriptor on success. Otherwise, an integer code
 *  matching a descriptive error in an enumeration in sys_fs_err.h.
 */
static inline int32_t open(const char *path, const char *mode) {
    return syscall(SYSCALL_open, (uint32_t)path, (uint32_t)mode, 0, 0, 0);
}

/**
 * @brief Writes to a file
 * @details Writes the specified number of bytes from the src to the file.
 *
 * @param src Pointer to the buffer to be written to the file.
 * @param bytes The number of bytes from the start of the buffer to be written
 * to the file.
 * @param fd The file descriptor which the buffer should be written to
 * @return On success the number of bytes written. Otherwise, an
 * integer code matching a descriptive error in an enumeration in sys_fs_err.h.
 */
static inline int32_t write(const char *src, uint32_t bytes,  uint32_t fd) {
    return syscall(SYSCALL_write, (uint32_t)src, bytes, fd, 0, 0);
}

#endif
