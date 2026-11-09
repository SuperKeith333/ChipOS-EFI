# CHIPOS-EFI
A low level chip-8 emulator

***Notes***
- Not tested on real hardware, if you were to do so, you need an sdcard reader to read roms
- VERY BUGGY, especially the file picker, going down through 2 folders currently cause memory issues
- honestly needs a rewrite, most systems are very generic and not very well made


Requires GCC to compile (the makefile only supports building for linux at the moment)

Other required packages:
```
base-devel
gnu-efi
qemu-system-x86
qemu-audio-pa
edk2-ovmf
git
```

To compile run:
```make```

To compile and run the emulator run:
```make run```

**How to make a SD Card image**
```
dd if=/dev/zero of=sdcard.img bs=1M count={size}
mkfs.fat -F 32 sdcard.img
```
(format as fat32)

sudo mount -o loop sdcard.img {mount point}

change this line in the make file to point to the location of your sdcard image ```-drive if=none,id=sd0,format=raw,file=sdcard.img```
