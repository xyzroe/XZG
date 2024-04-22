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

with open('task.json', 'r') as f:
    tasks = json.load(f)
    for task in tasks:
        dir_path = os.path.join('downloads', task['type'])
        os.makedirs(dir_path, exist_ok=True)
        download_and_extract(task['link'], dir_path)

for root, dirs, files in os.walk('downloads'):
    for file in files:
        if file.endswith(".hex"):
            hex_path = os.path.join(root, file)
            bin_path = hex_path[:-4] + ".bin"
            try:
                # Использование srec_cat для конвертации
                command = f"srec_cat {hex_path} -intel -o {bin_path} -binary"
                os.system(command)
                os.remove(hex_path)
            except Exception as e:
                print(f"Error converting file {hex_path}: {e}")
