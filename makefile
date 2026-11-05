# Compiler and Tools definitions
CC = gcc
AS = nasm
LD = ld


# Target flags to build a raw, standalone 32-bit protected-mode binary
CFLAGS = -m32 -ffreestanding -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra -c
ASFLAGS = -f elf32
LDFLAGS = -m elf_i386 -z noexecstack -T linker.ld

# Output targets
KERNEL = silverfox.bin
ISO_IMAGE = silverfox.iso
ISO_DIR = iso
GRUB_CFG = $(ISO_DIR)/boot/grub/grub.cfg
USB_DEVICE ?= /dev/sda
C_OBJECTS = $(patsubst %.c,%.o,$(wildcard *.c))

.PHONY: all usb USB clean

# Build pipeline
all: silverfox.iso

boot.o: boot.asm
	nasm -f elf32 boot.asm -o boot.o

silverfox.bin: boot.o $(C_OBJECTS)
	ld -m elf_i386 -T linker.ld $^ -o $@

silverfox.iso: silverfox.bin
	mkdir -p iso/boot/grub/i386-pc
	cp silverfox.bin iso/boot/os.bin
	
	# Create GRUB configuration
	printf '%s\n' \
		'set timeout=0' \
		'set default=0' \
		'menuentry "silverfox" {' \
		'    multiboot /boot/os.bin' \
		'    boot' \
		'}' > $(GRUB_CFG)

	# Generate lean core image with grub-mkimage
	grub-mkimage -O i386-pc -o iso/boot/grub/i386-pc/core.img \
		-p /boot/grub biosdisk multiboot normal part_msdos iso9660

	# Generate eltorito.img
	cat /usr/lib/grub/i386-pc/cdboot.img iso/boot/grub/i386-pc/core.img > \
	iso/boot/grub/i386-pc/eltorito.img

	# Build ISO using xorriso directly
	xorriso -as mkisofs \
		--grub2-mbr /usr/lib/grub/i386-pc/boot_hybrid.img \
		-b boot/grub/i386-pc/eltorito.img \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--grub2-boot-info \
		--protective-msdos-label \
		-o $@ $(ISO_DIR)

usb: silverfox.iso
	@test -b "$(USB_DEVICE)" || { \
		echo "Error: $(USB_DEVICE) is not a block device; set USB_DEVICE to the whole USB disk." >&2; \
		exit 1; \
	}
	@echo "Writing $(ISO_IMAGE) to $(USB_DEVICE)"
	sudo dd if=$(ISO_IMAGE) of=$(USB_DEVICE) bs=4M status=progress conv=fdatasync

USB: silverfox.bin
	@test -b "$(USB_DEVICE)" || { \
		echo "Error: $(USB_DEVICE) is not a block device; set USB_DEVICE to the whole USB disk." >&2; \
		exit 1; \
	}
	@echo "Writing $(KERNEL) to $(USB_DEVICE)"
	sudo dd if=$(KERNEL) of=$(USB_DEVICE) bs=4M status=progress conv=fdatasync

clean:
	rm -f *.o silverfox.bin silverfox.iso
	rm -rf iso

lines:
	find . -maxdepth 1 -type f \( -name '*.c' -o -name '*.h' -o -name '*.asm' \) -print0 | xargs -0 wc -l

run:
	qemu-system-i386 -drive format=raw,file=silverfox.iso