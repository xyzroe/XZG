# Changelog
All notable changes to this project will be documented in this file.

## [Unreleased] - Development

## [0.9.8] 20221119
### Added
- New GET/SET API for settings
- New WIFI coordinator mode arrived!
- Added a new WIFI page
- Added preloader. It is displayed during page loading
- Added hard reset. To reset settings, turn on device with button pressed, when yellow and blue LEDs start flashing, release button and settings will be erased

### Changed
- No more html.h all html code moved to separate page files
- Coordinator modes logic rework
- HUGE web interface update. Web interface now is "One Page App", no more senseless page refreshes
- Completely reworked saving settings in web.cpp, now used ArduinoJson
- Completely reworked transfering parameters from the device to the interface
- Settings "Hostname" and "Console log refresh interval" moved to "System and Tools" page
- Reworked "Logout" link
- Code optimisations for reduce memory usage

### Fixed

### Removed
- Removed unnecessary server endpoints
- Some of the parameters that are no longer used have been removed from the config
- Removed all code from html.h

------------

## [0.9.6] 20221028
### Added

### Changed
- Add margins between icons and text on tabs in page "System and Tools"

### Fixed
- Link to the site has been corrected on "About" page

### Removed

------------

## [0.9.5] 20221025
### Added

### Changed
- Sidenav position improvements

### Fixed
- Toast position fix

### Removed

------------

## [0.9.4] 20221025
### Added
- Add favicon.ico

### Changed
- Sidenav on mobile devices now hidden. Swipe from right screen corner to show sidenav

### Fixed

### Removed
- Cleaning unused files

------------

## [0.9.3] 20221025
### Added

### Changed
- Center link on "About page"
- Link now opens in new tab

### Fixed

### Removed

------------

## [0.9.2] 20221025
### Added
- Card grid now have responsible heigt

### Changed
- Merge "sys-tools" & "logs-browser" pages

### Fixed

### Removed

------------

## [0.9.1] 20221025
### Added
- Initial release

### Changed

### Fixed

### Removed



