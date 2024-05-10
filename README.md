# XZG Firmware
<div align="center"> 
<a href="https://github.com/xyzroe/xzg/releases"><img src="https://img.shields.io/github/release/xyzroe/xzg.svg" alt="GitHub version"></img></a>
<a href="https://github.com/xyzroe/XZG/actions/workflows/build_fw.yml"><img src="https://img.shields.io/github/actions/workflow/status/xyzroe/XZG/build_fw.yml" alt="GitHub Actions Workflow Status"></img></a>
<a href="https://github.com/xyzroe/xzg/releases/latest"><img src="https://img.shields.io/github/downloads/xyzroe/xzg/total.svg" alt="GitHub download"></img></a>
<a href="https://github.com/xyzroe/XZG/issues"><img src="https://img.shields.io/github/issues/xyzroe/XZG" alt="GitHub Issues or Pull Requests"></img></a>
<a href="LICENSE"><img src="https://img.shields.io/github/license/xyzroe/xzg.svg" alt="License"></img></a>
</div>
<div align="center"> 
<br><br>
<a href="https://xzg.xyzroe.cc"><img src="src/websrc/img/logo.svg" width="128" height="128" alt="XZG logo"></a>
<br><br>
XZG Firmware unifies the best innovations from<br>
previous Zigbee gateway projects into a single, comprehensive solution.<br>
<br>
By focusing the community's efforts on enhancing one product, XZG aims to streamline development,<br>
thereby improving the capabilities and efficiency of your Zigbee Gateways. 🌍
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

## 🍓 Firmware features

Visit [features page](https://xzg.xyzroe.cc/features/) to get information 

https://github.com/xyzroe/XZG/assets/6440415/ae312626-f1d9-41c4-b982-11a1f9ba9ed5

## 📔 Documentation

Visit [Wiki page](https://xzg.xyzroe.cc/quick-start/) to get information 

## 🚀 Installation

For a quick setup, use [XZG Web Flasher](https://xzg.xyzroe.cc/install) for an easy plug-and-flash experience.

Please follow the installation guide tailored to your hardware.

## 🛠️ Compiling from source

### Local

- You need npm and Python installed
- Install Visual Studio Code (VSC)  
- Install PlatformIO extension to VSC  
- Clone this repository  
  `git clone --recurse-submodules https://github.com/xyzroe/XZG.git`
- Open `XZG.code-workspace` in VSC
- Press "PlatformIO: Build" and wait until XZG*.bin are generated  

### Github
 - Fork this repository;
 - Made your changes;
 - Push a new tag to run workflow;
 - Just wait and get new release;
  
### Gitpod

[![Open in Gitpod](https://gitpod.io/button/open-in-gitpod.svg)](https://gitpod.io/#https://github.com/xyzroe/XZG)

## 🖥️ Contribute

Contributions are welcome! If you'd like to help improve the XZG Firmware, you can:

- Provide Pull Requests with enhancements or fixes. Please see our [contribution guidelines](CONTRIBUTING.md).
- Test newly released features and report issues.
- Help expand our documentation for better user support.

## 🎉 Credits 

<!-- Copy-paste in your Readme.md file -->
<a href="https://next.ossinsight.io/widgets/official/compose-recent-active-contributors?repo_id=777202050&limit=30" target="_blank" style="display: block" align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="https://next.ossinsight.io/widgets/official/compose-recent-active-contributors/thumbnail.png?repo_id=777202050&limit=30&image_size=auto&color_scheme=dark" width="655" height="auto">
    <img alt="Active Contributors of xyzroe/XZG - Last 28 days" src="https://next.ossinsight.io/widgets/official/compose-recent-active-contributors/thumbnail.png?repo_id=777202050&limit=30&image_size=auto&color_scheme=light" width="655" height="auto">
  </picture>
</a>
<!-- Made with [OSS Insight](https://ossinsight.io/) -->

Thanks to all the developers and contributors who make this project possible, and special thanks to [@mercenaruss](https://github.com/mercenaruss/) for **Zig Star devices development**.

#### All projects contributors:
<a href="https://github.com/xyzroe/XZG/graphs/contributors"><img src="https://contrib.rocks/image?repo=xyzroe/XZG" /></a>
<a href="https://github.com/xyzroe/ZigStarGW-FW/graphs/contributors"><img src="https://contrib.rocks/image?repo=xyzroe/ZigStarGW-FW" /></a>
<a href="https://github.com/mercenaruss/uzg-firmware/graphs/contributors"><img src="https://contrib.rocks/image?repo=mercenaruss/uzg-firmware" /></a>
<a href="https://github.com/smlight-dev/slzb-06-firmware/graphs/contributors"><img src="https://contrib.rocks/image?repo=smlight-dev/slzb-06-firmware" /></a>


Special thanks to all third-party library authors. Their work has significantly contributed to this project:

- [espressif / arduino-esp32](https://github.com/espressif/arduino-esp32), 
- [esprfid / esp-rfid](https://github.com/esprfid/esp-rfid), 
- [fairecasoimeme / zigate-ethernet](https://github.com/fairecasoimeme/ZiGate-Ethernet), 
- [bblanchon / arduinojson](https://github.com/bblanchon/ArduinoJson), 
- [rLOGDacco / circularbuffer](https://github.com/rLOGDacco/CircularBuffer), 
- [sstaub / ticker](https://github.com/sstaub/Ticker), 
- [vurtun / lib](https://github.com/vurtun/lib),
- [Tinkerforge / WireGuard-ESP32-Arduino](https://github.com/Tinkerforge/WireGuard-ESP32-Arduino),  
- [sstaub / Ticker](https://github.com/sstaub/Ticker),
- [Martin-Laclaustra / CronAlarms](https://github.com/Martin-Laclaustra/CronAlarms),
- [xreef / WebServer-Esp8266-ESP32-Tutorial](https://github.com/xreef/WebServer-Esp8266-ESP32-Tutorial),
- [marvinroger / async-mqtt-client](https://github.com/marvinroger/async-mqtt-client)


## 📄 License

XZG Firmware is released under the **GNU General Public License v3.0**. See the [LICENSE](LICENSE) file for more details.

Third-party libraries used in this project are under their respective licenses. Please refer to each for more information.

---

<div align="center"> Created with &#x2764;&#xFE0F; by <a href="https://xyzroe.cc/">xyzroe</a> © 2024</div>

---
