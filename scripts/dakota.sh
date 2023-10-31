#!/bin/sh

#  _______________________________________________________________________
#
#  Dakota: Explore and predict with confidence.
#  Copyright 2014-2023
#  National Technology & Engineering Solutions of Sandia, LLC (NTESS).
#  This software is distributed under the GNU Lesser General Public License.
#  For more information, see the README file in the top Dakota directory.
#  _______________________________________________________________________

# Wrapper script dakota.sh to help manage binary and library paths
# when running dakota.  Assume dakota is installed alongside this
# script and libraries are in the same directory and/or ../lib.
script_name=`basename ${0}`

# get the path to this wrapper script
# assume DAKOTA lives in same directory

if [ $(uname) == 'Darwin' ]; then
  if [ -L ${0} ]; then
    execpath=$( dirname $( readlink "${0}" ) )  
  else
    execpath=`dirname ${0}`
  fi
else
  # readlink reliably takes care of the case when dakota.sh is called via a
  # symlink, but doesn't work on Darwin with the -f option.
  execpath=$( dirname $( readlink -f "${0}" ) )  
fi

if [ ! -e "${execpath}/dakota" ]; then
  echo "Error in ${script_name}"
  echo "  Could not find dakota binary in ${execpath}"
  exit 1
elif [ ! -x "${execpath}/dakota" ]; then
  echo "Error in ${script_name}"
  echo "  dakota binary in ${execpath} is not executable."
  exit 1
fi
libpaths="${execpath}:${execpath}/../lib"

#echo "Prepending library path with ${libpaths}"
if [ `uname` = "Darwin" ]; then 
  DYLD_LIBRARY_PATH="${libpaths}:${DYLD_LIBRARY_PATH}"
  export DYLD_LIBRARY_PATH
else
  LD_LIBRARY_PATH="${libpaths}:${LD_LIBRARY_PATH}"
  export LD_LIBRARY_PATH
fi


# Workaround for 
#terminate called after throwing an instance of 'std::runtime_error'
#  what():  locale::facet::_S_create_c_locale name not valid
#/dakota/install.cygwin/bin/dakota.sh: line 47:  9824 Aborted                 (core dumped) "${execpath}/dakota" "$@"
if [ `uname | grep -c -i "cygwin"` -gt 0 ]; then
  export LC_ALL="C"
fi

PYTHONPATH="${PYTHONPATH}:${execpath}/../share/dakota/Python"
export PYTHONPATH

#echo "Appending PATH with ${execpath}:${execpath}/../share/dakota/test:."
PATH="$PATH:${execpath}:${execpath}/../share/dakota/test:."
export PATH
#echo "Launching ${execpath}/dakota with args: $@"
exec "${execpath}/dakota" "$@"
