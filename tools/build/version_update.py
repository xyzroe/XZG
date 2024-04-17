""" Create version header and tracker file if missing """

import datetime
import os
import sys

Import("env")

VERSION_HEADER = "version.h"

sys.path.append("./tools")
from func import print_logo
from func import print_colored

print_logo()

def get_formatted_date(dateTimeBuild):
    formatted_date = dateTimeBuild.strftime("%Y%m%d")
    return formatted_date

dateTimeBuild = datetime.datetime.now()

HEADER_FILE = """// AUTO GENERATED FILE
#ifndef VERSION
    #define VERSION "{}"
#endif
""".format(
    get_formatted_date(dateTimeBuild)
)

if os.environ.get("PLATFORMIO_INCLUDE_DIR") is not None:
    VERSION_HEADER = (
        os.environ.get("PLATFORMIO_INCLUDE_DIR") + os.sep + VERSION_HEADER
    )
elif os.path.exists("src"):
    VERSION_HEADER = "src" + os.sep + VERSION_HEADER
else:
    PROJECT_DIR = env.subst("$PROJECT_DIR")
    os.mkdir(PROJECT_DIR + os.sep + "include")
    VERSION_HEADER = "include" + os.sep + VERSION_HEADER

with open(VERSION_HEADER, "w+") as FILE:
    FILE.write(HEADER_FILE)

print_colored("Build: {}".format(str(get_formatted_date(dateTimeBuild))), "magenta")
