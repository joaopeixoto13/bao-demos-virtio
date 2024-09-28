#!/bin/bash -e

qemu-system-aarch64 \
    -nographic \
    -machine virt,secure=on, \
    -machine gic-version=3 \
    -machine virtualization=on \
    -cpu cortex-a53 \
    -smp 4 \
    -m 4G \
    -serial mon:stdio -serial pty -serial pty \
    -device virtio-blk-device,drive=drive1,id=virtblk0 \
    -drive file=/path/to/your/dir/ext4_image.img,id=drive1,if=none,format=raw \
    -bios bl1.bin \
    -semihosting-config enable,target=native \
    -s \
    $@
