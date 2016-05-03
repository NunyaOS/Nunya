int main();
int _start() {
    return main();

    // invoke system call that kills the process
}

#include "syscall.h"

int main() {
    create_window(10, 10, 400, 400);
    struct graphics_color c = {0,255,0};
    struct graphics_color bg = {0,0,0};
    struct event e;
    int x = 0;
    int count = 0;
    while (1) {
        int res = get_event(&e);
        if (res == 0) {
            if (count > 10) {
                clear();
                count = 0;
                x = 0;
            }
            if (e.type == EVENT_KEYBOARD_PRESS) {
                draw_char(x, 0, e.character, &c, &bg);
                x += 10;
                count++;
            }
        }
    }

    return 0;
}
