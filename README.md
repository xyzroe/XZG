# ZigStar LAN Gateway - Firmware
This firmware delevoped to use with [ZigStar LAN Gateway](https://github.com/mercenaruss/zigstar_gateways)

<div align="center">
<img src="https://github.com/mercenaruss/zigstar_gateways/raw/main/img/Default.png" width="45%">  
</div>

# Functions
- Zigbee UART port forwarding using socket LAN connection
- MQTT connection to view states and contorl ESP32
- Restarting Zigbee and enabling Zigbee BSL via webpage and MQTT
- ESP32 firmware update via webpage
- Automatic switching to Wi-Fi network when RJ45 is disconnected - "Emergency mode"
- If no Wi-Fi network is available, the hotspot will be configured


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



Base code was taken from [ZiGate-Ethernet](https://github.com/fairecasoimeme/ZiGate-Ethernet)  

Some ideas and code snippets was taken from [esp-rfid](https://github.com/esprfid/esp-rfid)
