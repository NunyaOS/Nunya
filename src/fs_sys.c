#include "fs_sys.h"
#include "string.h"
#include "kerneltypes.h"
#include "process.h"
#include "iso.h"
#include "console.h"

#define READ 4
#define WRITE 2
#define APPEND 1

void fs_print_allowances() {
    console_printf("fs_print_allowances: ");
    struct fs_allowance *iterator = current->fs_allowances_head;
    while (iterator) {
        console_printf("%s -> ", iterator->path);
        iterator = iterator->next;
    }
    console_printf("\n");
}

void fs_sys_init_security(struct process *p) {
    //Open files table
    p->files = kmalloc(sizeof(*(p->files)));
    int i;

    //set all open file pointers to null
    for (i = 0; i < PROCESS_MAX_OPEN_FILES; i++) {
        p->files->open_files[i] = 0;
    }

    p->files->num_open = 0;

    //Allowances alloc'ing.
    p->fs_allowances_head = kmalloc(sizeof(*(p->fs_allowances_head)));

    //Set the lone permission in the list to root (that is all of the fs)
    p->fs_allowances_head->next = 0;
    strcpy(p->fs_allowances_head->path, "/");
    p->fs_allowances_head->do_allow_below = 1;
}

// Look at the OS's table of open files to see if read write conflicts
// might be occurring amongst many processes if you allow it to be opened
bool fs_sys_check_openability(const char *path, const char *mode) {
    return 1;
}

int map_media_to_driver_id(int media) {
    return 1; //ISO for now is the only thing
}

int mode_str_to_int(const char *mode) {
    if (strcmp(mode, "a") == 0) { return APPEND; }
    if (strcmp(mode, "w") == 0) { return WRITE; }
    if (strcmp(mode, "r") == 0) { return READ; }
    if (strcmp(mode, "ar") == 0) { return READ | APPEND; }
    if (strcmp(mode, "ra") == 0) { return READ | APPEND; }
    if (strcmp(mode, "rw") == 0) { return READ | WRITE; }
    if (strcmp(mode, "wr") == 0) { return READ | WRITE; }
    return 0;
}

struct fs_agnostic_file *create_fs_agnostic_file(uint8_t ata_type, uint8_t ata_unit, const char *path, uint8_t mode) {
    struct fs_agnostic_file *new_file = kmalloc(sizeof(*new_file));
    new_file->ata_unit = ata_unit;
    new_file->ata_type = ata_type;
    new_file->mode = mode;
    strcpy(new_file->path, path);
    switch(ata_type) {
        case 1:   //ISO
            new_file->filep = (void *)iso_fopen(path, ata_unit);
            break;
        default:
            kfree(new_file);
            return 0;
    }
    if (!new_file->filep) {
        //media failed to open the file and returned 0
        kfree(new_file);
        return 0;
    }
    return new_file;
}

// Requires absolute path with /media_letter/first_level_dir/second_level_dir/file_name
//-1 means file does not exist
//-2 means illegal allowances
//-3 means wrong owner permission
//-4 means open by another process for writing, or open for reading and you want to write
//-5 means too many files opened
//-6 means illegal mode
uint32_t fs_sys_open(const char *path, const char *mode) {
    //checks both owner permissions (dummy true) and process permissions
    int success = fs_sys_security_check(path);
    if (success < 1) {
        return success;
    }
    if (!fs_sys_check_openability(path, mode)) {
        return -4;
    }
    if (current->files->num_open >= PROCESS_MAX_OPEN_FILES) {
        return -5;
    }
    int imode = mode_str_to_int(mode);
    if (imode == 0) {
        return -6;
    }

    //strip path name for media
    if (path[0] != '/') {
        return -1;
    }
    if (path[1] != '0' && path[1] != '1' && path[1] != '2' && path[1] != '3') {
        return -1;
    }
    if (path[2] != '/') {
        return -1;
    }

    char media_path[strlen(path) - 1];   //Two fewer than whole path length, + 1 for null terminator
    strcpy(media_path, path+2);

    int ata_unit = path[1] - 48; //48 is ascii for 0

    int ata_type = map_media_to_driver_id(ata_unit);

    int next_fd;
    //find current's next available fd by 'files' member
    //add to process file->file table
    bool opened = 0;
    for (next_fd = 0; opened == 0 && next_fd < PROCESS_MAX_OPEN_FILES; next_fd++) {
        if (current->files->open_files[next_fd] == 0) {
            //make new sys level file
            //mark it with the ata type
            //the ata unit
            //and a pointer to the union of below
            //sys level file types
            current->files->open_files[next_fd] = create_fs_agnostic_file(ata_type, ata_unit, media_path, imode);
            if (current->files->open_files[next_fd]) {
                opened = 1;
                current->files->num_open += 1;
                break;
            } else {
                return -1;
            }
        }
    }

    // TODO: keep on OS's open file table

    return next_fd;
}

// -1 means the fd was not an open file
uint32_t fs_sys_close(uint32_t fd) {
    if (fd >= PROCESS_MAX_OPEN_FILES) {
        return -1;
    }

    struct fs_agnostic_file *fp = current->files->open_files[fd];
    if (fp) {
        switch (fp->ata_type) {
            case 1:  //ISO
                iso_fclose((struct iso_file *)fp);
                break;
            default:
                break;
        }
        kfree(fp);
        current->files->open_files[fd] = 0;
        current->files->num_open -= 1;
        // TODO: decrement OS open files table
            //if count is 0, remove from OS open files table
        return 0;
    } else {
        return -1;
    }
}

// -1 fd is not for an open file
// -2 illegal allowances
uint32_t fs_sys_read(char *dest, uint32_t bytes, uint32_t fd) {
    int bytes_read = 0;
    if (fd >= PROCESS_MAX_OPEN_FILES) {
        return -1;
    }
    struct fs_agnostic_file *fp = current->files->open_files[fd];
    if(!fp) {
        return -1;
    }
    //must be okay with security and be allowed to read
    if (fs_sys_security_check(fp->path) && (fp->mode & READ)) {
        switch (fp->ata_type) {
            case 1:  //ISO
                bytes_read = iso_fread(dest, 1, bytes, (struct iso_file *)fp->filep);
                fp->at_EOF = ((struct iso_file *)fp->filep)->at_EOF;
                break;
            default:
                return -2;
        }
    } else {
        return -1;
    }

    return bytes_read;
}

uint32_t fs_sys_write(const char *src, uint32_t bytes, uint32_t fd) {
    int bytes_written = -1;
    // TODO: we'll need a writable system.
    return bytes_written;
}

uint32_t fs_sys_add_allowance(const char *path, bool do_allow_below) {
    //must be under current paths with is_just_dir = 0;
    if (fs_sys_security_check(path)) {
        struct fs_allowance *iterator;
        for (iterator = current->fs_allowances_head; iterator != 0; iterator = iterator->next) {
            if (strcmp(iterator->path, path) == 0) {
                //duplicate entry, do not actually add.
                return 1;
            }
        }

        struct fs_allowance *new = kmalloc(sizeof(*new));
        //insert at head of list
        new->next = current->fs_allowances_head;
        current->fs_allowances_head = new;

        //set other needed values
        strcpy(new->path, path);
        new->do_allow_below = do_allow_below;
        return 1;
    }
    return 0;
}

uint32_t fs_sys_remove_allowance(const char *path) {
    //does not recursively remove allowances from fs
    //should recursively remove allowances from processes,
    //but we can't, because we don't have pointers to our children
    struct fs_allowance *iterator;
    struct fs_allowance *trailing_iterator = 0;

    for(iterator = current->fs_allowances_head; iterator != 0; iterator = iterator->next) {
        if (strcmp(iterator->path, path) == 0) {
            //Found it! Remove it from the linked list
            if(trailing_iterator) {
                //patch linked list
                trailing_iterator->next = iterator->next;
            } else {
                //first item, must update head ptr
                current->fs_allowances_head = iterator->next;
            }
            kfree(iterator);
            return 1;
        } else {
            trailing_iterator = iterator;
        }
    }
    return 0;
}

bool fs_sys_owner_check(const char *path) {
    // TODO: This
    return 1;
}

/**
* Truncate a copy of the given path to match the length
* of the allowed->path. If these match and it's an allow_below
* allowance, give permission. If it's not an allow_below 

* allowance, only return 1 on exact match.
*/
bool path_permitted_by_allowance(const char *path, struct fs_allowance *allowed) {
    if (allowed->do_allow_below) {
        uint32_t allowed_path_len = strlen(allowed->path);
        char truncated_path[allowed_path_len + 1];
        memcpy(truncated_path, path, allowed_path_len);
        truncated_path[allowed_path_len] = '\0';
        if(strcmp(allowed->path, truncated_path) == 0) {
            // The allowance is above the requested path
            return 1;
        } else {
            return 0;
        }
    } else {
        //in this case, you're allowed to read dir contents
        //but not access them
        if (strcmp(allowed->path, path) == 0) {
            return 1;
        } else {
            return 0;
        }
    }
    return 0;
}

bool fs_sys_allowance_check(const char *path) {
    struct fs_allowance *iterator = current->fs_allowances_head;
    //iterate over list of allowances until we reach
    //an acceptable one. otherwise return 0;
    while (iterator) {
        if (path_permitted_by_allowance(path, iterator)) {
            return 1;
        } else {
            iterator = iterator->next;
        }
    }
    return 0;
}

uint32_t fs_sys_security_check(const char *path) {
    if(!fs_sys_owner_check(path)) {
        return -3;
    }

    if(!fs_sys_allowance_check(path)) {
        return -2;
    }
    return 1;
}
