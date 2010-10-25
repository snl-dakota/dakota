#
# This makefile copies all source files to PLTMP.
# Additional drivers should be uncommented from this
# file before running nmake.
#
# Questions/problems, send email to :
#
#       Arjen Markus    arjen.markus@wldelft.nl
#       WL | Delft Hydraulics
#       The Netherlands
#
# Previous maintainer:
#       Olof Svensson   svensson@esrf.fr
#       European Synchrotron Radiation Facility
#       France
#
# Originally written by
#       Paul Casteels   casteels@uia.ua.ac.be
#       University of Antwerp   Dpt. Physics
#       Belgium
#


#
# Location of plplot directories
#
PLDIR = ..\..\..
PLTMP = $(PLDIR)\tmp
PLLIB = $(PLDIR)\lib
PLBIN = $(PLDIR)\bin

#
# Device selection
#
META = PLD_plmeta
PS   = PLD_ps
PSTEX = PLD_pstex
CPS  = PLD_cps
#PNG  = PLD_png
MEM  = PLD_mem
NULL = PLD_null
PBM  = PLD_pbm
XFIG = PLD_xfig
#LJII = PLD_ljii
HPGL = PLD_hp7470
HPGLL = PLD_hp7580
LJHPGL = PLD_lj_hpgl

#BGI = PLD_bgi
WIN3 = PLD_win3
.IGNORE:
default: links common
links:
#
# Copy the necessary files to the temp dir
#
  @mkdir $(PLTMP)
  @mkdir $(PLTMP)\plplot
  @echo ``
  @echo `Copying source files to $(PLTMP)`
  @echo ``
  @copy $(PLDIR)\src\*.c $(PLTMP)\*.c
  @copy $(PLDIR)\include\*.h $(PLTMP)\*.h
  @copy $(PLDIR)\examples\c\*.c $(PLTMP)\*.c
  @copy $(PLDIR)\examples\c\*.h $(PLTMP)\*.h
  @copy $(PLDIR)\examples\c\lena.pgm $(PLTMP)\lena.pgm
  @copy $(PLDIR)\drivers\*.c $(PLTMP)\*.c
  @copy $(PLDIR)\utils\*.c $(PLTMP)\*.c
  @copy $(PLDIR)\data\*.fnt $(PLTMP)\*.fnt
  @copy $(PLDIR)\data\*.map $(PLTMP)\*.map
  @copy $(PLDIR)\bindings\python\plplotcmodule.i $(PLTMP)\plplotcmodule.i
  @copy $(PLDIR)\bindings\python\plplot_widgetmodule.c $(PLTMP)\plplot_widgetmodule.c
  @copy $(PLDIR)\bindings\python\*.py $(PLTMP)\*.py
#
# Copy the local source files to temp\
#
#  @copy src\*.c $(PLTMP)\*.cpp
  @copy src\win3.cpp $(PLTMP)
  @copy src\plconsole.cpp $(PLTMP)
  @copy src\plstub.cpp $(PLTMP)
  @copy src\resource.h $(PLTMP)
  @copy src\plplot.h $(PLTMP)
  @copy src\plplotd.dep $(PLTMP)
  @copy src\setup.py $(PLTMP)\setup.py
  @copy src\*.rc $(PLTMP)
  @copy src\*.ico $(PLTMP)
  @copy src\*.def $(PLTMP)
#
# Convert from Unix -> Dos
#
#  @echo ``
#  @echo `Converting Unix-2-Dos (add CRs)`
#  @echo ``
#  @crlf -fs $(PLTMP)\*.cpp
#  @crlf -fs $(PLTMP)\*.h
#  @crlf -fs $(PLTMP)\*.rc
#  @crlf -fs $(PLTMP)\*.def
common:
#
# Create plconfig.h file
#
  @echo ``
  @echo `Creating $(PLTMP)\plconfig.h`
  @echo <<$(PLTMP)\plconfig.h
#ifndef __PLCONFIG_H__
#define __PLCONFIG_H__
#define LIB_DIR "$(PLLIB:\=\\)"
#define BIN_DIR "$(PLBIN:\=\\)"
#define DATA_DIR "$(PLLIB:\=\\)"
#define DRV_DIR "$(PLLIB:\=\\)"
#define VERSION "5.5.0"
#define PL_DOUBLE
#endif
<<keep
# Create pldevs.h file
#
  @echo ``
  @echo `Creating $(PLTMP)\pldevs.h`
  @echo <<$(PLTMP)\pldevs.h
!ifdef META
#define PLD_plmeta
!endif
!ifdef PS
#define PLD_ps
!endif
!ifdef CPS
#define PLD_cps
!endif
!ifdef PBM
#define PLD_pbm
!endif
!ifdef XFIG
#define PLD_xfig
!endif
!ifdef LJII
#define PLD_ljii
!endif
!ifdef HPGL
#define PLD_hp7470
!endif
!ifdef HPGLL
#define PLD_hp7580
!endif
!ifdef LJHPGL
#define PLD_lj_hpgl
!endif
!ifdef NULL
#define PLD_null
!endif
!ifdef WIN3
#define PLD_win3
!endif
!ifdef PNG
#define PLD_png
!endif
!ifdef PSTEX
#define PLD_pstex
!endif
<<keep
install:
        @copy $(PLDIR)\sys\win32\msdev\plplib\plplot.lib $(PLLIB)
#        @copy $(PLDIR)\sys\win32\msdev\plpdll\Debug\plplotd.lib $(PLLIB)
#        @copy $(PLDIR)\sys\win32\msdev\plpdll\Release\plplotd.lib $(PLLIB)
#        @copy $(PLDIR)\sys\win32\msdev\plpdll\plplotd.dll $(PLBIN)
