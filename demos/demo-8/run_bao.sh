#!/bin/bash -e

qemu-system-riscv64 \
    -nographic \
    -M virt \
    -cpu rv64 \
    -smp 4 \
    -m 4G \
    -bios /path/to/your/opensbi.elf \
    -serial mon:stdio \
    -semihosting-config enable=on,target=native \
    -device virtio-serial-device -chardev pty,id=serial-backend -device virtconsole,chardev=serial-backend \
    -s \
    $@