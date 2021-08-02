# ZiGate-Ethernet
 Ethernet platform for PiZiGate.
 For more informations, please go to the [description page](https://zigate.fr/documentation/descriptif-de-la-zigate-ethernet/)
 
 ![ZiGate-Ethernet](https://i0.wp.com/zigate.fr/wp-content/uploads/2021/07/ZiGate-ethernet-definition.jpg)
 
# Firmware
The platform use an ESP32 (16MB flash) with LAN8720 driver for ethernet

This program use following libraries :
- Webserver
- LITTLEFS
- [SoftwareSerial](https://www.arduino.cc/en/Reference/softwareSerial)
- [WiFiClient](https://www.arduino.cc/en/Reference/WiFiClient)
- [CircularBuffer](https://github.com/rlogiacco/CircularBuffer)
- [ArduinoJson](https://github.com/bblanchon/ArduinoJson)

Please be sure libraries are installed before compile

# How to flash release
Just install esptools and run this command

## Windows

`esptool.py.exe --chip esp32 --port COMXX --baud 921600 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size detect 0xe000 boot_app0.bin 0x1000 bootloader_dio_80m.bin 0x10000 ZiGate-Ethernet.ino.bin 0x8000 ZiGate-Ethernet.ino.partitions.bin 0x910000 ZiGate-Ethernet.littlefs.bin`




# Screenshots
## Status
![status](https://github.com/fairecasoimeme/ZiGate-Ethernet/blob/master/screenshots/status.jpg)
## WiFi
![wifi](https://github.com/fairecasoimeme/ZiGate-Ethernet/blob/master/screenshots/wifi.jpg)
## Ethernet
![Ethernet](https://github.com/fairecasoimeme/ZiGate-Ethernet/blob/master/screenshots/ethernet.JPG)
## Console
![console](https://github.com/fairecasoimeme/ZiGate-Ethernet/blob/master/screenshots/console.jpg)
