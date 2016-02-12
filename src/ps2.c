/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "interrupt.h"
#include "ps2.h"
#include "clock.h"
#include "ioports.h"
#include "console.h"
#include "string.h"

static int second_channel_enabled = 0;

int ps2_second_channel_enabled() {
    return second_channel_enabled;
}

// for reading from data port. checks PS2_STATUS_REGISTER to see if data port full
int ps2_controller_read_ready() {
    clock_t start;
    clock_t elapsed = {0, 0};
    start = clock_read();
    // wait until bit 0 of status register is 1 (data port full)
    while ((inb(PS2_STATUS_REGISTER) & 0x1) != 1 && elapsed.seconds < PS2_TIMEOUT) {
        elapsed = clock_diff(start, clock_read());
    }
    if (elapsed.seconds > PS2_TIMEOUT) {
        console_printf("ps/2: timeout\n");
        return 0;
    }
    return 1;
}

// for writing 2 byte commands. checks PS2_STATUS_REGISTER to see if data port empty
int ps2_controller_write_ready() {
    clock_t start;
    clock_t elapsed = {0, 0};
    start = clock_read();
    // wait until bit 1 of status register is 0 (data port empty)
    while (((inb(PS2_STATUS_REGISTER) >> 1) & 0x1) != 0 && elapsed.seconds < PS2_TIMEOUT) {
        elapsed = clock_diff(start, clock_read());
    }
    if (elapsed.seconds > PS2_TIMEOUT) {
        console_printf("ps/2: timeout\n");
        return 0;
    }
    return 1;
}

uint8_t ps2_controller_config_byte() {
    // read controller config byte (0) from RAM
    outb(0x20, PS2_COMMAND_REGISTER);
    // TODO: check for timeout
    ps2_controller_read_ready();
    return inb(PS2_DATA_PORT);
}

// writes "next byte" for a command that is 2 bytes
void ps2_command_write(uint8_t command, uint8_t data) {
    outb(command, PS2_COMMAND_REGISTER);
    ps2_controller_write_ready();
    outb(data, PS2_DATA_PORT);
}

void ps2_init() {
    // need to initialize USB controllers and disable USB legacy support if USB implemented
    // if ACPI implemented need to determine if PS/2 controller exists

    // disable devices so that any PS/2 devices can't send data at the wrong time and mess up initialization
    outb(0xAD, PS2_COMMAND_REGISTER);
    outb(0xA7, PS2_COMMAND_REGISTER);

    // flush the output buffer of ps2 data port
    inb(PS2_DATA_PORT);

    // block interrupts
    interrupt_block();

    uint8_t cont_config_byte = ps2_controller_config_byte();

    // disable IRQs and port translation (bits 0, 1, 6)
    cont_config_byte &= ~((1) | (1 << 1) | (1 << 6));
    // if clear, not dual channel PS/2 controller (b/c second PS/2 port disabled)
    if (((cont_config_byte >> 5) & 0x01) == 1) {
        second_channel_enabled = 1;
    }

    ps2_command_write(0x60, cont_config_byte);

    // test PS/2 controller
    outb(0xAA, PS2_COMMAND_REGISTER);
    // TODO: check for timeout
    ps2_controller_read_ready();
    int test = inb(PS2_DATA_PORT);
    if (test == 0x55) {
        console_printf("ps/2: test successful\n");
    }
    else if (test == 0xFC) {
        console_printf("ps/2: test unsuccessful\n");
    }
    else {
        console_printf("ps/2: test failed with unknown error: %x\n", test);
    }

    if (second_channel_enabled == 1) {
        // enable second PS/2 port
        outb(0xA8, PS2_COMMAND_REGISTER);
        cont_config_byte = ps2_controller_config_byte();
        if (((cont_config_byte >> 5) & 0x01) != 0) {
            
        }
    }

    // unblock interrupts
    interrupt_unblock();
}

