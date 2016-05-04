void main();
void _start() {
    main();
}

#include "syscall.h"

void main() {
    int i = 0;
    for (i = 1; i < 11; i+=2) {
        debug_print(i);
        int j;
        for (j = 0; j < 500000000; j++) {} // busywait
    }

    exit(0);
}
