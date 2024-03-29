#!/bin/bash

# Create tap0 and tap1 devices
ip tuntap add dev tap0 mode tap
ip tuntap add dev tap1 mode tap

# Configure tap0 and tap1 devices
ifconfig tap0 192.168.42.14 up
ifconfig tap1 192.168.42.15 up

# Create bridge bao-br0
brctl addbr bao-br0

# Assign IP address to bao-br0
ifconfig bao-br0 192.168.42.13 netmask 255.255.255.0

# Add interfaces to bridge bao-br0
brctl addif bao-br0 eth0
brctl addif bao-br0 tap0
brctl addif bao-br0 tap1

# Launch 2 Bao VirtIO Device Models
nohup bao-virtio --config ~/config-virtio-vm1.yaml > ~/bao-virtio.log 2>&1 &
nohup bao-virtio --config ~/config-virtio-vm2.yaml > ~/bao-virtio.log 2>&1 &

# Launch only 1 Bao VirtIO Device Model
#nohup bao-virtio --config ~/config-virtio-vms.yaml > ~/bao-virtio.log 2>&1 &