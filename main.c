#include <stddef.h>
#include <stdint.h>

#include "asm.h"
#include "idt.h"
#include "keyboardDriver.h"
#include "shell.h"
#include "terminal.h"
#include "timer.h"
#include "vga.h"
#include "kstr.h"
#include "acpi.h"
#include "allocator.h"
#include "multiboot.h"
#include "memory.h"


void main(unsigned magic, size_t info_addr) {
    if (magic != 0x2BADB002) return;
    
    multi_info_t* mbi = (multi_info_t*)info_addr;

    if (mbi->flags & 1)
    {
        uint32_t low = mbi->mem_lower;
        uint32_t upper  = mbi->mem_upper;

        g_memory.kib = low + upper;
        g_memory.kb = (g_memory.kib * 1024U) / 1000U;
        g_memory.mb = (g_memory.kib * 1024U) / 1000000U;
    }

    int count =0;

    clearScreen(0, NULL);
    printWarning("Booting...\n");

    printWarning("[video] configuring display...\n");
    count += disableBlink();
    count += enableHardwareCursor(12, 13);

    printWarning("[Inturrupts] installing handlers...\n");
    count += set_intelException();
    count += set_function_idt(0x20, timer_isr, INT_GATE);
    count += fn_lidtr(idtr);
    //add keyboard to idt
    count += set_function_idt(0x21, keyboard_isr, INT_GATE);        
    count += init_timer(1000);
    //add test to idt
    count += set_function_idt(0x30, test_isr, INT_GATE);
    //add OOM to idt
    count += set_function_idt(0x32, OOM_isr, INT_GATE);
    count += install_bump();
    //add PIC_timer to idt
    count += pic_remap(0x20, 0x28);
    sti();

    int error = (count - TOTAL_OP) * -1;

    print("Error: ");
    print(int_to_str(error));

    if (error != 0)
    {
        printError("\n{FAILED}\n");
        
        print("A system installation failed\nDo you wish to contine? [y/n]\n? ");
        char* buff = readline();

        if (strComp(buff, "y") || strComp(buff, "Y"))
        {
            printTitle(0, NULL);
            terminal();
        }
        else if (strComp(buff, "n") || strComp(buff, "N"))
        {
            acpiPowerOff();
        } else {
            printWarning("Not an option\nShuting down\n");
            print("[ENTER]");
            readline();
            acpiPowerOff();
        }

    } else {
        print("\n{OK}\n");

        printTitle(0, NULL);
        terminal();
    }
}