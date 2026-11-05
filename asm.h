#ifndef ROOTOS_ASM_H
#define ROOTOS_ASM_H

#include <stdint.h>
#define trigger_interrupt(vector) __asm__ volatile ("int %0" : : "N"(vector))

void outb(uint16_t port, uint8_t value);
void outw(uint16_t port, uint16_t value);
uint8_t inb(uint16_t port);
uint16_t inw(uint16_t port);
void io_wait(void);
void sti(void);
void hlt(void);

extern void hang(void);
extern void recovery(void);
extern void test_isr(void);
extern void keyboard_isr(void);
extern void timer_isr(void);
extern void OOM_isr(void);

#endif // IDT_H