#!/bin/bash 
# Sample simulator to Dakota system call script -- Scilab example
# See User Manual for instructions
#
# yc 10/24/01
# Yann Chapalain, 4/05/2011

# $1 is params.in FROM Dakota
# $2 is results.out returned to Dakota


# Assuming Scilab .sci files and any necessary data are in ./
# from which DAKOTA is run

# NOTE: The workdir could be eliminated since the scilab wrapper
# accepts parameters file names as input, but is included for example

# ------------------------
# Set up working directory
# ------------------------

# you could simplify this and keep all files in your main directory
# if you are only running one simulation at a time.

# strip function evaluation number for making working directory

mkdir workdir

echo "test : p1 = $1"
echo "test : p2 = $2"

# copy parameters file from DAKOTA into working directory
cp $1 workdir/params.in

# copy any necessary .sci files and data files into workdir
cp automatic_test.xcos init_test_automatic.sce visualize_solution.sce f_pid.sci scilab_rosen_wrapper.sci workdir

# -------------------------------
# RUN the simulation from workdir
# -------------------------------
# launch Scilab with command mode (-r)
# scilab_rosen_wrapper.sci will call the actual analysis file
# rosenbrock.sci
cd workdir

echo scilab -nwni -e "exec init_test_automatic.sce; exec scilab_rosen_wrapper.sci; scilab_rosen_wrapper('params.in', 'results.out'); exec visualize_solution.sce; exit"

scilab -e "exec init_test_automatic.sce; exec scilab_rosen_wrapper.sci; scilab_rosen_wrapper('params.in', 'results.out'); exit"

# *** --> The 'exit' command is crucial so Scilab will exit after running
#         the analysis!

# -------------------------------
# write results.out and cleanup
# -------------------------------
mv results.out ../$2 

cd ..
\rm -rf workdir
