@echo off
setlocal enabledelayedexpansion
 
rem  _______________________________________________________________________
rem
rem  DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
rem  Copyright 2014 Sandia Corporation.
rem  This software is distributed under the GNU Lesser General Public License.
rem  For more information, see the README file in the top Dakota directory.
rem  _______________________________________________________________________
 
rem  _______________________________________________________________________
rem
rem see if the path where this .bat was launched is already in the PATH; if so
rem do not add
rem  _______________________________________________________________________
echo "%PATH%" | find "%~dp0" > NUL 2>&1 || set PATH=%~dp0;%~dp0\..\lib;%PATH%;%~dp0\..\test
 
rem  _______________________________________________________________________
rem
rem see if we can now find dakota.exe on the path; this approach handles
rem issues with spaces and/or parentheses in the path
rem  _______________________________________________________________________
for %%i in (dakota.exe) do set chk=%%~$PATH:i
 
if "%chk%" == "" (
   echo "**Failed to find Dakota binary"
   exit /B 1
)
 
rem  _______________________________________________________________________
rem
rem launch the found program, which will include the full path; this launch
rem   will handle spaces and parentheses in the path; pass along all of the
rem   other parameters specified to this file
rem  _______________________________________________________________________
 
"%chk%" %*
