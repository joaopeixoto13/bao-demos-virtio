# Two Linux Guests accessing the same network interface

This demo features a triple Linux guest configuration where one of them will serve as VirtIO backend and the remaining two will be guests.  Within this setup, only the backend possesses access to the physical peripheral, while the two guests utilize the VirtIO interface for network access.
Furthermore, the backend has been configured to access the SD Card, thereby reducing the backend image size and enabling users to pass configuration files to the root filesystem without the necessity of rebuilding the kernel with INITRAMFS and an overlay directory.

## 1. Prerequisites

- [Config file](zcu102.c)
- [Linux backend device tree](backend.dts)
- [Linux frontend #1 device tree](frontend1.dts)
- [Linux frontend #2 device tree](frontend2.dts)
- [Virtio VM1 Configuration file](config-virtio-vm1.yaml)
- [Virtio VM2 Configuration file](config-virtio-vm2.yaml) 
- [Virtio VMs Configuration file](config-virtio-vms.yaml)
- [Network configuration + VirtIO launch script](virtio_net_setup.sh)
- [U-boot patch](0001-u-boot.patch)
- Xilinx ZCU102/4 board
- SD Card with 4GB+
- PC ethernet configured with a static IP of `192.168.42.12`

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

Download the [Bao VirtIO device model](https://github.com/joaopeixoto13/bao-virtio-dm):
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

After preparing your sd card with two partitions (one for boot called `boot` and other for the root filesystem called `root`), 
copy the firmware, bao's final image and root filesystem to it:
```
cp /path/to/your/imgs/BOOT.BIN /path/to/your/sdcard/boot
cp /path/to/your/imgs/bao.img /path/to/your/sdcard/boot
cp /path/to/your/imgs/bao.bin /path/to/your/sdcard/boot
sudo cp -rv /path/to/your/rootfilesystem /path/to/your/sdcard/root
umount /path/to/your/sdcard
```

## 6. Run

Upon powering on the board, load Bao using the following command:
```
fatload mmc 0 0x200000 bao.bin; go 0x200000
```

Configure the network and initiate the VirtIO devices using the following command:
```
./virtio_net_setup.sh
```

**Note**: The command provided will initiate, by default, two separate processes to operate the two VirtIO devices. 
Nevertheless, you have the option to edit the file to launch only a single instance to manage both VirtIO devices simultaneously, by uncomment the last line in [virtio_net_setup.sh](virtio_net_setup.sh) and comment out the previous two lines.

Now you can `ssh` into the two Frontend VMs:

- To access the **Frontend VM1** run:
```
ssh root@192.168.42.17
```

- To access the **Frontend VM2** run:
```
ssh root@192.168.42.18
```

## 7. Test cases

### 7.1 Net Drivers inspection

You can inspect the Net drivers of each VM by running:

- `ethtool -i eth0` on the Frontend VMs to verify the `virtio_net` Net Driver
- `ethtool -i eth0` on the Backend VM to verify the `macb` Net Driver

### 7.2 Test the connection between the two Frontend VMs

Using the `ping` command:

From the **Frontend VM1**:
```
ping 192.168.42.18
```

Or from the **Frontend VM2**:
```
ping 192.168.42.17
```

Also you can use the `iperf3` tool to test the connection between the two Frontend VMs:

On the **Frontend VM1**, create the server:
```
iperf3 -s
```

On the **Frontend VM2**, create the client:
```
iperf3 -c 192.168.42.17
```

### 7.3 Test the connection with an external device

You can test the connection with an external device connected to the same network using also the `iperf3` tool:
- From your laptop run the server: `iperf3 -s`
- From the Frontend VM1/VM2 run the client: `iperf3 -c 192.168.42.12`