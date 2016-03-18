#ifndef FS_SYS_H 
#define FS_SYS_H

#include "kerneltypes.h"

#define PROCESS_MAX_OPEN_FILES 64
#define MAX_PATH_LENGTH 256
struct process_files {
    uint8_t num_open;
    struct fs_agnostic_file *open_files[PROCESS_MAX_OPEN_FILES];
};

struct fs_allowance {
    struct fs_allowance *next;
    char path[MAX_PATH_LENGTH + 1];  //null terminated, NO FOLLOWING forward slash... i.e. /a/home/dir_a
    bool do_allow_below;
};

struct fs_agnostic_file {
    uint32_t ata_unit;
    uint32_t mode;  //out of 7, r=4, w=2, a=1
    uint32_t offset;  //what byte into the file we are
};

uint32_t fs_sys_add_allowance(const char *path, bool do_allow_below);
uint32_t fs_sys_remove_allowance(const char *path);
uint32_t fs_sys_close(uint32_t fd);
uint32_t fs_sys_read(char *dest, uint32_t bytes, uint32_t fd);
uint32_t fs_sys_open(const char *path, const char *mode);
uint32_t fs_sys_write(const char *src, uint32_t bytes,  uint32_t fd);

//Does this need to be public...
//Does this help by being public?
bool fs_sys_security_check(const char *path);

#endif
