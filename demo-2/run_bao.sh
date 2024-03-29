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
    -device virtio-net-device,netdev=net0 \
    -netdev user,id=net0,net=192.168.42.0/24,hostfwd=tcp:127.0.0.1:5555-:22 \
    -bios bl1.bin \
    -semihosting-config enable,target=native \
    -s \
    $@
