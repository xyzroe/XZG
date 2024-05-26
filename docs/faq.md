# FAQ 


??? question "Is the XZG compatible with Home Assistant without requiring additional setup?"
    Indeed, it does. Simply connect the [XZG flashed device](./features.md#-supported-devices) to your LAN Ethernet or USB, and you're all set. The [XZG enabled device](./features.md#-supported-devices) comes pre-flashed and prepared for immediate use. 

??? question "Is it possible to configure more than one XZG firmware enabled coordinator with a single instance of Home Assistant?"
    Certainly, that's possible. You have the ability to establish numerous XZG enabled coordinators alongside a single Home Assistant setup. To achieve this, all you need to do is execute distinct instances of Zigbee2MQTT for each coordinator.<br>
    According to user reports, approximately 6-8 LAN devices have been successfully employed in conjunction with a single Home Assistant setup. Furthermore, it's worth nothing that you have the capability to link with devices situated beyond your LAN network, such as those located in different geographical locations with internet access.

??? question "Is it possible to remotely update the firmware of the [XZG enabled device](./features.md#-supported-devices) without needing physical access to the device?"
    Certainly, you have the capability to do so. The two main chips employed in the coordinator (CC2652P and ESP32) can both be updated remotely from a different location. This feature makes the device well-suited for installation in infrequently accessed or remote locations.
