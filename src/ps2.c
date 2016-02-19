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

void ps2_init() {
    // need to initialize USB controllers and disable USB legacy support if USB implemented
    // if ACPI implemented need to determine if PS/2 controller exists

    // disable devices so that any PS/2 devices can't send data at the wrong time and mess up initialization
    /*outb(0xAD, PS2_COMMAND_REGISTER);*/
    /*outb(0xA7, PS2_COMMAND_REGISTER);*/

    // flush the output buffer of ps2 data port
    inb(PS2_DATA_PORT);

    // block interrupts
    interrupt_block();

    uint8_t cont_config_byte = ps2_read_controller_config_byte();


    // disable IRQs and port translation (bits 0, 1, 6)
    cont_config_byte &= ~((1) | (1 << 1) | (1 << 6));
    // if clear, not dual channel PS/2 controller (b/c second PS/2 port disabled)
    if (((cont_config_byte >> 5) & 0x01) == 1) {
        second_channel_enabled = 1;
    }

    ps2_command_write(0x60, cont_config_byte);
    ps2_write_controller_config_byte(cont_config_byte);

    console_printf("free?: %d\n", memory_freemap_walk());
    /*
    // test PS/2 controller
    outb(0xAA, PS2_COMMAND_REGISTER);
    console_printf("free?: %d\n", memory_freemap_walk());
    ps2_controller_read_ready();
    console_printf("free?: %d\n", memory_freemap_walk());
    int test = inb(PS2_DATA_PORT);
    console_printf("free?: %d\n", memory_freemap_walk());
    if (test == 0x55) {
        console_printf("ps/2: controller test successful\n");
    }
    else if (test == 0xFC) {
        console_printf("ps/2: controller test unsuccessful\n");
    }
    else {
        console_printf("ps/2: controller test failed with unknown error: %x\n", test);
    }

    console_printf("free?: %d\n", memory_freemap_walk());
    */
    int test;
    // determine if there are 2 channels
    if (second_channel_enabled == 1) {
        // enable second PS/2 port
        outb(0xA8, PS2_COMMAND_REGISTER);
        cont_config_byte = ps2_read_controller_config_byte();
        // if the bit 5 is set, then PS/2 controller can't be dual channel (b/c second PS/2 port should be enabled)
        if (((cont_config_byte >> 5) & 0x01)) {
            second_channel_enabled = 0;
        }
        // redisable second PS/2 port
        else {
            outb(0xA7, PS2_COMMAND_REGISTER);
        }
    }
    // test PS/2 ports
    outb(0xAB, PS2_COMMAND_REGISTER);
    test = inb(PS2_DATA_PORT);
    if (test == 0x00) {
        console_printf("ps/2: first ps/2 port test successful\n");
    }
    else {
        console_printf("ps/2: first ps/2 port test unsuccessful (code %x)\n", test);
    }
    // only test second port if support enabled
    if (second_channel_enabled) {
        outb(0xA9, PS2_COMMAND_REGISTER);
        test = inb(PS2_DATA_PORT);
        if (test == 0x00) {
            console_printf("ps/2: second ps/2 port test successful\n");
        }
        else {
            console_printf("ps/2: second ps/2 port test unsuccessful (code %x)\n", test);
        }
    }

    // enable port 1 and 2
    outb(0xAE, PS2_COMMAND_REGISTER);
    if (second_channel_enabled) {
        outb(0xA8, PS2_COMMAND_REGISTER);
    }
    // enable interrupts for port 1 and 2
    cont_config_byte = ps2_read_controller_config_byte();
    cont_config_byte &= (1) | (1 << 1);
    ps2_write_controller_config_byte(cont_config_byte);

    // reset devices (1st then 2nd port)
    ps2_controller_write_ready();
    // send reset byte
    outb(0xFF, PS2_DATA_PORT);
    ps2_controller_read_ready();
    test = inb(PS2_DATA_PORT);
    if (test == 0xFA) {
        console_printf("ps/2: first ps/2 device self-test passed\n");
    }
    else {
        // TODO: handle failed/resend command
        console_printf("ps/2: first ps/2 device self-test failed (code %x)\n", test);
    }

    // 0xD4 command writes byte to second ps/2 input buffer
    ps2_command_write(0xD4, 0xFF);
    if (test == 0xFA) {
        console_printf("ps/2: second ps/2 device self-test passed\n");
    }
    else {
        // TODO: handle failed/resend command
        console_printf("ps/2: second ps/2 device self-test failed (code %x)\n", test);
    }

    // unblock interrupts
    interrupt_unblock();
}

