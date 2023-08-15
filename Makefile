SRCS := $(shell find kernel/ -name '*.c') 
OBJS := $(SRCS:.c=.o)
CC = gcc
LD = ld

CFLAGS := \
	-Ikernel/src \
	-I. \
	-Wall \
	-Wextra \
	-std=gnu11 \
	-ffreestanding \
	-fno-stack-protector \
	-fno-stack-check \
	-fno-lto \
	-fno-PIE \
	-w \
	-fno-PIC \
	-m64 \
	-g \
	-march=x86-64 \
	-mabi=sysv \
	-mno-80387 \
	-mno-mmx \
	-mno-sse \
	-mno-sse2 \
	-mno-red-zone \
	-mcmodel=kernel

LD_FLAGS := \
	-Tkernel/linker.ld \
	-nostdlib \
	-static \
	-m elf_x86_64 \
	-z max-page-size=0x1000

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

kernel.elf: $(OBJS)
	nasm kernel/src/arch/gdt.asm -felf64 -o gdt.o
	nasm kernel/src/syscall/syscall.asm -felf64 -o syscall.o
	$(LD) $(LD_FLAGS) $(OBJS) gdt.o syscall.o -o $@

iso:
	dd if=/dev/zero of=fat32.img bs=1M count=64
	mkfs.fat -F32 fat32.img
	mcopy -i fat32.img ./data/* ::/
	rm -rf iso_root
	mkdir -p iso_root
	cp kernel.elf \
		limine.cfg limine/limine.sys limine/limine-cd.bin limine/limine-cd-efi.bin iso_root/
	xorriso -as mkisofs -b limine-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot limine-cd-efi.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		iso_root -o taha.iso
	limine/limine-deploy taha.iso
	rm -rf iso_root

clean:
	rm -f $(OBJS)
	rm -f kernel.elf
	rm -f taha.iso
	rm *.img
	rm -f *.o

run:
	make iso
	qemu-system-x86_64 -enable-kvm -m 512M -debugcon stdio -cdrom taha.iso -vga std -audiodev pa,id=audio0 -machine pcspk-audiodev=audio0 -hda fat32.img -boot d

debug:
	make iso
	qemu-system-x86_64 -enable-kvm -m 512M -debugcon stdio -cdrom taha.iso -vga std -s -S -audiodev pa,id=audio0 -machine pcspk-audiodev=audio0 -hda fat32.img -boot d