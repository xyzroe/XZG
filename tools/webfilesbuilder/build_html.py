#!/usr/bin/env python3

Import("env")

import time
import os
import sys

# Create this file to avoid rebuilding webh files
NO_WEB_UPDATE = "tools/.no_web_update"

sys.path.append("./tools")
from func import print_logo
    
def build_html():
    if not os.path.exists(NO_WEB_UPDATE):

        print("")
        print("Try to build WEB files")
        print("")
        time.sleep(1)

        os.makedirs("./src/webh", exist_ok=True)

        os.chdir("./tools/webfilesbuilder/")

        env.Execute("npm install --silent")

        env.Execute("npx gulp xzg")

        print("")
        print("Finish building WEB files")
        print_logo()

if not any(target in sys.argv for target in ["--clean", "erase"]):
    build_html()