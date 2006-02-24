#!/usr/bin/python
#
# Windows py2exe setup script
# Usage: python setup.py py2exe
#

from distutils.core import setup
import py2exe
import os

opts = {
    "py2exe": {
        "includes": "sip",
        "packages": "AvidaGui2",
        "bundle_files": 1
    }
}

def recurse_dir(dir, filter, ltrim):
    "Recursively return list of files in directory.  filter is a functino to filter the directories and ltrim will trim off n characters from the directory."
    l = []
    for root, newdir, files in os.walk(dir):
        if filter(root):
            for file in files:
                if len(dir) >= root:
                    l.append(file)
                else:
                    l.append((root[ltrim:], [root + "/" + file]))
    return l

setup(
    version = "0.0.1",
    description = "Avida-ED Windows Executable",
    name = "Avida-ED",

    # targets to build
    console = ["Avida-ED.py"],
    data_files =  recurse_dir("../../default.workspace",
                              lambda x: not '.svn' in x, 6),
    options = opts,
    zipfile = None
)
