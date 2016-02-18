/*
Copyright (C) 2016 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef GRAPHICS_TEST_H
#define GRAPHICS_TEST_H

#include "clock.h"

/**
 * @brief The signature of graphics test functions
 * @details A graphics test is a function that represents a test of one graphics feature.
 * It should perform draw calls to the screen that attempt to cover the different use
 * cases of that feature.
 */
typedef void (*graphics_tester)(void);

struct graphics_test_runner {
	const char *title;
	graphics_tester *test;
	clock_t runtime_bound;
};

// The list of all graphics tests to be run when the graphics test suite is called upon
static struct graphics_test_runner graphics_tests[] = {};

/**
 * @brief Runs all graphics tests
 * @details Runs every test listed in graphics_tests, and compares the runtime to the expected
 */
void run_all_tests();

#endif