# Bootloader_core
AT&amp;T bootloader, kernel compiled by gcc, OS: linux. Emulator is qemu.

For comlining bootloader:

as --32 -o bootsect.o bootsect.asm

ld -Ttext 0x7c00 --oformat binary -m elf_i386 -o bootsect.bin bootsect.o

For comlining kernel:

g++ -ffreestanding -m32 -o kernel.o -c kernel.cpp

ld --oformat binary -Ttext 0x10000 -o kernel.bin --entry=kmain -m elf_i386 kernel.o

RUN:
qemu –fda bootsect.bin –fdb kernel.bin
