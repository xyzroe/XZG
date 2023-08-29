[![Stand With Ukraine](https://raw.githubusercontent.com/vshymanskyy/StandWithUkraine/main/banner-direct-single.svg)](https://stand-with-ukraine.pp.ua)

 
[![GitHub version](https://img.shields.io/github/release/xyzroe/ZigStarGW-FW.svg)](https://github.com/xyzroe/ZigStarGW-FW/releases)
[![GitHub download](https://img.shields.io/github/downloads/xyzroe/ZigStarGW-FW/total.svg)](https://github.com/xyzroe/ZigStarGW-FW/latest)
[![License](https://img.shields.io/github/license/xyzroe/ZigStarGW-FW.svg)](LICENSE.txt)


# ZigStar LAN Gateway - Firmware
This firmware delevoped to use with [ZigStar LAN Gateway](https://github.com/mercenaruss/zigstar_gateways)

# Functions
- Zigbee UART port forwarding using socket LAN connection
- MQTT connection to view states and contorl ESP32
- Restarting Zigbee and enabling Zigbee BSL via webpage and MQTT
- ESP32 firmware update via webpage
- Automatic switching to Wi-Fi network when RJ45 is disconnected - "Emergency mode"
- If no Wi-Fi network is available, the hotspot will be configured


<br>

## [Web installer 🚀](https://install.zig-star.com)
  
<br>
  
# MQTT

## Topics

### ZigStarGW-XXXX/**avty**
Contains the current state of the gateway connection to the MQTT broker.   
When a connection is established, the payload ```online``` is published.  

Using the Last Will and Testament (LWT) mechanism, if the connection is broken,  
the MQTT broker will publish ```offline``` payload within 30 seconds.

### ZigStarGW-XXXX/**state**
Contains information about the gateway.
Publish every N seconds. It is set in the MQTT setting - "Update interval".  
Payload example:  
```{"uptime":"0 d 00:00:08","temperature":"45.67","ip":"10.0.10.130","emergencyMode":"ON","hostname":"ZigStarGW"}```

### ZigStarGW-XXXX/**cmd**
Publishing messages to this topic allows you to control your gateway via MQTT.
Possible commands:  
```{cmd:"rst_zig"}``` - restart Zigbee module  
```{cmd:"rst_esp"}``` - restart ESP32  
```{cmd:"enbl_bsl"}``` - enable BSL in Zigbee module  

### ZigStarGW-XXXX/io/**rst_zig**, **rst_esp**, **enbl_bsl**, **emrgncMd**

Status topics contain the current state of various operating modes of the gateway.  
Possible states: ```ON``` or ```OFF```

<br><br>

<table>
<tr>
<td width="70%">

## Auto Discovery
There is also a MQTT AutoDiscovery function.
The following entities are available:
- homeassistant/sensor/*
    - Uptime
    - IP
    - CPU temperature
    - Hostname
- homeassistant/binary_sensor/*
    - Socket
    - Emergency mode
- homeassistant/switch/*
    - Restart ESP
    - Restart Zigbee
    - Enable BSL


In the Home Assistant, in the device information section, the board model and software version will also be available.
</td>
<td><img src="https://github.com/xyzroe/ZigStarGW-FW/raw/main/images/HA_device.png"></td>
</tr>
</table>

<br>

![](https://github.com/xyzroe/ZigStarGW-FW/raw/main/images/main_eth.png)  

![](https://github.com/xyzroe/ZigStarGW-FW/raw/main/images/main_emergency_mode.png)  

![](https://github.com/xyzroe/ZigStarGW-FW/raw/main/images/logs.png)  

![](https://github.com/xyzroe/ZigStarGW-FW/raw/main/images/update.png)  


### Development

Project's build environment is based on [PlatformIO](http://platformio.org).
So just open platformio.ini using it.

After build PlatformIO will generate 2 file in bin folder:
ZigStarGW_v*.*.*.full.bin - with integrated bootloader and partitions table
bin/ZigStarGW.bin - just firmware.

Version increment made automatically by using  version_increment_pre.py calling from PlatformIO and version_increment_post.py calling while Git pre commit.
Use make_git_hook.sh to made it automatically.


You can not simply edit Web UI files because you will need to convert them to C arrays, which can be done automatically by a gulp script that can be found in ```tools/webfilesbuilder``` directory.

If you want to edit Web UI you will need:
* NodeJS
* npm (comes with NodeJS installer)
* Gulp (can be installed with npm)

Gulp script also minifies CSS and JS files and compresses (gzip) them.

To minify and compress the frontend, enter the folder ```tools/webfilesbuilder``` and:
* Run ```npm install``` to install dependencies  
* Run ```npx gulp``` to compress the web UI to make it ready for the ESP  
cd

### Like ♥️?
[!["Buy Me A Coffee"](https://www.buymeacoffee.com/assets/img/custom_images/orange_img.png)](https://www.buymeacoffee.com/xyzroe)

#### Thanks

Base code was taken from [ZiGate-Ethernet](https://github.com/fairecasoimeme/ZiGate-Ethernet)  

Some ideas and code snippets was taken from [esp-rfid](https://github.com/esprfid/esp-rfid)
