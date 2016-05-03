int main();
int _start() {
    return main();
}

#include "syscall.h"

int main() {
    int i = 0;
    for (i = 0; i < 11; i+=2) {
        debug_print(i);
        int j;
        for (j = 0; j < 500000000; j++) {} // busywait
    }

    exit(0);

    return 0;
}
