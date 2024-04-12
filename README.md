# VirtIO Tutorial

This tutorial endeavors to demonstrate the sequential steps required to construct and activate peripheral sharing on the [Bao Hypervisor](https://github.com/bao-project/bao-hypervisor) using the VirtIO interface.

**Notes**:
- The infrastructure is still in the development phase and is only being tested for ARM platforms (QEMU and ZCU102).
- For physical boards (e.g. ZCU102), there is a problem already identified when the Frontend VM's CPU goes to the idle state, as a result of the trap process and MMIO emulation. In this scenario, currently (March 28 2024) Bao Hypervisor is unable to save the state of the registers before powering off. However, there is work underway to implement a standby mechanism. To temporarily resolve this problem, in the `cpu_arch_profile_idle` function replace the `psci_power_down` call with a simple `asm volatile("wfi")`
- These demo tutorials may contain errors/inaccuracies, as they are continually being updated

For any questions regarding the infrastructure, correction of an error in the tutorials or even any improvement/contribution to the VirtIO infrastructure itself, please send an email to <joaopeixotooficial@gmail.com>.

## 1. Clone Bao Hypervisor VirtIO branch

Clone the Bao Hypervisor VirtIO branch:

```
git clone git@github.com:bao-project/bao-hypervisor.git /path/to/your/bao-hypervisor --depth=1 --branch=wip/virtio
```

## 2. Setup the VirtIO Backend VM

### 2.1 Buildroot Configuration

Clone the latest buildroot at the latest stable version: 

```
git clone git@github.com:buildroot/buildroot.git /path/to/your/buildroot --depth=1 --branch=2023.08
cd /path/to/your/buildroot
```

Use the provided [buildroot defconfig](buildroot-v6.5-backend.config), or use one at your own risk:

```
make defconfig BR2_DEFCONFIG=/path/to/your/defconfig
```

### 2.2 Linux Backend Configuration

Download Linux from the official repo: 

```
git clone git@github.com:torvalds/linux.git /path/to/your/linux -depth=1 --branch=v6.5
cd /path/to/your/linux
```

Override the Linux source directory: 

```
export LINUX_OVERRIDE_SRCDIR=/path/to/your/linux
```

To integrate the I/O Request Management System into Linux, apply the designated [patch](0001-linux-v6.5-backend-dispatcher-io.patch):

```
git apply 0001-linux-v6.5-backend-dispatcher-io.patch
```

---

### 2.3 First Build

Build buildroot:

```
make -j$(nproc)
```

### 2.4 Second Build

After building Linux, you have two options: Utilize the provided [Linux configuration](linux-v6.5-backend.config) or manually configure yours. If you want to use the provided configuration, first you need to copy the configuration file to the linux-custom directory, rename the file to .config and apply the changes:
```
cp /path/to/your/linux-v6.5-backend.config /path/to/your/buildroot/output/build/linux-custom/.config
cd /path/to/your/buildroot && make olddefconfig
```

If you want to manually configure your Linux, you must enable two options:
1. The **BAO_VIRTIO** driver, used to manage all VirtIO requests from the associated Backend VM. To enable this feature, type `make linux-menuconfig` and navigating to: `Device Drivers > Virtualization Drivers > Bao Hypervisor Module for VirtIO`. Upon selecting the aforementioned driver, the system should automatically enable the `Bao Hypervisor I/O Request System configured by Interrupts`.

2. The **BAO_SHMEM** driver, used to enable the IPC channel between the frontend and the backend VMs. Select `Device Drivers > Bao shared memory support`

Build buildroot again:

```
make -j$(nproc)
```

## 3. Setup the VirtIO Frontend VM

For the VM frontend you can use any kernel that has VirtIO drivers for the device that you want to use. However, it is imperative to restrict the driver's memory access to a specific memory location, specifically tailored to the shared memory designated for passing VirtIO data information. Presently, if you are utilizing Linux, the approach has been implemented using the `CONFIG_DMA_RESTRICTED_POOL` feature.

## 4. Setup your demo

Presented below is a table detailing the description of each demo alongside its respective target platform.

|  Demo                   | PLATFORM            |  ARCH |
| ------------------- | ----------------- | --- |
| [Two Linux Guests accessing the same console](demo-1/README.md) | QEMU   | aarch64 |
| [A Linux Guest with network and storage interface](demo-2/README.md)     | QEMU            | aarch64 |
| [A Linux Guest with virtual filesystem interface](demo-3/README.md)       | QEMU            | aarch64 |
| [Two Linux Guests accessing the same network interface](demo-4/README.md)    | ZCU102/4    | aarch64 |
| [Inter-VM communication using Virtual Sockets](demo-5/README.md)    | ZCU102/4    | aarch64 |