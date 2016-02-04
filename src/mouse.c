/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#define MOUSE_PORT 0x60
#define 

static struct miscdevice our_mouse = {

};
// IRQ12: mouse

static int mouse_scan() {
    int code;
    int ack;
    code = inb
}

void mouse_init() {
    interrupt_register(44, mouse_interrupt);
    if(chek
}


