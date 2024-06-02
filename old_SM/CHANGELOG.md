# Changelog

All notable changes to this project will be documented in this file.

## [Unreleased] - Development

## [0.9.9] 20230303
### Added
- Added whitelist for socket IP in "Security". You can choose the IP address that will have access to the TCP socket;
- Added USB mode option for Config generator;
- (c) sign

### Changed
- Minor code improvements and optimizations;

### Fixed
- WiFi mode toast visibility fix; 

### Removed
- Removed gateway ping check;

------------

## [0.9.8] 20221226
### Added
- NEW WIFI COORDINATOR MODE ARRIVED!;
- ZHA zeroconf discovery [PR 84111](https://github.com/home-assistant/core/pull/84111)
- Added visual confirmation of command execution success for all buttons in the interface;
- New feature "Keep network & web server" available in USB mode. This function allows you to leave one of the communication channels active and have access to the web interface. The device itself will select an available channel according to priority: WIFI, ETHERNET, WIFI AP;
- "Serial" page renamed to "Z2M and ZHA";
- Added Z2M and ZHA config generator on "Z2M and ZHA" page;
- Added a check of the success of connecting to Wi-Fi, after entering SSID and PASS;
- Added a check of operation and communication with the Zigbee module at startup. The blue LED flashes - waiting for a response. The green LED lit up - the check was successful. Blinking blue and yellow - there is no communication with the Zigbee module or the module is not working;
- New GET/SET API for settings;
- Added a new WIFI page;
- Added preloader. It is displayed during page loading;
- Added hard reset. To reset settings, turn on device with button pressed, when yellow and blue LEDs start flashing, release button and settings will be erased;
- Add favicon.ico;
- Card grid now have responsible height.

### Changed
- Now all buttons work through a single API;
- Zigbee socket is now available in access point mode. This means that you can now flash the Zigbee module in access point mode;
- All svg icons combined into one;
- "About" page rework;
- No more html.h all html code moved to separate page files;
- Coordinator modes logic rework - according to the [latest v0.9.8 scheme](https://github.com/smlight-dev/slzb-06-firmware/blob/main/images/mode-logic-v0.9.8.jpg);
- HUGE web interface update. Web interface now is "One Page App", no more senseless page refreshes;
- Completely reworked saving settings in web.cpp, now used ArduinoJson;
- Completely reworked transfering parameters from the device to the interface;
- Settings "Hostname" and "Console log refresh interval" moved to "System and Tools" page;
- Reworked "Logout" link;
- Code optimisations for reduce memory usage;
- Add margins between icons and text on tabs in page "System and Tools";
- Sidenav position improvements;
- Sidenav on mobile devices now hidden. Swipe from right screen corner to show sidenav;
- Center link on "About page";
- Link at the "About" page now opens in new tab;
- Merge "sys-tools" & "logs-browser" pages;

### Fixed
- Wi-Fi access point is no longer disabled during network scanning; 
- Toast position fix;
- Link to the site has been corrected on "About" page;


### Removed
- Removed unnecessary server endpoints;
- Some of the parameters that are no longer used have been removed from the config;
- Removed all code from html.h;
- Cleaning unused files;


------------

## [0.9.1] 20221025
### Added
- Initial release

### Changed

### Fixed

### Removed



