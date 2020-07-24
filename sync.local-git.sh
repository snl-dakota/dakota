#!/bin/sh
# Clone SRN repos
# Need to convert this to work on Windows - RWH
if [ -d local ]; then
  cd local
  git fetch origin
  cd ../
else
  git clone software-srn.sandia.gov:/git/dakota local
fi

cd local
if [ $# == 1 ] && [ $1 == "-u" ]; then
  #git checkout master
  #git submodule update --init
  git log --pretty=format:"%H" -1 > ../external.local
elif [ -f ../external.local ]; then
  git checkout `cat ../external.local`
  if [ -d packages ]; then
    rm -rf packages
  fi
  if [ -d .git/modules ]; then
    rm -rf .git/modules
  fi
fi
cd -
