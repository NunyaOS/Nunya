void main();
void _start() {
    main();
}

#include "syscall.h"

void main() {
    create_window(10, 10, 400, 400);
    struct graphics_color c = {0,255,0};
    struct graphics_color b = {0,0,255};
    struct graphics_color bg = {0,0,0};

    int fd = open("/3/USR/USERS.TXT", "r");
    if (fd < 0) {
        draw_string(10,160, "Attempting to open /USR/USERS.TXT in read mode.\n", &c, &bg);
        draw_string(10, 170, "Failed to open file.\n", &c, &bg);
        exit(0);
    } else {
        draw_string(10,120, "Attempting to open /USR/USERS.TXT in read mode.\n", &c, &bg);
        draw_string(10, 130, "Opened file successfully.\n", &c, &bg);
        int bytes_read;
        int cum_bread = 0;
        char buffer[45];
        int reads = 0;
        //perform read
        while (cum_bread < 44) {
            bytes_read = read(buffer, 44, fd);
            cum_bread += bytes_read;
            buffer[bytes_read] = '\0';
            draw_string(10, 140 + reads * 12, buffer, &b, &bg);
            reads++;
        }
        uint32_t identifier = permissions_capability_create();
        draw_string(10,150, "Now restricting access to only /BIN\n", &c, &bg);
        permissions_capability_add_allowance(identifier, "/3/BIN", 1);
        permissions_capability_remove_allowance(identifier, "/3/");
        run("/BIN/TEST_FS_.NUN", identifier);
        permissions_capability_delete(identifier);
    }

    close(fd);
    exit(0);
}
