/*
Copyright (C) 2016 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "graphics_test.h"

#include "console.h"

// A function to time a specific test
// Returns the amount of time that elapsed during the test
clock_t time_test(graphics_tester *test) {
	clock_t start = clock_read();
	(*test)();
	clock_t end = clock_read();
	return clock_diff(start, end);
}

// A test runner
// Returns true if a test was executed in a passing amount of time
int run_test(struct graphics_test_runner runner) {
	clock_t actual_time = time_test(runner.test);
	if(clock_compare(actual_time, runner.runtime_bound) < 0) {
		// The test failed
		console_printf("Test %s FAILED with a time of %d.%ds\n", runner.title, actual_time.seconds, actual_time.millis);
		return 0;
	} else {
		console_printf("Test %s passed with a time of %d.%ds\n", runner.title, actual_time.seconds, actual_time.millis);
		return 1;
	}
}

void run_all_tests() {
	int size = sizeof(graphics_tests);
	int i;
	int failed = 0;
	for(i = 0; i < size; i += 1) {
		if(!run_test(graphics_tests[i])) {
			failed += 1;
		}
	}

	if(failed > 0) {
		console_printf("Graphics tests failed with %d failures\n", failed);
	} else {
		console_printf("Graphics tests all passed\n");
	}
}