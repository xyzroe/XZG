# Device Operation Guide

!!! question "Check your device"
    Not all devices with this firmware support LEDs and a button. These features are only available on devices that have them physically present.  
    If your device does not support these features, the corresponding options will not appear in the web interface.

## LED Indicators

There are three LEDs on the device: Power, Mode, and Zigbee. Each LED behaves differently depending on the device's state and configuration.

### Power LED

- **Blinks at 1 Hz (1)**: Indicates no TCP connection in network mode.
    { .annotate }

    1. :light_bulb: 1 Hz means the LED blinks once per second.
   
- **Constantly on**: Signifies an active TCP connection with the Zigbee chip in network mode.

### Mode LED

- **Blinks at 1 Hz (1)**: Active during the Zigbee chip connection check at bridge startup.
    { .annotate }

    1. :light_bulb: 1 Hz means the LED blinks once per second.
   
- **Constantly on**: Indicates USB mode is active.
- **Off**: Indicates network mode is active.
- **Blinks at 3 Hz (1)**: Signals a communication error with the Zigbee chip.
    { .annotate }

    1. :light_bulb: 3 Hz means the LED blinks three times per second.

### Zigbee LED

- **Controlled by the Zigbee software**: The state can be toggled from the Tools menu.

### LED Control Options

Through the General menu, you can:

- **Disable either LED**: Completely deactivates the selected LED.
- **Enable Night Mode**: Specify start and end times to completely turn off the LEDs during these periods.

## Button Operations

The device button supports multiple types of presses to manage various functions:

### Factory Reset

To perform a factory reset:

1. Power off the device.
2. Hold down the button.
3. Power on the device.
4. Release the button when both the Power and Mode LEDs start flashing at 3 Hz(1).
    { .annotate }

    1. :light_bulb: 3 Hz means the LED blinks three times per second.

!!! danger "Factory Reset"
    Performing a factory reset will erase all your settings and restore the device to its original factory state.

### Types of Button Presses

1. **Short Press (under 3 seconds)**:
      - Deactivates the LEDs temporarily (this setting resets upon reboot).


2. **Long Press (over 3 seconds but under 5 seconds)**:
      - Switches between network and USB mode.
      - The Mode LED will start blinking at 1 Hz (1) to indicate that you can release the button.
        { .annotate }

        1. :light_bulb: 1 Hz means the LED blinks once per second.

3. **Extended Press (over 5 seconds)**:
      - Activates BSL mode on the Zigbee chip.
      - The Mode LED will start blinking at 3 Hz (1) to indicate that you can release the button.
        { .annotate }

        1. :light_bulb: 3 Hz means the LED blinks three times per second.

!!! note
    This functionality is particularly useful in USB mode with the network disabled.
  
