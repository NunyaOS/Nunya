/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

/**
 * @brief   Handle a pagefault exception
 * @details Handle a pagefault exception originated by the CPU. This function
 *          should *NEVER* be called directly.
 */
void exception_handle_pagefault(int intr, int code);
