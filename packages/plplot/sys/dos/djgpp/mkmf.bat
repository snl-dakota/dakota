@echo off
REM -----------------------------------------------------------------------
REM Example Makefile build script for DOS.
REM -----------------------------------------------------------------------

@echo "creating tmp\Makefile"

rem set enable_f77="no"
rem set enable_cxx="yes"

REM set plcf=..\..\..\cf

REM Makefile initialization

type cf\init.in		>tmp\Makefile

REM Default target, core source and object file lists

type cf\dist.in	>>tmp\Makefile

REM Copy the list of DJGPP drivers to makefile

type cf\drivers.in	>>tmp\Makefile

REM Copy source file lists and add djgpp specific source to makefile

type cf\djgpp.in	>>tmp\Makefile

REM Optional packages

rem if %enable_f77% == "yes" type ..\..\..\cf\pkg_f77.in	>>tmp\Makefile

rem if %enable_cxx% == "yes" type ..\..\..\cf\pkg_cxx.in	>>tmp\Makefile

REM Library targets

type cf\version.in	>>tmp\Makefile
type cf\initlib.in	>>tmp\Makefile
type cf\lib.in		>>tmp\Makefile

REM Program and demo file dependencies, targets

type cf\exes.in	>>tmp\Makefile
type cf\demos.in	>>tmp\Makefile

REM Installation and miscellaneous.

type cf\install.in	>>tmp\Makefile
type cf\Misc.in		>>tmp\Makefile

REM Object file dependencies

type cf\objs.in	>>tmp\Makefile

REM -----------------------------------------------------------------------
REM Now build Makedemo.in.
REM Makedemo is a stand-alone makefile for the demo programs.
REM Note: it links against the installed PLplot library.
REM -----------------------------------------------------------------------

echo "creating tmp\Makedemo"

type cf\init.in		>tmp\Makedemo
type cf\djgpp.in	>>tmp\Makedemo
type cf\initdemo.in	>>tmp\Makedemo

type cf\demos.in	>>tmp\Makedemo

REM  Fix up CR/LF now

utod tmp\Makedemo
utod tmp\Makefile
