.PHONY: all
all: angl-os.iso
	 cd kernel && objdump -d angl-os.elf > angl-os.asmdump && objdump -s angl-os.elf > angl-os.secdump && cd ..

.PHONY: all-hdd
all-hdd: angl-os.hdd
	 cd kernel && objdump -d angl-os.elf > angl-os.asmdump && objdump -s angl-os.elf > angl-os.secdump && cd ..

.PHONY: run
run: angl-os.iso
	qemu-system-x86_64 -M q35 -m 8192 -device VGA -cdrom angl-os.iso -boot d

.PHONY: run-uefi
run-uefi: ovmf-x64 angl-os.iso
	qemu-system-x86_64 -M q35 -m 8192 -device VGA -bios ovmf-x64/OVMF.fd -cdrom angl-os.iso -boot d

.PHONY: run-hdd
run-hdd: angl-os.hdd
	qemu-system-x86_64 -M q35 -m 8192 -device VGA -hda angl-os.hdd

.PHONY: run-hdd-uefi
run-hdd-uefi: ovmf-x64 angl-os.hdd
	qemu-system-x86_64 -M q35 -m 8192 -device VGA -bios ovmf-x64/OVMF.fd -hda angl-os.hdd

ovmf-x64:
	mkdir -p ovmf-x64
	cd ovmf-x64 && curl -o OVMF-X64.zip https://efi.akeo.ie/OVMF/OVMF-X64.zip && 7z x OVMF-X64.zip

limine:
	git clone https://github.com/limine-bootloader/limine.git --branch=v4.x-branch-binary --depth=1
	make -C limine

.PHONY: kernel
kernel:
	$(MAKE) -C kernel

angl-os.iso: limine kernel
	rm -rf iso_root
	mkdir -p iso_root
	cp kernel/angl-os.elf \
		limine.cfg limine/limine.sys limine/limine-cd.bin limine/limine-cd-efi.bin iso_root/
	xorriso -as mkisofs -b limine-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot limine-cd-efi.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		iso_root -o angl-os.iso
	limine/limine-deploy angl-os.iso
	rm -rf iso_root

angl-os.hdd: limine kernel
	rm -f angl-os.hdd
	dd if=/dev/zero bs=1M count=0 seek=64 of=angl-os.hdd
	parted -s angl-os.hdd mklabel gpt
	parted -s angl-os.hdd mkpart ESP fat32 2048s 100%
	parted -s angl-os.hdd set 1 esp on
	limine/limine-deploy angl-os.hdd
	sudo losetup -Pf --show angl-os.hdd >loopback_dev
	sudo mkfs.fat -F 32 `cat loopback_dev`p1
	mkdir -p img_mount
	sudo mount `cat loopback_dev`p1 img_mount
	sudo mkdir -p img_mount/EFI/BOOT
	sudo mkdir -p img_mount/resources
	sudo cp -v kernel/angl-os.elf limine.cfg limine/limine.sys img_mount/
	sudo cp -v resources/test.txt resources/VGA8.F16 img_mount/resources/
	sudo cp -v limine/BOOTX64.EFI img_mount/EFI/BOOT/
	sync
	sudo umount img_mount
	sudo losetup -d `cat loopback_dev`

.PHONY: clean
clean:
	rm -rf iso_root angl-os.iso angl-os.hdd
	rm -rf loopback_dev img_mount
	rm -rf kernel/angl-os.dump
	$(MAKE) -C kernel clean

.PHONY: distclean
distclean: clean
	rm -rf limine ovmf-x64
	$(MAKE) -C kernel distclean