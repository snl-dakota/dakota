#!/usr/bin/env python

import sys
import os

ext_file_name = "externals.local"

with open(ext_file_name) as ext_file:
    print "INFO: Checking out directories specified in " + ext_file_name
    for ext in ext_file:
        # skip commented lines
        if not ext.startswith('#'):
            tokens = ext.split()
            if len(tokens) != 3:
                raise IOError("Unexpected line format in " + ext_file_name)
            ext_dir = tokens[2]
            if os.path.isdir(ext_dir):
                # perform svn update
                print "INFO: Updating in " + ext_dir      
                os.system("svn update " + ext_dir)
            else:
                # perform svn checkout
                ext_rev = tokens[0]
                ext_url = tokens[1]
                print "INFO: Checking out " + ext_url + " at " + ext_rev + \
                    " into " + ext_dir
                os.system("svn checkout " + ext)
