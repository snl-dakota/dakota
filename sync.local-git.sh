#!/bin/sh
# Clone SRN repos
if [ -d local ]; then
  cd local
  git pull origin master --recurse-submodules=on-demand 
  cd ../
else
  git clone --recursive development.sandia.gov:/git/dakota local
fi

cd local
if [ $# == 1 ] && [ $1 == "-u" ]; then
  git checkout master
  git submodule update --init
  git log --pretty=format:"%H" -1 > ../external.local
elif [ -f ../external.local ]; then
  git checkout `cat ../external.local`
  git submodule update --init
fi
cd -
