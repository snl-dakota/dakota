@echo off
:# --------------------------------------------------------------------
:# link
:# copy source files into the tmp directory
:#
:# it *must* be run from the emx directory
:#
:# Mark Olesen
:# olesen@Weber.QueensU.CA
:# 8-Sept-1994
:# -----------------------------------------------------------------------

set base=..\..\..

:check
if "%1" == "main"	goto loop
if "%1" == "stubs"	goto loop
if "%1" == "examples"	goto examples

:next
shift
if not "%1" == "" goto check
goto unset

:main
	copy %base%\lib\*.fnt	.\tmp
	copy %base%\src\*.c	.\tmp
	copy %base%\include\*.h	.\tmp
	copy %base%\drivers\*.c	.\tmp
	copy %base%\utils\*.*	.\tmp
	copy .\src\*.c		.\tmp
	copy .\cf\*.h		.\tmp
goto next

:stubs
	copy %base%\src\stubc\*.c	.\tmp
	copy %base%\src\stubf\*.*	.\tmp
goto next

:examples
	copy %base%\examples\C\*.c	.\tmp
	copy %base%\examples\f77\*.f	.\tmp
goto next

:# unset local variables
:unset
set base=
:# -------------------------------------------------------- end of file
