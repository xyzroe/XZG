# FAQ 


??? question "Is the XZG compatible with Home Assistant without requiring additional setup?"
    Indeed, it does. Simply connect the XZG to your LAN Ethernet or USB, and you're all set. The XZG comes pre-flashed and prepared for immediate use. Go to [installation](installation.md) 

??? question "Is it possible to configure more than one XZG coordinator with a single instance of Home Assistant?"
    Certainly, that's possible. You have the ability to establish numerous XZG coordinators alongside a single Home Assistant setup. To achieve this, all you need to do is execute distinct instances of Zigbee2MQTT for each coordinator.<br>
    According to user reports, approximately 6-8 LAN devices have been successfully employed in conjunction with a single Home Assistant setup. Furthermore, it's worth noting that you have the capability to link with devices situated beyond your LAN network, such as those located in different geographical locations with internet access.

??? question "Is it possible to remotely update the firmware of the XZG without needing physical access to the device?"
    Certainly, you have the capability to do so. The two main chips employed in the XZG (CC2652P and ESP32) can both be updated remotely from a different location. This feature makes the device well-suited for installation in infrequently accessed or remote locations.

??? question "What is ESP32 Autoboot feature?"
    Indeed, ESP32 Autoboot is a default feature when flashing the XZG using the type-C connection. In this context, ESP32 Autoboot refers to the built-in programming mechanism and circuitry that simplifies the process of flashing the ESP32 microcontroller. With this feature, you are not required to manually press buttons to put the ESP32 in flash mode, and you also don't need to connect additional programming tools. The process is streamlined – just connect the type-C cable and initiate the flashing process. This streamlined approach makes flashing the device straightforward and convenient.




