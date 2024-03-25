# XZG Firmware
<div align="center"><img src="src/websrc/img/logo.svg" width="128" height="128" alt="XZG logo">
  
This repository contains the firmware for various, ESP32 based, Zigbee gateways.  
 
Firmware is opensource, so feel free to improve it <br> by making a commit to this repository. 
</div>

[![GitHub version](https://img.shields.io/github/release/xyzroe/xzg.svg)](https://github.com/xyzroe/xzg/releases)
[![GitHub download](https://img.shields.io/github/downloads/xyzroe/xzg/total.svg)](https://github.com/xyzroe/xzg/releases/latest)
[![License](https://img.shields.io/github/license/xyzroe/xzg.svg)](LICENSE)





### Previous versions:
| [ZigStarGW-FW](https://github.com/xyzroe/ZigStarGW-FW/releases/latest)                                                                                      | [UZG-01](https://github.com/mercenaruss/uzg-firmware/releases/latest)                                                                                                    | [SLZB-06](https://github.com/smlight-dev/slzb-06-firmware/releases/)                                                                                                             |
| ----------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [![ZigStarGW-FW's download](https://img.shields.io/github/downloads/xyzroe/ZigStarGW-FW/total.svg)](https://github.com/xyzroe/ZigStarGW-FW/releases/latest) | [![UZG-01 Firmware's download](https://img.shields.io/github/downloads/mercenaruss/uzg-firmware/total.svg)](https://github.com/mercenaruss/uzg-firmware/releases/latest) | [![UZG-01 Firmware's download](https://img.shields.io/github/downloads/smlight-dev/slzb-06-firmware/total.svg)](https://github.com/smlight-dev/slzb-06-firmware/releases/latest) |
  
  
  
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
Please refer to the installation and configuration articles based on your hardware.  

Please follow this link for web-flasher - just plug-and-flash: [XZG Web flasher 🚀](https://xzg.xyzroe.cc/) 

## Compiling from source

### Local

- You need npm and Python installed;
- Download this repository;
- Install Visual Studio Code (VSC);
- Install PlatformIO extension to VSC;
- Press "PlatformIO: Build" and wait until XZG*.bin is generated;

### Github
 - Fork this repository;
 - Made your changes;
 - Push a new tag in format "vX.X.X" to run workflow;
 - Just wait and get new release;
  
### Gitpod
[![Gitpod Ready-to-Code](https://img.shields.io/badge/Gitpod-Ready--to--Code-blue?logo=gitpod)](https://github.com/xyzroe/xzg)




## Contribute

You can contribute to XZG Firmware by
- Providing Pull Requests - please read contributing approach [here](CONTRIBUTING.md)
- Testing newly released features and reporting issues
- Contributing missing documentation

## Credits

People helping to keep the show on the road - **developers and contributors**:

- [@mercenaruss](https://github.com/mercenaruss/) for **Zig Star devices development**, for initial firmware release of UZG-01 version and giving me motivation and energy to implement new functions.

- [@Tarik2142](https://github.com/Tarik2142) for refactoring, code optimizations done under [smlight-dev](https://github.com/smlight-dev/)

- **TO-DO** - add all contributors automatic **HERE**!


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
- [knolleary / PubSubClient](https://github.com/knolleary/pubsubclient),
- [ESP Async WebServer](https://github.com/me-no-dev/ESPAsyncWebServer),
- [Martin-Laclaustra / CronAlarms](https://github.com/Martin-Laclaustra/CronAlarms)

  
## License and attribution

**XZG Firmware** is licensed under **GNU General Public License v3**.

3rd party libraries that are used by this project are licensed under different license schemes, please check them out as well. 

Copyright (c) 2024 xyzroe  

The GNU General Public License v3 ensures that if you use any part of this software in any way, your software must be released under the same license.  
