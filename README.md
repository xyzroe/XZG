# Firmware Release Branch

Welcome to the Firmware Releases branch. This branch contains all the binary and manifest files associated with different releases of our firmware. Each folder corresponds to a specific release version and contains everything necessary to upgrade your device.

## Directory Structure

Each release version is stored in a separate directory named after the version of the firmware it contains. For example:

- `releases/20240420/`
  - `XZG_20240420.full.bin` - Full binary firmware file.
  - `manifest.json` - Manifest file describing the firmware and its components.

## How to Use

To update your device with the latest firmware, follow these steps:

1. **Download the Firmware:**
   - Navigate to the folder corresponding to the latest version.
   - Download the `.bin` file you need. The `.full.bin` file is typically used for a complete flash.

2. **Download the Manifest File:**
   - The `manifest.json` file contains metadata about the firmware, including version information and a description of the contents. This file is necessary for certain deployment tools and procedures.

3. **Updating Your Device:**
   - Connect your device to your computer.
   - Use your preferred flashing tool to upload the `.bin` file to your device. The exact steps may vary depending on the device and the flashing tool you are using.
   - Follow the specific instructions provided for your device to ensure a successful update.

## Safety and Precautions

- **Backup Important Data:** Always backup important data from your device before attempting a firmware update.
- **Power Supply:** Ensure that your device is connected to a stable power supply during the update process.
- **Verification:** After downloading, verify the integrity of the firmware file using checksums if provided.

## Contributing

If you have suggestions or have identified issues with a particular release, please open an issue. Contributions to improve the firmware are always welcome.

Thank you for supporting our project!


## 📄 License

XZG Firmware is released under the **GNU General Public License v3.0**. See the [LICENSE](https://github.com/xyzroe/XZG/blob/main/LICENSE) file for more details.

Third-party libraries used in this project are under their respective licenses. Please refer to each for more information.

---

<div align="center"> Created with &#x2764;&#xFE0F; by <a href="https://xyzroe.cc/">xyzroe</a> © 2024</div>

---
