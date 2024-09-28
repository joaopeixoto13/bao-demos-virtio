# A Linux Guest with a virtual console

This demo features a dual Linux guest configuration where one of them will serve as VirtIO backend and the other will be the guest. In this demo, only the backend has access to the real console and the Linux guest console can be accessed by Bao's VirtIO interface.

## 1. Prerequisites

- [Run script](run_bao.sh)
- [Config file](qemu-riscv64-virt.c)
- [Linux backend device tree](backend.dts)
- [Linux frontend device tree](frontend.dts)
- [Bao VirtIO DM config file](config-console.yaml)

## 2. Run

Following the execution of the [run script](run_bao.sh), and
once logged into the **Backend VM**, launch Bao's VirtIO device model:

```
nohup bao-virtio-dm --config /etc/config-console.yaml > /etc/bao-vmm.log 2>&1 &
```

Following this, verify the location to which the guest console was redirected by executing the command:

```
cat /etc/bao-vmm.log
```

You should observe output similar to:

```
virtio-console at /dev/pts/0
```

From now on, you can access the Linux guest by executing the following command:

```
picocom -b 115200 /dev/pts/0
```

Note that you first need to login into your guest by typing `root` (buildroot login) followed by `root` (buildroot password).
