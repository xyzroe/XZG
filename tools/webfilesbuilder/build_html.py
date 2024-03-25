#!/usr/bin/env python3

Import("env")

import time
import os
import sys

sys.path.append("./tools")
from func import print_logo

if not os.path.exists("tools/.no_web_update"):

    print("")
    print("Try to build WEB files")
    print("")
    time.sleep(1)

    os.makedirs("./src/webh", exist_ok=True)

    os.chdir("./tools/webfilesbuilder/")

    env.Execute("npm install --silent")

    env.Execute("npx gulp")

    print("")
    print("Finish building WEB files")
    print_logo()

