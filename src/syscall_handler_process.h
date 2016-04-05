/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "process.h"

#ifndef SYSCALL_HANDLER_PROCESS_H
#define SYSCALL_HANDLER_PROCESS_H

uint32_t sys_exit(uint32_t code);
uint32_t sys_yield();
uint32_t sys_run(const char *process_path, const uint32_t permissions_identifier, struct process *parent);

#endif