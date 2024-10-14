# VirtIO Tutorial

This tutorial endeavors to demonstrate the sequential steps required to add peripheral sharing support on the [Bao Hypervisor](https://github.com/bao-project/bao-hypervisor) using the [VirtIO](https://docs.oasis-open.org/virtio/virtio/v1.3/virtio-v1.3.html) interface.

**Validation**:
- Platforms
  - Armv8-A AArch64
    - Xilinx Zynq UltraScale+ MPSoC ZCU102/4
    - NVIDIA Jetson TX2
    - QEMU virt
  - RISC-V RV64
    - QEMU virt
- System configuration
  - Multiple VirtIO frontend drivers on multiple guest VMs, each running on multiple vCPUs
  - Multiple VirtIO backend devices on multiple backend VMs, each running on multiple vCPUs
- Backend devices
  - Rust-VMM backends ([VirtIO-Block](https://github.com/rust-vmm/vm-virtio/tree/main/virtio-blk), [VirtIO-Net](https://github.com/rust-vmm/vmm-reference/pull/49), [VirtIO Console](https://github.com/rust-vmm/vm-virtio/tree/main/virtio-console))
  - QEMU (VirtIO-Block, VirtIO-Net, VirtIO-Console)
  - Vhost-user backends ([vhost-user-fs](https://gitlab.com/virtio-fs/virtiofsd), [vhost-user-vsock](https://github.com/rust-vmm/vhost-device/tree/main/vhost-device-vsock))
  - Vhost backends (vhost-vsock)

For any questions regarding the infrastructure, correction of an error in the tutorials or even any improvement/contribution to the VirtIO infrastructure itself, please contact the code owner via [LinkedIn](https://www.linkedin.com/in/jo%C3%A3o-peixoto-667a8821a/) or [Email](joaopeixotooficial@gmail.com) message.

## General Setup

Setup global environment variables:
```
export ARCH=aarch64|riscv64
export PLATFORM=qemu-aarch64-virt|qemu-riscv64-virt|zcu102|tx2
export CROSS_COMPILE=path/to/your/toolchain
export BAO_DEMOS_WRKDIR=/path/to/your/working-directory
export BAO_DEMOS_VIRTIO_URL=git@github.com:joaopeixoto13/bao-demos-virtio.git
export BAO_DEMOS_VIRTIO_DIR=$BAO_DEMOS_WRKDIR/bao-demos-virtio
```

Clone this repo:
```
git clone $BAO_DEMOS_VIRTIO_URL $BAO_DEMOS_VIRTIO_DIR --depth=1 --branch=main
```

## 2. Backend VM (Linux) Setup

You can opt to:

1) Compiling the I/O Dispatcher kernel module within the Linux kernel’s source tree ([in tree build](#in-tree-build))
2) Compiling the I/O Dispatcher kernel module outside the Linux kernel source tree ([out tree build](#out-of-tree-build))

### In Tree Build

Setup linux environment variables:
```
export BAO_DEMOS_LINUX_VERSION=v6.11-rc3
export BAO_DEMOS_LINUX_DIR=$BAO_DEMOS_WRKDIR/linux-$BAO_DEMOS_LINUX_VERSION
export BAO_DEMOS_LINUX_CONFIG=$BAO_DEMOS_VIRTIO_DIR/configs/linux
export BAO_DEMOS_LINUX_URL=git@github.com:joaopeixoto13/linux.git
export BAO_DEMOS_LINUX_BAO_IO_DISPATCHER=$BAO_DEMOS_VIRTIO_DIR/patches/0001-linux-v6.11-rc3-bao-io-dispatcher.patch
export BAO_DEMOS_LINUX_CFG_FRAG=$(ls $BAO_DEMOS_LINUX_CONFIG/base.config\
                                     $BAO_DEMOS_LINUX_CONFIG/$ARCH.config\ 
                                     $BAO_DEMOS_LINUX_CONFIG/virtio.config 2> /dev/null)
```

Clone Linux from the repo:
```
git clone $BAO_DEMOS_LINUX_URL $BAO_DEMOS_LINUX_DIR -depth=1 --branch=bao-io-dispatcher
```

Override the Linux source directory: 
```
export LINUX_OVERRIDE_SRCDIR=$BAO_DEMOS_LINUX_DIR
```

**Note**: If you already have a Linux source code in your system and you will use one of the buildroot config files ([aarch64.config](configs/buildroot/aarch64.config) / [riscv64.config](configs/buildroot/riscv64.config)), you can simply 
export the `BAO_DEMOS_LINUX_PATCHES` and point that to the I/O Dispatcher [patch](patches/0001-linux-v6.11-rc3-bao-io-dispatcher.patch) file:
```
export BAO_DEMOS_LINUX_PATCHES=$BAO_DEMOS_LINUX_BAO_IO_DISPATCHER
```

Otherwise, you can apply the [patch](patches/0001-linux-v6.11-rc3-bao-io-dispatcher.patch) directly:
```
cd $BAO_DEMOS_LINUX_DIR
git apply $BAO_DEMOS_LINUX_BAO_IO_DISPATCHER
```

Please note that the patch was made for Linux v6.11-rc3, so earlier and future versions may have some conflicts that you must resolve manually.
```
export BAO_DEMOS_LINUX_PATCHES=$BAO_DEMOS_LINUX_BAO_IO_DISPATCHER
```

### Out of Tree Build

Setup environment variables:
```
export BAO_DEMOS_IODISPATCHER_DIR=$BAO_DEMOS_WRKDIR/bao-iodispatcher
export BAO_DEMOS_IODISPATCHER_URL=git@github.com:joaopeixoto13/bao-iodispatcher.git
```

Clone the out of tree I/O Dispatcher kernel module from the repo:
```
git clone $BAO_DEMOS_IODISPATCHER_URL $BAO_DEMOS_IODISPATCHER_DIR -depth=1 --branch=main
cd $BAO_DEMOS_IODISPATCHER_DIR
```

Follow the out of tree I/O Dispatcher kernel module [instructions](https://github.com/joaopeixoto13/bao-iodispatcher).

## 3. Buildroot Setup

Setup buildroot environment variables and directories:
```
export BAO_DEMOS_BUILDROOT_DIR=$BAO_DEMOS_WRKDIR/buildroot-$ARCH-$BAO_DEMOS_LINUX_VERSION
export BAO_DEMOS_BUILDROOT_DEFCFG=$BAO_DEMOS_LINUX/buildroot/$ARCH.config
export BAO_DEMOS_BUILDROOT_OVERLAY_DIR=$BAO_DEMOS_WRKDIR/buildroot_overlay
mkdir -p $BAO_DEMOS_BUILDROOT_OVERLAY_DIR/bin $BAO_DEMOS_BUILDROOT_OVERLAY_DIR/etc
export BAO_DEMOS_BUILDROOT_VERSION=2024.08
```

Clone buildroot:

```
git clone git@github.com:buildroot/buildroot.git $BAO_DEMOS_BUILDROOT_DIR --depth=1 --branch=$BAO_DEMOS_BUILDROOT_VERSION
```

Apply the buildroot defconfig changes:
```
cd $BAO_DEMOS_BUILDROOT_DIR
make defconfig BR2_DEFCONFIG=$BAO_DEMOS_VIRTIO_DIR/configs/buildroot/$ARCH.config
```

## 4. Device Model Setup

Depending on your [demo](#appendix-i), you have two options:

- To use the [Bao VirtIO Device Model](https://github.com/joaopeixoto13/bao-virtio-dm/tree/main) written in Rust, please follow the [build](configs/device-model/bao-virtio-dm/setup.md) instructions.

- To use [QEMU Device Model](https://github.com/joaopeixoto13/qemu/tree/bao-virtio), follow the respective [build](configs/device-model/qemu/setup.md) instructions.

## 5. Buildroot Build

Build your system with:

```
cd $BAO_DEMOS_BUILDROOT_DIR
make linux-reconfigure all -j$(nproc)
```

## 6. Frontend VM Setup

For the frontend VM you can use any kernel that has VirtIO drivers for the device that you want to use. However, if you want to use Linux (perhaps the same image as
the backend VM), you need to enable the `CONFIG_DMA_RESTRICTED_POOL` feature.
If you use the same image as the backend VM, you don't need to do nothing since the previously mentioned feature is enabled by default in the [VirtIO config file](configs/linux/virtio.config).

## 7. Bao Hypervisor Setup

Setup Bao environment variables:
```
export BAO_DEMOS_BAO_URL=git@github.com:joaopeixoto13/bao-hypervisor.git
export BAO_DEMOS_BAO_DIR=$BAO_DEMOS_WRKDIR/bao-hypervisor
```

Clone the Bao Hypervisor Remote I/O branch:
```
git clone $BAO_DEMOS_BAO_URL $BAO_DEMOS_BAO_DIR --depth=1 --branch=feat/remote_io
```

Build Bao hypervisor according to [bao-demos](https://github.com/bao-project/bao-demos/tree/master) tutorial. 
[Appendix I](#appendix-i) includes the configuration files for each demo.

## Appendix I

|  Demo                   | PLATFORM            |  ARCH | BACKEND DEVICE | DATAPLANE | DEVICE MODEL |
| ------------------- | ----------------- | --- | --- | --- | --- |
| [Two Linux Guests accessing the same console](demos/demo-1/README.md) | QEMU   | aarch64 | [Console](https://docs.oasis-open.org/virtio/virtio/v1.3/csd01/virtio-v1.3-csd01.html#x1-3210003) | VirtIO | QEMU |
| [A Linux Guest with network and storage interface](demos/demo-2/README.md)     | QEMU            | aarch64 | [Network](https://docs.oasis-open.org/virtio/virtio/v1.3/csd01/virtio-v1.3-csd01.html#x1-2340001) + [Block](https://docs.oasis-open.org/virtio/virtio/v1.3/csd01/virtio-v1.3-csd01.html#x1-3050002) | VirtIO + VirtIO | QEMU |
| [Shared file system across frontend and backend VMs](demos/demo-3/README.md)       | QEMU            | aarch64 | [File System](https://docs.oasis-open.org/virtio/virtio/v1.3/csd01/virtio-v1.3-csd01.html#x1-49600011) | Vhost-user | Bao VirtIO DM (Rust) |
| [Two Linux Guests accessing the same network interface](demos/demo-4/README.md)    | ZCU102/4    | aarch64 | [Network](https://docs.oasis-open.org/virtio/virtio/v1.3/csd01/virtio-v1.3-csd01.html#x1-2340001) | VirtIO | Bao VirtIO DM (Rust) |
| [Inter-VM communication using Virtual Sockets](demos/demo-5/README.md)    | ZCU102/4    | aarch64 | [Socket](https://docs.oasis-open.org/virtio/virtio/v1.3/csd01/virtio-v1.3-csd01.html#x1-47200010) | Vhost | Bao VirtIO DM (Rust) |
| [A Linux Guest with storage interface](demos/demo-6/README.md)     | QEMU | aarch64 | [Block](https://docs.oasis-open.org/virtio/virtio/v1.3/csd01/virtio-v1.3-csd01.html#x1-3050002) | VirtIO | Bao VirtIO DM (Rust) |
| [A Linux Guest with a virtual console](demos/demo-7/README.md)    | QEMU    | riscv64 | [Console](https://docs.oasis-open.org/virtio/virtio/v1.3/csd01/virtio-v1.3-csd01.html#x1-3210003) | VirtIO | Bao VirtIO DM (Rust) |
| [Inter-VM communication using Virtual Sockets](demos/demo-8/README.md)    | QEMU    | riscv64 | [Socket](https://docs.oasis-open.org/virtio/virtio/v1.3/csd01/virtio-v1.3-csd01.html#x1-47200010) | Vhost-user | Bao VirtIO DM (Rust) |