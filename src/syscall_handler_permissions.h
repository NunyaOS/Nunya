/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef SYSCALL_HANDLER_PERMISSIONS_H
#define SYSCALL_HANDLER_PERMISSIONS_H

#include "kerneltypes.h"

int32_t sys_capability_create();
void sys_capability_delete(uint32_t identifier);
int32_t sys_capability_fs_add_allowance(uint32_t identifier, const char *path, bool do_allow_below);
int32_t sys_capability_fs_remove_allowance(uint32_t identifier, const char *path);

#endif
