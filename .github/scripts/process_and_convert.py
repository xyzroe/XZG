import json
import os
import requests
from zipfile import ZipFile, BadZipFile
import re
from datetime import datetime
import shutil

DATE_PATTERN = re.compile(r'_(\d{4}[01]\d[0-3]\d)')

def download_and_extract(url, extract_to):
    print(url)
    try:
        response = requests.get(url)
        response.raise_for_status()
        zip_path = os.path.join(extract_to, os.path.basename(url))
        with open(zip_path, "wb") as f:
            f.write(response.content)
        
        with ZipFile(zip_path, "r") as zip_ref:
            for zip_info in zip_ref.infolist():
                zip_ref.extract(zip_info, extract_to)
                extracted_path = os.path.join(extract_to, zip_info.filename)
                date_time = datetime(*zip_info.date_time)
                mod_time = date_time.timestamp()
                os.utime(extracted_path, (mod_time, mod_time))
        
        os.remove(zip_path)
    except requests.RequestException as e:
        print(f"Error downloading from {url}: {e}")
    except BadZipFile as e:
        print(f"Error unpacking archive: {e}")

def update_manifest(root, file, chip, version):
    manifest_path = os.path.join("ti", "manifest.json")
    link = f"https://raw.githubusercontent.com/xyzroe/XZG/zb_fws/{root}/{file}"
    root = root.replace("ti/", "")
    data = {chip: {file: {"ver": version, "link": link, "notes": ""}}}
    if os.path.exists(manifest_path):
        with open(manifest_path, "r+") as f:
            manifest = json.load(f)
            if root not in manifest:
                manifest[root] = {}
            if chip not in manifest[root]:
                manifest[root][chip] = {}
            if file not in manifest[root][chip]:
                manifest[root][chip][file] = data[chip][file]
            else:
                manifest[root][chip][file].setdefault("ver", data[chip][file]["ver"])
                manifest[root][chip][file]["link"] = data[chip][file]["link"]
                manifest[root][chip][file].setdefault("notes", "")
                manifest[root][chip][file].setdefault("baud", "115200")
            f.seek(0)
            json.dump(manifest, f, indent=4)
            f.truncate()
    else:
        with open(manifest_path, "w") as f:
            json.dump({root: data}, f, indent=4)

def find_date_in_filename(filename):
    match = DATE_PATTERN.search(filename)
    match_str = match
    if match_str == None:
        match_str = "None"
    else:
        match_str = str(match.group(1))
    return match.group(1) if match else None

def append_date_to_filename(filepath, date_str):
    directory, filename = os.path.split(filepath)
    name, ext = os.path.splitext(filename)
    name = DATE_PATTERN.sub('', name)
    new_filename = f"{name}_{date_str}{ext}"
    new_filepath = os.path.join(directory, new_filename)
    os.rename(filepath, new_filepath)
    return new_filepath

def get_creation_date(filepath):
    return datetime.fromtimestamp(os.path.getmtime(filepath)).strftime('%Y%m%d')

with open("task.json", "r") as f:
    print("Read task.json")
    tasks = json.load(f)
    for task in tasks:
        dir_path = os.path.join("ti", task["type"])
        os.makedirs(dir_path, exist_ok=True)
        download_and_extract(task["link"], dir_path)

print("hex2bin")
for root, dirs, files in os.walk("ti"):
    for file in files:
        if file.endswith(".hex"):
            print(file)
            hex_path = os.path.join(root, file)
            bin_path = hex_path[:-4] + ".bin"
            try:
                command = f"srec_cat {hex_path} -intel -o {bin_path} -binary"
                os.system(command)
            except Exception as e:
                print(f"Error converting file {hex_path}: {e}")

print("update filenames")
for root, dirs, files in os.walk("ti"):
    for file in files:
        if file.endswith(".bin"):
            print(file)
            bin_path = os.path.join(root, file)
            name, ext = os.path.splitext(file)
            # Check if filename ends with a date
            if not re.search(r'_\d{8}$', name):
                date_str = find_date_in_filename(name)
                print(date_str)
                if date_str == None:
                    corresponding_hex = bin_path[:-4] + ".hex"
                    if os.path.exists(corresponding_hex):
                        date_str = get_creation_date(corresponding_hex)
                if date_str:
                    new_bin_path = append_date_to_filename(bin_path, date_str)
                    file = os.path.basename(new_bin_path)
                    print('New name - ' + new_bin_path)
            else:
                print('OK')

print("update manifest")
for root, dirs, files in os.walk("ti"):
    for file in files:
        if file.endswith(".bin"):
            #print(file)
            bin_path = os.path.join(root, file)
            # Extract chip and version from the file name
            parts = file.split("_")
            chip_mapping = {
                "CC2652P_launchpad": "CC2652P2_launchpad",
                "1352P_RFS": "CC2652P2_launchpad",
                "2652P_RFS": "CC2652P2_launchpad",
                "CC2652PSIP": "CC2652P2_launchpad",
                "2652P_other": "CC2652P2_other",
                "1352P_E72": "CC2652P2_other",
                "2652P_E72": "CC2652P2_other",
                "1352P7_": "CC2652P7",
                "2652RB_": "CC2652RB",
            }

            current_chip = "_".join(
                parts[:-1]
            )  # Chip is everything before the date part

            chip = None
            for key, value in chip_mapping.items():
                if key in current_chip:
                    chip = value
                    break

            if chip is None:
                chip = current_chip

            version = parts[-1].split(".")[
                0
            ]  # Assuming the version is the last part before '.bin'
            update_manifest(root, file, chip, version)


def clean_directory(directory):
    print("clean directory")
    for root, dirs, files in os.walk(directory):
        for file in files:
            if not (file.endswith(".bin") or file == "manifest.json"):
                os.remove(os.path.join(root, file))


# Calling the clean_directory function to remove unwanted files
clean_directory("ti")
