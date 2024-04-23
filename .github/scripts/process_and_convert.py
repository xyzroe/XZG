import json
import os
import requests
from zipfile import ZipFile

def download_and_extract(url, extract_to):
    try:
        response = requests.get(url)
        response.raise_for_status()
        zip_path = os.path.join(extract_to, os.path.basename(url))
        with open(zip_path, "wb") as f:
            f.write(response.content)
        with ZipFile(zip_path, 'r') as zip_ref:
            zip_ref.extractall(extract_to)
        os.remove(zip_path)
    except requests.RequestException as e:
        print(f"Error downloading from {url}: {e}")
    except zipfile.BadZipFile as e:
        print(f"Error unpacking archive: {e}")

def update_manifest(root, file, chip, version):
    manifest_path = os.path.join('ti', 'manifest.json')
    link = f"https://raw.githubusercontent.com/xyzroe/XZG/zb_fws/{root}/{file}"
    data = {
        chip: {
            file: {
                "ver": version,
                "link": link,
                "notes": ""
            }
        }
    }
    if os.path.exists(manifest_path):
        with open(manifest_path, 'r+') as f:
            manifest = json.load(f)
            if root not in manifest:
                manifest[root] = {}
            if chip not in manifest[root]:
                manifest[root][chip] = {}
            manifest[root][chip][file] = data[chip][file]
            f.seek(0)
            json.dump(manifest, f, indent=4)
            f.truncate()
    else:
        with open(manifest_path, 'w') as f:
            json.dump({root: data}, f, indent=4)

with open('task.json', 'r') as f:
    tasks = json.load(f)
    for task in tasks:
        dir_path = os.path.join('ti', task['type'])
        os.makedirs(dir_path, exist_ok=True)
        download_and_extract(task['link'], dir_path)

for root, dirs, files in os.walk('ti'):
    for file in files:
        if file.endswith(".bin"):
            bin_path = os.path.join(root, file)
            # Extract chip and version from the file name
            parts = file.split('_')
            chip = '_'.join(parts[:-1])  # Chip is everything before the date part
            version = parts[-1].split('.')[0]  # Assuming the version is the last part before '.bin'
            update_manifest(root, file, chip, version)
