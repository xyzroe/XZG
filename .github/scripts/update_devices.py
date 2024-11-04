import re

def read_hw_file(hw_file_path):
    with open(hw_file_path, 'r') as file:
        return file.read()

def parse_brd_configs(hw_content):
    pattern = re.compile(r'BrdConfigStruct brdConfigs\[\] = \{(.*?)\};', re.DOTALL)
    match = pattern.search(hw_content)
    if match:
        print("Found brdConfigs")
        return match.group(1)
    print("brdConfigs not found")
    return ""

def extract_devices(brd_configs, mist_configs):
    devices = []
    device_pattern = re.compile(r'\{\s*"([^"]+)",\s*\.ethConfigIndex = (-?\d+),\s*\.zbConfigIndex = -?\d+,\s*\.mistConfigIndex = (-?\d+)\s*\}', re.DOTALL)
    for device_match in device_pattern.finditer(brd_configs):
        device_name = device_match.group(1)
        eth_config_index = int(device_match.group(2))
        mist_config_index = int(device_match.group(3))
        
        eth_is = eth_config_index > -1
        btn_is = mist_configs[mist_config_index]['btnPin'] > -1
        led_is = mist_configs[mist_config_index]['ledModePin'] > -1 or mist_configs[mist_config_index]['ledPwrPin'] > -1

        device = {
            'name': device_name,
            'eth': ':white_check_mark:' if eth_is else ':x:',
            'button': ':white_check_mark:' if btn_is else ':x:',
            'led': ':white_check_mark:' if led_is else ':x:',
            'network_usb': ':white_check_mark:'
        }
        devices.append(device)
        print(f"Extracted device: {device}")
    return devices

def parse_mist_configs(hw_content):
    pattern = re.compile(r'MistConfig mistConfigs\[\] = \{(.*?)\};', re.DOTALL)
    match = pattern.search(hw_content)
    if match:
        print("Found mistConfigs")
        mist_configs = match.group(1)
        return extract_mist_configs(mist_configs)
    print("mistConfigs not found")
    return []

def extract_mist_configs(mist_configs):
    configs = []
    config_pattern = re.compile(r'\{\s*\.btnPin = (-?\d+),\s*\.btnPlr = \d+,\s*\.uartSelPin = -?\d+,\s*\.uartSelPlr = \d+,\s*\.ledModePin = (-?\d+),\s*\.ledModePlr = \d+,\s*\.ledPwrPin = (-?\d+),\s*\.ledPwrPlr = \d+\s*\}', re.DOTALL)
    for config_match in config_pattern.finditer(mist_configs):
        config = {
            'btnPin': int(config_match.group(1)),
            'ledModePin': int(config_match.group(2)),
            'ledPwrPin': int(config_match.group(3)),
        }
        configs.append(config)
        print(f"Extracted mistConfig: {config}")
    return configs

def read_features_file(features_file_path):
    with open(features_file_path, 'r') as file:
        return file.read()

def extract_existing_links(features_content):
    device_links = {}
    table_pattern = re.compile(r'\| \[(.*?)\]\((.*?)\)', re.DOTALL)
    for match in table_pattern.finditer(features_content):
        device_name = match.group(1)
        link = match.group(2)
        device_links[device_name] = link
        print(f"Found link: {device_name} -> {link}")
    return device_links

def update_features_content(features_content, devices, device_links):
    # Define the regular expression to match the whole Supported devices section
    table_pattern = re.compile(r'(.*## 🎮 Supported devices)(\s+\| .+\|.+?)(\* Some devices do not support all features.*)', re.DOTALL)
    match = table_pattern.search(features_content)
    if match:

        header = match.group(1)
        footer = match.group(3)

        updated_devices_table = "| Device Name                                                 |       Button       |     ESP32 LEDs     | Remote Network / USB mode selection |      Ethernet      |\n"
        updated_devices_table += "| :---------------------------------------------------------- | :----------------: | :----------------: | :---------------------------------: | :----------------: |\n"

        for device in devices:
            device_name = device['name']
            link = device_links.get(device_name, "")
            if link:
                device_name = f"[{device_name}]({link})"
            device_row = f"| {device_name} | {device['button']} | {device['led']} | {device['network_usb']} | {device['eth']} |\n"
            updated_devices_table += device_row

        updated_features_content = header + "\n\n" + updated_devices_table + "\n" + footer
        print("Updated features content")
        return updated_features_content
    print("Supported devices section not found")
    return features_content

def write_features_file(features_file_path, updated_content):
    with open(features_file_path, 'w') as file:
        file.write(updated_content)
    print(f"Updated features file: {features_file_path}")

def main():
    hw_file_path = 'main_branch/src//const/hw.cpp'
    features_file_path = 'mkdocs_branch/docs/features.md'

    hw_content = read_hw_file(hw_file_path)
    print(f"Read hw.cpp content: {len(hw_content)} characters")

    brd_configs = parse_brd_configs(hw_content)
    mist_configs = parse_mist_configs(hw_content)

    devices = extract_devices(brd_configs, mist_configs)

    features_content = read_features_file(features_file_path)
    print(f"Read features.md content: {len(features_content)} characters")

    device_links = extract_existing_links(features_content)

    updated_content = update_features_content(features_content, devices, device_links)

    write_features_file(features_file_path, updated_content)

if __name__ == "__main__":
    main()
