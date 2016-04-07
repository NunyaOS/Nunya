/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "process.h"

#ifndef SYS_PROCESS_LIFECYCLE_H
#define SYS_PROCESS_LIFECYCLE_H

uint32_t sys_exit(uint32_t code);
uint32_t sys_yield();
uint32_t sys_run(char *process_path, struct process_permissions *child_permissions, struct process *parent);

#endif