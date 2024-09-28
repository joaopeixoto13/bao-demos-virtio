# Two Linux Guests accessing the same network interface

This demo features a triple Linux guest configuration (plus a FreeRTOS) where one of them will serve as VirtIO backend and the remaining two Linux will be guests. Within this setup, only the backend possesses access to the physical peripheral, while the two Linux guests utilize the VirtIO interface for network access.
Furthermore, the backend has been configured to access the SD Card, thereby reducing the backend image size and enabling users to pass configuration files to the root filesystem without the necessity of rebuilding the kernel with INITRAMFS and an overlay directory.

## 1. Prerequisites

- [Config file](zcu102.c)
- [Linux backend device tree](backend.dts)
- [Linux frontend #1 device tree](frontend1.dts)
- [Linux frontend #2 device tree](frontend2.dts)
- [Virtio VM1 Configuration file](config-net-vm1.yaml)
- [Virtio VM2 Configuration file](config-net-vm2.yaml) 
- [Virtio VMs Configuration file](config-net-vms.yaml)
- [Network configuration + VirtIO launch script](virtio_net_setup.sh)
- [U-boot patch](../../patches/0001-u-boot-switch-to-el1.patch)
- Xilinx ZCU102/4 board
- SD Card with 4GB+
- PC ethernet configured with a static IP of `192.168.42.12`

## 2. Build firmware

To build the firmware for ZCU102 platform, follow the [build](../../configs/firmware/zcu102.md) instructions.

## 3. Build Bao

After completing the preceding steps, you may proceed to build Bao by following the [bao-demos](https://github.com/bao-project/bao-demos/tree/master) tutorial. 

## 4. Setup SD card

After preparing your sd card with two partitions (one for boot called `boot` and other for the root filesystem called `root`), 
copy the firmware, bao's final image and root filesystem to it:
```
cp /path/to/your/imgs/BOOT.BIN /path/to/your/sdcard/boot
cp /path/to/your/imgs/bao.img /path/to/your/sdcard/boot
cp /path/to/your/imgs/bao.bin /path/to/your/sdcard/boot
sudo cp -rv /path/to/your/rootfilesystem /path/to/your/sdcard/root
umount /path/to/your/sdcard
```

## 5. Run

Upon powering on the board, load Bao using the following command:
```
fatload mmc 0 0x200000 bao.bin; go 0x200000
```

Configure the network and initiate the VirtIO devices using the following command:
```
./virtio_net_setup.sh
```

**Note**: The command provided will initiate, by default, two separate processes to run the two VirtIO backends. 
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

## 6. Test cases

### 6.1 Net Drivers inspection

You can inspect the Net drivers of each VM by running:

- `ethtool -i eth0` on the Frontend VMs to verify the `virtio_net` Net Driver
- `ethtool -i eth0` on the Backend VM to verify the `macb` Net Driver

### 6.2 Test the connection between the two Frontend VMs

Using the `ping` command:

From the **Frontend VM1**:
```
ping 192.168.42.18
```

Or from the **Frontend VM2**:
```
ping 192.168.42.17
```

Also you can use the `iperf3` tool to test the network bandwidth between the two Frontend VMs:

On the **Frontend VM1**, create the server:
```
iperf3 -s
```

On the **Frontend VM2**, create the client:
```
iperf3 -c 192.168.42.17
```

### 6.3 Test the network bandwidth with an external device

You can test the connection with an external device connected to the same network using also the `iperf3` tool:
- From your laptop run the server: `iperf3 -s`
- From the Frontend VM1/VM2 run the client: `iperf3 -c 192.168.42.12`