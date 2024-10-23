# Bao QEMU Device Model Setup

Setup global environment variables:
```
export BAO_DEMOS_QEMU_DM_URL=git@github.com:joaopeixoto13/qemu.git
export BAO_DEMOS_QEMU_DM_DIR=$BAO_DEMOS_WRKDIR/qemu
export QEMU_OVERRIDE_SCRDIR=$BAO_DEMOS_QEMU_DM_DIR
```

Clone the repo:
```
git clone $BAO_DEMOS_QEMU_DM_URL $BAO_DEMOS_QEMU_DM_DIR --depth=1 --branch=bao-virtio
```

**Note**: If you already have a QEMU source code in your system and you don't want to donwload a new one, you can simply apply one of the following patches: [qemu-v8.0.3 patch](../../../patches/0001-qemu-v8.0.3-bao-device-model.patch) or [qemu-v8.1.0 patch](../../../patches/0001-qemu-v8.1.0-bao-device-model.patch).
The two patches are identical with the difference that in QEMU v8.0.3 we added the offset field to QEMU's `-object memory-backend-file` property.
In QEMU v8.1.0+, this field has already been implemented.

```
export BAO_DEMOS_QEMU_DM_DIR=/path/to/your/qemu
cd $BAO_DEMOS_QEMU_DM_DIR
```

If you are using QEMU v8.0.3:
```
git apply $BAO_DEMOS_VIRTIO_DIR/patches/0001-qemu-v8.0.3-bao-device-model.patch
```

If you are using QEMU v8.1.0:
```
git apply $BAO_DEMOS_VIRTIO_DIR/patches/0001-qemu-v8.1.0-bao-device-model.patch
```

Please note that the patches were made for QEMU v8.0.3 and v8.1.0, so earlier and future versions may have some conflicts that you must resolve manually.

Configure QEMU:
```
cd $BAO_DEMOS_QEMU_DM_DIR
./configure CROSS_COMPILE=$CROSS_COMPILE ARCH=$ARCH --target-list=aarch64-softmmu --enable-slirp --enable-bao
```

Enable QEMU package in the buildroot configuration file:
```
echo "BR2_PACKAGE_QEMU=y" >> $BAO_DEMOS_VIRTIO_DIR/configs/buildroot/$ARCH.config
echo "BR2_PACKAGE_QEMU_SYSTEM=y" >> $BAO_DEMOS_VIRTIO_DIR/configs/buildroot/$ARCH.config
echo "BR2_PACKAGE_QEMU_BLOBS=y" >> $BAO_DEMOS_VIRTIO_DIR/configs/buildroot/$ARCH.config
echo "BR2_PACKAGE_QEMU_SLIRP=y" >> $BAO_DEMOS_VIRTIO_DIR/configs/buildroot/$ARCH.config
echo "BR2_PACKAGE_QEMU_FDT=y" >> $BAO_DEMOS_VIRTIO_DIR/configs/buildroot/$ARCH.config
```