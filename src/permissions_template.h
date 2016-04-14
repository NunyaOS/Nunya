/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef PERMISSIONS_TEMPLATE_H
#define PERMISSIONS_TEMPLATE_H

#include "process.h"

struct permissions_template {
    // The owner of the template
    struct process *owner;

    // File permissions
    struct list *directories;

    // Memory permissions
    int max_number_of_pages;

    // Window permissions, to override parents'
    int max_width;
    int max_height;
    int offset_x;
    int offset_y;
};

/**
 * @brief   Creates a new permissions template for a process to use.
 * @details Allocates a process permissions template, to be adjusted as needed
 *          before passed as a parameter to the run() syscall.
 *          The template will need to be freed by the process
 *          when no longer needed so as to stop counting toward the process'
 *          memory allocation.
 *
 * @return  The identifier of the newly created process identifier.
 */
uint32_t create_permissions_template();

/**
 * @brief   Deletes a process permissions template.
 * @details Deletes the process permissions template associated with the identifier.
 *          This function *DOES NOT* check the ownership of the template before deleting.
 *
 * @param   identifier The idenfitier of the process permissions template to be deleted.
 */
void delete_permissions_template(uint32_t identifier);

/**
 * @brief   Deletes all templates owned by the given process.
 * @details Use this function when a process ends in order to clean up its templates.
 *
 * @param   p The process whose templates should be deleted.
 */
void delete_templates_owned_by_process(struct process *p);

/**
 * @brief   Creates a set of process_permissions corresponding to a permissions template.
 * @details Using the information stored in the template, creates and returns a pointer to a
 *          process_permissions struct to be associated with a process. It will need to be
 *          freed upon conclusion of use (typically when the process storing the reference to it ends).
 *
 * @param   identifier The identifier for the template to be used for permissions
 *          creation.
 * @return  The process_permissions struct with the necessary information.
 */
struct process_permissions *permissions_from_identifier(uint32_t identifier);

/**
 * @brief   Returns the template associated with the given identifier
 * @details Looks up and returns a pointer to the template in the table. This
 *          function is for kernel use only, for other modules to use to
 *          manipulate the template struct. Users will manipulate it through
 *          other syscalls. This function does not check the ownership of the
 *          template before returning.
 *
 * @param   identifier The identifier for the template to return.
 * @return  A pointer to the desired template, or 0 if not found.
 */
struct permissions_template *template_for_identifier(uint32_t identifier);

/**
 * @brief   Checks if a template is owned by a process.
 * @details Accesses the template table to check if the given identifier is
 *          owned by the given process.
 *
 * @param   identifier The identifier of the template to check.
 * @param   process The process to check ownership of.
 *
 * @return  1 if the process owns the template and 0 otherwise.
 */
int template_owned_by_process(uint32_t identifier, struct process *p);

#endif
