build/bootloader.img:
build/kernel8.img:
	mkdir -p build
	cmake -B build
	make -C build

dump-asm: build/bootloader.img
	qemu-system-aarch64 -M raspi3b -kernel build/bootloader.img -display none -d in_asm

run: build/bootloader.img
	qemu-system-aarch64 -M raspi3b -kernel build/bootloader.img -display none -serial null -serial stdio
run-dbg: build/bootloader.img
	qemu-system-aarch64 -M raspi3b -kernel build/bootloader.img -display none -serial null -serial stdio -S -s

run-tty: build/bootloader.img
	qemu-system-aarch64 -M raspi3b -kernel build/bootloader.img -display none -serial null -serial pty -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb
run-tty-dbg: build/bootloader.img
	qemu-system-aarch64 -M raspi3b -kernel build/bootloader.img -display none -serial null -serial pty -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb -S -s

run-kernel: build/kernel8.img
	qemu-system-aarch64 -M raspi3b -kernel $< -display none -serial null -serial stdio -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb

run-kernel-dbg: build/kernel8.img
	qemu-system-aarch64 -M raspi3b -kernel $< -display none -serial null -serial stdio -initrd initramfs.cpio -dtb bcm2710-rpi-3-b-plus.dtb -S -s
