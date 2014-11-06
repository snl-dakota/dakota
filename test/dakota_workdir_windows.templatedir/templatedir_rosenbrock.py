#!/usr/bin/env python

import sys
import os
import subprocess

# Test work_directory named trydir
workdir_basename="trydir"

if len(sys.argv) > 3:
    mode = sys.argv[1]
    paramsfile = sys.argv[2]
    resultsfile = sys.argv[3]
else:
    mode = None
    paramsfile = sys.argv[1]
    resultsfile = sys.argv[2]


if mode in set(("tag","tag_save")):
    pwd = os.getcwd()
    mytag = pwd.split(".")[-1]
    try:
        mytag = int(mytag)
    except ValueError:
        print "Error: Unexpected directory tag!"
        sys.exit(1)

if mode is "tag_save":
    if mytag > 1:
        lastdir = workdir_basename + ".%d" % (mytag - 1,)
        if not os.path.isdir(lastdir):
            print "Error: Failed to find saved directory",lastdir
            sys.exit(1)
elif mode in set(("templatefiles","templatedir")):
    if not os.path.islink("workdir_rosenbrock.dat"):
        if os.path.isfile("workdir_rosenbrock.dat"):
	    print "Error: workdir_rosenbrock.dat is regular file; should be symlink"
        else:
            print "Error: Failed to find template file workdir_rosenbrock.dat"
        sys.exit(1)

elif mode in set(("templatedir_copy","templatefiles_copy")):
    if os.path.islink("workdir_rosenbrock.dat"):
        print "Error: workdir_rosenbrock.dat is symlink; should be regular file"
        sys.exit(1)
    if not os.isfile("workdir_rosenbrock.dat"):
        print "Error: Failed to find template file workdir_rosenbrock.dat"
        sys.exit(1)

elif mode is "named_params":
    if not os.path.isfile("params.in"):
        print "Error: named parameters file params.in does not exist in trydir"
        sys.exit(1)

print "Running rosenbrock..."

args = ["rosenbrock",paramsfile,resultsfile]

returncode = subprocess.call(args, stdout=sys.stdout,stderr=sys.stderr)
sys.exit(returncode)

