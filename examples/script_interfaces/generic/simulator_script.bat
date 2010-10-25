rem prompt off
rem After possibly adjusting $PATH to include a cygwin\bin containing perl, you can
rem use a "dakota" compiled under MinGW (MSys) to run "dakota dakota_rosenbrock_mingw.in"
rem  --------------
rem  PRE-PROCESSING
rem  --------------
rem  Incorporate the parameters from DAKOTA into the template, writing ros.in
rem  Use the following line if SNL's APREPRO utility is used instead of DPrePro.
rem  ../aprepro -c '*' -q --nowarning ros.template ros.in

perl ../../dprepro %1% ros.template ros.in

rem  --------
rem  ANALYSIS
rem  --------

rosenbrock_bb

rem  ---------------
rem  POST-PROCESSING
rem  ---------------

rem  extract function value from the simulation output
grep 'Function value' ros.out | c:\msys\1.0\bin\sed s/^.................// >results.tmp
rem  extract gradients from the simulation output (in this case will be ignored
rem  by DAKOTA if not needed)
grep -i "Function g" ros.out | c:\msys\1.0\bin\sed s/^....................//  >> results.tmp
ren results.tmp %2%
