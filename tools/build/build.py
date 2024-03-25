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

def after_build(source, target, env):
    time.sleep(2)
    shutil.copy(firmware_source, "bin/firmware.bin")
    for f in glob("bin/XZG*.bin"):
        os.unlink(f)

    exit_code = call(
        "python tools/build/merge_bin_esp.py --output_folder ./bin --output_name XZG.bin --bin_path bin/bootloader_dio_40m.bin bin/firmware.bin bin/partitions.bin --bin_address 0x1000 0x10000 0x8000",
        shell=True,
    )

    VERSION_FILE = "tools/version"
    try:
        with open(VERSION_FILE) as FILE:
            VERSION_NUMBER = FILE.readline()
    except:
        print("No version file found")
        VERSION_NUMBER = "0.0.0"

    NEW_NAME_FULL = "bin/XZG_v" + VERSION_NUMBER + ".full.bin"
    NEW_NAME = "bin/XZG.bin"

    shutil.move("bin/XZG.bin", NEW_NAME_FULL)
    shutil.move("bin/firmware.bin", NEW_NAME)

    print("")
    print("--------------------------------------------------------")
    print("{} created with success !".format(str(NEW_NAME_FULL)))
    print("{} created with success !".format(str(NEW_NAME)))
    print("--------------------------------------------------------")
    print("")
    print_logo()


env.AddPostAction("buildprog", after_build)

firmware_source = os.path.join(env.subst("$BUILD_DIR"), "firmware.bin")
