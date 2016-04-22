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
#include "console.h"
#include "window_manager.h"

static uint8_t mouse_cycle = 0;
static uint8_t mouse_byte[3];
struct graphics_color mouse_fg_color = {0, 255, 0};
static bool mouse_enabled = 1;

void set_mouse_fg_color(uint8_t r, uint8_t g, uint8_t b) {
    mouse_fg_color.r = r;
    mouse_fg_color.g = g;
    mouse_fg_color.b = b;
}

bool get_mouse_enabled() {
	return mouse_enabled;
}

void set_mouse_enabled(bool enable) {
    mouse_enabled = enable;
}

int mouse_scan() {
    // see if data to be read on PS2_DATA_PORT
    ps2_controller_read_ready();
    int data = inb(PS2_DATA_PORT);
    iowait();

    return data;
}

// maps user mouse bytes to buttons and movements; returns 1 if mouse position changed, 0 otherwise
// TODO: handle button events (e.g., downclick and release, doubleclick)
int mouse_map() {
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

    old_mouse_x = mouse_x;
    old_mouse_y = mouse_y;

    mouse_x += byte2;
    // negative values are down with mouse offsets (opposite of graphics
    mouse_y -= byte3;

    // keep mouse coords in bounds
    mouse_x = mouse_x < 0 ? 0 : mouse_x;
    mouse_x = mouse_x > graphics_width() - 1 ? graphics_width() - 1 : mouse_x;
    mouse_y = mouse_y < 0 ? 0 : mouse_y;
    mouse_y = mouse_y > graphics_height() - 1 ? graphics_height() - 1 : mouse_y;

    if (old_mouse_x != mouse_x || old_mouse_y != mouse_y) {
        return 1;
    }
    return 0;
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
            if (mouse_enabled && mouse_map()) {
                graphics_mouse();
                //TODO: Filter out mouse events that happen
                // outside the current window
                send_event_mouse_move();
            }
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

    old_mouse_x = mouse_x;
    old_mouse_y = mouse_y;

    graphics_copy_to_color_buffer(mouse_x - MOUSE_SIDE_2 + 1, mouse_y - MOUSE_SIDE_2 + 1, MOUSE_SIDE - 1, MOUSE_SIDE - 1, mouse_draw_buffer, (MOUSE_SIDE - 1) * (MOUSE_SIDE - 1));

    interrupt_register(44, mouse_interrupt);
    mouse_scan();
    interrupt_enable(44);

    // set flag to flip buffers
    mouse_inited = 1;
    console_printf("mouse: ready\n");
}

