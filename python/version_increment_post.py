""" Remove version increment guard file if present """
import os

if os.path.exists(".version_no_increment"):
    os.remove(".version_no_increment")

#  Copyright (C) 2020  Davide Perini