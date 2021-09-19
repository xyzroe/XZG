#!/usr/bin/env python3
from subprocess import call
Import("env")
import shutil
import os
from glob import glob

def after_build(source, target, env):
    
    shutil.copy(firmware_source, 'bin/firmware.bin')
    for f in glob ('bin/ZigStarGW*.debug.bin'):
        os.unlink (f)

    VERSION_FILE = 'tools/version'
    try:
        with open(VERSION_FILE) as FILE:
            VERSION_NUMBER = FILE.readline()
    except:
        print('No version file found')
        VERSION_NUMBER = '0.0.0'

    NEW_NAME = 'bin/ZigStarGW_v'+VERSION_NUMBER+'.debug.bin'

    shutil.move('bin/firmware.bin', NEW_NAME)

    print('')
    print('--DEBUG--DEBUG--DEBUG--DEBUG--DEBUG--DEBUG--DEBUG--')
    print('{} created with success !'.format(str(NEW_NAME)))
    print('--DEBUG--DEBUG--DEBUG--DEBUG--DEBUG--DEBUG--DEBUG--')
    print('')

env.AddPostAction("buildprog", after_build)

firmware_source = os.path.join(env.subst("$BUILD_DIR"), "firmware.bin")