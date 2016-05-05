/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "process.h"
#include "console.h"
#include "memory.h"
#include "string.h"
#include "list.h"
#include "x86.h"
#include "interrupt.h"
#include "memorylayout.h"
#include "kernelcore.h"
#include "graphics.h"

#include "fs.h" //struct process->files
#include "memory_raw.h" // memory_alloc_page, memory_free_page

#include "permissions_capabilities.h"

struct process *current = 0;
struct list ready_list = { 0, 0 };

static uint32_t pid_count = 1;

extern uint32_t last_interrupt;

void process_init() {
    // Create a dummy process with no code and no data, and load its pagetable
    // Even though it's dummy, at least kernel memory is direct mapped, so
    // kernel code can run as usual
    current = process_create(0, 0);
    pagetable_load(current->pagetable);

    // Enable paging
    pagetable_enable();

    current->state = PROCESS_STATE_READY;

    // establish initial permissions
    struct process_permissions *initial_permissions = kmalloc(sizeof(struct process_permissions));
    initial_permissions->max_number_of_pages = memory_pages_total();
    initial_permissions->max_width = graphics_width();
    initial_permissions->max_height = graphics_height();
    initial_permissions->offset_x = 0;
    initial_permissions->offset_y = 0;

    // todo: the rest of the permissions
    struct list l = LIST_INIT;
    struct fs_allowance *f = kmalloc(sizeof(*f));
    strcpy(f->path, "/3/");
    f->do_allow_below = 1;
    list_push_head(&l, (struct list_node*)f);
    initial_permissions->fs_allowances = l;

    fs_copy_allowances_list(&(current->fs_allowances_list), &l);

    console_printf("total pages: %d\n", memory_pages_total());
    current->permissions = initial_permissions;

    console_printf("process %d: ready\n", current->pid);
}

static void process_stack_init(struct process *p) {
    struct x86_stack *s;

    p->state = PROCESS_STATE_CRADLE;

    p->kstack_top = p->kstack + PAGE_SIZE - sizeof(*s);
    p->stack_ptr = p->kstack_top;

    s = (struct x86_stack *)p->stack_ptr;

    s->regs2.ebp = (uint32_t)(p->stack_ptr + 28);
    s->old_ebp = (uint32_t)(p->stack_ptr + 32);
    s->old_addr = (unsigned)intr_return;
    s->ds = X86_SEGMENT_USER_DATA;
    s->cs = X86_SEGMENT_USER_CODE;
    s->eip = p->entry;
    s->eflags.interrupt = 1;
    s->eflags.iopl = 3;
    s->esp = PROCESS_STACK_INIT;
    s->ss = X86_SEGMENT_USER_DATA;
}

struct process *process_create(unsigned code_size, unsigned stack_size) {
    struct process *p;

    p = (struct process *)memory_alloc_page(1);
    p->pid = pid_count++;

    p->pagetable = pagetable_create();
    pagetable_init(p->pagetable);
    pagetable_alloc(p->pagetable, PROCESS_ENTRY_POINT, code_size,
                    PAGE_FLAG_USER | PAGE_FLAG_READWRITE);
    pagetable_alloc(p->pagetable, PROCESS_STACK_INIT - stack_size, stack_size,
                    PAGE_FLAG_USER | PAGE_FLAG_READWRITE);

    p->kstack = memory_alloc_page(1);
    p->entry = PROCESS_ENTRY_POINT;

    struct list l = LIST_INIT;
    p->fs_allowances_list = l;

    fs_init_security(p);

    process_stack_init(p);

    p->window = 0;

    return p;
}

static void process_switch(int newstate) {
    interrupt_block();

    if (current) {
        if (newstate == PROCESS_STATE_GRAVE) {
            process_cleanup(current);
        } else if (current->state != PROCESS_STATE_CRADLE) {
            asm("pushl %ebp");
            asm("pushl %edi");
            asm("pushl %esi");
            asm("pushl %edx");
            asm("pushl %ecx");
            asm("pushl %ebx");
            asm("pushl %eax");
            asm("movl %%esp, %0":"=r"(current->stack_ptr));
        }
        interrupt_stack_pointer = (void *)INTERRUPT_STACK_TOP;
        current->state = newstate;
        if (newstate == PROCESS_STATE_READY) {
            list_push_tail(&ready_list, &current->node);
        }
    }

    current = 0;

    while (1) {
        current = (struct process *)list_pop_head(&ready_list);
        if (current) {
            break;
        }
        interrupt_unblock();
        interrupt_wait();
        interrupt_block();
    }

    current->state = PROCESS_STATE_RUNNING;
    interrupt_stack_pointer = current->kstack_top;
    asm("movl %0, %%cr3"::"r"(current->pagetable));
    asm("movl %0, %%esp"::"r"(current->stack_ptr));

    asm("popl %eax");
    asm("popl %ebx");
    asm("popl %ecx");
    asm("popl %edx");
    asm("popl %esi");
    asm("popl %edi");
    asm("popl %ebp");

    interrupt_unblock();
}

int allow_preempt = 1;

void process_preempt() {
    if (allow_preempt && current && ready_list.head) {
        process_switch(PROCESS_STATE_READY);
    }
}

void process_yield() {
    process_switch(PROCESS_STATE_READY);
}

void process_cleanup(struct process *p) {
    // return memory to parent
    p->parent->number_of_pages_using -= p->permissions->max_number_of_pages;

    kfree(p->permissions);
    delete_capabilities_owned_by_process(p);

    // todo: kill the process' children

    // todo: free additional memory related to process

    // free the actual process struct memory
    memory_free_page(p->kstack);
    pagetable_delete(p->pagetable);
    memory_free_page(p);
}

void process_exit(int code) {
    console_printf("Process %d exiting with status: %d...\n", current->pid, code);
    current->exitcode = code;

    process_switch(PROCESS_STATE_GRAVE);
}

void process_wait(struct list *q) {
    list_push_tail(q, &current->node);
    process_switch(PROCESS_STATE_BLOCKED);
}

void process_wakeup(struct list *q) {
    struct process *p;
    p = (struct process *)list_pop_head(q);
    if (p) {
        p->state = PROCESS_STATE_READY;
        list_push_tail(&ready_list, &p->node);
    }
}

void process_wakeup_all(struct list *q) {
    struct process *p;
    while ((p = (struct process *)list_pop_head(q))) {
        p->state = PROCESS_STATE_READY;
        list_push_tail(&ready_list, &p->node);
    }
}

void add_process_to_ready_queue(struct process *p) {
    list_push_tail(&ready_list, &p->node);
}

void process_dump(struct process *p) {
    console_printf("Dumping process %d:\n", p->pid);
    console_printf("last interrupt: %d\n", last_interrupt);
    struct x86_stack *s =
        (struct x86_stack *)(p->kstack + PAGE_SIZE - sizeof(*s));
    console_printf("kstack: %x\n", p->kstack);
    console_printf("stackp: %x\n", p->stack_ptr);
    console_printf("eax: %x     cs: %x\n", s->regs1.eax, s->cs);
    console_printf("ebx: %x     ds: %x\n", s->regs1.ebx, s->ds);
    console_printf("ecx: %x     ss: %x\n", s->regs1.ecx, s->ss);
    console_printf("edx: %x eflags: %x\n", s->regs1.edx, s->eflags);
    console_printf("esi: %x\n", s->regs1.esi);
    console_printf("edi: %x\n", s->regs1.edi);
    console_printf("ebp: %x\n", s->regs1.ebp);
    console_printf("esp: %x\n", s->esp);
    console_printf("eip: %x\n", s->eip);
}
