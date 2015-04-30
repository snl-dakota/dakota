::
:: A script used for pre-build copying in Acro packages
::
:: link_all <target-dir> <header-dir1> ...
::
@echo off

set targetdir=%1
if not exist %targetdir% mkdir %targetdir%
:iterate
shift
if "%1" == "" goto end
for %%a in (%1\*.h) do if not exist %targetdir%\%%~na.h fsutil hardlink create %targetdir%\%%~na.h %%a
goto iterate
:end
set targetdir=
