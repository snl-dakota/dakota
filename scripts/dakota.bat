@echo off

rem  _______________________________________________________________________
rem
rem  DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
rem  Copyright (c) 2010, Sandia National Laboratories.
rem  This software is distributed under the GNU Lesser General Public License.
rem  For more information, see the README file in the top Dakota directory.
rem  _______________________________________________________________________

rem Wrapper script dakota.bat to help manage binary and library paths
rem when running dakota.  Assume dakota is installed alongside this
rem script and libraries are in the same directory and/or ..\lib.

rem get the name and extension of this script
set script_name=%~nx0

rem get the path (drive letter and path) to this wrapper script
rem assume DAKOTA lives in same directory
set execpath=%~dp0

if not exist %execpath%\dakota.exe (
  echo Error in %script_name%
  echo   Could not find dakota binary in %execpath%
  exit /B 1
) 

set libpaths=%execpath%;%execpath%\lib

rem echo "Prepending library path with %libpaths%"
set PATH=%libpaths%;%PATH%

rem echo "Appending PATH with %execpath%;%execpath%\..\test"
set PATH=%PATH%;%execpath%;%execpath%\..\test

rem echo "Launching %execpath%\dakota.exe with args: %*"
%execpath%\dakota.exe %*
