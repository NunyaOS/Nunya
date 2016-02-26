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

#include "memory_raw.h"
#include "kernelcore.h"

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

// writes "next byte" for a command that is 2 bytes
void ps2_command_write(uint8_t command, uint8_t data) {
    outb(command, PS2_COMMAND_REGISTER);
    ps2_controller_write_ready();
    outb(data, PS2_DATA_PORT);
}

uint8_t ps2_read_controller_config_byte() {
    // read controller config byte (0) from RAM
    outb(0x20, PS2_COMMAND_REGISTER);
    ps2_controller_read_ready();
    return inb(PS2_DATA_PORT);
}

void ps2_write_controller_config_byte(uint8_t cont_config_byte) {
    ps2_command_write(0x60, cont_config_byte);
}

