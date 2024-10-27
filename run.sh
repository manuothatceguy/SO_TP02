#!/bin/bash

sudo chmod a+x ./Image/*
# qemu-system-x86_64: -audiodev pa,id=speaker: Parameter 'driver' does not accept value 'pa'
# sudo qemu-system-x86_64 -hda Image/x64BareBonesImage.qcow2 -m 512 -audiodev pa,id=speaker -machine pcspk-audiodev=speaker


## MACOS
sudo qemu-system-x86_64 -hda Image/x64BareBonesImage.qcow2 -m 512 -audiodev coreaudio,id=speaker -machine pcspk-audiodev=speaker