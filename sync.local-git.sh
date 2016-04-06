#!/bin/sh
# Clone SRN repos
git clone --recursive development.sandia.gov:/git/dakota local
cd local
#git log --pretty=format:"%H" -1 > ../external.local
if [ -f ../external.local ]; then
  git checkout `cat ../external.local`
  git submodule update --init
fi
cd -
