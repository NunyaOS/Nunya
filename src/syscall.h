/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef SYSCALL_H
#define SYSCALL_H

#include "kerneltypes.h"

#define SYSCALL_exit     1
#define SYSCALL_testcall 2
#define SYSCALL_yield    3

#define SYSCALL_open     601
#define SYSCALL_close    602
#define SYSCALL_read     603
#define SYSCALL_write    604

uint32_t syscall(uint32_t n, uint32_t a, uint32_t b, uint32_t c, uint32_t d,
                 uint32_t e);

#include "sys_fs.h"

#endif
