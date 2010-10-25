REM DOS batch script DAKOTA will call to execute rosenbrock binary
REM DAKOTA will execute this script as
REM   rosenbrock_wrapper.bat params.in results.out
REM so %1 will be the parameters file and
REM    %2 will be the results file to return to DAKOTA

REM mock pre-processing
copy %1 rosen.in

REM run binary
echo "Running Rosenbrock binary"
rosenbrock rosen.in rosen.out
del rosen.in

REM mock post-processing
move rosen.out %2
