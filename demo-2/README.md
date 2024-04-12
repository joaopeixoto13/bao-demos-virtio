# A Linux Guest with network and storage interface

This demo features a dual Linux guest configuration where one of them will serve as VirtIO backend and the other will be the guest. In this demo, only the backend has access to the physical peripheral and the Linux guest use the VirtIO interface to access both network and storage.

## 1. Prerequisites

- [Run script](run_bao.sh)
- [Config file](qemu-aarch64-virt.c)
- [Linux backend device tree](backend.dts)
- [Linux frontend device tree](frontend.dts)

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

## 3. Setup your filesystem

Before executing the run script, it's necessary to prepare a demo filesystem designated for both the backend VM and the frontend VM. Open a terminal on your laptop and execute the following command:

```
mkdir ext4_mountpoint
mkdir ext4_frontend_vm
```

Create empty disk images:

```
dd if=/dev/zero of=ext4_image.img bs=1M count=100
dd if=/dev/zero of=ext4_frontend_vm_image.img bs=1M count=10
```

Format the disk images as ext4:
```
sudo mkfs.ext4 ext4_image.img
sudo mkfs.ext4 ext4_frontend_vm_image.img
```

**Note**: In this instance, we're generating a partition of 100MB and a sub-partition of 10MB. Feel free to adjust these sizes, ensuring consideration for the sizes of the VM's images involved.

Create the two mount points and mount the ext4 images:

```
sudo mount -o loop ext4_image.img ext4_mountpoint
sudo mount -o loop ext4_frontend_vm_image.img ext4_frontend_vm
```

Feel free to modify the filesystem according to your requirements at this stage. Create folders, files, or make any necessary edits to suit your needs.

Unmount the ext4 frontend image:
```
sudo umount ext4_frontend_vm
```

Move the frontend disk image to the backend VM directory:
```
sudo mv ext4_frontend_vm_image.img ext4_mountpoint
```

Unmount the backend ext4 image:

```
sudo umount ext4_mountpoint
```

## 4. Run

Following the execution of the [run script](run_bao.sh), you should observe output similar to the following:

```
char device redirected to /dev/pts/x1 (label serial1)
char device redirected to /dev/pts/x2 (label serial2)
```

Open three separate terminals and establish connections. Below is an illustrative example:

```
picocom -b 115200 /dev/pts/x1
picocom -b 115200 /dev/pts/x2
```

As you proceed with the execution, expect the frontend to pause when the VirtIO configuration initiates. This pause occurs because the VirtIO backends, responsible for handling VirtIO requests, have not been launched yet.

**Note**: Most VirtIO implementations typically demand setting up VirtIO backends before any frontends. However, our approach remains entirely agnostic, offering the flexibility to attach VirtIO clients both before and after frontend execution, without strict dependencies on order.

Once logged into the **Backend VM**, first you will need to mount the partition into your filesystem. 

Create a directory to mount the filesystem:

```
mkdir /etc/ext4_mountpoint
```

Mount the `/dev/vda` to the directory previously created:

```
mount /dev/vda /etc/ext4_mountpoint
```

**Note**: `/dev/vda` is the first detected paravirtualizated disk driver.

Edit the filesystem as you need.

Now, you have two options available for launching the VirtIO backends:

1) Execute the launch in distinct QEMU processes to maintain the isolation of each entity involved

2) Initiate a single QEMU process designated to manage the specified VirtIO backends

For simplicity, let's use the second approach. However, if you want to launch two independent QEMU processes, please consult [demo-1](../demo-1/README.md).

Run the command:
```
nohup qemu-system-aarch64 -nographic -object memory-backend-file,id=virt.ram,size=32M,mem-path=/dev/baoipc0,offset=0,share=on -machine virt,memory-backend=virt.ram,virtualization=on -cpu cortex-a53 -smp 1 -accel bao -device virtio-blk-device,drive=drive2,id=virtblk1,iommu_platform=on -drive file=/etc/ext4_mountpoint/ext4_frontend_vm_image.img,id=drive2,if=none,format=raw -device virtio-net-device,netdev=net1,iommu_platform=on -netdev user,id=net1,net=192.168.43.0/24,hostfwd=tcp:127.0.0.1:5555-:22 -global virtio-mmio.force-legacy=false -bao dm_id=0-1,irq=47-46,shmem_addr=536870912-553648128,shmem_size=16777216-16777216 > /etc/log-blk-net 2>&1 &
```

Once logged into the **Frontend VM**, create a directory to mount the partition:

```
mkdir /etc/ext4_mountpoint
```

Mount the `/dev/vda` to the directory previously created:
```
mount /dev/vda /etc/ext4_mountpoint
```

Edit the filesystem as you need.

To unmount the filesystem, type:

```
umount /etc/ext4_mountpoint
```

You should now be able to verify whether the changes have been correctly implemented, visible either in the Backend VM or on your laptop. These alterations persist even after restarting Linux (**non-volatile memory**), ensuring that all previously made changes remain intact and observable.

To test the VirtIO network interface, you can establish a connection from the **Backend VM** to the Frontend VM using the `ssh` command.

```
ssh localhost -p 5555
```

At this point, you should have access to navigate the file system within the Frontend VM, allowing you to retrieve and interact with all the content residing within it.
