# Two Linux Guests accessing the same console

This demo features a triple Linux guest configuration where one of them will serve as VirtIO backend and the remaining two will be guests. In this demo, only the backend has access to the physical peripheral and the two guests use the VirtIO interface to access the console.

## 1. Prerequisites

- [Run script](run_bao.sh)

Launch one single QEMU process to handle **each** VirtIO devices:
- [Config file](qemu-aarch64-virt.c)
- [Linux backend device tree](backend.dts)
- [Linux frontend #1 device tree](frontend1.dts)
- [Linux frontend #2 device tree](frontend2.dts)

Launch one single QEMU process to handle **both** VirtIO devices:
- [Config file](qemu-aarch64-virt2.c)
- [Linux backend device tree](backend-2.dts)
- [Linux frontend #1 device tree](frontend1.dts)
- [Linux frontend #2 device tree](frontend2-2.dts.dts)

**Note**: When two distinct QEMU processes are initiated, QEMU will expect the VirtIO devices to be at the same memory location (`0xa003e00`). However, when a single QEMU process is launched with the two VirtIO devices, QEMU will expect the VirtIO devices at addresses `0xa003e00` and `0xa003c00`, respectively. In this case, you should change the address of the VirtIO device of the second frontend VM from `0xa003e00` to `0xa003c00` and the interrupt from `0x2f` to `0x2e`, as already presented in this section.

Also, to facilitate the execution of this demo, it's essential to apply the [patch](../0001-arm-virt-add-two-more-uarts.patch) to your local/laptop QEMU. This patch enables the launch of two additional UARTs, allowing each guest to operate on distinct consoles.

## 2. Setup QEMU as VirtIO Backend provider

Add the QEMU package to the Linux Backend by typing `make menuconfig` and navigating to: `Target Packages > Miscellaneous > QEMU`. The package name is: `BR2_PACKAGE_QEMU`.
If you opt for an alternate buildroot defconfig and if the package is not available, change the C++ library from `uClibc-ng` to `glibc` and add the respective package dependecies.

Download QEMU from the official repo: 

```
git clone git@github.com:qemu/qemu.git /path/to/your/qemu --branch=v8.0.3 --depth=1
cd /path/to/your/qemu
```

Override the QEMU source directory: 

```
export QEMU_OVERRIDE_SRCDIR=/path/to/your/qemu
```

To enable QEMU to serve as the VirtIO backend provider for the Bao Hypervisor, apply the specified [patch](0001-qemu-v8.0.3-as-virtio-backend-provider-for-bao-hypervisor.patch) accordingly:

```
git apply 0001-qemu-v8.0.3-as-virtio-backend-provider-for-bao-hypervisor.patch
```

Configure QEMU:

```
./configure CROSS_COMPILE=/path/to/your/buildroot/aarch64/toolchain ARCH="aarch64" --target-list=aarch64-softmmu --enable-slirp --enable-bao
```

**Note**: The buildroot toolchain should be: `/path/to/your/buildroot/output/host/bin/aarch64-buildroot-linux-gnu-`

Rebuild again your Backend VM.

## 3. Run

Following the execution of the [run script](run_bao.sh), you should observe output similar to the following:

```
char device redirected to /dev/pts/x1 (label serial-backend)
char device redirected to /dev/pts/x2 (label serial1)
char device redirected to /dev/pts/x3 (label serial2)
```

Open three separate terminals and establish connections. Below is an illustrative example:

```
picocom -b 115200 /dev/pts/x1
picocom -b 115200 /dev/pts/x2
picocom -b 115200 /dev/pts/x3
```

As you proceed with the execution, expect the two frontends to pause when the VirtIO configuration initiates. This pause occurs because the VirtIO backends, responsible for handling VirtIO requests, have not been launched yet.

**Note**: Most VirtIO implementations typically demand setting up VirtIO backends before any frontends. However, our approach remains entirely agnostic, offering the flexibility to attach VirtIO clients both before and after frontend execution, without strict dependencies on order.

Once logged into the **Backend VM**, you have two options available for launching the VirtIO backends:

1) Execute the launch in distinct QEMU processes to maintain the isolation of each entity involved

2) Initiate a single QEMU process designated to manage the specified VirtIO backends


To execute the launch in **distinct** QEMU processes, run the following commands:

```
nohup qemu-system-aarch64 -nographic -object memory-backend-file,id=virt.ram,size=16M,mem-path=/dev/baoipc0,offset=0,share=on -machine virt,memory-backend=virt.ram,virtualization=on -cpu cortex-a53 -smp 1 -accel bao -device virtio-serial-device,iommu_platform=on,max_ports=1 -chardev pty,id=serial-bao-1 -device virtconsole,chardev=serial-bao-1 -global virtio-mmio.force-legacy=false -bao dm_id=0,irq=47,shmem_addr=536870912,shmem_size=16777216 > /etc/log-serial-1 2>&1 &
```

```
nohup qemu-system-aarch64 -nographic -object memory-backend-file,id=virt.ram,size=16M,mem-path=/dev/baoipc1,offset=0,share=on -machine virt,memory-backend=virt.ram,virtualization=on -cpu cortex-a53 -smp 1 -accel bao -device virtio-serial-device,iommu_platform=on,max_ports=1 -chardev pty,id=serial-bao-2 -device virtconsole,chardev=serial-bao-2 -global virtio-mmio.force-legacy=false -bao dm_id=1,irq=47,shmem_addr=553648128,shmem_size=16777216 > /etc/log-serial-2 2>&1 &
```

To initiate a **single** QEMU process to manage the two VirtIO backends, run the command:

```
nohup qemu-system-aarch64 -nographic -object memory-backend-file,id=virt.ram,size=32M,mem-path=/dev/baoipc0,offset=0,share=on -machine virt,memory-backend=virt.ram,virtualization=on -cpu cortex-a53 -smp 1 -accel bao -device virtio-serial-device,iommu_platform=on,max_ports=1 -chardev pty,id=serial-bao-1 -device virtconsole,chardev=serial-bao-1 -device virtio-serial-device,iommu_platform=on,max_ports=1 -chardev pty,id=serial-bao-2 -device virtconsole,chardev=serial-bao-2 -global virtio-mmio.force-legacy=false -bao dm_id=0-1,irq=47-46,shmem_addr=536870912-553648128,shmem_size=16777216-16777216 > /etc/log-serials 2>&1 &
```

**Note**: The `nohup <command> > /etc/log-file 2>&1 &` command is utilized to execute a program in the background while redirecting both `stderr` and `stdout` outputs to the specified file.

Following this, verify the location to which the character devices were redirected by executing the command:

```
cat /etc/log-serial-1
cat /etc/log-serial-2
```

or

```
cat /etc/log-serials
```

Respectively, depending on whether you launched two or one QEMU instances. You should observe output similar to:

```
char device redirected to /dev/pts/0 (label serial1)
char device redirected to /dev/pts/1 (label serial2)
```

Open one terminal (frontend #1) from the console:

```
picocom -b 115200 /dev/pts/0
```

Open other terminal in your laptop, connect it to the backend VM through `ssh` and open the other terminal (frontend #2):

```
ssh root@localhost -p 5555
picocom -b 115200 /dev/pts/1
```

Once logged into the **Frontend VM #1**, open the virtual console:

```
picocom -b 115200 /dev/hvc0
```

Once logged into the **Frontend VM #2**, open the virtual console:

```
picocom -b 115200 /dev/hvc0
```

You can now input data into the frontend virtual consoles, and the corresponding output will be displayed in the backend consoles. Similarly, entering data into each backend console will be visible in the respective frontend virtual console.
