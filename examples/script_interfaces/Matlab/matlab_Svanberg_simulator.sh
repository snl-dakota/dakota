#!/bin/csh -f
# Sample simulator to Dakota system call script -- Matlab example
# See User Manual for instructions
#
# Robert A. Cafield, 2/13/2018

# $argv[1] is params.in.(fn_eval_num) FROM Dakota
# $argv[2] is results.out.(fn_eval_num) returned to Dakota


# Assuming Matlab .m files and any necessary data are in ./
# from which DAKOTA is run

# NOTE: The workdir.* could be eliminated since the matlab wrapper
# accepts parameters file names as input, but is included for example

# ------------------------
# Set up working directory
# ------------------------

# you could simplify this and keep all files in your main directory
# if you are only running one simulation at a time.

# strip function evaluation number for making working directory

set num = `echo $argv[1] | cut -c 11-`

mkdir workdir.$num

alias matlab /Applications/MATLAB_R2017b.app/bin/matlab

# copy parameters file from DAKOTA into working directory
cp $argv[1] workdir.$num/params.in

# copy any necessary .m files and data files into workdir
cp *Svanberg*.m workdir.$num/

# ------------------------------------
# RUN the simulation from workdir.num
# ------------------------------------
# launch Matlab with command mode (-r)
# matlab_rosen_wrapper.m will call the actual analysis files
# fSvanbergBeam.m and fSvanbergBeam.m
cd workdir.$num

echo matlab -nojvm -nodesktop -nodisplay -nosplash -r "matlab_Svanberg_wrapper('params.in', 'results.out'); exit"

matlab -nojvm -nodesktop -nodisplay -nosplash -r "matlab_Svanberg_wrapper('params.in', 'results.out'); exit"
# *** --> The 'exit' command is crucial so Matlab will exit after running
#         the analysis!


# -------------------------------
# write results.out.X and cleanup
# -------------------------------
mv results.out ../$argv[2] 

cd ..
\rm -rf workdir.$num