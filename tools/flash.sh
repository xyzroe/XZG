#!/bin/sh
python esptool.py --port "/dev/cu.usbserial-0001" erase_flash
python esptool.py --port "/dev/cu.usbserial-0001" --baud 460800 write_flash --verify -z --flash_mode dio --flash_freq 40m --flash_size detect 0x0 ../bin/ZigStarGW*.bin
