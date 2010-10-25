@echo off
:# --------------------------------------------------------------------
:# mkmf - a Makefile build batch for DOS
:# it *must* be run from the emx directory
:# --------------------------------------------------------------------

set enable_f77="no"
set plcf=..\..\..\cf

:# --------------------------------------------------------------------
if exist mkmf.bat goto istmp
echo "ERROR: mkmf *must* be run from the emx directory"
goto unset

:# make certain that "tmp" directory exists
:istmp
if not exist .\tmp\con mkdir .\tmp

:# Makefile initialization
echo "creating tmp\Makefile"

type cf\init.in		>tmp\Makefile

:# Default target, core source and object file lists

type %plcf%\dist.in	>>tmp\Makefile

:# Copy source file lists and add compiler specific source to makefile

type cf\emx.in		>>tmp\Makefile

:# Optional packages

if %enable_f77% == "yes" type %plcf%\pkg_f77.in	>>tmp\Makefile

:# Library targets

type cf\initlib.in	>>tmp\Makefile
type cf\lib.in		>>tmp\Makefile

:# Program and demo file dependencies, targets

type %plcf%\exes.in	>>tmp\Makefile
type %plcf%\demos.in	>>tmp\Makefile

:# Installation and miscellaneous.

type cf\install.in	>>tmp\Makefile
type cf\Misc.in		>>tmp\Makefile

:#  Object file dependencies

type %plcf%\objs.in	>>tmp\Makefile

:# make list of objects for the response file

echo "creating tmp\libfiles.tmp"

type cf\all_o.rsp >tmp\libfiles.tmp
if %enable_f77% == "yes" type cf\fstubs_o.rsp >>tmp\libfiles.tmp

:# --------------------------------------------------------------------
:# Now build Makedemo.in.
:# Makedemo is a stand-alone makefile for the demo programs.
:# Note: it links against the installed PLplot library.
:# --------------------------------------------------------------------

echo "creating tmp\Makedemo"

type cf\init.in		>tmp\Makedemo
type cf\initdemo.in	>>tmp\Makedemo

type %plcf%\demos.in	>>tmp\Makedemo
type cf\Miscdemo.in	>>tmp\Makedemo

:# give some reminders of what is to be done
echo done
echo.
echo.
echo 
echo dos$  .\link main stubs		(ordinary installation -- only done once)
echo.
echo.
echo To Compile:
echo.
echo dos$ cd tmp
echo dos$ make
echo dos$ make install

:# unset local variables
:unset
set enable_f77=
set plcf=
:# -------------------------------------------------------- end of file
