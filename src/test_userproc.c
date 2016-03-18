int main();
int _start() {
    return main();

    // invoke system call that kills the process
}

#include "syscall.h"

int main() {
    while (1) {
        testcall(37);
    }
    return 0;
}
