#
# Makefile for PLPLOT
#
# This makefile is intended to be used with Microsft NMAKE, as supplied with
# MS C 6.0 Professional Development System (it's not actually very expensive).
# It is currently set up to be run in an OS/2 session, and will build the
# PLPLOT system of libraries, utilities and examples for execution under either
# OS/2 native mode or under DOS (and the OS/2 DOS box).
#
# To build the DOS version, you will need to modify the makefile in a couple
# of places, in order to include the MSCVGA driver.  These changes are marked
# in the makefile with comments starting with "# DOS".
#
# Questions or problems, send (e)mail to:
#       <insert supporter's name here>

PLDIR	= c:\plplot\
PLFNT_DIR=c:\etc\
PLLIBDIR = .
FONTFLAG= "-DPLFONTDEV1=\"c:\\etc\\ \""

CC	= cl
F77	= fl
# CFLAGS	= /FPi87  /nologo /Od /Zi /AL
# FFLAGS	= /FPi87  /nologo /Od /Zi /AL
#
# CFLAGS	= /FPi87  /nologo /Ox /G2 /AL
# FFLAGS	= /FPi87  /nologo /Ox /G2 /AL

CFLAGS	= /FPi87  /nologo /Ox /G2 /AL /Za /W4
FFLAGS	= /FPi87  /nologo /Ox /G2 /AL

# OS/2	Enable -DOS2PM if you have purchassed this add-on driver.
#	Also, you'll need to enable it in the driver list below...
# PLDEVICES	= -DPS -DLJII -DPLMETA -DHP7470 -DIMP -DOS2PM -DNULLDEV

# DOS
# PLDEVICES	= -DPS -DLJII -DPLMETA -DHP7470 -DIMP -DVGA -DNULLDEV
# PLDEVICES = -DLJII -DPLMETA -DHP7470 -DVGA -DNULLDEV
#
# define a minimum set of drivers
PLDEVICES =  -DPLMETA -DHP7470 -DHP7580 -DVGA

# Rules
.SUFFIXES:
.SUFFIXES: .o .c .for .exe

.c.o:
	$(CC) -c $(CFLAGS) -DMSDOS $(PLDEVICES) /Fo$*.o $*.c

.c.exe:
	$(CC) $(CFLAGS) -DMSDOS $*.c -link graphics.lib plplot.lib

.for.o:
	$(F77) -c $(FFLAGS) /Fo$*.o $*.for

##############################################################################
#
# Dependencies for plplot package.
#
##############################################################################

# Object file macros
# Main sources.

OBJ =	\
	pdfutils.o \
	plargs.o \
	plbox.o \
	plcont.o \
	plcore.o \
	plctest.o \
	plctrl.o \
	plcvt.o \
	pldtik.o \
	plfill.o \
	plhist.o \
	plline.o \
	plot3d.o \
	plpage.o \
	plsdef.o \
	plshade.o \
	plsym.o \
	pltick.o \
	plvpor.o \
	plwind.o

# Support files for font generators.

FONT_OBJ = \
	font01.o \
	font02.o \
	font03.o \
	font04.o \
	font05.o \
	font06.o \
	font07.o \
	font08.o \
	font09.o \
	font10.o \
	font11.o

# C & Fortran stubs for linking Plplot to Fortran.

CSTUB_OBJ = \
	sc3d.o \
	sccont.o \
	scstubs.o

FSTUB_OBJ = \
	strutil.o \
	sfstubs.o


# DOS *or* OS/2         comment/uncomment of the next two declarations

# DOS
OS2_OBJ = mscvga.o

# OS/2
# OS2_OBJ = os2pm.o

# Drivers

DRIVERS_OBJ = \
	dg300.o \
	hpgl.o \
	impress.o \
	ljii.o \
	null.o \
	plbuf.o \
	plmeta.o \
	ps.o

#################

# Library dependencies

libs:	libc libf
libc:	$(PLLIBDIR)\plplot.lib
libf:	$(PLLIBDIR)\plstub.lib

$(PLLIBDIR)\plplot.lib:	$(OBJ) $(DRIVERS_OBJ) $(OS2_OBJ)
	@echo Ready to build plplot.lib
	-@del plplot.lib
	lib @<<
plplot
y
+ $(OBJ:.o =.o &
+) &
+ $(DRIVERS_OBJ:.o =.o &
+) &
+ $(OS2_OBJ:.o =.o &
+) ;
<<

$(PLLIBDIR)\plstub.lib:	$(CSTUB_OBJ)
	@echo Ready to build plstub.lib
	-@del plstub.lib
	lib @<<
plstub
y
+ $(CSTUB_OBJ:.o =.o &
+) ;
<<

# $(PLLIBDIR)\plstub.lib:	$(CSTUB_OBJ) $(FSTUB_OBJ)
# 	@echo Ready to build plstub.lib
# 	-@del plstub.lib
# 	lib @<<
# plstub
# y
# + $(CSTUB_OBJ:.o =.o &
# +) &
# + $(FSTUB_OBJ:.o =.o &
# +) ;
# <<

# In case you REALLY mean it!

force: links libs

# First time only, should be executed from $(PLDIR)tmp\

links: linkmain linkstubs

linkmain:
	copy ..\src\*.c .
	copy ..\include\*.h .
	copy ..\examples\c\*.c .
	copy ..\examples\f77\*.f .
	copy ..\drivers\*.* .
	copy ..\fonts\*.* .
	copy ..\utils\*.* .
	copy ..\sys\dos\msc\*.c .
	ren *.f *.for

linkstubs:	
	copy ..\sys\os2\stubf\*.* .
	copy ..\src\stubc\*.* .
	ren *.f *.for

# The metafile renderer.

plrender: plrender.exe

# 	$(CC) $(CFLAGS) plrender.c plplot.lib
# 	cl $(CFLAGS) plrender.c getopt.c plplot.lib

#----------------------------------------------------------------------#
# Font files.
# Note 6 font files actually created.
# sfont - standard font
# xfont - extended fonts

fonts:	sfont xfont
	copy *.fnt $(PLFNT_DIR)

sfont:	$(PLFNT_DIR)plstnd.fnt
xfont:	$(PLFNT_DIR)plxtnd.fnt

$(PLFNT_DIR)plstnd.fnt:
	$(CC) $(CFLAGS:AL=AH) /Gt16 stndfont.c font*.c pdfutils.c
	-stndfont

$(PLFNT_DIR)plxtnd.fnt:
	$(CC) $(CFLAGS:AL=AH) /Gt16 xtndfont.c font*.obj pdfutils.obj
	-xtndfont

#----------------------------------------------------------------------#
# Build all the demos

demos : cdemos fdemos

# Example programs, in c.

heap: chkheap.exe

cdemos: x01c.exe x02c.exe x03c.exe x04c.exe\
        x05c.exe x06c.exe x07c.exe x08c.exe\
        x09c.exe x10c.exe x11c.exe x12c.exe\
        x13c.exe x15c.exe tutor.exe

# remove x14c.exe, since dos does not have xwindows

# Example programs, in Fortran.

fdemos: x01f.exe x02f.exe x03f.exe x04f.exe x05f.exe x06f.exe x07f.exe x08f.exe x09f.exe x10f.exe x11f.exe x12f.exe x13f.exe

x01f.exe:
	$(F77) $(FFLAGS) x01f.for plplot.lib plstub.lib
x02f.exe:
	$(F77) $(FFLAGS) x02f.for plplot.lib plstub.lib
x03f.exe:
	$(F77) $(FFLAGS) x03f.for plplot.lib plstub.lib
x04f.exe:
	$(F77) $(FFLAGS) x04f.for plplot.lib plstub.lib
x05f.exe:
	$(F77) $(FFLAGS) x05f.for plplot.lib plstub.lib
x06f.exe:
	$(F77) $(FFLAGS) x06f.for plplot.lib plstub.lib
x07f.exe:
	$(F77) $(FFLAGS) x07f.for plplot.lib plstub.lib
x08f.exe:
	$(F77) $(FFLAGS) x08f.for plplot.lib plstub.lib
x09f.exe:
	$(F77) $(FFLAGS) /Gt512 x09f.for plplot.lib plstub.lib
x10f.exe:
	$(F77) $(FFLAGS) x10f.for plplot.lib plstub.lib
x11f.exe:
	$(F77) $(FFLAGS) x11f.for plplot.lib plstub.lib
x12f.exe:
	$(F77) $(FFLAGS) x12f.for plplot.lib plstub.lib
x13f.exe:
	$(F77) $(FFLAGS) x13f.for plplot.lib plstub.lib
