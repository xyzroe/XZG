# SMLIGHT SLZB-06 Zigbee 3.0 PoE Ethernet USB Adapter's FIRMWARE
## Latest stable release: [v0.9.9](https://github.com/smlight-dev/slzb-06-firmware/releases/tag/v0.9.9)
## Latest dev release: [v1.0.1](https://github.com/smlight-dev/slzb-06-firmware/releases/tag/v1.0.1-dev)
<hr></hr>

This repository contains latest firmware for ESP32 peripheral module of [SLZB-06 Zigbee Ethernet PoE USB Adapter](https://smlight.tech/product/slzb-06). Firmware is opensource, so feel free to improve it by making commit to this repository. 

[![GitHub version](https://img.shields.io/github/release/smlight-dev/slzb-06-firmware.svg)](https://github.com/smlight-dev/slzb-06-firmware/releases)
[![GitHub download](https://img.shields.io/github/downloads/smlight-dev/slzb-06-firmware/total.svg)](https://github.com/smlight-dev/slzb-06-firmware/releases/latest)
[![License](https://img.shields.io/github/license/smlight-dev/slzb-06-firmware.svg)](LICENSE.txt)
[![Gitpod Ready-to-Code](https://img.shields.io/badge/Gitpod-Ready--to--Code-blue?logo=gitpod)](https://github.com/smlight-dev/slzb-06-firmware)

<hr></hr>

**In light of current war against Ukraine, we kinly asking you to support Ukrainian people by any means - support Ukrainian refugees please, [donate to Ukraine](https://bank.gov.ua/en/news/all/natsionalniy-bank-vidkriv-spetsrahunok-dlya-zboru-koshtiv-na-potrebi-armiyi), buy Ukrainian products or do business with Ukrainians.**

<hr></hr>



## KEY FIRMWARE FEATURES
- **Update Zigbee firmware chip right from the web-interface** in once click;
- Change Ethernet/USB adapter mode through firmware or by physical button short press (Blue led On = USB mode, Blue led Off = Ethernet mode);
- Adapter mode selector throug web-interface: `Zigbee-to-Ethernet`, `Zigbee-to-USB` and `Zigbee-to-WiFI`;
- support mDNS autodiscovery in your network (go in the borwser to `slzb-06.local`), in Zigbee2MQTT (set `port: mdns://slzb-06`) and in ZHA;
- Secure login through username and password;
- Zigbee2MQTT and ZHA config helper;
- Control behaviour of LED (you can disable Mode LED, Power LED through firmware, or toggle both by the button long press);
- DHCP or static IP address for Ethernet connection;
- ESP32 (peripheral) and CC2652P (Zigbee) OTA updates, ESP32  and CC2652P restart;
- Switch CC2652P (Zigbee) to flash mode;
- Flashing router firmware from the web-interface;
- Pairing mode for adapter in router mode;
- Firewall for incoming connections to secure your network;
- Fully responsive web-interface based on the latest Bootstrap 5.2.

### 5 minutes video review on Youtube (click):

[![Video review firmware v1.0.0 for SLZB-06](https://github.com/smlight-dev/slzb-06-firmware/blob/main/img/title.jpg)](https://www.youtube.com/watch?v=ps-x_-CQXp0)

### SLZB-06 firmware screenshots
![](https://github.com/smlight-dev/slzb-06-firmware/blob/main/img/0.9.8_1.png)
![](https://github.com/smlight-dev/slzb-06-firmware/blob/main/img/0.9.8_2.png)
![](https://github.com/smlight-dev/slzb-06-firmware/blob/main/img/0.9.8_3.png)
![](https://github.com/smlight-dev/slzb-06-firmware/blob/main/img/0.9.8_4.png)
![image](https://user-images.githubusercontent.com/31830530/230929441-cbba10c7-0f5f-4b3c-aec1-0d66dd088d49.png)

## Installation and Configuration

Please refer to the installation and configuration articles in our [documentation](https://smlight.tech/manual/slzb-06/).


## Compiling from source

If you made changes to the code and want to compile you own firmware, please do the following:

### You did not change web-interface appearence
- download repository;
- install Microsoft Visual Code (MVC);
- Install PlatformIO extension to MVC;
- Press "PlatformIO: Build" and wait untill firmware.bin is generated;

### You made changes to web-interface

In such case - you have to rebuild web-interface fisrt before building the firmware.
- go to the folder tools/webfilesbuilder;
- run npm install
- run npx gulp


## Contribute

You can contribute to SLZB-06 Firmware by
- Providing Pull Requests (Features, Proof of Concepts or Fixes) - please read SLZB-06 Firmware's contributing approach [here](CONTRIBUTING.md)
- Testing new released features and report issues
- Contributing missing [documentation](https://github.com/smlight-dev/slzb-06-manual);

[Support Ukraine](https://bank.gov.ua/en/news/all/natsionalniy-bank-vidkriv-spetsrahunok-dlya-zboru-koshtiv-na-potrebi-armiyi)


## Credits

People helping to keep the show on the road - **developers and contributors**:
- [@Tarik2142](https://github.com/Tarik2142) for providing initial code, issues resolution and code optimizations.  

Special thanks goes also to all authors of 3rd party libraries which are used in this project.  


## License and attribution

SLZB-06-Firmware is licensed under GNU General Public License v3.
3rd party libraries that are used by this project are licensed under different license schemes, please check them out as well.  
Copyright (c) 2023 SMLIGHT-DEV.  
The GNU General Public License v3 ensures that if you use any part of this software in any way, your software must be released under the same license.  
