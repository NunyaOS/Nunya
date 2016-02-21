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
typedef int (*tester)(void);

struct test_runner {
	const char *title;
	tester test;
	clock_t runtime_bound;
	// TODO: Add type or grouping so we can run specific groups of tests
};

// The list of all tests to be run when the graphics test suite is called upon
static struct test_runner tests[] = {
};

/**
 * @brief Runs all tests
 * @details Runs every test listed in tests, and compares the runtime to the expected
 */
void run_all_tests();

#endif