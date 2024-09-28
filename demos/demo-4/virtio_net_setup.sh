#!/bin/bash

# Down eth0
ifconfig eth0 down

# Create tap0 and tap1 devices
ip tuntap add dev tap0 mode tap
ip tuntap add dev tap1 mode tap

# Create bridge bao-br0
brctl addbr bao-br0

# Assign IP address to bao-br0
ifconfig bao-br0 192.168.42.13 netmask 255.255.255.0

# Add interfaces to bridge bao-br0
brctl addif bao-br0 eth0
brctl addif bao-br0 tap0
brctl addif bao-br0 tap1

# Bring up eth0
ifconfig eth0 up

# Configure tap0 and tap1 devices and bring it up
ifconfig tap0 192.168.42.14 up
ifconfig tap1 192.168.42.15 up

# Launch 2 Bao VirtIO Device Models to serve the two Frontend's
nohup bao-virtio-dm --config ~/config-net-vm1.yaml > ~/bao-virtio-dm.log 2>&1 &
nohup bao-virtio-dm --config ~/config-net-vm2.yaml > ~/bao-virtio-dm.log 2>&1 &

# Launch only 1 Bao VirtIO Device Model to serve the two Frontend's
#nohup bao-virtio-dm --config ~/config-net-vms.yaml > ~/bao-virtio.log 2>&1 &