int main();
int _start() {
    return main();

    // invoke system call that kills the process
}

#define CHAR_SIZE 8
#include "syscall.h"

int main() {
    create_window(5, 10, 600, 400);
    struct graphics_color c = {0,255,0};
    struct graphics_color bg = {0,0,0};
    int x = 0;
    int y = 0;

	int off_x = get_current_offset_x();
	int off_y = get_current_offset_y();
	int max_width = get_current_max_width();
	int max_height = get_current_max_height();

	uint32_t identifier = permissions_capability_create();

		draw_string(x, y, "wtf yo\n", &c, &bg);
	// should fail as it is too big
	if (capability_set_window(identifier, off_x/2, off_y/2, max_width, max_height) < 0) {
		draw_string(x, y, "first capability call failed\n", &c, &bg);
	}
	else {
		draw_string(x, y, "first capability call succeeded (bad)\n", &c, &bg);
	}
	y += CHAR_SIZE;

	if (capability_set_window(identifier, off_x + 60, off_y + 30, max_width / 2, max_height / 2) < 0) {
		draw_string(x, y, "second capability call failed (bad)\n", &c, &bg);
	}

	run("test_clock", identifier);
}

