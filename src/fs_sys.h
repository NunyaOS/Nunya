#ifndef FS_SYS_H
#define FS_SYS_H

#include "kerneltypes.h"
#include "process.h"
#include "fs_sys_structs.h"

struct fs_agnostic_file {
    uint8_t ata_unit;
    uint8_t mode;  //out of 7, r=4, w=2, a=1
    uint8_t ata_type;  //ISO = 1, maybe or something FAT32=2?
    bool at_EOF;
    void *filep;
    char path[256];
};

uint32_t fs_sys_add_allowance(const char *path, bool do_allow_below);
uint32_t fs_sys_remove_allowance(const char *path);
uint32_t fs_sys_close(uint32_t fd);
uint32_t fs_sys_read(char *dest, uint32_t bytes, uint32_t fd);
uint32_t fs_sys_open(const char *path, const char *mode);
uint32_t fs_sys_write(const char *src, uint32_t bytes,  uint32_t fd);
void fs_sys_init_security(struct process *p);

//Does this need to be public...
//Does this help by being public?
uint32_t fs_sys_security_check(const char *path);

#endif
