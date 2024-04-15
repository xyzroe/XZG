# XZG Firmware
<div align="center"> 
<a href="https://github.com/xyzroe/xzg/releases">
  <img src="https://img.shields.io/github/release/xyzroe/xzg.svg" alt="GitHub version">
</a>
<img src="https://img.shields.io/github/actions/workflow/status/xyzroe/XZG/build_release_push" alt="GitHub Actions Workflow Status">
<a href="https://github.com/xyzroe/xzg/releases/latest">
  <img src="https://img.shields.io/github/downloads/xyzroe/xzg/total.svg" alt="GitHub download">
</a>
<img src="https://img.shields.io/github/issues/xyzroe/XZG" alt="GitHub Issues or Pull Requests">
<a href="LICENSE">
  <img src="https://img.shields.io/github/license/xyzroe/xzg.svg" alt="License">
</a>
</div>
<div align="center"> 
<br><br>
<img src="src/websrc/img/logo.svg" width="128" height="128" alt="XZG logo">
<br><br>
This repository contains the firmware for various, ESP32 based, Zigbee gateways.<br><br>
Firmware is opensource, so feel free to improve it <br> by making a commit to this repository. 
</div>
<br><br> 
<table width="40%" align="center">
<tr align="center">
 <td colspan="3"><i>Previous versions:</i></td>
</tr>
  <tr align="center">
    <td><a href="https://github.com/xyzroe/ZigStarGW-FW/releases/latest">ZigStarGW-FW</a></td>
    <td><a href="https://github.com/mercenaruss/uzg-firmware/releases/latest">UZG-01</a></td>
    <td><a href="https://github.com/smlight-dev/slzb-06-firmware/releases/">SLZB-06</a></td>
  </tr>
  <tr align="center">
    <td><a href="https://github.com/xyzroe/ZigStarGW-FW/releases/latest"><img src="https://img.shields.io/github/downloads/xyzroe/ZigStarGW-FW/total.svg" alt="ZigStarGW-FW's download"></a></td>
    <td><a href="https://github.com/mercenaruss/uzg-firmware/releases/latest"><img src="https://img.shields.io/github/downloads/mercenaruss/uzg-firmware/total.svg" alt="UZG-01 Firmware's download"></a></td>
    <td><a href="https://github.com/smlight-dev/slzb-06-firmware/releases/latest"><img src="https://img.shields.io/github/downloads/smlight-dev/slzb-06-firmware/total.svg" alt="SLZB-06 Firmware's download"></a></td>
  </tr>
</table>
<br> 

## 🍓 KEY FIRMWARE FEATURES

<table align="center" width=60%>
<tr>
    <td><img src="https://img.shields.io/badge/Visual%20Studio%20Code-007ACC?logo=visualstudiocode&logoColor=fff&style=plastic" alt="Visual Studio Code Badge"></td>
    <td><img src="https://img.shields.io/badge/npm-CB3837?logo=npm&logoColor=fff&style=plastic" alt="npm Badge"></td>
    <td><img src="https://img.shields.io/badge/Arduino-00878F?logo=arduino&logoColor=fff&style=plastic" alt="Arduino Badge"></td>
    <td><img src="https://img.shields.io/badge/JSON-000?logo=json&logoColor=fff&style=plastic" alt="JSON Badge"></td>
    <td><img src="https://img.shields.io/badge/gulp-CF4647?logo=gulp&logoColor=fff&style=plastic" alt="gulp Badge"></td>
    <td><img src="https://img.shields.io/badge/Bootstrap-7952B3?logo=bootstrap&logoColor=fff&style=plastic" alt="Bootstrap Badge"></td>
    <td><img src="https://img.shields.io/badge/jQuery-0769AD?logo=jquery&logoColor=fff&style=plastic" alt="jQuery Badge"></td>
    <td><img src="https://img.shields.io/badge/i18next-26A69A?logo=i18next&logoColor=fff&style=plastic" alt="i18next Badge"></td>
    <td><img src="https://img.shields.io/badge/GitHub%20Actions-2088FF?logo=githubactions&logoColor=fff&style=plastic" alt="GitHub Actions Badge"></td>
</tr>
</table>
<table align="center" width=80%>
<tr><td colspan="4" align="center">Localized Translation</td></tr>
<tr><td colspan="4" align="center">🇬🇧 | 🇺🇦 | 🇨🇳 | 🇪🇸 | 🇸🇦 | 🇵🇹 | 🇷🇺 | 🇫🇷 | 🇩🇪 | 🇯🇵 | 🇹🇷 | 🇮🇹 | 🇵🇱</td></tr>
<tr><td colspan="4"></td></tr>
<tr align="center"><td colspan="2">Light mode</td><td colspan="2">Dark mode</td></tr>
<tr align="center"><td colspan="2">🌞</td><td colspan="2">🌑</td></tr>
<tr align="center"><td colspan="2"></td><td colspan="2"></td></tr>
<tr align="center"><td colspan="2"><b>Network mode</b></td><td colspan="2"><b>USB mode</b></td></tr>
<tr align="center"><td>LAN</td><td>Wi-Fi</td><td>Zigbee</td><td>ESP32</td></tr>
<tr align="center"><td colspan="2"><i><b>at the same time</b></i></td><td colspan="2"><i>selectable</i></td></tr>
<tr><td colspan="4"></td></tr>
<tr align="center"><td>NTP time</td><td>LED control</td><td>VPN access</td><td>MQTT client</td></tr>
<tr align="center"><td>Check and install updates, LEDs control</td><td>Night mode, Manual control, Full disable</td><td>Full access via WireGuard</td><td>View gateway stats, change modes, control LEDs. <br> Home Assistant Auto Discovery.</td></tr>
</table>


- Change Ethernet/USB adapter mode through firmware or by physical button short press (Red LED On = USB mode, RED LED Off = Ethernet mode);
- Adapter mode selector through web-interface: `Zigbee-to-Network` or `Zigbee-to-USB`;
- Support mDNS autodiscovery in your network (go in the browser to `xzg.local`), in Zigbee2MQTT (set `port: mdns://xzg`), and in ZHA;
- Secure login through username and password;
- Zigbee2MQTT and ZHA config helper;

- ESP32 (peripheral) and CC2652P (Zigbee) OTA updates, ESP32  and CC2652P restart;
- Switch CC2652P (Zigbee) to flash mode;
- Pairing mode for the adapter in router mode;
  
- Filter IP addresses for incoming connections to secure your network;
- Fully responsive web interface based on the latest Bootstrap 5.3.
- WireGuard VPN
- MQTT client for gateway monitoring and control

## 🚀 Installation

For a quick setup, use [XZG Web Flasher](https://xzg.xyzroe.cc/) for an easy plug-and-flash experience.

Please follow the installation guide tailored to your hardware.

## 🛠️ Compiling from source

### Local

- You need npm and Python installed;
- Download this repository;
- Install Visual Studio Code (VSC);
- Install PlatformIO extension to VSC;
- Press "PlatformIO: Build" and wait until XZG*.bin are generated;

### Github
 - Fork this repository;
 - Made your changes;
 - Push a new tag to run workflow;
 - Just wait and get new release;
  
### Gitpod
[![Gitpod Ready-to-Code](https://img.shields.io/badge/Gitpod-Ready--to--Code-blue?logo=gitpod)](https://github.com/xyzroe/xzg)

## 🖥️ Contribute

Contributions are welcome! If you'd like to help improve the XZG Firmware, you can:

- Provide Pull Requests with enhancements or fixes. Please see our [contribution guidelines](CONTRIBUTING.md).
- Test newly released features and report issues.
- Help expand our documentation for better user support.

## 🎉 Credits 

Thanks to all the developers and contributors who make this project possible:

- Special thanks to [@mercenaruss](https://github.com/mercenaruss/) for **Zig Star devices development**.

### Our Awesome Contributors:

<a href="https://github.com/xyzroe/XZG/graphs/contributors"><img src="https://contrib.rocks/image?repo=xyzroe/XZG" /></a><br>
<a href="https://github.com/xyzroe/ZigStarGW-FW/graphs/contributors"><img src="https://contrib.rocks/image?repo=xyzroe/ZigStarGW-FW" /></a><br>
<a href="https://github.com/mercenaruss/uzg-firmware/graphs/contributors"><img src="https://contrib.rocks/image?repo=mercenaruss/uzg-firmware" /></a><br>
<a href="https://github.com/smlight-dev/slzb-06-firmware/graphs/contributors"><img src="https://contrib.rocks/image?repo=smlight-dev/slzb-06-firmware" /></a>


Special thanks to all third-party library authors. Their work has significantly contributed to this project:

- [espressif / arduino-esp32](https://github.com/espressif/arduino-esp32), 
- [esprfid / esp-rfid](https://github.com/esprfid/esp-rfid), 
- [fairecasoimeme / zigate-ethernet](https://github.com/fairecasoimeme/ZiGate-Ethernet), 
- [bblanchon / arduinojson](https://github.com/bblanchon/ArduinoJson), 
- [rlogiacco / circularbuffer](https://github.com/rlogiacco/CircularBuffer), 
- [sstaub / ticker](https://github.com/sstaub/Ticker), 
- [vurtun / lib](https://github.com/vurtun/lib),
- [Tinkerforge / WireGuard-ESP32-Arduino](https://github.com/Tinkerforge/WireGuard-ESP32-Arduino),  
- [sstaub / Ticker](https://github.com/sstaub/Ticker),
- [knolleary / PubSubClient](https://github.com/knolleary/pubsubclient),
- [Martin-Laclaustra / CronAlarms](https://github.com/Martin-Laclaustra/CronAlarms),
- [xreef / WebServer-Esp8266-ESP32-Tutorial](https://github.com/xreef/WebServer-Esp8266-ESP32-Tutorial)


## 📄 License

XZG Firmware is released under the **GNU General Public License v3.0**. See the [LICENSE](LICENSE) file for more details.

Third-party libraries used in this project are under their respective licenses. Please refer to each for more information.

---

<div align="center"> Created with &#x2764;&#xFE0F; by <a href="https://xyzroe.cc/">xyzroe</a> © 2024</div>

---