#include "fs_sys.h"
#include "string.h"
#include "kerneltypes.h"
#include "process.h"

uint32_t fs_sys_open(const char *path, const char *mode) {
    //checks both owner permissions (dummy true) and process permissions
    bool success = fs_sys_security_check(path);
    int fd = 0;
    //add to OS open files table

    //find current's next available fd by 'files' member
    //add to process file->file table

    return fd;
}

uint32_t fs_sys_close(uint32_t fd) {
    //remove from process's file->file table by fd

    //decrement OS open files table
        //if count is 0, remove from OS open files table
    return 0;
}

uint32_t fs_sys_read(char *dest, uint32_t bytes, uint32_t fd) {
    int bytes_read = 0;

    //assume okay on security for reading for now, until file table is fleshed out in process.
//    if (fs_sys_security_check(get_path_from_fd(fd)) {
//        
//    }

    //check file in table for reading mode

    //check media type from ata table
    //and call appropriate driver's version of read
        //dummy operation for now, always iso
    return bytes_read;
}

uint32_t fs_sys_write(const char *src, uint32_t bytes, uint32_t fd) {
    int bytes_written = -1;

    return bytes_written;
}

uint32_t fs_sys_add_allowance(const char *path, bool do_allow_below) {
    //must be under current paths with is_just_dir = 0;
    if (fs_sys_security_check(path)) {
        struct fs_allowance *new = kmalloc(sizeof(*new));

        //insert at head of list
        new->next = current->fs_allowances_head->next;
        current->fs_allowances_head->next = new;
        //set other needed values
        strcpy(new->path, path);
        new->do_allow_below = (bool)do_allow_below;
        return 1;
    }
    return 0;
}

uint32_t fs_sys_remove_allowance(const char *path) {
    //does not recursively remove allowances from fs
    //should recursively remove allowances from processes
    struct fs_allowance *iterator;
    struct fs_allowance *trailing_iterator = 0;
    
    uint32_t removals = 0;

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
        }
        trailing_iterator = iterator;
    }
    return removals;
}

bool fs_sys_owner_check(const char *path) {
    // TODO: This
    return 1;
}

/**
* Truncate a copy of the given path to match the lenght
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
bool fs_sys_security_check(const char *path) {
    return (fs_sys_owner_check(path) && fs_sys_allowance_check(path));
}

