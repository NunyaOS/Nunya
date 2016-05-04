int main();
int _start() {
    return main();
}

#include "syscall.h"

int main() {
    create_window(10, 10, 400, 400);
    // ls usr
    // --> jesse, ryan, alex
    // restrict /usr/jesse
    // ls usr
    // --> nothing
    struct graphics_color c = {0,255,0};
    struct graphics_color bg = {0,0,0};

    int fd = open("/3/USR/USERS.TXT", "r");
    if (fd < 0) {
        draw_string(10, 170, "Failed to open file.\n", &c, &bg);
        exit(0);
    } else {
        draw_string(10, 150, "Opened file successfully.\n", &c, &bg);
        uint32_t identifier = permissions_capability_create();
        permissions_capability_add_allowance(identifier, "/3/BIN", 1);
        permissions_capability_remove_allowance(identifier, "/3/");
        run("/BIN/TEST_FS_.NUN", identifier);
    }

    close(fd);
    exit(0);

    return 0;
}
