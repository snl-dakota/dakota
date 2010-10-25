@echo off
cls
echo *
echo *
echo * Configuring the system to use Microsoft C 6.0 Professional
echo * Development System, and Microsoft Fortran 5.0 under OS/2.
echo *
echo * Target environment is DOS
echo *

set	 path=c:\c6.0\binp;c:\c6.0\binb;c:\f5.0\bin;%syspath%
set	 path=%path%c:\util2;c:\bndutil;c:\cmd

set	  lib=c:\c6.0\lib;c:\f5.0\lib
set   include=c:\c6.0\include;c:\f5.0\include
rem set  init=c:\c6.0\init	Actually, just leave it set to \util\tools
set helpfiles=c:\c6.0\help\*.hlp
set	   cl= /FPi87 /Lr /link graphics
set	   fl= /FPi87 /link /NOE /NOD:llibfor7 /NOD:llibc7 llibc7r llibf7rc graphics

echo * Okay, Buddy, the computer is configured for using both products.
echo * Here is how the environment is set:
echo ------------------------------------------------------------------------
set
echo ------------------------------------------------------------------------
echo * Any questions?
echo *
