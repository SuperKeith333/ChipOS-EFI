CC      = gcc
LD      = ld
OBJCOPY = objcopy

BUILD_DIR = build
ISO_DIR   = $(BUILD_DIR)/iso
EFI_BOOT  = $(ISO_DIR)/EFI/BOOT

CFLAGS  = -I/usr/include/efi \
          -I/usr/include/efi/x86_64 \
          -I./include \
          -fpic \
          -ffreestanding \
          -fno-stack-protector \
          -fno-stack-check \
          -fshort-wchar \
          -mno-red-zone \
          -maccumulate-outgoing-args \
          -O2

LDFLAGS = -nostdlib \
          -znocombreloc \
          -shared \
          -Bsymbolic \
          -L/usr/lib \
          -T /usr/lib/elf_x86_64_efi.lds \
          /usr/lib/crt0-efi-x86_64.o

TARGET = BOOTX64.EFI

# 1. Update the objects tracking string to look inside 'emulator'
OBJECTS = $(BUILD_DIR)/boot/bootloader.o \
          $(BUILD_DIR)/kernel/kernel.o \
          $(BUILD_DIR)/kernel/renderer.o \
          $(BUILD_DIR)/emulator/cpu.o \
          $(BUILD_DIR)/font.o 
# ... (keep all intermediate rules the same) ...

# 2. Update the directory creation and compiler target rule for the emulator



.PHONY: all run clean

all: $(TARGET)

# Compile raw font file
$(BUILD_DIR)/font.o: font.bin | $(BUILD_DIR)
	$(OBJCOPY) -I binary -O elf64-x86-64 -B i386:x86-64 $< $@

# Catch rules for src files creating dynamic targets in the build folder
$(BUILD_DIR)/boot/%.o: src/boot/%.c | $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/boot
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/kernel/%.o: src/kernel/%.c | $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/kernel
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/emulator/%.o: src/emulator/%.c | $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/emulator
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/main.so: $(OBJECTS)
	$(LD) $(LDFLAGS) $^ -o $@ -lefi -lgnuefi

$(TARGET): $(BUILD_DIR)/main.so
	$(OBJCOPY) -j .text -j .sdata -j .data -j .dynamic -j .dynsym \
	           -j .rel -j .rela -j '.rel.*' -j '.rela.*' -j .reloc \
	           -j .rodata -j .srodata \
	           --input-target=elf64-x86-64 \
	           --output-target=efi-app-x86_64 \
	           $< $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Updated QEMU run command enabling host audio emulation
run: $(TARGET)
	mkdir -p $(EFI_BOOT)
	cp $(TARGET) $(EFI_BOOT)/
	qemu-system-x86_64 -bios /usr/share/edk2-ovmf/x64/OVMF.4m.fd \
	                   -drive format=raw,file=fat:rw:$(ISO_DIR) \
	                   -net none \
	                   -audiodev driver=pa,id=snd0 -machine pcspk-audiodev=snd0


clean:
	rm -rf $(BUILD_DIR) $(TARGET)
