# 🌐 XZG Firmware Repository (zb_fws Branch)

Welcome to the **zb_fws** branch of the XZG Firmware repository! This branch is dedicated to storing files necessary for flashing Zigbee chips in the required format, primarily focusing on coordinators and routers.

## 📋 Repository Structure

- **task.json**: Contains a structured list of tasks related to the available firmware files and their types (e.g., coordinator, router).
- **manifest.json**: Maps specific firmware versions to their devices and provides detailed notes about each firmware update.
- **.github**:
  - **workflows/hex2bin.yml**: GitHub Actions workflow that automates converting firmware files from HEX to BIN format.
  - **scripts/process_and_convert.py**: Python script that processes and converts firmware files to the required format.
- **ti**:
  - **coordinator**: Contains `.bin` firmware files for coordinator devices like `CC1352P2`, `CC2652P`, `CC2652R7`, etc.
  - **router**: Contains `.bin` firmware files for router devices like `CC1352P2_CC2652P_other_router_20221102`.

## ⚙️ How to Use

All firmware files and scripts in this branch are intended to be used with the XZG Firmware Repository. Please refer to the main repository for more detailed information and instructions.

## 🚀 Contributions and Feedback

Contributions are always welcome! Feel free to submit a pull request or create an issue for any updates, fixes, or improvements.
