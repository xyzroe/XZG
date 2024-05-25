""" Create version header and tracker file if missing """
import datetime
import os
import sys

Import("env")

VERSION_HEADER = "version.h"

sys.path.append("./tools")
from func import print_logo, print_colored

print_logo()

def get_formatted_date(dateTimeBuild):
    return dateTimeBuild.strftime("%Y%m%d")

def read_version_from_file(file_path):
    if os.path.exists(file_path):
        with open(file_path, "r") as file:
            for line in file:
                if "#define VERSION" in line:
                    version = line.split('"')[1]
                    return version
    return None

def write_version_to_file(file_path, version):
    header_content = f"""// AUTO GENERATED FILE
#ifndef VERSION
    #define VERSION "{version}"
#endif
"""
    with open(file_path, "w") as file:
        file.write(header_content)

dateTimeBuild = datetime.datetime.now()
current_date = get_formatted_date(dateTimeBuild)

if os.environ.get("PLATFORMIO_INCLUDE_DIR") is not None:
    VERSION_HEADER = os.environ.get("PLATFORMIO_INCLUDE_DIR") + os.sep + VERSION_HEADER
elif os.path.exists("src"):
    VERSION_HEADER = "src" + os.sep + VERSION_HEADER
else:
    PROJECT_DIR = env.subst("$PROJECT_DIR")
    if not os.path.exists(PROJECT_DIR + os.sep + "include"):
        os.mkdir(PROJECT_DIR + os.sep + "include")
    VERSION_HEADER = "include" + os.sep + VERSION_HEADER

current_version = read_version_from_file(VERSION_HEADER)

if current_version:
    if current_version.startswith(current_date):
        new_version = current_version
    else:
        new_version = current_date
else:
    new_version = current_date

write_version_to_file(VERSION_HEADER, new_version)

print_colored(f"Build: {new_version}", "magenta")
