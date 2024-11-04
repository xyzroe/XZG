#!/usr/bin/env python3

Import("env")

from subprocess import call
import shutil
import os
import time
from glob import glob
import sys

sys.path.append("./tools")
from func import print_logo
from func import print_colored

VERSION_HEADER = "version.h"

def extract_version_from_file(file_path):
    try:
        with open(file_path, 'r') as file:
            for line in file:
                if '#define VERSION' in line:
                    version = line.split('"')[1]
                    return version
    except FileNotFoundError:
        print("File not found")
    return None

def after_build(source, target, env):
    time.sleep(2)
    shutil.copy(firmware_source, "bin/firmware.bin")
    for f in glob("bin/XZG_*.bin"):
        os.unlink(f)

    exit_code = call(
        "python tools/build/merge_bin_esp.py --output_folder ./bin --output_name XZG.full.bin --bin_path bin/bootloader_dio_40m.bin bin/partitions.bin bin/firmware.bin bin/XZG.fs.bin --bin_address 0x1000 0x8000 0x10000 0x290000",
        shell=True,
    )

    VERSION_FILE = "src/" + VERSION_HEADER
    
    VERSION_NUMBER = extract_version_from_file(VERSION_FILE)
    
    NEW_NAME_BASE = "bin/XZG_" + VERSION_NUMBER
    
    build_env = env['PIOENV']
    if "debug" in build_env:
        NEW_NAME_BASE += "_" + build_env
    
    NEW_NAME_FULL = NEW_NAME_BASE + ".full.bin"
    NEW_NAME_OTA = NEW_NAME_BASE + ".ota.bin"
    NEW_NAME_FS = NEW_NAME_BASE + ".fs.bin"

    shutil.move("bin/XZG.full.bin", NEW_NAME_FULL)
    shutil.move("bin/firmware.bin", NEW_NAME_OTA)
    shutil.move("bin/XZG.fs.bin", NEW_NAME_FS)

    print("")
    print_colored("--------------------------------------", "yellow")
    print_colored("{} created !".format(str(NEW_NAME_FULL)), "blue")
    print_colored("{} created !".format(str(NEW_NAME_OTA)), "magenta")
    print_colored("{} created !".format(str(NEW_NAME_FS)), "green")
    print_colored("--------------------------------------", "yellow")
    print_logo()
    print_colored("Build " + VERSION_NUMBER, "cyan")
    print("")

env.AddPostAction("buildprog", after_build)

firmware_source = os.path.join(env.subst("$BUILD_DIR"), "firmware.bin")
