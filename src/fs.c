/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "fs.h"
#include "string.h"
#include "kerneltypes.h"
#include "process.h"
#include "iso.h"
#include "console.h"
#include "sys_fs_err.h"

#define READ 4
#define WRITE 2
#define APPEND 1

int32_t fs_security_check(const char *path);

#define MAX_OS_OPEN_FILES 1024

struct fs_agnostic_file open_files_table[MAX_OS_OPEN_FILES];

/*
 * A dummy function to help us in our debugging
 */
void fs_print_allowances() {
    struct list_node *iterator = current->fs_allowances_list.head;
    while (iterator) {
        console_printf("%s -> ", ((struct fs_allowance *)iterator)->path);
        iterator = iterator->next;
    }
    console_printf("\n");
}

void fs_sys_init_open_files_table() {
    int i;
    for (i = 0; i < MAX_OS_OPEN_FILES; i++) {
        open_files_table[i].filep = 0;  // Set each file pointer to null
    }
}

void fs_init_security(struct process *p) {
    // Initialize all file descriptor slots to 0
    int i;
    for (i = 0; i < PROCESS_MAX_OPEN_FILES; i++) {
        p->fd_table[i].is_open = 0;
        p->fd_table[i].ptr = 0;
    }

    p->files->num_open = 0;

    //Temporarily copy the entire list so that the in place checks work out
    fs_copy_allowances_list(&(p->fs_allowances_list), &(p->permissions->fs_allowances));

}

// Look at the OS's table of open files to see if read write conflicts
// might be occurring amongst many processes if you allow it to be opened
bool fs_check_open_conflict(const char *path, const char *mode) {
    return 1;
}

int map_media_to_driver_id(int media) {
    return ISO;
}

int mode_str_to_int(const char *mode) {
    if (strcmp(mode, "a") == 0) {
        return APPEND;
    }
    if (strcmp(mode, "w") == 0) {
        return WRITE;
    }
    if (strcmp(mode, "r") == 0) {
        return READ;
    }
    if (strcmp(mode, "ar") == 0) {
        return READ | APPEND;
    }
    if (strcmp(mode, "ra") == 0) {
        return READ | APPEND;
    }
    if (strcmp(mode, "rw") == 0) {
        return READ | WRITE;
    }
    if (strcmp(mode, "wr") == 0) {
        return READ | WRITE;
    }
    if (strcmp(mode, "aw") == 0) {
        return APPEND | WRITE;
    }
    if (strcmp(mode, "wa") == 0) {
        return WRITE | APPEND;
    }
    if (strcmp(mode, "rwa") == 0) {
        return READ | WRITE | APPEND;
    }
    if (strcmp(mode, "raw") == 0) {
        return READ | APPEND | WRITE;
    }
    if (strcmp(mode, "wra") == 0) {
        return WRITE | READ | APPEND;
    }
    if (strcmp(mode, "war") == 0) {
        return WRITE | APPEND | READ;
    }
    if (strcmp(mode, "arw") == 0) {
        return APPEND | READ | WRITE;
    }
    if (strcmp(mode, "awr") == 0) {
        return APPEND | WRITE |READ;
    }
    return 0;
}

struct fs_agnostic_file *create_fs_agnostic_file(enum ata_kind ata_type, uint8_t ata_unit, const char *path, uint8_t mode) {
    int next_open_slot;
    for (next_open_slot = 0; next_open_slot < MAX_OS_OPEN_FILES; next_open_slot++) {
        if (!open_files_table[next_open_slot].filep) {  // If the file pointer hasn't been assigned, this is an unused file slot
            break;
        }
    }
    struct fs_agnostic_file *new_file = &(open_files_table[next_open_slot]);
    new_file->ata_unit = ata_unit;
    new_file->ata_type = ata_type;
    new_file->mode = mode;
    strcpy(new_file->path, path);
    switch (ata_type) {
        case ISO:
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

int32_t fs_open(const char *path, const char *mode) {
    //checks both owner permissions (dummy true) and process permissions
    int32_t success = fs_security_check(path);
    if (success < 1) {
        return success;
    }
    if (!fs_check_open_conflict(path, mode)) {
        return ERR_OPEN_CONFLICT;
    }
    int imode = mode_str_to_int(mode);
    if (imode == 0) {
        return ERR_BAD_MODE;
    }

    //strip path name for media
    if (path[0] != '/') {
        return ERR_BAD_PATH;
    }
    if (path[1] != '0' && path[1] != '1' && path[1] != '2' && path[1] != '3') {
        return ERR_BAD_PATH;
    }
    if (path[2] != '/') {
        return ERR_BAD_PATH;
    }

    char media_path[strlen(path) - 1];   //Two fewer than whole path length, + 1 for null terminator
    strcpy(media_path, path+2);

    int ata_unit = path[1] - '0';

    enum ata_kind ata_type = map_media_to_driver_id(ata_unit);

    int next_fd;
    //find current's next available fd by 'files' member
    //add to process file->file table
    bool opened = 0;
    for (next_fd = 0; opened == 0 && next_fd < PROCESS_MAX_OPEN_FILES; next_fd++) {
        // See if the process's file descriptor table has an unopen slot
        if (!current->fd_table[next_fd].is_open) {
            //make new sys level file
            //mark it with the ata type
            //the ata unit
            //and a pointer to the union of below
            //sys level file types

            // Create a fs_agnostic_file and give its reference to the process fd_table
            current->fd_table[next_fd].ptr = create_fs_agnostic_file(ata_type, ata_unit, media_path, imode);
            if (current->fd_table[next_fd].ptr) {
                current->fd_table[next_fd].is_open = 1;
            } else {
                return ERR_KERNEL_OPEN_FAIL;
            }
        } else if (next_fd == PROCESS_MAX_OPEN_FILES - 1) {
            return ERR_FDS_EXCEEDED;
        }
    }

    return next_fd;
}

int32_t fs_close(uint32_t fd) {
    if (fd >= PROCESS_MAX_OPEN_FILES) {
        return ERR_FD_OOR;
    }

    struct fs_agnostic_file *fp = current->fd_table[fd].ptr;
    if (fp) {
        switch (fp->ata_type) {
            case 1:  //ISO
                iso_fclose((struct iso_file *)fp);
                break;
            default:
                return ERR_BAD_ATA_KIND;
        }
        fp->filep = 0;
        current->fd_table[fd].ptr = 0;
        current->fd_table[fd].is_open = 0;
        return 0;
    } else {
        return ERR_WAS_NOT_OPEN;
    }
}

int32_t fs_read(char *dest, uint32_t bytes, uint32_t fd) {
    int bytes_read = 0;
    if (fd >= PROCESS_MAX_OPEN_FILES) {
        return ERR_FD_OOR;
    }

    struct fs_agnostic_file *fp = current->fd_table[fd].ptr;
    if (!fp || current->fd_table[fd].is_open == 0) {
        return ERR_WAS_NOT_OPEN;
    }
    //must be okay with security and be allowed to read
    if (!fs_security_check(fp->path)) {
        return ERR_NO_ALLOWANCE;
    }
    if (!(fp->mode & READ)) {
        return ERR_BAD_ACCESS_MODE;
    }
    switch (fp->ata_type) {
        case ISO:  //ISO
            bytes_read = iso_fread(dest, 1, bytes, (struct iso_file *)fp->filep);
            fp->at_EOF = ((struct iso_file *)fp->filep)->at_EOF;
            break;
        default:
            return ERR_BAD_ATA_KIND;
    }

    return bytes_read;
}

int32_t fs_write(const char *src, uint32_t bytes, uint32_t fd) {
    // TODO: we'll need a writable system.
    return ERR_BAD_ACCESS_MODE;
}

bool fs_owner_check(const char *path) {
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
        if (strcmp(allowed->path, truncated_path) == 0 &&
           (path[allowed_path_len] == '/' || path[allowed_path_len] == 0)
           ) {
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

bool fs_allowance_check(const char *path) {
    struct list_node *iterator = current->fs_allowances_list.head;
    //iterate over list of allowances until we reach
    //an acceptable one. otherwise return 0;
    while (iterator) {
        if (path_permitted_by_allowance(path, (struct fs_allowance *)iterator)) {
            return 1;
        } else {
            iterator = iterator->next;
        }
    }
    return 0;
}

int32_t fs_security_check(const char *path) {
    if (!fs_owner_check(path)) {
        return ERR_NOT_OWNER;
    }

    if (!fs_allowance_check(path)) {
        return ERR_NO_ALLOWANCE;
    }
    return 1;
}

// Copies one fs allowance to another piece by piece
void copy_fs_allowance(struct fs_allowance *to, const struct fs_allowance *from) {
    to->do_allow_below = from->do_allow_below;
    strcpy(to->path, from->path);
    // DO NOT copy the struct list_node info... this would make this fs_allowance
    // behave as if it is in the linked list of allowances that the argument from
    // is in.
}

void fs_copy_allowances_list(struct list *to, struct list *from) {
    struct list_node *iterator;
    for (iterator = from->head; iterator != 0; iterator = iterator->next) {
        struct fs_allowance *new_allowance = kmalloc(sizeof(*new_allowance));
        copy_fs_allowance(new_allowance, (struct fs_allowance *)iterator);
        list_push_head(to, (struct list_node *)new_allowance);
    }
}

void fs_free_allowances_list(struct list *to_free) {
    struct list_node *n;
    while (to_free->head != 0) {
        n = list_pop_head(to_free);
        kfree((struct fs_allowance *) n);
    }
}
