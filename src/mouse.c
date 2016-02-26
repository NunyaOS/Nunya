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
#include "graphics.h"

static uint8_t mouse_cycle = 0;
static uint8_t mouse_byte[3];
struct graphics_color mouse_fg_color = {0, 255, 0};
static int mouse_x;
static int mouse_y;
static uint8_t mouse_button;

int get_mouse_x() {
    return mouse_x;
}

int get_mouse_y() {
    return mouse_y;
}

int mouse_scan() {
    // see if data to be read on PS2_DATA_PORT
    ps2_controller_read_ready();
    int data = inb(PS2_DATA_PORT);
    iowait();

    return data;
}

// maps user mouse bytes to buttons and movements
// TODO: handle button events (e.g., downclick and release, doubleclick)
void mouse_map() {
    // signs for x and y movement (bytes 2 and 3); default to positive
    bool x_sign = 0;
    bool y_sign = 0;

    // handle byte 1
    int8_t byte1 = mouse_byte[0];
    if ((byte1 & 0x01) == 1) {
        // handle left button
    }
    if ((byte1 & 0x02) == 1) {
        // handle right button
    }
    if ((byte1 & 0x04) == 1) {
        // handle middle button
    }
    // mouse movement sign x
    if ((byte1 & 0x10) == 1) {
        x_sign = 1;
    }
    // mouse movement sign y
    if ((byte1 & 0x20) == 1) {
        y_sign = 1;
    }

    int8_t byte2 = mouse_byte[1];
    int8_t byte3 = mouse_byte[2];

    // neg sign bytes, if necessary
    byte2 = x_sign ? (byte2 | 0xFFFFFF00) : byte2;
    byte3 = y_sign ? (byte3 | 0xFFFFFF00) : byte3;
    mouse_x += byte2;
    // negative values are down with mouse offsets (opposite of graphics
    mouse_y -= byte3;
}

int mouse_request_packet() {
    // command to request a single packet
    ps2_command_write(0xEB , PS2_COMMAND_REGISTER);
    ps2_controller_read_ready();
    int ack = inb(PS2_DATA_PORT);
    if (ack != MOUSE_ACK) {
        // TODO: handle ack
        console_printf("mouse: bad ack: %x\n", ack);
    }
    ps2_controller_read_ready();
    return inb(PS2_DATA_PORT);
}

// handles mouse interrupts (IRQ 12); waits for every third byte to process
void mouse_interrupt() {
    // mice sends 3 bytes; only process on third byte
    // TODO: handle 4th byte (scroll wheel)
    switch(mouse_cycle) {
        case 0:
            mouse_byte[0] = inb(PS2_DATA_PORT);
            mouse_cycle++;
            break;
        case 1:
            mouse_byte[1] = inb(PS2_DATA_PORT);
            mouse_cycle++;
            break;
        case 2:
            mouse_byte[2] = inb(PS2_DATA_PORT);
            mouse_cycle++;
            mouse_cycle = 0;
            mouse_map();
            graphics_mouse();
            break;
    }

}

void mouse_init() {
    // enable port 2 and interrupts for port 2 (enable IRQ12)
    outb(0xA8, PS2_COMMAND_REGISTER);
    uint8_t cont_config_byte = ps2_read_controller_config_byte();
    // enable port 2
    cont_config_byte |= 0x02;
    // clear bit 5 (disable mouse clock)
    cont_config_byte &= ~0x08;
    ps2_write_controller_config_byte(cont_config_byte);
    // send set defaults command to mouse
    ps2_command_write(0xD4, 0xF6);
    ps2_controller_read_ready();
    int ack = inb(PS2_DATA_PORT);
    if (ack != MOUSE_ACK) {
        // TODO: handle bad ack
        console_printf("mouse: bad ack: %x\n", ack);
    }
    // enable packet streaming
    ps2_command_write(0xD4, 0xF4);
    ps2_controller_read_ready();
    ack = inb(PS2_DATA_PORT);
    if (ack != MOUSE_ACK) {
        // TODO: handle bad ack
        console_printf("mouse: bad ack: %x\n", ack);
    }

    // init pointer location to middle of screen
    mouse_x = graphics_width() / 2;
    mouse_y = graphics_height() / 2;


    interrupt_register(44, mouse_interrupt);
    mouse_scan();
    interrupt_enable(44);
    console_printf("mouse: ready\n");
}

