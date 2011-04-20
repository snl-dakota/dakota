#!/bin/sh
# $1 is container.in.(fn_eval_num) FROM Dakota
# $2 is container.out.(fn_eval_num) returned to Dakota
# Original csh script by Boyd Schimel, 3/08/00
# Bourne shell changes by Shane Brown, 3/16/05

# create a unique temporary directory using $1
num=`echo $1 | cut -c 14-`
mkdir workdir.$num

#make workdir.$1 the current working directory
cp $1 workdir.$num
cd workdir.$num

#run the container optimization interface from workdir.$1
container $1 $2

#move the completed output file to the dakota working directory
mv $2 ../.

#remove the temporary working directory
cd ..
rm -rf workdir.$num

