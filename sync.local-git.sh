#!/bin/sh
# Clone SRN repos
git clone --recursive development.sandia.gov:/git/dakota local
cd local
git log --pretty=format:"%H" -1 > ../external.local
cd -
