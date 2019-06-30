#!/bin/sh

set -e -u

if [ $# -lt 1 ]; then
    echo "Usage: $0 boot-target"
    exit 1
fi

KERNEL=$1

exec qemu-system-x86_64 -cpu Broadwell \
     -machine q35,accel=kvm,kernel-irqchip=split \
     -display none -smp 1 -no-reboot -vga none -debugcon stdio -kernel "$KERNEL" -s
