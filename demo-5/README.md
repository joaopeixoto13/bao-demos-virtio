# Inter-VM communication using Virtual Sockets

This demo entails a dual VM configuration wherein communication between the two VMs is facilitated through virtual sockets. Vsock, a Linux socket family, has been specifically crafted to enable bidirectional and many-to-one communication among VMs. Leveraging the VM Sockets API (AF_VSOCK), both user-level applications within a VM and those on the host system can efficiently communicate. This framework facilitates swift communication between guest VMs and their host without relying on the host's networking stack. Consequently, VMs can be configured entirely without traditional networking, relying solely on VM sockets for communication. Each host and VM possesses a 32-bit Context IDentifier (CID) and can connect or bind to a 32-bit port number. Notably, ports with values less than 1024 are considered privileged ports.

## 1. Prerequisites

- [Config file](zcu102.c)
- [Linux backend device tree](backend.dts)
- [Linux frontend device tree](frontend.dts)
- [Vhost-vsock Configuration file](config-vhost-vosck.yaml) 
- [U-boot patch](0001-u-boot.patch)
- Xilinx ZCU102/4 board
- VirtIO vsockets support on the Frontend VM (e.g. `CONFIG_VIRTIO_VSOCKETS` on buildroot)
- Vhost vsock device support on the Backend VM (e.g. `CONFIG_VHOST_VSOCK` on buildroot)

---

## 2. Build the Bao VirtIO Device Model

If Rust is not already installed, you can install it using:
```
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
```

When you first install a toolchain, rustup installs only the standard library for your host platform - that is, the architecture and operating system you are presently running. To compile to other platforms you must install other target platforms. In this instance, execute the following command to add the `aarch64` target:
```
rustup target add aarch64-unknown-linux-gnu
```

Download the [Bao VirtIO device model](https://github.com/joaopeixoto13/bao-virtio-dm)
```
mkdir /path/to/your/bao-virtio-dm
git clone git@github.com:joaopeixoto13/bao-virtio-dm.git /path/to/your/bao-virtio-dm
```

Build the Bao VirtIO Device Model:
```
cd /path/to/your/bao-virtio-dm
cargo build --target=aarch64-unknown-linux-gnu --release
```

## 3. Build firmware

Download latest Xilinx u-boot:
```
git clone git@github.com:Xilinx/u-boot-xlnx.git --branch=xilinx-v2023.2 --depth=1 /path/to/your/u-boot-xlnx
cd /path/to/your/u-boot-xlnx
```

Apply the u-boot patch:
```
git apply /path/to/your/demo-4/0001-u-boot.patch
```

Configure u-boot:
```
make distclean
make xilinx_zynq_virt_defconfig
```

Export your ZCU102/4 device tree defconfig:
```
export xilinx_zynqmp_zcu102_rev1_0_defconfig
```

Build u-boot:
```
make -j12
```

Download the latest pre-built Zynq UltraScale+ MPSoC firmware:
```
git clone https://github.com/Xilinx/soc-prebuilt-firmware.git --branch=xilinx_v2023.1 --depth=1 /path/to/your/zcu-firmware
```

Replace the u-boot binary under your zcu-firmware platform directory:
```
cp /path/to/your/u-boot-xlnx/u-boot.elf /path/to/your/zcu-firmware/zcu102-zynqmp/u-boot.elf
```

Use bootgen to build the firmware binary:
```
cd /path/to/your/zcu-firmware/zcu102-zynqmp
bootgen -arch zynqmp -image bootgen.bif -w -o /path/to/your/imgs/BOOT.BIN
```

Run mkimage to create the final system image:
```
mkimage -n bao_uboot -A arm64 -O linux -C none -T kernel -a 0x200000 -e 0x200000 -d /path/to/your/imgs/bao.bin /path/to/your/imgs/bao.img
```

## 4. Build Bao

After completing the preceding steps, you may proceed to build Bao by following the instructions provided on the [official Bao website](https://github.com/bao-project/bao-hypervisor).

## 5. Setup SD card

After preparing your sd, copy the firmware and bao's final image to it:
```
cp /path/to/your/imgs/BOOT.BIN /path/to/your/sdcard/boot
cp /path/to/your/imgs/bao.img /path/to/your/sdcard/boot
cp /path/to/your/imgs/bao.bin /path/to/your/sdcard/boot
umount /path/to/your/sdcard
```

## 6. Run

Upon powering on the board, load Bao using the following command:
```
fatload mmc 0 0x200000 bao.bin; go 0x200000
```

Launch the **device model** with vhost vsock frontend device:

```
nohup bao-virtio-dm --config /PATH/TO/YOUR/config-virtio-vsock.yaml > /etc/bao-virtio-dm.log 2>&1 &
```

## 7. Validate the communication

To validate the communication between the two VMs, you can use a modified [iperf3](https://github.com/stefano-garzarella/iperf-vsock) version with `AF_VSOCK` support.

In this scenario, from the **Frontend VM** you can create the server:
```
iperf3 --vsock -s
```

And from the **Backend VM** you can create the client:
```
iperf3 --vsock -c 3
```

**Note**: The value `3` is the Context IDentifier (CID) passed on the [configuration file](config-vhost-vosck.yaml).

Also, you can use the `socat` command to verify the communication channel. From the **Frontend VM** type:
```
socat - SOCKET-LISTEN:40:0:x00x00x00x04x00x00x03x00x00x00x00x00x00x00
```

And from the **Backend VM**:
```
sudo socat - SOCKET-CONNECT:40:0:x00x00x00x04x00x00x03x00x00x00x00x00x00x00
```

Additionaly, you can `ssh` into the Frontend VM from the **Backend VM** on port 2222:
```
ssh root@localhost -p 2222
```