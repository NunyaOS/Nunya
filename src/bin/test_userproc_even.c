int main();
int _start() {
    return main();

    // invoke system call that kills the process
}

#include "../syscall.h"

int main() {
    int has_spawned = 0;
    while(1) {
        int i = 0;
        for(i = 0; i < 10; i+=2) {
            testcall(i);
            int j;
            for(j = 0; j < 100000000; j++) {} // busywait
        }
        if (!has_spawned) {
            // runproc_odd();
            has_spawned = 1;
        } else {
            yield();
        }
    }

    return 0;
}
