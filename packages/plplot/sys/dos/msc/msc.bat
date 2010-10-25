:------ settings for running msC [microsoft C] - cl version 7
:
if not %1. == . goto end
:
: set the next line to reflect your setup
:
set SetMSC=c:\l\c7
if not exist %SetMSC%\con goto end
:
set PATH=%PATH%;%SetMSC%\bin
set LIB=%SetMSC%\lib;c:\dev\clib\msc
set INCLUDE=c:\dev\clib;%SetMSC%\include
set HELPFILES=%SetMSC%\help\*.hlp
set INIT=%SetMSC%\init
:end
set SetMSC=
