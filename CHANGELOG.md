# Changelog

All notable changes to this project will be documented in this file.

## [Unreleased] - Development

## [0.9.8-beta] 20221119
### Added
- NEW WIFI COORDINATOR MODE ARRIVED!;
- New GET/SET API for settings;
- Added a new WIFI page;
- Added preloader. It is displayed during page loading;
- Added hard reset. To reset settings, turn on device with button pressed, when yellow and blue LEDs start flashing, release button and settings will be erased;
- Add favicon.ico;
- Card grid now have responsible height.

### Changed
- No more html.h all html code moved to separate page files
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
- Merge "sys-tools" & "logs-browser" pages.

### Fixed
- Toast position fix;
- Link to the site has been corrected on "About" page.


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



