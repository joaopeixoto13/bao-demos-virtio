# Inter-VM communication using Virtual Sockets

This demo entails a dual VM configuration wherein the communication between the two VMs is achieved through virtual sockets.

## 1. Prerequisites

- [Run script](run_bao.sh)
- [Config file](qemu-riscv64-virt.c)
- [Linux backend device tree](backend.dts)
- [Linux frontend device tree](frontend.dts)
- [Bao VirtIO DM config file](config-vhost-user-vsock.yaml)

## 2. Build Rust-VMM vhost-user-vsock device

To build the vhost-user-vsock device, follow [Rust-VMM instructions](https://github.com/rust-vmm/vhost-device/tree/main). 

**Note**: To compile for the aarch64 and riscv64 platforms, ensure that the [.cargo](https://github.com/joaopeixoto13/bao-virtio-dm/blob/main/.cargo/config.toml) file is included within the vhost-device root directory.

Copy the `.cargo` folder to the virtual filesystem root directory:
```
cp -r $BAO_DEMOS_BAO_VIRTIO_DM_DIR.cargo /path/to/your/vhost-device
```

Bbuild the vhost-user vsock device by executing:
```
cargo build --target=$BAO_DEMOS_BAO_VIRTIO_DM_TARGET --release -p vhost-device-vsock
```

Transfer the binary and the [configuration file](config-vhost-user-vsock.yaml) to your overlay directory:
```
cp /path/to/your/vhost-device/binary $BAO_DEMOS_BUILDROOT_OVERLAY_DIR/bin
cp $BAO_DEMOS_VIRTIO_DIR/demos/demo-8/config-vhost-user-vsock.yaml $BAO_DEMOS_BUILDROOT_OVERLAY_DIR/etc
```

## 3. Run

Following the execution of the [run script](run_bao.sh), and
once logged into the **Backend VM**, run the vhost-user vsock backend:
```
nohup vhost-device-vsock --vm guest-cid=4,uds-path=/tmp/vm4.vsock,socket=/tmp/Vsock0.sock > /etc/vhost-vsock.log 2>&1 &
```

Launch Bao's VirtIO device model:

```
nohup bao-virtio-dm --config /etc/config-vhost-user-vsock.yaml > /etc/bao-vmm.log 2>&1 &
```

Now you can use [iperf3](https://github.com/stefano-garzarella/iperf-vsock) tool to test both transmit and receive bandwidths.

### Test the transmit bandwidth

On the **Backend VM**:
```
iperf3 --vsock -s -B /tmp/vm4.vsock
```

On the **Frontend VM**:
```
iperf3 --vsock -c 2
```

### Test the receive bandwith

On the **Frontend VM**:
```
iperf3 --vsock -s
```

On the **Backend VM**:
```
iperf3 --vsock -c /tmp/vm4.vsock
```