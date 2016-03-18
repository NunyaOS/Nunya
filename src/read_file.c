int main();
int start() {
    main();
    while(1) {}
    return 0;
}

#include "syscall.h"

int main() {
    //start with root permissions, but only allow secondary ata units
    add_fs_allow("/2/", 1);
    add_fs_allow("/3/", 1);
    remove_fs_allow("/");

    //open file
    int fd = open("/3/GETTYSBU.TXT", "r");

    //testcall on fd, should be 0
    testcall(fd);

    int bytes_read;
    char buffer[27];

    //perform read
    bytes_read = read(buffer, 27, fd);
    int i = 0;
    while (i < 27) {
        testcall(buffer[i]);
        i++;
    }
    close(fd);

    //Just something to easily identify the gap by
    testcall(-40);

    //Remove allowances
    remove_fs_allow("/3/");
    fd = open("/3/GETTYSBU.TXT", "r");

    //-2 means no success on open, missing allowance
    testcall(fd);
    return 0;
}
