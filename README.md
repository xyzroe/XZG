# ZiGate-Ethernet
 Ethernet platform for PiZiGate.
 For more informations, please go to the [description page](https://zigate.fr/documentation/descriptif-de-la-zigate-ethernet/)
 
 ![ZiGate-Ethernet](https://i0.wp.com/zigate.fr/wp-content/uploads/2021/07/ZiGate-ethernet-definition.jpg)
 
# Firmware
The platform use an ESP32 (16MB flash) with LAN8720 driver for ethernet

This program use following libraries :
- Webserver
- SPIFFS
- [SoftwareSerial](https://www.arduino.cc/en/Reference/softwareSerial)
- [WiFiClient](https://www.arduino.cc/en/Reference/WiFiClient)
- [CircularBuffer](https://github.com/rlogiacco/CircularBuffer)
- [ArduinoJson](https://github.com/bblanchon/ArduinoJson)

Please be sure libraries are installed before compile
