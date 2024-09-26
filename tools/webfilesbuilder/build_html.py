#!/usr/bin/env python3

Import("env")

import time
import os
import sys
import shutil

# Create this file to avoid rebuilding webh files
NO_WEB_UPDATE = "tools/.no_web_update"

sys.path.append("./tools")
from func import print_logo


import os
import subprocess
import logging
from SCons.Script import DefaultEnvironment

# Настройка логирования
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

env = DefaultEnvironment()

def get_directory_size(directory, block_size=4096):
    total_size = 0
    for dirpath, dirnames, filenames in os.walk(directory):
        for f in filenames:
            fp = os.path.join(dirpath, f)
            file_size = os.path.getsize(fp)
            # Учитываем размер блоков и выравнивание
            total_size += (file_size + block_size - 1) // block_size * block_size
    return total_size

def build_filesystem(env):
    filesystem_dir = os.path.join(env['PROJECT_DIR'], 'data')
    size = get_directory_size(filesystem_dir)
    size = (size + 4095) // 4096 * 4096  # округление до ближайшего блока 4KB

    # Добавляем дополнительное пространство для метаданных и выравнивания
    metadata_overhead = 0.10  # 10% от общего размера данных
    size = int(size * (1 + metadata_overhead))
    size = (size + 4095) // 4096 * 4096  # округление до ближайшего блока 4KB

    filesystem_image = os.path.join(env['BUILD_DIR'], 'littlefs.bin')
    mklittlefs_path = os.path.join(env['PROJECT_PACKAGES_DIR'], 'tool-mklittlefs', 'mklittlefs')

    if not os.path.exists(mklittlefs_path):
        logger.error(f"Path to mklittlefs tool is not set or invalid: {mklittlefs_path}")
        return

    logger.info(f"Building filesystem image at {filesystem_image}")
    logger.info(f"Using mklittlefs tool at {mklittlefs_path}")
    logger.info(f"Filesystem directory: {filesystem_dir}")
    logger.info(f"Calculated filesystem size: {size} bytes")

    cmd = [
        mklittlefs_path,
        '-c', filesystem_dir,
        '-b', '4096',
        '-p', '256',
        '-s', str(size),
        filesystem_image
    ]

    logger.info(f"Executing: {' '.join(cmd)}")
    result = env.Execute(" ".join(cmd))
    if result == 0:
        logger.info("Filesystem image built successfully")
    else:
        logger.error("Failed to build filesystem image")
    
def build_html():
    if (not os.path.exists(NO_WEB_UPDATE)) or any(target in sys.argv for target in ["buildfs"]):

        print("")
        print("Try to build WEB files")
        print("")
        time.sleep(1)

        # os.makedirs("./src/webh", exist_ok=True)
        os.makedirs("./data", exist_ok=True)
        
        os.chdir("./tools/webfilesbuilder/")

        marker_file = ".npm_install_marker"

        if not os.path.exists(marker_file):

            os.system("npm install --silent")
            
            with open(marker_file, 'w') as f:
                f.write("npm install executed")

            print("npm install executed and marker file created.")
        else:
            print("npm install already executed. Skipping.")

        env.Execute("npx gulp xzg")

        print("")
        print("Finish building WEB files")
        
        
        os.chdir("../../")
        
        current_env = env['PIOENV']

        if not current_env:
            raise ValueError("Environment variable PIOENV is not set")
        
        #env.Execute(f"pio run -e {current_env} --target buildfs")
        build_filesystem(env)
        print("File system built successfully")

        src_fs_path = f"./.pio/build/{current_env}/littlefs.bin"
        dest_fs_path = "./bin/XZG.fs.bin"
        os.makedirs("./bin", exist_ok=True)
        
        shutil.move(src_fs_path, dest_fs_path)
        print(f"File system moved to {dest_fs_path}")
        
        if any(target in sys.argv for target in ["buildfs"]):
            sys.exit(0)
            # print_logo()

#if not any(target in sys.argv for target in ["--clean", "erase"]):
if  not any(target in sys.argv for target in ["--clean", "erase"]):
    #print(sys.argv)
    build_html()
