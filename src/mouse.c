/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "mouse.h"
#include "interrupt.h"
#include "console.h"
#include "ps2.h"
#include "ioports.h"
#include "process.h"
#include "graphics.h"

static struct list queue = { 0, 0 };

int mouse_scan() {
    int code;
    int data;
    data = inb(PS2_DATA_PORT);
    iowait();

    // see if data to be read on MOUSE_DATA_PORT
    ps2_controller_read_ready();
    if (((data >> 1) & 0x01) == 0) {
        return -1;
    }

    // mouse bit (assume no polling)
    /*if (((data >> 5) & 0x01) != 0) {*/
        /*return -1;*/
    /*}*/
    console_printf("scanning: %d\n", data);
    return data;
}

int mouse_map() {
    return 0;
}

void mouse_interrupt() {
    /*int data = inb(MOUSE_DATA_PORT);*/
    int command;
    /*command = mouse_map(mouse_scan());*/

    /*if (!command) {*/

    /*}*/
    /*process_wakeup(&queue);*/
    console_printf("mouse int\n");
}

void mouse_init() {
    // enable port 2 and interrupts for port 2 (enable IRQ12)
    outb(0xA8, PS2_COMMAND_REGISTER);
    uint8_t cont_config_byte = ps2_read_controller_config_byte();
    // set bit 1 and clear bit 5
    console_printf("CONT_CONFIG_BYTE: %d\n", cont_config_byte);
    cont_config_byte |= (1 << 1);
    console_printf("CONT_CONFIG_BYTE: %d\n", cont_config_byte);
    // set dat
    cont_config_byte &= ~(1 << 3);
    console_printf("CONT_CONFIG_BYTE: %d\n", cont_config_byte);
    ps2_write_controller_config_byte(cont_config_byte);
    ps2_command_write(0xD4, 0xF6);
    ps2_controller_read_ready();
    int ack = inb(PS2_DATA_PORT);
    ps2_command_write(0xD4, 0xF4);

    interrupt_register(44, mouse_interrupt);
    mouse_scan();
    interrupt_enable(44);
    console_printf("mouse: ready\n");
}

