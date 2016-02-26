/*
Copyright (C) 2016 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "testing.h"

#include "console.h"

// The tests will be defined in a separate file
extern struct test_unit tests[];
extern int tests_size;

// A flag used to record the return value of the most recent test
static int recent_test_result = 1;

// A function to time a specific test
// Returns the amount of time that elapsed during the test
clock_t time_test(test_func_t test) {
    clock_t start = clock_read();
    recent_test_result = (test)();
    clock_t end = clock_read();
    return clock_diff(start, end);
}

// A test runner
// Returns true if a test was executed in a passing amount of time
int run_test(struct test_unit *runner) {
    clock_t actual_time = time_test(runner->test);
    if(clock_compare(actual_time, runner->runtime_bound) > 0 || !recent_test_result) {
        // The test failed
        console_printf("Test %s FAILED with a time of %d.%ds\n", runner->title, actual_time.seconds, actual_time.millis);
        return 0;
    } else {
        console_printf("Test %s passed with a time of %d.%ds\n", runner->title, actual_time.seconds, actual_time.millis);
        return 1;
    }
}

void run_all_tests() {
    console_printf("Running %d tests\n", tests_size);
    int i;
    int failed = 0;
    for(i = 0; i < tests_size; i += 1) {
        if(!run_test(&(tests[i]))) {
            failed += 1;
        }
    }

    if(failed > 0) {
        console_printf("Tests failed with %d failures\n", failed);
    } else {
        console_printf("All tests passed\n");
    }
}