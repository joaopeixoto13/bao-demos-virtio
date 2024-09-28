# Two Linux Guests accessing the same console

This demo features a triple Linux guest configuration where one of them will serve as VirtIO backend and the remaining two will be guests. In this demo, only the backend has access to the physical peripheral and the two guests use the VirtIO interface to access the console.

## 1. Prerequisites

- [Run script](run_bao.sh)

Launch one single QEMU process to handle **each** VirtIO device:
- [Config file](qemu-aarch64-virt.c)
- [Linux backend device tree](backend1.dts)
- [Linux frontend #1 device tree](frontend11.dts)
- [Linux frontend #2 device tree](frontend12.dts)

Launch one single QEMU process to handle **both** VirtIO devices:
- [Config file](qemu-aarch64-virt2.c)
- [Linux backend device tree](backend2.dts)
- [Linux frontend #1 device tree](frontend11.dts)
- [Linux frontend #2 device tree](frontend22.dts.dts)

To facilitate the execution of this demo, it's essential to apply the [patch](../../patches/0001-arm-virt-add-two-more-uarts.patch) to your QEMU platform that you will use to run Bao. This patch enables the launch of two additional UARTs, allowing each guest to operate on distinct consoles.

## 2. Run

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
nohup qemu-system-aarch64 -nographic -object memory-backend-file,id=virt.ram,size=16M,mem-path=/dev/bao-io-dispatcher,offset=0,share=on -machine virt,memory-backend=virt.ram,virtualization=on -cpu cortex-a53 -smp 1 -accel bao -device virtio-serial-device,iommu_platform=on,max_ports=1 -chardev pty,id=serial-bao-1 -device virtconsole,chardev=serial-bao-1 -global virtio-mmio.force-legacy=false -bao dm_id=0,irq=47 > /etc/log-serial-1 2>&1 &
```

```
nohup qemu-system-aarch64 -nographic -object memory-backend-file,id=virt.ram,size=16M,mem-path=/dev/bao-io-dispatcher,offset=1,share=on -machine virt,memory-backend=virt.ram,virtualization=on -cpu cortex-a53 -smp 1 -accel bao -device virtio-serial-device,iommu_platform=on,max_ports=1 -chardev pty,id=serial-bao-2 -device virtconsole,chardev=serial-bao-2 -global virtio-mmio.force-legacy=false -bao dm_id=1,irq=47 > /etc/log-serial-2 2>&1 &
```

To initiate a **single** QEMU process to manage the two VirtIO backends, run the command:

```
nohup qemu-system-aarch64 -nographic -object memory-backend-file,id=virt.ram,size=32M,mem-path=/dev/bao-io-dispatcher,offset=0,share=on -machine virt,memory-backend=virt.ram,virtualization=on -cpu cortex-a53 -smp 1 -accel bao -device virtio-serial-device,iommu_platform=on,max_ports=1 -chardev pty,id=serial-bao-1 -device virtconsole,chardev=serial-bao-1 -device virtio-serial-device,iommu_platform=on,max_ports=1 -chardev pty,id=serial-bao-2 -device virtconsole,chardev=serial-bao-2 -global virtio-mmio.force-legacy=false -bao dm_id=0-1,irq=47-46 > /etc/log-serials 2>&1 &
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
