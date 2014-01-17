#!/usr/bin/env python

import sys
import os

if (len(sys.argv) > 2):
    print "Usage: " + sys.argv[0] + " [workingdir]"
    sys.exit(1)

wd = "."
if (len(sys.argv) == 2):
  wd = sys.argv[1]
  if (not os.path.isdir(wd)):
      raise StandardError("Specified working dir " + wd + " does not exist")

ext_file_name = wd + "/externals.local"

if (not os.path.isfile(ext_file_name)):
    raise StandardError("Specified externals file " + ext_file_name +
                        " does not exist")

with open(ext_file_name) as ext_file:
    print "INFO: Checking out directories specified in " + ext_file_name
    for ext in ext_file:
        # skip commented lines
        if not ext.startswith('#'):
            tokens = ext.split()
            if len(tokens) != 3:
                raise IOError("Unexpected line format in " + ext_file_name)
            ext_dir = wd + '/' + tokens[2]
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
                os.system("svn checkout " + ext_rev + ' ' + ext_url + ' ' +
                          ext_dir)
