#!/bin/sh
python esptool.py --port "/dev/cu.usbserial-0001" erase_flash
python esptool.py --port "/dev/cu.usbserial-0001" --baud 460800 write_flash --verify -z --flash_mode dio --flash_freq 40m --flash_size detect 0x0 ../bin/SLZB-06*.full.bin

#python esptool.py --port "/dev/cu.usbserial-0001" --baud 460800 write_flash --verify -z --flash_mode dio --flash_freq 40m --flash_size detect 0x1000 bootloader_dio_40m.bin 0x8000 partitions.bin 0xe000 boot_app0.bin 0x10000 SLZB-06*.bin
