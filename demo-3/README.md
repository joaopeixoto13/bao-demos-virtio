# A Linux Guest with virtual filesystem interface

This demo features a dual Linux guest configuration where one of them will serve as VirtIO backend and the other will be the guest. The interface utilizes VirtIO, while the dataplane is realized through the [vhost-user protocol](https://www.redhat.com/en/blog/journey-vhost-users-realm). Both sides of the vhost-user protocol are implemented in Rust, where the backend devices are created as hypervisor-agnostic and rust-based vhost-user backends by the rust-vmm and ORKO project community. The master/frontend leverages the crates offered by both the rust-vmm and ORKOS projects and its implementation is encapsulated within the [bao-virtio workspace](https://github.com/joaopeixoto13/bao-virtio).

About the VirtIO file system device, it provides file system access and can either directly manages a file system or acting as a gateway to a remote file system. The device interface is based on the Linux Filesystem in Userspace (FUSE) protocol. The device acts as the FUSE file system daemon and the driver acts as the FUSE client mounting the file system. The virtio file system device provides the mechanism for transporting FUSE requests, much like */dev/fuse* in a traditional FUSE application.

## 1. Prerequisites

- [Run script](run_bao.sh)
- [Config file](qemu-aarch64-virt.c)
- [Linux backend device tree](backend.dts)
- [Linux frontend device tree](frontend.dts)
- [Virtual file system binary](virtiofsd)
- [Virtual filesystem configuration file example](config-fs.yaml)
- `VIRTIO_FS` driver support on the Frontend VM

Also, to facilitate the execution of this demo, it's essential to apply the [patch](../0001-arm-virt-add-two-more-uarts.patch) to your local/laptop QEMU. This patch enables the launch of two additional UARTs, allowing each guest to operate on distinct consoles.

Rebuild the backend buildroot.

## 2. Build the Bao VirtIO Device Model

If Rust is not already installed, you can install it using:
```
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
```

When you first install a toolchain, rustup installs only the standard library for your host platform - that is, the architecture and operating system you are presently running. To compile to other platforms you must install other target platforms. In this instance, execute the following command to add the `aarch64` target:
```
rustup target add aarch64-unknown-linux-gnu
```

Download the [Bao VirtIO device model](https://github.com/joaopeixoto13/bao-virtio)
```
mkdir /path/to/your/bao-virtio
git clone git@github.com:joaopeixoto13/bao-virtio.git /path/to/your/bao-virtio
```

Build the Bao VirtIO Device Model:
```
cd /path/to/your/bao-virtio
cargo build --target=aarch64-unknown-linux-gnu --release
```

## 3. Build the Virtual Filesystem backend

Build the VirtIO file system backend accordingly by following link: https://gitlab.com/virtio-fs/virtiofsd

**Note**: To compile for the aarch64 platform, ensure that the same `.cargo` file in bao-virtio is included within the virtual filesystem root directory. Additionally, a small [patch](https://github.com/joaopeixoto13/vm-memory/commit/f53e1b9b30e19679ca531ac4fb1a77215d826b46) to the vm-memory crate must be applied.

Copy the `.cargo` folder to the virtual filesystem root directory:
```
cp -r /path/to/your/bao-vhost-frontend/.cargo /path/to/your/virtiofsd
```

To apply the patch, Rust provides a mechanism in Cargo.toml to override a dependency. Consequently, navigate to your `Cargo.toml` file within the virtual filesystem root directory and **append the following lines at the end of the file**:
```
[patch.crates-io]
vm-memory = { git = "https://github.com/joaopeixoto13/vm-memory", branch = "bao-hypervisor" }
```

After confirming/installing the presence of both `libcap-ng` and `libseccomp` dependencies for the aarch64 architecture, proceed to build the virtual file system daemon by executing:
```
cargo build --target=aarch64-unknown-linux-gnu --release
```

**Note**: Note that the **Backend VM** should have the same libraries. If you are using buildroot, you can select both using `make menuconfig` under `Libraries -> Other -> libcap-ng` and `Libraries -> Other -> libseccomp`.

Transfer the binaries (`virtiofsd` and `bao-virtio`) and the [configuration file](config-fs.yaml) to your overlay directory (if you are using INITRAMFS) or directly into your file system and proceed with rebuilding the backend buildroot.

**Note**: If you are using `INITRAMFS` and Buildroot to build your OS, include the following line in the `.config` file and rebuild the system.
```
BR2_ROOTFS_OVERLAY="/path/to/your/overlay_directory"
```

## 4. Run

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

As you proceed with the execution, expect the frontend to pause when the VirtIO configuration initiates. This pause occurs because the VirtIO backends, responsible for handling VirtIO requests, have not been launched yet.

**Note**: Most VirtIO implementations typically demand setting up VirtIO backends before any frontends. However, our approach remains entirely agnostic, offering the flexibility to attach VirtIO clients both before and after frontend execution, without strict dependencies on order.

Once logged into the **Backend VM**, run the vhost-user filesystem backend:
```
nohup virtiofsd --socket-path=/root/Fs.sock --shared-dir /mnt --tag=myfs --announce-submounts --sandbox chroot > /etc/vhost-device-virtiofsd.log 2>&1 &
```

**Note**: This command configures the /mnt folder as the shared directory, defines the tag for the VirtIO device as myfs and specifies the socket path. Additionally some flags are set to enable multiple mountpoints within the same shared directory and to designate the shared directory tree as its root.

Next, run Bao's VirtIO device model:
```
nohup bao-virtio --config /etc/config-fs.yaml > /etc/bao-virtio.log 2>&1 &
```

Following the boot of the **Frontend VM**, mount the shared directory on the guest using the respective tag:
```
mount -t virtiofs myfs /mnt
```

At this point, the guest has access to the filesystem, enabling read and write operations. You can create or edit files on the guest side and verify corresponding changes on the host, and vice versa.
