# Inter-VM communication using Virtual Sockets

This demo entails a dual VM configuration wherein the communication between the two VMs is achieved through virtual sockets. Vsock, a Linux socket family, has been specifically crafted to enable bidirectional and many-to-one communication among VMs. Leveraging the VM Sockets API (AF_VSOCK), both user-level applications within a VM and those on the host system can efficiently communicate. This framework facilitates swift communication between guest VMs and their host without relying on the host's networking stack. Consequently, VMs can be configured entirely without traditional networking, relying solely on VM sockets for communication. Each host and VM possesses a 32-bit Context IDentifier (CID) and can connect or bind to a 32-bit port number. Notably, ports with values less than 1024 are considered privileged ports.

## 1. Prerequisites

- [Config file](zcu102.c)
- [Linux backend device tree](backend.dts)
- [Linux frontend device tree](frontend.dts)
- [Vhost-vsock Configuration file](config-vosck.yaml) 
- [U-boot patch](../../patches/0001-u-boot-switch-to-el1.patch)
- Xilinx ZCU102/4 board
- VirtIO vsockets support on the Frontend VM (e.g. `CONFIG_VIRTIO_VSOCKETS`)
- Vhost vsock device support on the Backend VM (e.g. `CONFIG_VHOST_VSOCK`)

## 2. Build firmware

To build the firmware for ZCU102 platform, follow the [build](../../configs/firmware/zcu102.md) instructions.

## 3. Build Bao

After completing the preceding steps, you may proceed to build Bao by following the [bao-demos](https://github.com/bao-project/bao-demos/tree/master) tutorial. 

## 4. Setup SD card

After preparing your sd, copy the firmware and bao's final image to it:
```
cp /path/to/your/imgs/BOOT.BIN /path/to/your/sdcard/boot
cp /path/to/your/imgs/bao.img /path/to/your/sdcard/boot
cp /path/to/your/imgs/bao.bin /path/to/your/sdcard/boot
umount /path/to/your/sdcard
```

## 5. Run

Upon powering on the board, load Bao using the following command:
```
fatload mmc 0 0x200000 bao.bin; go 0x200000
```

Launch the **device model** with vhost vsock frontend device:

```
nohup bao-virtio-dm --config /etc/config-vsock.yaml > /etc/bao-virtio-dm.log 2>&1 &
```

To measure the communication bandwidth between the two VMs, you can use a modified [iperf3](https://github.com/stefano-garzarella/iperf-vsock) version with `AF_VSOCK` support. Follow the respective build instructions and copy the binary into your root filesystem.

In this scenario, from the **Frontend VM** you can create the server:
```
iperf3 --vsock -s
```

And from the **Backend VM** you can create the client to measure the receive bandwidth (host --> guest):
```
iperf3 --vsock -c 3
```

**Note**: The value `3` is the Context IDentifier (CID) passed on the [configuration file](config-vosck.yaml).

You can create the server also from the **Backend VM**:
```
iperf3 --vsock -s
```

And run the client client on the **Frontend VM** to measure the transmit bandwith (guest --> host):
```
iperf3 --vsock -c 2
```

**Note**: The CID of the host of the communication (in this case the backend VM) always assume the value of `2`.

Also, you can use the `socat` command to verify the communication channel.
The corresponding package (`BR2_PACKAGE_SOCAT`) is enabled by default if you use one of the buildroot config files.
From the **Frontend VM** type:
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