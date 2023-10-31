#!/bin/sh

#  _______________________________________________________________________
#
#  Dakota: Explore and predict with confidence.
#  Copyright 2014-2023
#  National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________

# Wrapper script to help Dakota GUI find Dakota CLI runtime libraries
# needed, e.g., for surrogate import.
#
# Assumes dakota is installed alongside this script and libraries are
# in the same directory and/or ../lib.
#
# Assuems gui launcher is installed in OS-dependent path ../gui/*...

get_abs_path() {
  # $1 : relative filename
  echo "$(cd "$(dirname "$1")" && pwd)"
}

script_name=`basename ${0}`

# get the path to this wrapper script
# assume dakota executable lives in same directory

if [ $(uname) == 'Darwin' ]; then
  if [ -L "${0}" ]; then
    execpath=$( get_abs_path $( readlink "${0}" ) )
  else
    execpath=$( get_abs_path "${0}" )
  fi
else
  # readlink reliably takes care of the case when dakota.sh is called via a
  # symlink, but doesn't work on Darwin with the -f option.
  execpath=$( dirname $( readlink -f "${0}" ) )  
fi

if [ ! -e "${execpath}/dakota" ]; then
  echo "Warning in ${script_name}"
  echo "  Could not find dakota binary in ${execpath}"
elif [ ! -x "${execpath}/dakota" ]; then
  echo "Warning in ${script_name}"
  echo "  dakota binary in ${execpath} is not executable."
fi

PYTHONPATH="${PYTHONPATH}:${execpath}/../share/dakota/Python"
export PYTHONPATH

#echo "Appending PATH with ${execpath}:${execpath}/../share/dakota/test:."
PATH="$PATH:${execpath}:${execpath}/../share/dakota/test:."
export PATH

libpaths="${execpath}:${execpath}/../lib"
#echo "Prepending library path with ${libpaths}"
if [ `uname` = "Darwin" ]; then 
  DYLD_LIBRARY_PATH="${libpaths}:${DYLD_LIBRARY_PATH}"
  export DYLD_LIBRARY_PATH

  gui_path="${execpath}/../gui/Dakota_*.app/Contents/MacOS"
  # The explicit cd is a workaround for DakotaUI to find the JNI
  # surrogates lib. The gui_path isn't quoted here so the wildcard
  # will get evaluated and work across versions:
  cd ${gui_path}
  ./DakotaUI "$@"
else
  LD_LIBRARY_PATH="${libpaths}:${LD_LIBRARY_PATH}"
  export LD_LIBRARY_PATH

  gui_path="${execpath}/../gui/"
  "${gui_path}/DakotaUI" "$@"
fi
