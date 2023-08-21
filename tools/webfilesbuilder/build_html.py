#!/usr/bin/env python3

Import("env")
import os

print("Try to build HMTL gzip files")

os.mkdir('./src/webh')
os.chdir('./tools/webfilesbuilder/')


cwd = os.getcwd()
print("{0}".format(cwd))         

env.Execute("npm install")

env.Execute("npx gulp")