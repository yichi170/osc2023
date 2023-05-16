.PHONY: all clean

all: build

clean: 
	rm -r build

build: kernel/* lib/* include/*
	mkdir -p build
	cmake -B build -DNDEBUG=TRUE
	make -C build

build-v: kernel/* lib/* include/*
	mkdir -p build
	cmake -B build -DNDEBUG=FALSE
	make -C build

run: run-kernel

run-kernel: build/kernel8.img
	qemu-system-aarch64 -M raspi3b -kernel $< -display none -serial null -serial stdio -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb

run-kernel-dbg: build/kernel8.img
	qemu-system-aarch64 -M raspi3b -kernel $< -display none -serial null -serial stdio -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb -S -s
