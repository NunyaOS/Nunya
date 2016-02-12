/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/



#define MOUSE_DATA_PORT 0x60
// used to check if data on port 0x60 is for the mouse or the keyboard
#define MOUSE_CHECK_PORT 0x64

static struct list queue = { 0, 0 };
// send 0xA8 to enable PS2/port; 0xA7 to disable
// 0xA9 to test second (0x00 if test passed)
// 0x20 to read byte 0 (controller config byte) from internal RAM of PS/2 controller
// 0x60 to write byte to byte 0 (controller config byte) of internal RAM of PS2 Controller
// bit 1: second PS/2 port interrupt (1 enabled)
// bit 7 must be zero
static int mouse_scan() {
    int code;
    int data;
    code = inb(MOUSE_DATA_PORT);
    iowait();
    data = inb(MOUSE_CHECK_PORT);
    iowait();

    // see if data to be read on MOUSE_DATA_PORT
    if (((code >> 1) & 0x01) == 0) {
        return -1;
    }

    // mouse bit
    if (((data >> 5) & 0x01) != 0) {
        return -1;
    }
    return code;
}

void mouse_interrupt() {
    int command;
    command = mouse_map(mouse_scan());
    process_wakeup(&queue);
}

void mouse_init() {
    interrupt_register(44, mouse_interrupt);
    mouse_scan();
    interrupt_enable(44);
    console_printf("mouse: read\n");
}


