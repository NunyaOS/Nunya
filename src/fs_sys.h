#ifndef FS_SYS_H
#define FS_SYS_H

#include "kerneltypes.h"
#include "process.h"
#include "fs_sys_structs.h"

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
 * @brief Add a duplicate allowance that is lower on the file tree than a
 * current fs allowance
 * @details Ensures that the allowances being added are within current
 * allowances before adding a new fs_allowance to the current process's list
 * of fs_allowances
 *
 * @param path The path to add as an allowance for this process.
 * @param do_allow_below bool to represent whether access should be granted
 * just at this path or to everything below this path
 * @return 1 if allowance is present in the list of allowances after the call, 0 if not
 */
uint32_t fs_sys_add_allowance(const char *path, bool do_allow_below);

/**
 * @brief Remove allowance from fs allowance list
 * @details Search the allowances list for an exact match of the path and
 * remove if found
 *
 * @param path The exact path of the allowance which should be removed.
 * @return 1 if the path was removed from allowances, 0 if the path was not
 * found on the list.
 */
uint32_t fs_sys_remove_allowance(const char *path);

/**
 * @brief Closes a file
 * @details Closes and frees the resources associated with the given file
 * descriptor
 *
 * @param fd The file descriptor of the file to be closed.
 * return 1 if the file was open and is now closed, otherwise an integer code
 * matching a descriptive error in an enumeration in fs_sys_err.h
 */
uint32_t fs_sys_close(uint32_t fd);

/**
 * @brief Read from a file into a buffer
 * @details Reads up to the specified number of bytes from the specified file
 * descriptor into the specified buffer
 *
 * @param dest The buffer to be filled with what is read from the file
 * @param bytes The maximum number of bytes to be read from the file
 * @param fd The file descriptor for the file which is to be read from
 * @return On success the number of bytes read, otherwise an integer code
 * matching a descriptive error in enumeration in fs_sys_err.h
 */
uint32_t fs_sys_read(char *dest, uint32_t bytes, uint32_t fd);

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
 * integer code matching a descriptive error in an enumeration in fs_sys_err.h.
 */
uint32_t fs_sys_open(const char *path, const char *mode);

/**
 * @brief Writes to a file
 * @details Writes the specified number of bytes from the src to the file.
 *
 * @param src Pointer to the buffer to be written to the file.
 * @param bytes The number of bytes from the start of the buffer to be written
 * to the file.
 * @param fd The file descriptor which the buffer should be written to
 * @return On success the number of bytes written. Otherwise, a very high
 * integer code matching a descriptive error in an enumeration in fs_sys_err.h.
 */
uint32_t fs_sys_write(const char *src, uint32_t bytes,  uint32_t fd);

/**
 * @brief Initializes security aspects for file system regarding a process
 * @details Creates a process's list of security allowances (currently default
 * root access) and an empty open files table.
 *
 * @param p A process which the initialization should occur on.
 */
void fs_sys_init_security(struct process *p);

#endif
