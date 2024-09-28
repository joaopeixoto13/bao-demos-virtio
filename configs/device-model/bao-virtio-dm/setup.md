# Bao VirtIO Device Model Setup

Setup global environment variables:
```
export BAO_DEMOS_BAO_VIRTIO_DM_URL=git@github.com:joaopeixoto13/bao-virtio-dm.git
export BAO_DEMOS_BAO_VIRTIO_DM_DIR=$BAO_DEMOS_WRKDIR/bao-virtio-dm
```

If your target is aarch64:
```
export BAO_DEMOS_BAO_VIRTIO_DM_TARGET=aarch64-unknown-linux-gnu
```

If your target is riscv64:
```
export BAO_DEMOS_BAO_VIRTIO_DM_TARGET=riscv64gc-unknown-linux-gnu
```

Export a variable pointing to the final binary:
```
export BAO_DEMOS_BAO_VIRTIO_DM_BIN=$BAO_DEMOS_BAO_VIRTIO_DM_DIR/target/$BAO_DEMOS_BAO_VIRTIO_DM_TARGET/release
```

Clone the repo:
```
git clone $BAO_DEMOS_BAO_VIRTIO_DM_URL $BAO_DEMOS_BAO_VIRTIO_DM_DIR --depth=1 --branch=main
```

Build:
```
cd $BAO_DEMOS_BAO_VIRTIO_DM_DIR
cargo build --target=$BAO_DEMOS_BAO_VIRTIO_DM_TARGET --release
```

Copy the binary to the overlay bin directory:
```
cp $BAO_DEMOS_BAO_VIRTIO_DM_BIN/bao-virtio-dm $BAO_DEMOS_BUILDROOT_OVERLAY_DIR/bin
```