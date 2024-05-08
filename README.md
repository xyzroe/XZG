# XZG 📁 Firmware zb_fws Branch

Welcome to the **zb_fws** branch of the XZG Firmware repository! This branch is dedicated to storing files necessary for flashing Zigbee chips in binary format.

## 🗂 Directory Structure

- **.github**:
  - **workflows/hex2bin.yml**: GitHub Actions workflow that automates converting firmware files from HEX to BIN format.
  - **scripts/process_and_convert.py**: Python script that processes and converts firmware files to the required format.
- **task.json**: Contains a structured list of tasks related to the available firmware files and their types (e.g., coordinator, router).
- **ti**:
  - **manifest.json**: Maps specific firmware versions to their devices and provides detailed notes about each firmware update.
  - **coordinator**: Contains `.bin` files for Zigbee coordinator firmware.
  - **router**: Contains `.bin` files for Zigbee router firmware.
  - **thread**: Contains `.bin` files for Thread firmware.

## 🛠 How to Use

All firmware files and scripts in this branch are intended to be used with the XZG Firmware Repository. Please refer to the main repository for more detailed information and instructions.

## 🤝 Contributing

Contributions are always welcome! Feel free to submit a pull request or create an issue for any updates, fixes, or improvements.

---

<div align="center"> Created with &#x2764;&#xFE0F; by <a href="https://xyzroe.cc/">xyzroe</a> © 2024</div>

---
