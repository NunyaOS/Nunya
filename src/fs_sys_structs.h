#ifndef FS_SYS_STRUCTS_H
#define FS_SYS_STRUCTS_H

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

#endif
