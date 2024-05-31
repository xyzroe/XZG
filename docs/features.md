
XZG Firmware unifies the best innovations from previous Zigbee gateway projects into a single, comprehensive solution.

By focusing the community's efforts on enhancing one product, XZG aims to streamline development, thereby improving the features and efficiency of your Zigbee Gateways. 🌍



<div class="badges">
  <img src="https://img.shields.io/badge/Visual%20Studio%20Code-007ACC?logo=visualstudiocode&logoColor=fff&style=plastic" alt="Visual Studio Code Badge">
  <img src="https://img.shields.io/badge/npm-CB3837?logo=npm&logoColor=fff&style=plastic" alt="npm Badge">
  <img src="https://img.shields.io/badge/Arduino-00878F?logo=arduino&logoColor=fff&style=plastic" alt="Arduino Badge">
  <img src="https://img.shields.io/badge/JSON-000?logo=json&logoColor=fff&style=plastic" alt="JSON Badge">
  <img src="https://img.shields.io/badge/gulp-CF4647?logo=gulp&logoColor=fff&style=plastic" alt="gulp Badge">
  <img src="https://img.shields.io/badge/Bootstrap-7952B3?logo=bootstrap&logoColor=fff&style=plastic" alt="Bootstrap Badge">
  <img src="https://img.shields.io/badge/jQuery-0769AD?logo=jquery&logoColor=fff&style=plastic" alt="jQuery Badge">
  <img src="https://img.shields.io/badge/i18next-26A69A?logo=i18next&logoColor=fff&style=plastic" alt="i18next Badge">
  <img src="https://img.shields.io/badge/GitHub%20Actions-2088FF?logo=githubactions&logoColor=fff&style=plastic" alt="GitHub Actions Badge">
</div>


## 🍓 Key features   

|                       |                                                                                                                                                                                                                          |
| :-------------------- | :----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------: |
| Localized Translation |                                                                                    <div class="badges">🇬🇧 🇺🇦 🇨🇳 🇪🇸 🇵🇹 🇷🇺 🇫🇷 🇩🇪 🇯🇵 🇹🇷 🇮🇹 🇵🇱 🇨🇿</div>                                                                                     |
| Zigbee OTA            | Install new Zigbee firmware or change role <span style="color:red">Coordinator</span> / <span style="color:green">Router</span> / <span style="color:blue">OpenThread</span> using **only the Web UI** of your gateway 🚀 |
| NVRAM                 |                                                                                 Erase NVRAM using **only the Web UI** of your gateway 🎉                                                                                  |
| Web UI theme          |                                                                                                   🌞 Light *or* Dark 🌑                                                                                                    |
| Zigbee mode           |                                                                                 :material-lan: Network *or* USB :fontawesome-brands-usb:                                                                                 |
| Network mode          |                                                                        :material-ethernet: RJ45 **and** Wi-Fi :material-wifi: *(simultaneously)*                                                                         |
| VPN support           |                                                                                               :simple-wireguard: WireGuard                                                                                               |
| MQTT client           |                                                                       :simple-mqtt: Publish gateway states to brokers and subscribes for commands                                                                        |
| Access point          |                                                                       :material-antenna: If there isn't available any wire and wireless connection                                                                       |
| NTP client            |                                                                                                   :material-clock: Yes                                                                                                   |
| Custom DNS servers    |                                                                              :material-dns: Yes *(when DHCP is **off**)*, Ethernet and WiFi                                                                              |
| Realtime web updates  |                                                                                       :material-run-fast:  Without page refreshing                                                                                       |
| Button                |                                                                    :material-button-pointer:   Change current modes, factory reset :material-factory:                                                                    |
| LEDs                  |                                                                            :material-led-on: Different blinks helps to indicate current mode                                                                             |
| Disable LEDs modes    |                                                                     :material-led-variant-off:   Full disable and Night    :material-weather-night:                                                                      |
| Code                  |                                                                                     XZG is developed, coded and tested in Ukraine 🇺🇦                                                                                      |
| Build                 |                                                    :fontawesome-brands-github: All builds are published using GitHub Actions. No more manual publishing of binaries.                                                     |

## 🎮 Supported devices 

|                                                               |       Button       |    ESP32 LEDs     | Remote Network / USB mode selection |
| :------------------------------------------------------------ | :----------------: | :----------------: | :---------------------------------: |
| [UZG-01](https://uzg.zig-star.com)                            | :white_check_mark: | :white_check_mark: |         :white_check_mark:          |
| [SLZB-06]()                                                   | :white_check_mark: | :white_check_mark: |         :white_check_mark:          |
| [Olizig PoE](https://zig-star.com/projects/zigstar-olizig/)   |        :x:         |        :x:         |         :white_check_mark:          |
| [LilyZig PoE](https://zig-star.com/projects/zigstar-lilyzig/) |        :x:         |        :x:         |         :white_check_mark:          |
| [ZigStar LAN](https://zig-star.com/projects/zigbee-gw-lan/)   |        :x:         |        :x:         |         :white_check_mark:          |
| [TubesZB-eth]()                                               |        :x:         |        :x:         |         :white_check_mark:          |
| [TubesZB-eth_usb]()                                           |        :x:         |        :x:         |         :white_check_mark:          |
| [TubesZB-poe]()                                               |        :x:         |        :x:         |         :white_check_mark:          |
| [CZC-1.0]()                                                   | :white_check_mark: | :white_check_mark: |         :white_check_mark:          |

* Some devices do not support all features

!!! tip "Custom device support"
    You can add **any** custom device by editing `configHw.json` in Tools/Debug/File browser after installing XZG firmware.

    Please share your configuration with our community after testing. The most popular devices will be added. 🚀

    ![image](https://private-user-images.githubusercontent.com/6440415/332863162-874a8053-a19b-4a37-97f8-c2abab9ea588.png?jwt=eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJnaXRodWIuY29tIiwiYXVkIjoicmF3LmdpdGh1YnVzZXJjb250ZW50LmNvbSIsImtleSI6ImtleTUiLCJleHAiOjE3MTcxMTM1NTgsIm5iZiI6MTcxNzExMzI1OCwicGF0aCI6Ii82NDQwNDE1LzMzMjg2MzE2Mi04NzRhODA1My1hMTliLTRhMzctOTdmOC1jMmFiYWI5ZWE1ODgucG5nP1gtQW16LUFsZ29yaXRobT1BV1M0LUhNQUMtU0hBMjU2JlgtQW16LUNyZWRlbnRpYWw9QUtJQVZDT0RZTFNBNTNQUUs0WkElMkYyMDI0MDUzMCUyRnVzLWVhc3QtMSUyRnMzJTJGYXdzNF9yZXF1ZXN0JlgtQW16LURhdGU9MjAyNDA1MzBUMjM1NDE4WiZYLUFtei1FeHBpcmVzPTMwMCZYLUFtei1TaWduYXR1cmU9MWRkMjI2OTgwZjk5OTNkOTgzNDFiMDExZTlkMjA2Nzg5YWNkMmI1MTk3MDgzY2YxZjVlMDZlZTE2ZjlkYTUwNiZYLUFtei1TaWduZWRIZWFkZXJzPWhvc3QmYWN0b3JfaWQ9MCZrZXlfaWQ9MCZyZXBvX2lkPTAifQ.zjqcG8rRvDht15-RZ-keyt1izZC23p9buAY2WGlZ0ig)


