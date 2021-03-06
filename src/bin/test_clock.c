int main();
int _start() {
    return main();

    // invoke system call that kills the process
}

#include "syscall.h"
#define CHAR_SIZE 8

#define DRAW_OFFSET 10
clock_t clock_diff(clock_t start, clock_t stop) {
    clock_t result;
    if (stop.millis < start.millis) {
        stop.millis += 1000;
        stop.seconds -= 1;
    }
    result.seconds = stop.seconds - start.seconds;
    result.millis = stop.millis - start.millis;
    return result;
}

int main() {
    int curr_char = DRAW_OFFSET;
    struct rtc_time t;
    struct graphics_color fgc = {255, 255, 255};
    struct graphics_color bgc = {0, 0, 0};
    create_window(250, 250, 100, 30);
    while (1) {
        sleep(1000);
        read_rtc(&t);
        clear();
        if (t.hour > 9) {
            draw_char(0 + DRAW_OFFSET, curr_char, (t.hour / 10) + '0', &fgc, &bgc);
        }
        else {
            draw_char(0 + DRAW_OFFSET, curr_char, '0', &fgc, &bgc);
        }
        draw_char(CHAR_SIZE + DRAW_OFFSET, curr_char, (t.hour % 10) + '0', &fgc, &bgc);
        draw_char((2 * CHAR_SIZE) + DRAW_OFFSET, curr_char, ':', &fgc, &bgc);
        if (t.minute > 9) {
            draw_char((3 * CHAR_SIZE) + DRAW_OFFSET, curr_char, (t.minute / 10) + '0', &fgc, &bgc);
        }
        else {
            draw_char((3 * CHAR_SIZE) + DRAW_OFFSET, curr_char, '0', &fgc, &bgc);
        }
        draw_char((4 * CHAR_SIZE) + DRAW_OFFSET, curr_char, (t.minute % 10) + '0', &fgc, &bgc);
        draw_char((5 * CHAR_SIZE) + DRAW_OFFSET, curr_char, ':', &fgc, &bgc);
        if (t.second > 9) {
            draw_char((6 * CHAR_SIZE) + DRAW_OFFSET, curr_char, (t.second / 10) + '0', &fgc, &bgc);
        }
        else {
            draw_char((6 * CHAR_SIZE) + DRAW_OFFSET, curr_char, '0', &fgc, &bgc);
        }
        draw_char((7 * CHAR_SIZE) + DRAW_OFFSET, curr_char, (t.second % 10) + '0', &fgc, &bgc);
        // curr_char += CHAR_SIZE;
    }
    return 0;
}

