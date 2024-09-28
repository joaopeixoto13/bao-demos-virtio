# A Linux Guest with storage interface

This demo features a dual Linux guest configuration where one of them will serve as VirtIO backend and the other will be the guest. In this demo, only the backend has access to the peripheral and the Linux guest use the VirtIO interface to access the storage.

## 1. Prerequisites

- [Run script](run_bao.sh)
- [Config file](qemu-aarch64-virt.c)
- [Linux backend device tree](backend.dts)
- [Linux frontend device tree](frontend.dts)
- [Bao VirtIO DM config file](config-block.yaml)

Also, to facilitate the execution of this demo, it's essential to apply the [patch](../../patches/0001-qemu-arm-virt-add-two-more-uarts.patch) to your local/laptop QEMU. This patch enables the launch of two additional UARTs, allowing each guest to operate on distinct consoles.

## 2. Setup your filesystem

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

## 3. Run

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

Run the device model which have the VirtIO backend device:
```
nohup bao-virtio-dm --config /etc/config-block.yaml > /etc/bao-virtio-dm.log 2>&1 &
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
