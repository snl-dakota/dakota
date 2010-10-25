@echo off
echo Installing PLplot in "%1"
rem
rem PLplot: installation script for Windows
rem Requires one argument: the directory where everything
rem should be stored

rem If the directory exist, assume we can simply copy the files
rem
if exists %1 goto copy
md "%1"
md "%1\doc"
md "%1\include"
md "%1\lib"

:copy
copy .\plplib\plplot.lib "%1\lib"
copy .\plpdll\Release\plplotd.lib "%1\lib"
copy .\plpdll\Release\plplotd.dll "%1\lib"
copy .\plplot_win.html "%1\doc"
copy .\..\..\..\tmp\*.h "%1\include"
