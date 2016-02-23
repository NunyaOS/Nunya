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

static uint8_t mouse_cycle = 0;
static uint8_t mouse_byte[3];

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

// return requested packet (and check for an ack)
int mouse_request_packet() {
	// command to request a single packet
	ps2_command_write(0xEB , PS2_COMMAND_REGISTER);
	ps2_controller_read_ready();
    int ack = inb(PS2_DATA_PORT);
	if (ack != MOUSE_ACK) {
		// TODO: handle ack
		console_printf("mouse: bad ack: %x\n", ack);
		return ack;
	}
	ps2_controller_read_ready();
	return inb(PS2_DATA_PORT);
}

void mouse_interrupt() {
	console_printf("mouse int data\n");
	/*uint8_t data = inb(PS2_DATA_PORT);*/
    /*console_printf("mouse int data: %x\n", data);*/
    /*int command;*/
    /*command = mouse_map(mouse_scan());*/

    /*if (!command) {*/

    /*}*/
    /*process_wakeup(&queue);*/
}

void mouse_init() {
    // enable port 2 and interrupts for port 2 (enable IRQ12)
    outb(0xA8, PS2_COMMAND_REGISTER);
    uint8_t cont_config_byte = ps2_read_controller_config_byte();
	// enable port 2
    cont_config_byte |= (1 << 1);
    // clear bit 5 (disable mouse clock)
    cont_config_byte &= ~(1 << 3);
    ps2_write_controller_config_byte(cont_config_byte);
	// send set defaults command to mouse
    ps2_command_write(0xD4, 0xF6);
    ps2_controller_read_ready();
    int ack = inb(PS2_DATA_PORT);
	if (ack != MOUSE_ACK) {
		// TODO: handle ack
		console_printf("mouse: bad ack: %x\n", ack);
	}
	// disable packet streaming (F4 to enable)
	ps2_command_write(0xD4, 0xF4);
    ps2_controller_read_ready();
    ack = inb(PS2_DATA_PORT);
	if (ack != MOUSE_ACK) {
		// TODO: handle ack
		console_printf("mouse: bad ack: %x\n", ack);
	}

	interrupt_register(44, mouse_interrupt);
	mouse_scan();
	interrupt_enable(44);
    console_printf("mouse: ready\n");
}


// try: http://forum.osdev.org/viewtopic.php?t=10247

// 0 is read ready; 1 is write ready
inline void mouse_wait(uint8_t a_type) {
	unsigned int time_out=100000; //unsigned int
	if(a_type==0) {
		// data
		while(time_out--) {
			if((inb(PS2_STATUS_REGISTER) & 1)==1) {
				return;
			}
		}
		return;
	}
	else {
		// signal
		while(time_out--) {
			if((inb(0x64) & 2)==0) {
				return;
			}
		}
		return;
	}
}

inline void mouse_write(uint8_t a_write) {
	//Wait to be able to send a command
	mouse_wait(1);
	//Tell the mouse we are sending a command
	outb(0xD4, PS2_COMMAND_REGISTER);
	//Wait for the final part
	mouse_wait(1);
	//Finally write
	outb(a_write, PS2_DATA_PORT);
}

uint8_t mouse_read() {
	//Get's response from mouse
	mouse_wait(0);
	return inb(PS2_DATA_PORT);
}

void m_interrupt() {
	console_printf("mouse int data\n");
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
			break;
	}
}


void mouse_init_2() {
	uint8_t status;

	// enable auxiliary mouse device
	mouse_wait(1);
    outb(0xA8, PS2_COMMAND_REGISTER);

	// enable interrupts
	mouse_wait(1);
	outb(0x20, PS2_COMMAND_REGISTER);
	mouse_wait(0);
	// set bit 1
	status = inb(PS2_DATA_PORT) | 2;
	mouse_wait(1);
	// write cont config byte
	outb(0x60, PS2_COMMAND_REGISTER);
	mouse_wait(1);
	outb(status, PS2_DATA_PORT);

	// tell mouse to use default settings
	mouse_write(0xF6);
	mouse_read();

	// enable the mouse
	mouse_write(0xF4);
	mouse_read();


    interrupt_register(44, m_interrupt);
    interrupt_enable(44);
    console_printf("mouse2: ready\n");
}

