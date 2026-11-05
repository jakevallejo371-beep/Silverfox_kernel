#include <stddef.h>

#include "acpi.h"
#include "asm.h"

struct acpi_rsdp {
    char     signature[8];
    uint8_t  checksum;
    char     oem_id[6];
    uint8_t  revision;
    uint32_t rsdt_address;
} __attribute__((packed));

struct acpi_header {
    char     signature[4];
    uint32_t length;
    uint8_t  revision;
    uint8_t  checksum;
    char     oem_id[6];
    char     oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} __attribute__((packed));

struct acpi_fadt {
    struct acpi_header header;
    uint32_t firmware_ctrl;
    uint32_t dsdt;
    uint8_t  reserved;
    uint8_t  pm_profile;
    uint16_t sci_int;
    uint32_t smi_cmd;
    uint8_t  acpi_enable;
    uint8_t  acpi_disable;
    uint8_t  s4bios_req;
    uint8_t  pstate_cnt;
    uint32_t pm1a_evt_blk;
    uint32_t pm1b_evt_blk;
    uint32_t pm1a_cnt_blk;
    uint32_t pm1b_cnt_blk;
} __attribute__((packed));

static bool memEqual(const void* a, const char* b, int n) {
    const char* p = (const char*)a;
    for (int i = 0; i < n; i++) {
        if (p[i] != b[i]) return false;
    }
    return true;
}

static struct acpi_rsdp* findRSDP(void) {
    uint32_t ebda = ((uint32_t)*(volatile uint16_t*)0x40E) << 4;
    uint32_t start[2] = { ebda, 0xE0000 };
    uint32_t end[2]   = { ebda + 1024, 0x100000 };

    for (int r = 0; r < 2; r++) {
        if (start[r] == 0) continue;
        for (uint32_t addr = start[r]; addr < end[r]; addr += 16) {
            if (memEqual((void*)addr, "RSD PTR ", 8)) {
                uint8_t sum = 0;
                for (int i = 0; i < 20; i++) sum += ((uint8_t*)addr)[i];
                if (sum == 0) return (struct acpi_rsdp*)addr;
            }
        }
    }
    return NULL;
}

static struct acpi_fadt* findFADT(struct acpi_rsdp* rsdp) {
    struct acpi_header* rsdt = (struct acpi_header*)rsdp->rsdt_address;
    if (!memEqual(rsdt->signature, "RSDT", 4)) return NULL;

    uint32_t count = (rsdt->length - sizeof(struct acpi_header)) / 4;
    uint32_t* entries = (uint32_t*)((uint8_t*)rsdt + sizeof(struct acpi_header));

    for (uint32_t i = 0; i < count; i++) {
        struct acpi_header* h = (struct acpi_header*)entries[i];
        if (memEqual(h->signature, "FACP", 4)) return (struct acpi_fadt*)h;
    }
    return NULL;
}

void acpiPowerOff(void) {
    struct acpi_rsdp* rsdp = findRSDP();
    struct acpi_fadt* fadt = rsdp ? findFADT(rsdp) : NULL;

    if (fadt != NULL && fadt->dsdt != 0 && fadt->pm1a_cnt_blk != 0) {
        uint16_t pm1a = (uint16_t)fadt->pm1a_cnt_blk;
        uint16_t pm1b = (uint16_t)fadt->pm1b_cnt_blk;

        // Switch the chipset into ACPI mode if the firmware hasn't already (SCI_EN = bit 0)
        if ((inw(pm1a) & 1) == 0 && fadt->smi_cmd != 0 && fadt->acpi_enable != 0) {
            outb((uint16_t)fadt->smi_cmd, fadt->acpi_enable);
            for (int i = 0; i < 3000000 && (inw(pm1a) & 1) == 0; i++) io_wait();
        }

        // Find the _S5_ object in the DSDT to get the sleep type values
        struct acpi_header* dsdt = (struct acpi_header*)fadt->dsdt;
        uint8_t* p   = (uint8_t*)dsdt + sizeof(struct acpi_header);
        uint8_t* end = (uint8_t*)dsdt + dsdt->length;

        for (; p + 8 < end; p++) {
            if (memEqual(p, "_S5_", 4) && p[4] == 0x12 &&
                (p[-1] == 0x08 || (p[-2] == 0x08 && p[-1] == '\\')))
            {
                uint8_t* s = p + 5;
                s += ((*s & 0xC0) >> 6) + 2;       // skip PkgLength and NumElements
                if (*s == 0x0A) s++;               // skip byte prefix
                uint16_t typA = (uint16_t)(*s << 10);
                s++;
                if (*s == 0x0A) s++;
                uint16_t typB = (uint16_t)(*s << 10);

                outw(pm1a, typA | (1 << 13));      // SLP_EN = bit 13
                if (pm1b != 0) outw(pm1b, typB | (1 << 13));
                break;
            }
        }
    }

    // Fallbacks for emulators
    outw(0x604,  0x2000);   // QEMU
    outw(0xB004, 0x2000);   // older QEMU / Bochs
    outw(0x4004, 0x3400);   // VirtualBox
}