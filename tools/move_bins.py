import os
from shutil import copyfile
from glob import glob

for f in glob ('bin/*.bin'):
   os.unlink (f)

VERSION_FILE = 'tools/version'
try:
    with open(VERSION_FILE) as FILE:
        VERSION_NUMBER = FILE.readline()
except:
    print('No version file found')
    VERSION_NUMBER = '0.0.0'

NEW_NAME_FW = 'bin/ZigStarGW_v' + VERSION_NUMBER + '_FW.bin'
NEW_NAME_FS = 'bin/ZigStarGW_v' + VERSION_NUMBER + '_FS.bin'

copyfile(".pio/build/esp32/firmware.bin", NEW_NAME_FW)
copyfile(".pio/build/esp32/spiffs.bin", NEW_NAME_FS)