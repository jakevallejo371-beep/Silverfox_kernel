#include <stddef.h>

#include "idt.h"
#include "asm.h"
#include "config.h"
#include "kstr.h"
#include "shell.h"
#include "vga.h"

static idt_entry_t idt[256];

idtr_t idtr = {
    .limit = 2047,
    .base  = (uint32_t)idt
};

/*

[IDT layout]

 - 0-31   -> Intel exceptions
 - 32-47  -> PIC hardware IRQs (pic_remap)
    + #32 -> timer
    + #33 -> keyboard
 - 48-255 -> Software interrupts
    + #48 -> test interrupt
    + #49 -> out of memory OOM

*/

int set_function_idt(int index, void (*fn)(void), uint8_t fn_attributes) {
    uint32_t addr = (uint32_t)fn;

    idt[index].isr_low  = addr & 0xFFFF;              // bottom 16 bits
    idt[index].kernel_cs = updateCSvar();            // almost always 0x08
    idt[index].reserved = 0;                        // always 0
    idt[index].attributes = fn_attributes;         // amlost always 0x8E(Present, Ring 0, Interrupt Gate) for hardware interupts and 0xEE(Present, Ring 3, Interrupt Gate) for user-mode system calls
    idt[index].isr_high = (addr >> 16) & 0xFFFF;  // top 16 bits

    return 1;
}

extern void isr0(void);  extern void isr1(void);  extern void isr2(void);  extern void isr3(void);
extern void isr4(void);  extern void isr5(void);  extern void isr6(void);  extern void isr7(void);
extern void isr8(void);  extern void isr9(void);  extern void isr10(void); extern void isr11(void);
extern void isr12(void); extern void isr13(void); extern void isr14(void); extern void isr15(void);
extern void isr16(void); extern void isr17(void); extern void isr18(void); extern void isr19(void);
extern void isr20(void); extern void isr21(void); extern void isr22(void); extern void isr23(void);
extern void isr24(void); extern void isr25(void); extern void isr26(void); extern void isr27(void);
extern void isr28(void); extern void isr29(void); extern void isr30(void); extern void isr31(void);

int set_intelException(void) {
    void (*exception_functions[32])(void) = {
        isr0,  isr1,  isr2,  isr3,  isr4,  isr5,  isr6,  isr7,
        isr8,  isr9,  isr10, isr11, isr12, isr13, isr14, isr15,
        isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23,
        isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31
    };

    for (int i=0; i < 32; i++)
    {
        set_function_idt(i, exception_functions[i], INT_GATE);
    }

    fn_lidtr(idtr);

    return 1;
}

void exception_handler(int vector, int error_code) {
    printError("Runtime error!\n");
    print("Vector: ");
    printWarning(int_to_str(vector));
    print("\nerror code: ");
    printWarning(int_to_str(error_code));
    print("\n");

    if (vector == 8 || vector == 18 || vector == 10)
    {
        shutdown(0, NULL);
    }
}

int pic_remap(int offset1, int offset2) {

    // 2. ICW1: Start initialization sequence on both PICs
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();

    // 3. ICW2: Set vector offsets
    outb(PIC1_DATA, offset1); // Master vector offset (e.g., 0x20)
    io_wait();
    outb(PIC2_DATA, offset2); // Slave vector offset (e.g., 0x28)
    io_wait();

    // 4. ICW3: Configure cascade relationship
    outb(PIC1_DATA, 4); // Tell Master PIC that Slave PIC is at IRQ2 (0000 0100b)
    io_wait();
    outb(PIC2_DATA, 2); // Tell Slave PIC its cascade identity (2)
    io_wait();

    // 5. ICW4: Set 8086 mode
    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

    outb(PIC1_DATA, 0xFC);
    outb(PIC2_DATA, 0xFF);

    return 1;
}