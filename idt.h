#ifndef IDT_H
#define IDT_H

#include <stdint.h>

// IDT Entry data structure
typedef struct {
    uint16_t isr_low;
    uint16_t kernel_cs;
    uint8_t  reserved;
    uint8_t  attributes;
    uint16_t isr_high;
} __attribute__((packed)) idt_entry_t;

// IDTR data structure
typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idtr_t;

// Share the idtr variable with other modules
extern idtr_t idtr;

// Function blueprints
uint16_t updateCSvar(void);
int set_function_idt(int index, void (*fn)(void), uint8_t fn_attributes);
int set_intelException(void);
void exception_handler(int vector, int error_code);

// Assembly-implemented lidtr wrapper
extern int fn_lidtr(idtr_t pointer);

int pic_remap(int offset1, int offset2);

#endif // IDT_H