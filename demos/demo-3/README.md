# Shared file system across frontend and backend VMs

This demo enables the frontend and backend VMs to share information in a more convenient and high-level manner via a shared file system.
The interface utilizes VirtIO, while the dataplane is realized through the [vhost-user protocol](https://www.redhat.com/en/blog/journey-vhost-users-realm).

About the VirtIO file system device, it provides file system access and can either directly manages a file system or acting as a gateway to a remote file system. The device interface is based on the Linux Filesystem in Userspace (FUSE) protocol. The device acts as the FUSE file system daemon and the driver acts as the FUSE client mounting the file system. The virtio file system device provides the mechanism for transporting FUSE requests, much like */dev/fuse* in a traditional FUSE application.

## 1. Prerequisites

- [Run script](run_bao.sh)
- [Config file](qemu-aarch64-virt.c)
- [Linux backend device tree](backend.dts)
- [Linux frontend device tree](frontend.dts)
- [Virtual filesystem configuration file example](config-fs.yaml)
- [Virtual file system binary](virtiofsd) (compiled for aarch64)

Also, to facilitate the execution of this demo, it's essential to apply the [patch](../../patches/0001-qemu-arm-virt-add-two-more-uarts.patch) to your local/laptop QEMU. This patch enables the launch of two additional UARTs, allowing each guest to operate on distinct consoles.

## 2. Build the Virtual Filesystem backend

Build the VirtIO file system backend according to the official [source](https://gitlab.com/virtio-fs/virtiofsd).

**Note**: To compile for the aarch64 and riscv64 platforms, ensure that the same [.cargo](https://github.com/joaopeixoto13/bao-virtio-dm/blob/main/.cargo/config.toml) file in bao-virtio-dm is included within the virtual filesystem root directory.

Copy the `.cargo` folder to the virtual filesystem root directory:
```
cp -r $BAO_DEMOS_BAO_VIRTIO_DM_DIR.cargo /path/to/your/virtiofsd
```

After confirming/installing the presence of both `libcap-ng` and `libseccomp` dependencies for the aarch64 architecture, proceed to build the virtual file system daemon by executing:
```
cargo build --target=$BAO_DEMOS_BAO_VIRTIO_DM_TARGET --release
```

**Note**: Note that the **Backend VM** should have the same libraries. If you are using buildroot, you can select both using 
`BR2_PACKAGE_LIBCAP_NG=y` and `BR2_PACKAGE_LIBSECCOMP=y`. If you are using the provided buildroot config files, these options are enabled by default. 

Transfer the binaries (`virtiofsd` and `bao-virtio-dm`) and the [configuration file](config-fs.yaml) to your overlay directory (if you are using INITRAMFS) or directly into your file system and proceed with rebuilding the backend buildroot.
```
cp $BAO_DEMOS_BAO_VIRTIO_DM_BIN/bao-virtio-dm $BAO_DEMOS_BUILDROOT_OVERLAY_DIR/bin
cp /path/to/your/virtiofsd/binary $BAO_DEMOS_BUILDROOT_OVERLAY_DIR/bin
cp $BAO_DEMOS_VIRTIO_DIR/demos/demo-3/config-fs.yaml $BAO_DEMOS_BUILDROOT_OVERLAY_DIR/etc
```

## 3. Run

Following the execution of the [run script](run_bao.sh), you should observe output similar to the following:

```
char device redirected to /dev/pts/x1 (label serial1)
char device redirected to /dev/pts/x2 (label serial2)
```

Open two separate terminals and establish connections. Below is an illustrative example:

```
picocom -b 115200 /dev/pts/x1
picocom -b 115200 /dev/pts/x2
```

Once logged into the **Backend VM**, run the vhost-user filesystem backend:
```
nohup virtiofsd --socket-path=/root/Fs0.sock --shared-dir /mnt --tag=myfs --announce-submounts --sandbox chroot > /etc/vhost-device-virtiofsd.log 2>&1 &
```

**Note**: This command configures the /mnt folder as the shared directory, defines the tag for the VirtIO device as `myfs` and specifies the socket path. Additionally some flags are set to enable multiple mountpoints within the same shared directory and to designate the shared directory tree as its root.

Next, run Bao's VirtIO device model:
```
nohup bao-virtio-dm --config /etc/config-fs.yaml > /etc/bao-virtio-dm.log 2>&1 &
```

Following the boot of the **Frontend VM**, mount the shared directory on the guest using the respective tag:
```
mount -t virtiofs myfs /mnt
```

At this point, the guest has access to the filesystem, enabling read and write operations. You can create or edit files on the guest side and verify corresponding changes on the host, and vice versa.
