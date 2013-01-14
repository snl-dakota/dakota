rem Sample simulator to Dakota system call script -- Scilab example
rem See User Manual for instructions
rem
rem bvbw 10/24/01
rem Brian M. Adams, 11/17/2005; 5/11/2009

rem $1 is params.in FROM Dakota
rem $2 is results.out returned to Dakota

rem Assuming Scilab .sci files and any necessary data are in ./
rem from which DAKOTA is run

rem NOTE: The workdir could be eliminated since the scilab wrapper
rem accepts parameters file names as input, but is included for example

rem ------------------------
rem Set up working directory
rem ------------------------

rem you could simplify this and keep all files in your main directory
rem if you are only running one simulation at a time.

rem strip function evaluation number for making working directory

mkdir workdir

echo "test : p1 = $1"
echo "test : p2 = $2"

rem copy parameters file from DAKOTA into working directory
copy $1 workdir\params.in

rem copy any necessary .m files and data files into workdir
copy rosenbrock.sci scilab_rosen_wrapper.sci workdir

rem ------------------------------------
rem RUN the simulation from workdir.num
rem ------------------------------------
rem launch Scilab with command mode (-r)
rem scilab_rosen_wrapper.sci will call the actual analysis file
rem rosenbrock.sci
cd workdir

echo scilab -nwni -e "exec scilab_rosen_wrapper.sci; scilab_rosen_wrapper('params.in', 'results.out'); exit"

scilab -nwni -e "exec scilab_rosen_wrapper.sci; scilab_rosen_wrapper('params.in', 'results.out'); exit"

rem *** --> The 'exit' command is crucial so Scilab will exit after running
rem         the analysis!


rem -------------------------------
rem write results.out.X and cleanup
rem -------------------------------
copy results.out ../$2 

cd ..
rmdir /s /q workdir


