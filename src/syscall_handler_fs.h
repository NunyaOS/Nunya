/*
Copyright (C) 2016 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef SYSCALL_HANDLER_FS_H
#define SYSCALL_HANDLER_FS_H 

#include "kerneltypes.h"

int32_t sys_fs_open(const char *path, const char *mode);

int32_t sys_fs_close(uint32_t fd);

int32_t sys_fs_read(char *dest, uint32_t bytes, uint32_t fd);

int32_t sys_fs_write(const char *src, uint32_t bytes, uint32_t fd);

#endif
