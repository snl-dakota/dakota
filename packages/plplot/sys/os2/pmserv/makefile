#
# Makefile for PLPLOT
#
# This makefile is intended to be used with GNU make and EMX/GCC 2.7.2 (EMX 09b)
# It is currently set up to be run in an OS/2 session
#
# Original taken from:
#
# Questions or problems, send (e)mail to:
#	Geoffrey Furnish
#	furnish@fusion.ph.utexas.edu
#	Institute for Fusion Studies
#	Department of Physics
#	The University of Texas at Austin
#	Austin, Texas	78712
#
# Modified (quick and dirty to get it running) by
#
# Thorsten Feiweier
# thorsten@fuj.physik.uni-dortmund.de


PLDIR	= d:\usr\bin\plplot

CC	= gcc
CFLAGS      = -O3 -Zomf -Zmt -Zcrtdll -Wall

# Rules
.SUFFIXES:
.SUFFIXES: .obj .c

.c.obj:
	$(CC) -c $(CFLAGS) -o $*.obj $*.c 

help:
	@echo * set up the right path in the beginning of this makefile
	@echo * copy the makefile in a subdir of the current one (i.e. you are in '../os2/pmserv', make a 'tmp') 
	@echo * type 'make links' in the subdir to copy the files you need
	@echo * type 'make pmserv' to make the PMServer for PLPLOT
	@echo * (mybe you have to 'set include=/emx09b/include' to get it right)
	@echo * continue making the PLPLOT-part of this contribution

# Links.  Get the files we need for this development.

links:
	copy ..\pmdefs.h .
	copy ..\pmserv.c .
	copy ..\pmserv.def .
	copy ..\pmserv.rc .
	copy ..\read.me .
	copy $(PLDIR)\include\plplot.h .
	
# The Presentation Manager Server.

pmserv: pmserv.c pmdefs.h pmserv.res
	$(CC) -c $(CFLAGS) pmserv.c
	$(CC) $(CFLAGS) pmserv.obj pmserv.def -o pmserv.exe
	rc pmserv.res

pmserv.res: pmserv.rc pmdefs.h
	rc -r pmserv
