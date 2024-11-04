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

import subprocess
from SCons.Script import DefaultEnvironment

env = DefaultEnvironment()

def get_directory_size(directory, block_size=4096):
    total_size = 0
    for dirpath, dirnames, filenames in os.walk(directory):
        for f in filenames:
            fp = os.path.join(dirpath, f)
            if os.path.exists(fp):
                file_size = os.path.getsize(fp)
                rounded_size = (file_size + block_size - 1) // block_size * block_size
                total_size += rounded_size
    return total_size

def calculate_filesystem_size(directory, metadata_overhead=0.12, block_size=4096):
    size = get_directory_size(directory, block_size)
    size = int(size * (1 + metadata_overhead))  
    size = (size + block_size - 1) // block_size * block_size  
    return size

def build_filesystem(env):
    try:
        filesystem_dir = os.path.join(env['PROJECT_DIR'], 'data')
        size_file_path = os.path.join(env['PROJECT_DIR'], 'src', 'websrc', 'size.fs')

        if os.path.exists(size_file_path):
            with open(size_file_path, 'r') as size_file:
                filesystem_size = int(size_file.read().strip())
            print(f"Using existing filesystem size from {size_file_path}: {filesystem_size} bytes")
        else:
            filesystem_size = calculate_filesystem_size(filesystem_dir)
            with open(size_file_path, 'w') as size_file:
                size_file.write(str(filesystem_size))
            print(f"Calculated and saved filesystem size to {size_file_path}: {filesystem_size} bytes")

        filesystem_image = os.path.join(env['BUILD_DIR'], 'littlefs.bin')
        mklittlefs_path = os.path.join(env['PROJECT_PACKAGES_DIR'], 'tool-mklittlefs', 'mklittlefs')

        if not os.path.exists(mklittlefs_path):
            print(f"Path to mklittlefs tool is not set or invalid: {mklittlefs_path}")
            return False

        print(f"Building filesystem image at {filesystem_image}")
        print(f"Using mklittlefs tool at {mklittlefs_path}")
        print(f"Filesystem directory: {filesystem_dir}")
        print(f"Filesystem size: {filesystem_size} bytes")

        cmd = [
            mklittlefs_path,
            '-c', filesystem_dir,
            '-b', '4096',
            '-p', '256',
            '-s', str(filesystem_size),
            filesystem_image
        ]

        print(f"Executing: {' '.join(cmd)}")
        result = env.Execute(" ".join(cmd))
        if result == 0:
            return True
        else:
            return False
    except Exception as e:
        print(f"An error occurred: {e}")
        return False

def build_html():
    if (not os.path.exists(NO_WEB_UPDATE)) or any(target in sys.argv for target in ["buildfs", "uploadfs"]):

        print("")
        print("Try to build WEB files")
        print("")
        # time.sleep(1)

        os.makedirs("./data", exist_ok=True)
        
        try:
            result = subprocess.run(["git", "log", "-n", "1", "--pretty=format:%h", "--", "./src/websrc"], capture_output=True, text=True, check=True)
            git_commit_sha = result.stdout.strip()
            os.makedirs("./data/x", exist_ok=True)
            with open("./data/x/commit", "w") as f:
                f.write(git_commit_sha)
            print(f"Git commit SHA saved to ./data/x/commit: {git_commit_sha}")
        except subprocess.CalledProcessError as e:
            print(f"Failed to get Git commit SHA: {e}")
        
        os.chdir("./tools/webfilesbuilder/")

        marker_file = ".npm_install_marker"

        if not os.path.exists(marker_file):

            os.system("npm install")
            
            with open(marker_file, 'w') as f:
                f.write("npm install executed")

        else:
            print("npm install already executed. Skipping.")

        env.Execute("npx gulp xzg")

        print("")
        print("Finish building WEB files")
        
        
        os.chdir("../../")
        
        current_env = env['PIOENV']

        if not current_env:
            raise ValueError("Environment variable PIOENV is not set")
        

        if not build_filesystem(env):
            print("Failed to build filesystem. Exiting.")
            sys.exit(1)
        print("File system built successfully")

        src_fs_path = f"./.pio/build/{current_env}/littlefs.bin"
        dest_fs_path = "./bin/XZG.fs.bin"
        os.makedirs("./bin", exist_ok=True)
        
        shutil.move(src_fs_path, dest_fs_path)
        print(f"File system moved to {dest_fs_path}")
        
        if any(target in sys.argv for target in ["buildfs"]):
            sys.exit(0)
            

if  not any(target in sys.argv for target in ["--clean", "erase"]):
    #print(sys.argv)
    build_html()