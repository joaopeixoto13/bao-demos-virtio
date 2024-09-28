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
    -device virtio-net-device,netdev=net0 \
    -netdev user,id=net0,net=192.168.42.0/24,hostfwd=tcp:127.0.0.1:5555-:22 \
    -device virtio-serial-device -chardev pty,id=serial-backend \
    -device virtconsole,chardev=serial-backend \
    -bios bl1.bin \
    -semihosting-config enable,target=native \
    -s \
    $@