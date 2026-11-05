#include "asm.h"
#include "idt.h"

void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

void outw(uint16_t port, uint16_t value) {
    __asm__ volatile ("outw %0, %1" : : "a"(value), "Nd"(port));
}

uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

uint16_t inw(uint16_t port) {
    uint16_t value;
    __asm__ volatile ("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

void io_wait(void) {
    outb(0x80, 0);
}

void sti(void) {
    __asm__ volatile ("sti" : : : "memory");
}

void hlt(void) {
    __asm__ volatile ("hlt" : : : "memory");
}

uint16_t updateCSvar(void) {
    uint16_t code_segment;
    __asm__ volatile ("mov %%cs, %0" : "=r"(code_segment));
    return code_segment;
}

int fn_lidtr(idtr_t pointer) {
    __asm__ volatile ("lidt %0" : : "m"(pointer));

    return 1;
}