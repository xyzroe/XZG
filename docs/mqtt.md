# MQTT description

## Topics
!!! example "XZG-XXXX/avty"
    Contains the current state of the gateway connection to the MQTT broker.
    When a connection is established, the payload online is published.

    Using the Last Will and Testament (LWT) mechanism, if the connection is broken,
    the MQTT broker will publish offline payload within 30 seconds.

!!! example "XZG-XXXX/state"

    Contains information about the gateway. Publish every N seconds. It is set in the MQTT setting - "Update interval".
    Payload example:

    ``` json 
    {
        "uptime":"0 d 00:01:08",
        "wlanIp":"::",
        "wlanRssi":"0",
        "lanIp":"192.168.178.90",
        "temperature":"41.89",
        "hostname":"XZG",
        "connections":"1",
        "mode":"Zigbee-to-Network",
        "zbfw":"20240316"
    }
    ```

!!! warning "XZG-XXXX/cmd"
    Publishing messages to this topic allows you to control your gateway via MQTT. Possible commands:
    {cmd:"rst_zig"} - restart Zigbee module
    {cmd:"rst_esp"} - restart ESP32
    {cmd:"enbl_bsl"} - enable BSL in Zigbee module

!!! info "XZG-XXXX/io/rst_zig, rst_esp, enbl_bsl"
    Status topics contain the current state of various operating modes of the gateway.
    Possible states: ON or OFF

!!! info "XZG-XXXX/io/socket"
    Possible states: ON or OFF

!!! info "XZG-XXXX/io/update_esp"
    Possible states: ON or OFF




## Auto Discovery
There is also a MQTT AutoDiscovery function. The following entities are available:

!!! example "Buttons"
        homeassistant/button/XZG-XXXX/rst_esp
        homeassistant/button/XZG-XXXX/rst_zig
        homeassistant/button/XZG-XXXX/enbl_bsl

!!! example "Binary sensors"
        homeassistant/binary_sensor/XZG-XXXX/socket
        homeassistant/binary_sensor/XZG-XXXX/update_esp

!!! example "Sensors"
        homeassistant/sensor/XZG-XXXX/uptime
        homeassistant/sensor/XZG-XXXX/wlanIp
        homeassistant/sensor/XZG-XXXX/wlanSsid
        homeassistant/sensor/XZG-XXXX/wlanRssi
        homeassistant/sensor/XZG-XXXX/lanIp
        homeassistant/sensor/XZG-XXXX/temperature
        homeassistant/sensor/XZG-XXXX/hostname
        homeassistant/sensor/XZG-XXXX/connections
        homeassistant/sensor/XZG-XXXX/mode
        homeassistant/sensor/XZG-XXXX/zbfw
        homeassistant/sensor/XZG-XXXX/zbhw


!!! info "In the Home Assistant, in the device information section, the board model and software version will also be available."
