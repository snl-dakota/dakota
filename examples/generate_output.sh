#!/bin/sh

# Run from dakota/build/examples/[tutorial/methods]/ to generate .sav
# files to copy to the source tree.
#
# Assumes Dakota src and test are on the PATH

for file in `ls *.in`;
do
  echo "Generating output for ${file}"
  name=`basename $file .in`
  dakota -input ${file} -output ${name}.out 1> ${name}.stdout
  cp ${name}.out  ${name}.out.sav
  cp ${name}.stdout  ${name}.stdout.sav
  if [ -f ${name}.dat ]; then
    cp ${name}.dat ${name}.dat.sav
  fi
done
