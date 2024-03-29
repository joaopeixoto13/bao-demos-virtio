#!/bin/bash -e

qemu-system-aarch64 \
    -nographic \
    -machine virt,secure=on, \
    -machine gic-version=3 \
    -machine virtualization=on \
    -cpu cortex-a53 \
    -smp 4 \
    -m 4G \
    -serial mon:stdio -serial pty -serial pty\
    -bios bl1.bin \
    -semihosting-config enable=on,target=native \
    -s \
    $@
