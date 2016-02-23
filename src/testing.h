/*
Copyright (C) 2016 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef TESTING_H
#define TESTING_H

#include "clock.h"

/**
 * @brief The signature of a test function
 * @details A graphics test is a function that represents a test of one graphics feature.
 * It should perform draw calls to the screen that attempt to cover the different use
 * cases of that feature.
 * @return A value indicating whether the test was successful or failed
 */
typedef int (*test_func_t)(void);

struct test_unit {
    const char *title;
    test_func_t test;
    clock_t runtime_bound;
    // TODO: NUN-14 Add type or grouping so we can run specific groups of tests
};

/**
 * @brief Runs all tests
 * @details Runs every test listed in tests, and compares the runtime to the expected
 */
void run_all_tests();

#endif