#
# This makefile copies all source files to PLTMP and
# creates a makefile in this directory.
# Additional drivers should be uncommented from pldevs.h
#
# Questions/problems,send email to :
#       Paul Casteels   casteels@uia.ac.be
#       University of Antwerp   Dpt. Physics
#       Belgium
#

#
# Location of the Borland include directory
#  (change this for V3.1 !)
#
BCCINCLUDE = c:\bc4\include

#
# Location of plplot directories
#
PLDIR = c:\plplot
PLTMP = $(PLDIR)\tmp
PLLIB = $(PLDIR)\lib
PLBIN = $(PLDIR)\bin

#
# Device selection
#
META =  PLD_plmeta
#PS = PLD_ps
#XFIG = PLD_xfig
#LJII = PLD_ljii
#HPGL = PLD_hp7470
#HPGLL = PLD_hp7580
#LJHPGL = PLD_lj_hpgl
NULL = PLD_null

BGI = PLD_bgi
WIN3 = PLD_win3

default:
  @echo Usage : make win3 , make bgi or make files
files:
#
# Copy the necessary files to the temp dir
#
  @echo Copying source files to $(PLTMP)
  @copy $(PLDIR)\src\*.c $(PLTMP)
  @copy $(PLDIR)\include\*.h $(PLTMP)
  @copy $(PLDIR)\examples\c\*.c $(PLTMP)
  @copy $(PLDIR)\examples\c\*.h $(PLTMP)
  @copy $(PLDIR)\drivers\*.* $(PLTMP)
  @copy $(PLDIR)\utils\*.* $(PLTMP)
#
# Copy the local source files to temp\
#
  @copy src\*.c $(PLTMP)
  @copy src\*.h $(PLTMP)
  @copy src\*.rc $(PLTMP)
  @copy src\*.def $(PLTMP)
#
# Convert from Unix -> Dos
#
  @echo Converting Unix to Dos (add CRs)
  @crlf -fs $(PLTMP)\*.c
  @crlf -fs $(PLTMP)\*.h
  @crlf -fs $(PLTMP)\*.rc
  @crlf -fs $(PLTMP)\*.def
#
# Make some include file available everywhere
#
  @echo Putting some .h files in default include directory
  @copy $(PLDIR)\include\plplot.h $(BCCINCLUDE)
  @copy $(PLDIR)\include\pdf.h $(BCCINCLUDE)

common:
#
# Create plconfig.h file
#
  @echo off
  @echo Creating $(PLTMP)\plconfig.h
  @echoo /c ifndef __PLCONFIG_H__ > $(PLTMP)\plconfig.h
  @echoo /c define __PLCONFIG_H__ >> $(PLTMP)\plconfig.h
  @echoo /c define LIB_DIR \"$(PLLIB:\=\\)\" >> $(PLTMP)\plconfig.h
  @echoo /c define BIN_DIR \"$(PLBIN:\=\\)\" >> $(PLTMP)\plconfig.h
  @echoo /c endif >> $(PLTMP)\plconfig.h
#
# Create pldevs.h file
#
  @echo Creating $(PLTMP)\pldevs.h
!ifdef META
  @echoo /c define PLD_plmeta > $(PLTMP)\pldevs.h
!endif
!ifdef PS
  @echoo /c define PLD_ps >> $(PLTMP)\pldevs.h
!endif
!ifdef XFIG 
  @echoo /c define PLD_xfig >> $(PLTMP)\pldevs.h
!endif
!ifdef LJII 
  @echoo /c define PLD_ljii >> $(PLTMP)\pldevs.h
!endif
!ifdef HPGL
  @echoo /c define PLD_hp7470 >> $(PLTMP)\pldevs.h
!endif
!ifdef HPGLL
  @echoo /c define PLD_hp7580 >> $(PLTMP)\pldevs.h
!endif
!ifdef LJHPGL
  @echoo /c define PLD_lj_hpgl >> $(PLTMP)\pldevs.h
!endif
!ifdef NULL
  @echoo /c define PLD_null >> $(PLTMP)\pldevs.h
!endif
#
# Write first part of the makefile
#
  @echo Creating $(PLTMP)\makefile
#
# Compiler options
#
  @echoo CC = bcc > $(PLTMP)\makefile
  @echoo CFLAGS = -3 -f287 -Od -ml -P -v -w-par -DMSDOS >> $(PLTMP)\makefile
  @echoo BRC = brc >> $(PLTMP)\makefile
  @echoo BRCFLAGS = >> $(PLTMP)\makefile

win3: common
  @echoo /c define $(WIN3) >> $(PLTMP)\pldevs.h
#
# Add Windows definitions to the makefile
#
  @type cf\objects.in >> $(PLTMP)\makefile
  @type cf\win3.in >> $(PLTMP)\makefile
  @type cf\demos.in >> $(PLTMP)\makefile

bgi: common
  @echoo /c define $(BGI) >> $(PLTMP)\pldevs.h
#
# Add dos/bgi definitions to the makefile
#
  @type cf\objects.in >> $(PLTMP)\makefile
  @type cf\bgi.in >> $(PLTMP)\makefile
  @type cf\demos.in >> $(PLTMP)\makefile

