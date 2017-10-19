echo off

rem  This simulator script example requires Perl on the PATH.
rem  It can be installed with, e.g., a Windows installer, via Cygwin, or MSYS.

rem  --------------
rem  PRE-PROCESSING
rem  --------------
rem  Incorporate the parameters from DAKOTA into the template, writing ros.in
rem  Use the following line if SNL's APREPRO utility is used instead of DPrePro.
rem  ../aprepro -c '*' -q --nowarning ros.template ros.in

rem  As shipped, example uses workdir, so dprepro is up one directory level
perl ..\dprepro %1% ros.template ros.in


rem  --------
rem  ANALYSIS
rem  --------

rosenbrock_bb


rem  ---------------
rem  POST-PROCESSING
rem  ---------------

rem  Extract function value and gradient from the simulation output
rem  (in this case DAKOTA ignores gradient if not needed)
rem  with examples of unix utilities (grep/sed) and Perl

rem  Example with grep/sed installed via MSYS

rem grep 'Function value' ros.out | c:\msys\1.0\bin\sed s/^.................// >results.tmp
rem grep -i "Function g" ros.out | c:\msys\1.0\bin\sed s/^....................//  >> results.tmp

perl -n -e "if (/Function value = (.*)/) { print \"$1\n\" }" ros.out > results.tmp
perl -n -e "if (/Function gradient = (.*)/) { print $1 }" ros.out >> results.tmp

ren results.tmp %2%
