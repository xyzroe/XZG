# ZigStar UZG-01 Zigbee 3.0 PoE Ethernet USB Adapter's Firmware


This repository contains the latest firmware for the ESP32 peripheral module of [UZG-01 Zigbee Ethernet PoE USB Adapter](https://uzg.zig-star.com/product). Firmware is opensource, so feel free to improve it by making a commit to this repository. 

[![GitHub version](https://img.shields.io/github/release/mercenaruss/uzg-firmware.svg)](https://github.com/mercenaruss/uzg-firmware/releases)
[![GitHub download](https://img.shields.io/github/downloads/mercenaruss/uzg-firmware/total.svg)](https://github.com/mercenaruss/uzg-firmware/releases/latest)
[![License](https://img.shields.io/github/license/mercenaruss/uzg-firmware.svg)](LICENSE.txt)
[![Gitpod Ready-to-Code](https://img.shields.io/badge/Gitpod-Ready--to--Code-blue?logo=gitpod)](https://github.com/mercenaruss/uzg-firmware)


## KEY FIRMWARE FEATURES
- Change Ethernet/USB adapter mode through firmware or by physical button short press (Red LED On = USB mode, RED LED Off = Ethernet mode);
- Adapter mode selector through web-interface: `Zigbee-to-Ethernet`, `Zigbee-to-USB` and `Zigbee-to-WiFI`;
- Support mDNS autodiscovery in your network (go in the browser to `uzg-01.local`), in Zigbee2MQTT (set `port: mdns://uzg-01`), and in ZHA;
- Secure login through username and password;
- Zigbee2MQTT and ZHA config helper;
- Control behavior of LED (you can disable Mode LED, Power LED through firmware, or toggle both by the button long press);
- DHCP or static IP address for Ethernet connection;
- ESP32 (peripheral) and CC2652P (Zigbee) OTA updates, ESP32  and CC2652P restart;
- Switch CC2652P (Zigbee) to flash mode;
- Pairing mode for the adapter in router mode;
- Filter IP addresses for incoming connections to secure your network;
- Fully responsive web interface based on the latest Bootstrap 5.3.
- WireGuard VPN
- MQTT client for gateway monitoring and control

## Installation and Configuration
Please refer to the installation and configuration articles in our [documentation](https://uzg.zig-star.com).  
Please follow this link for web-flasher - just plug-and-flash: [ZigStar WebFlasher](https://uzg.zig-star.com/webinstall/).

## Compiling from source

If you made changes to the code and want to compile you own firmware, please do the following:

- Download repository;
- Install Microsoft Visual Code (MVC);
- Install PlatformIO extension to MVC;
- Press "PlatformIO: Build" and wait until firmware.bin is generated;

## Contribute

You can contribute to UZG-01 Firmware by
- Providing Pull Requests (Features, Proof of Concepts or Fixes) - please read UZG-01 Firmware's contributing approach [here](CONTRIBUTING.md)
- Testing newly released features and reporting issues
- Contributing missing [documentation](https://uzg.zig-star.com);

## Credits

People helping to keep the show on the road - **developers and contributors**:

- [@xyzroe](https://github.com/xyzroe/) for initial firmware released for ZigStar devices and all work being doing till now  
- [@Tarik2142](https://github.com/Tarik2142) for refactoring, code optimizations done under [smlight-dev](https://github.com/smlight-dev/)


Special thanks goes also to all authors of 3rd party libraries which are used in this project:

- [espressif / arduino-esp32](https://github.com/espressif/arduino-esp32), 
- [esprfid / esp-rfid](https://github.com/esprfid/esp-rfid), 
- [fairecasoimeme / zigate-ethernet](https://github.com/fairecasoimeme/ZiGate-Ethernet), 
- [bblanchon / arduinojson](https://github.com/bblanchon/ArduinoJson), 
- [rlogiacco / circularbuffer](https://github.com/rlogiacco/CircularBuffer), 
- [sstaub / ticker](https://github.com/sstaub/Ticker), 
- [vurtun / lib](https://github.com/vurtun/lib),
- [ciniml / WireGuard-ESP32-Arduino](https://github.com/ciniml/WireGuard-ESP32-Arduino),  
- [plerup / EspSoftwareSerial](https://github.com/plerup/espsoftwareserial),
- [marian-craciunescu / ESP32Ping](https://github.com/marian-craciunescu/ESP32Ping),
- [sstaub / Ticker](https://github.com/sstaub/Ticker),
- [knolleary / PubSubClient](https://github.com/knolleary/pubsubclient);
- [ESP Async WebServer](https://github.com/me-no-dev/ESPAsyncWebServer),
- [Martin-Laclaustra / CronAlarms](https://github.com/Martin-Laclaustra/CronAlarms)

## License and attribution

UZG-01-Firmware is licensed under GNU General Public License v3.
3rd party libraries that are used by this project are licensed under different license schemes, please check them out as well.  
Copyright (c) 2023 ZigStar.  
The GNU General Public License v3 ensures that if you use any part of this software in any way, your software must be released under the same license.  
