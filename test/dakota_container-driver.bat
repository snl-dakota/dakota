rem %1 is container.in.(fn_eval_num) FROM Dakota
rem %2 is container.out.(fn_eval_num) returned to Dakota
rem Original csh script by Boyd Schimel, 3/08/00
rem Bourne shell changes by Shane Brown, 3/16/05

rem create a unique temporary directory using %1 from character 14 to end
set params_file=%1
set num=%params_file:~13%

mkdir workdir.%num%

rem make workdir.%1 the current working directory
copy %1 workdir.%num%
cd workdir.%num%

rem run the container optimization interface from workdir.%1
container %1 %2

rem move the completed output file to the dakota working directory
move %2 ../.

rem remove the temporary working directory
cd ..
rmdir /s /q workdir.%num%
