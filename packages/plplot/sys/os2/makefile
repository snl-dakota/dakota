# -----------------------------------------------------------------------
# $Id: makefile 3186 2006-02-15 18:17:33Z slbrow $
#
# Maurice LeBrun
# IFS, University of Texas at Austin
# 14-Jul-1994
#
# Basic make assignments for PLplot under Unix (driven by "configure").
# These are used in both the library and demo Makefiles.
# -----------------------------------------------------------------------

# Utilities

CC		= gcc
LDC		= gcc
OCC		= gcc -traditional
LDF		= gcc
M4		= @M4@
LN		=
BIND		= emxbind -aq
BIND_OPTS	= -acim
STRIP		= emxbind -sq


# Flags and other miscellanea

F77_FLAGS	=
LDC_FLAGS	=
LDF_FLAGS	=
LDC_LIBS	=
LDF_LIBS	=
LIB_TAG		= 
TO		= -o
O		= .o
A		= .a
E		= .exe

# Library names
# (PLLIB_NAME) is for ld/linking
# (PLLIB_BASE) is the base library name for installation
#
# good for filesystems that truncate names (eg, DOS)

PLLIB_NAME	= plplot
PLLIB_BASE	= plplot

# Install directories (use forward-slashes with `F')

LIB_BASE        = d:\usr\bin\plplot
LIB_DIR		= d:\usr\bin\plplot\lib
LIB_DIRF	= d:/usr/bin/plplot/lib
DOC_DIR	= d:\usr\bin\plplot\doc
BIN_DIR		= d:\usr\bin\plplot\bin
INFO_DIR	= d:\usr\bin\plplot\info
INCLUDE_DIR	= d:\usr\bin\plplot\include
INCLUDE_DIRF	= d:/usr/bin/plplot/include
TCL_DIR	= d:\usr\bin\plplot\tcl
DEMOS_DIR	= d:\usr\bin\plplot\demos
# -----------------------------------------------------------------------
# $Id: makefile 3186 2006-02-15 18:17:33Z slbrow $
#
# Maurice LeBrun
# IFS, University of Texas at Austin
# 22-Jul-1994
#
# PLplot source and object file lists.
# -----------------------------------------------------------------------

# The default target -- a convenient forward reference

default: help


# Core library files.

CORE_C = \
	pdfutils.c \
	plargs.c \
	plbox.c \
	plcont.c \
	plcore.c \
	plctrl.c \
	plcvt.c \
	pldtik.c \
	plfill.c \
	plhist.c \
	plline.c \
	plmap.c \
	plot3d.c \
	plpage.c \
	plsdef.c \
	plshade.c \
	plsym.c \
	pltick.c \
	plvpor.c \
	plwind.c 

CORE_OBJ = \
	pdfutils$O \
	plargs$O \
	plbox$O \
	plcont$O \
	plcore$O \
	plctrl$O \
	plcvt$O \
	pldtik$O \
	plfill$O \
	plhist$O \
	plline$O \
	plmap$O \
	plot3d$O \
	plpage$O \
	plsdef$O \
	plshade$O \
	plsym$O \
	pltick$O \
	plvpor$O \
	plwind$O 

# Support files for font generators.

FONT_OBJ = \
	font01$O \
	font02$O \
	font03$O \
	font04$O \
	font05$O \
	font06$O \
	font07$O \
	font08$O \
	font09$O \
	font10$O \
	font11$O

# Drivers

DRIVERS_C = \
	dg300.c \
	hpgl.c \
	impress.c \
	ljiip.c \
	ljii.c \
	next.c \
	null.c \
	plbuf.c \
	plmeta.c \
	ps.c \
	tek.c \
	xfig.c \
	xwin.c

DRIVERS_OBJ = \
	dg300$O \
	hpgl$O \
	impress$O \
	ljiip$O \
	ljii$O \
	next$O \
	null$O \
	plbuf$O \
	plmeta$O \
	ps$O \
	tek$O \
	xfig$O \
	xwin$O

# Utilities

UTIL_C = \
	plrender.c \
	plserver.c \
	pltcl.c 

# howto

help:
	@echo * first, set up the right paths in the beginning of this makefile
	@echo * make a subdir (i.e. 'tmp') in within the current one ('sys\os2')
	@echo * copy this makefile in this subdir
	@echo * type 'make links' to copy necessary files
	@echo * type 'make libs'   to make plplot libraries
	@echo * type 'make install' to copy them to their destination (ignore errors)
	@echo * type 'make cdemos' to make the demo files
	@echo * see pmserv for the PM-Server for PLplot (needed by the demos)
	@echo *
	@echo * If you want to include/exclude device drivers, you have to:
	@echo * 1. edit 'libfile.tmp' and add/delete the apropiate line(s)
	@echo * 2. edit 'pmdevs.h'  to define/undefine the apropiate vars
	@echo * 3. make the WHOLE stuff mentioned above new

links:
	copy $(LIB_DIR)\\*.fnt .
	copy $(LIB_BASE)\\src\\*.c .
	copy $(LIB_BASE)\\include\\*.h .
	copy $(LIB_BASE)\\drivers\\*.c .
	copy $(LIB_BASE)\\utils\\*.* .
	copy ..\\*.c .
	copy ..\\*.h
	copy ..\\pmserv\\*.h .
	copy ..\\libfiles.tmp .
	copy $(LIB_BASE)\\examples\c\\*.c .
	copy $(LIB_BASE)\\examples\c\\*.h .


# -----------------------------------------------------------------------
# $Id: makefile 3186 2006-02-15 18:17:33Z slbrow $
#
# Mark Olesen
# olesen@Weber.QueensU.CA
# 8-Sept-1994
#
# -----------------------------------------------------------------------

# Add compiler specific driver to driver list

ADD_DRIVERS_C = os2pm.c
ADD_DRIVERS_OBJ = os2pm$O
# -----------------------------------------------------------------------
# $Id: makefile 3186 2006-02-15 18:17:33Z slbrow $
#
# Maurice LeBrun
# IFS, University of Texas at Austin
# 24-Jul-1994
#
# Initializations for building PLplot library.
# -----------------------------------------------------------------------

# Package major/minor version number, for shared libraries on some systems
# (e.g. SunOS).  Warning: apps linked to it may fail on major version
# changes (depending on OS).

MAJOR_VERSION	= 4
MINOR_VERSION	= 99

# Utilities

RANLIB		= ar s
ARLIB_BUILD	= ar q
SHLIB_BUILD	= @SHLIB_BUILD@

# Flags and other miscellanea

CC_FLAGS	= -c -O -Wall -I.
CC_FLAGS_DEMOS	= $(CC_FLAGS)
SHLIB_CCFLAGS	= @SHLIB_CCFLAGS@
SHLIB_F77FLAGS	= @SHLIB_F77FLAGS@
SHLIB_SUFFIX	= @SHLIB_SUFFIX@
SALIB_SUFFIX	= @SALIB_SUFFIX@

# Hacks to deal with optimizer failures.

CC_FLAGS_SCCONT = $(CC_FLAGS)
CC_FLAGS_XWIN	= $(CC_FLAGS)

# Dependent directories (use forward-slashes with `F')

PLLIB_DIR	= .
PLLIB_DIRF	= .
PLFNT_DIR	= .
PLFNT_DIRF	= .
PLLIB_PATH	= $(PLLIB_DIRF)/
PLFNT_PATH	= $(PLFNT_DIRF)/

# The library components

LIB_OBJS_SO	= $(CORE_OBJ) $(TCL_OBJ) $(TK_OBJ) $(DRIVERS_OBJ) $(ADD_DRIVERS_OBJ) $(FSTUB_OBJ)
LIB_OBJS_SA	= $(TCL_OBJ_SA) $(TK_OBJ_SA) $(FSTUB_OBJ_SA)
LIB_OBJS	= $(LIB_OBJS_SO) $(LIB_OBJS_SA)

# Library names

PLLIB_AR	= $(PLLIB_PATH)$(PLLIB_BASE)$(LIB_TAG)$A
PLLIB_SH	= $(PLLIB_PATH)$(PLLIB_BASE)$(LIB_TAG)$(SHLIB_SUFFIX)
PLLIB_SA	= $(PLLIB_PATH)$(PLLIB_BASE)$(LIB_TAG)$(SALIB_SUFFIX)

# This is used for making the install versions of plrender & plserver
# (Must relink after installing shared library!)

LIB_INSTALL	= -L$(LIB_DIRF) -l$(PLLIB_NAME)$(LIB_TAG)

# These are what we link against.

PLLIB_LDC	= -L$(PLLIB_DIRF) -l$(PLLIB_NAME)$(LIB_TAG)
PLLIB_LDF	= -L$(PLLIB_DIRF) -l$(PLLIB_NAME)$(LIB_TAG)

# Default is to build library(s) and utils.

everything: libs plrender$E pltek$E $(TCL_EXE) $(TK_EXE)

# Clear the suffix list to prevent problems.

.SUFFIXES:
.SUFFIXES: $O .cc .c .f .h .fm4

# Inference rules, not specific to archive or shared libraries.

.fm4.f:
	$(M4) $(M4_FLAGS) $< >$*.f

# -----------------------------------------------------------------------
# $Id: makefile 3186 2006-02-15 18:17:33Z slbrow $
#
# Maurice LeBrun
# IFS, University of Texas at Austin
# 14-Jul-1994
#
# PLplot build rule for library under Unix.
# -----------------------------------------------------------------------

# -----------------------------------------------------------------------
# Set up inference rules appropriate for building archive libraries.
# -----------------------------------------------------------------------

.c$O:
	$(CC) $(CC_FLAGS) $*.c

.f$O:
#	$(F77) $(F77_FLAGS) $*.f
	f2c -A $*.f
	$(CC) $(CC_FLAGS) $*.c
	del $*.c

# -----------------------------------------------------------------------
# Library dependency list and targets
# -----------------------------------------------------------------------

PLLIBS = $(PLLIB_AR)

libs:	$(PLLIBS)

$(PLLIB_AR):	$(LIB_OBJS)
#	-rm $(PLLIB_AR)
	@echo "Building archive library"
	$(ARLIB_BUILD) $(PLLIB_AR) $(ADD_DRIVERS_OBJ) @libfiles.tmp
	$(RANLIB) $(PLLIB_AR)

# -----------------------------------------------------------------------
# Sick hacks.
# -----------------------------------------------------------------------

# Needs to be compiled using K&R C.

#tcpip$O:	plserver.h plplotTK.h plplotX.h plplotP.h plplot.h pdf.h \
#		plstream.h tcpip.c
#	$(OCC) $(CC_FLAGS) tcpip.c

# Optimizer under AIX 3.2.3 bogues out here.

#xwin$O:		plplotP.h plplot.h drivers.h pdf.h plstream.h plplotX.h xwin.c
#	$(CC) $(CC_FLAGS_XWIN) xwin.c

# Optimizer on SX-3 makes bogus code here.

sccont$O:	plstubs.h plplotP.h plplot.h sccont.c
	$(CC) $(CC_FLAGS_SCCONT) sccont.c

# -----------------------------------------------------------------------
# $Id: makefile 3186 2006-02-15 18:17:33Z slbrow $
#
# Maurice LeBrun
# IFS, University of Texas at Austin
# 19-Jul-1994
#
# PLplot program file dependencies and targets.
# 
# Targets supported here include: 
#
#	plrender	Metafile viewer
#	plserver	Renderer for Tk/DP driver 
#	pltcl		PLplot tcl interpreter
#	plmodule$(SO)	Dynamically loaded module for PLplot python scripts
#	pltek		Renderer for tek files
#	fonts		font generators and font files
#
# All but "fonts" are in the default build rule.
# -----------------------------------------------------------------------

# Utility programs.

pltek$O:	pltek.c
	$(CC) $(CC_FLAGS) pltek.c

pltek$E:	pltek$O
	$(LDC) $(LDC_FLAGS) $(STARTUP) pltek$O $(TO) $@

# This is what the pltek build rule used to be, but it's wrong, b/c
# pltek doesn't need to link with tk, tcl, X11, etc.  Should make a
# new macro for libs this thing depends on...
#	$(LDC) $(LDC_FLAGS) $(STARTUP) pltek$O $(TO) $@ \
#		$(LDC_LIBS)

plrender$O:	$(PLLIBS) plrender.c
	$(CC) $(CC_FLAGS) plrender.c

plrender$E:	$(PLLIBS) plrender$O
	$(LDC) $(LDC_FLAGS) $(STARTUP) plrender$O $(PLLIB_LDC) $(TO) $@ \
		$(LDC_LIBS)

plserver$O:	$(PLLIBS) plserver.c
	$(CC) $(CC_FLAGS) plserver.c

plserver$E:	$(PLLIBS) plserver$O
	$(LDC) $(LDC_FLAGS) $(STARTUP) plserver$O $(PLLIB_LDC) $(TO) $@ \
		$(LDC_LIBS)

pltcl$O:	$(PLLIBS) pltcl.c
	$(CC) $(CC_FLAGS) pltcl.c

pltcl$E:	$(PLLIBS) pltcl$O
	$(LDC) $(LDC_FLAGS) $(STARTUP) pltcl$O $(PLLIB_LDC) $(TO) $@ \
		$(LDC_LIBS)

plmodule$O:	$(PLLIBS) plmodule.c
	$(CC) $(SHLIB_CCFLAGS) $(CC_FLAGS) -I$(PYTHON_INC_DIR) plmodule.c

plmodule$(SO):	$(PLLIBS) plmodule$O
	$(LDSHARED) plmodule$O $(PLLIB_LDC) @LIBS@ $(TO) $@

# -----------------------------------------------------------------------
# Font files.
# Note it is no longer necessary to actually make these since as of
# PLPLOT 4.99a the font files are portable.
#
# sfont - standard font
# xfont - extended fonts

fonts:	sfont xfont

sfont:	$(PLFNT_PATH)plstnd5.fnt
xfont:	$(PLFNT_PATH)plxtnd5.fnt

$(PLFNT_PATH)plstnd5.fnt: stndfont$E pdfutils$O $(FONT_OBJ)
	-./stndfont$E
	-mv *.fnt $(PLFNT_DIR)

$(PLFNT_PATH)plxtnd5.fnt: xtndfont$E pdfutils$O $(FONT_OBJ)
	-./xtndfont$E
	-mv *.fnt $(PLFNT_DIR)

stndfont$E:	$(PLLIBS) stndfont$O
	$(LDC) $(LDC_FLAGS) $(STARTUP) stndfont$O $(FONT_OBJ) \
		$(PLLIB_LDC) $(TO) $@ $(LDC_LIBS)

xtndfont$E:	$(PLLIBS) xtndfont$O
	$(LDC) $(LDC_FLAGS) $(STARTUP) xtndfont$O $(FONT_OBJ) \
		$(PLLIB_LDC) $(TO) $@ $(LDC_LIBS)

# -----------------------------------------------------------------------
# $Id: makefile 3186 2006-02-15 18:17:33Z slbrow $
#
# Maurice LeBrun
# IFS, University of Texas at Austin
# 22-Jul-1994
#
# PLplot demo file dependencies and targets.
# 
# Targets supported here include: 
#
#	cdemos		All the C demos
#	fdemos		All the Fortran demos
#	tkdemos		All the extended wish (TK) demos
#	x??[cf]		Demo number ??, C or fortran versions
#	xtk??		Demo number ??, extended wish versions
# -----------------------------------------------------------------------

# -----------------------------------------------------------------------
# Example programs, in C.

CDEMOS= x01c$E x02c$E x03c$E x04c$E x05c$E x06c$E x07c$E x08c$E x09c$E \
	x10c$E x11c$E x12c$E x13c$E x14c$E x15c$E x16c$E x17c$E x18c$E \
	x19c$E tutor$E

cdemos:	$(CDEMOS)

x01c$O:	$(CDEMO_HEADERS) x01c.c
	$(CC) $(CC_FLAGS_DEMOS) x01c.c

x01c$E:	$(PLLIBS) x01c$O
	$(LDC) $(LDC_FLAGS) $(STARTUP) x01c$O \
	       $(PLLIB_LDC) $(TO) $@ $(LDC_LIBS)

x02c$O:	$(CDEMO_HEADERS) x02c.c
	$(CC) $(CC_FLAGS_DEMOS) x02c.c

x02c$E:	$(PLLIBS) x02c$O
	$(LDC) $(LDC_FLAGS) $(STARTUP) x02c$O \
	       $(PLLIB_LDC) $(TO) $@ $(LDC_LIBS)

x03c$O:	$(CDEMO_HEADERS) x03c.c
	$(CC) $(CC_FLAGS_DEMOS) x03c.c

x03c$E:	$(PLLIBS) x03c$O
	$(LDC) $(LDC_FLAGS) $(STARTUP) x03c$O \
	       $(PLLIB_LDC) $(TO) $@ $(LDC_LIBS)

x04c$O:	$(CDEMO_HEADERS) x04c.c
	$(CC) $(CC_FLAGS_DEMOS) x04c.c

x04c$E:	$(PLLIBS) x04c$O
	$(LDC) $(LDC_FLAGS) $(STARTUP) x04c$O \
	       $(PLLIB_LDC) $(TO) $@ $(LDC_LIBS)

x05c$O:	$(CDEMO_HEADERS) x05c.c
	$(CC) $(CC_FLAGS_DEMOS) x05c.c

x05c$E:	$(PLLIBS) x05c$O
	$(LDC) $(LDC_FLAGS) $(STARTUP) x05c$O \
	       $(PLLIB_LDC) $(TO) $@ $(LDC_LIBS)

x06c$O:	$(CDEMO_HEADERS) x06c.c
	$(CC) $(CC_FLAGS_DEMOS) x06c.c

x06c$E:	$(PLLIBS) x06c$O
	$(LDC) $(LDC_FLAGS) $(STARTUP) x06c$O \
	       $(PLLIB_LDC) $(TO) $@ $(LDC_LIBS)

x07c$O:	$(CDEMO_HEADERS) x07c.c
	$(CC) $(CC_FLAGS_DEMOS) x07c.c

x07c$E:	$(PLLIBS) x07c$O
	$(LDC) $(LDC_FLAGS) $(STARTUP) x07c$O \
	       $(PLLIB_LDC) $(TO) $@ $(LDC_LIBS)

x08c$O:	$(CDEMO_HEADERS) x08c.c
	$(CC) $(CC_FLAGS_DEMOS) x08c.c

x08c$E:	$(PLLIBS) x08c$O
	$(LDC) $(LDC_FLAGS) $(STARTUP) x08c$O \
	       $(PLLIB_LDC) $(TO) $@ $(LDC_LIBS)

x09c$O:	$(CDEMO_HEADERS) x09c.c
	$(CC) $(CC_FLAGS_DEMOS) x09c.c

x09c$E:	$(PLLIBS) x09c$O
	$(LDC) $(LDC_FLAGS) $(STARTUP) x09c$O \
	       $(PLLIB_LDC) $(TO) $@ $(LDC_LIBS)

x10c$O:	$(CDEMO_HEADERS) x10c.c
	$(CC) $(CC_FLAGS_DEMOS) x10c.c

x10c$E:	$(PLLIBS) x10c$O
	$(LDC) $(LDC_FLAGS) $(STARTUP) x10c$O \
	       $(PLLIB_LDC) $(TO) $@ $(LDC_LIBS)

x11c$O:	$(CDEMO_HEADERS) x11c.c
	$(CC) $(CC_FLAGS_DEMOS) x11c.c

x11c$E:	$(PLLIBS) x11c$O
	$(LDC) $(LDC_FLAGS) $(STARTUP) x11c$O \
	       $(PLLIB_LDC) $(TO) $@ $(LDC_LIBS)

x12c$O:	$(CDEMO_HEADERS) x12c.c
	$(CC) $(CC_FLAGS_DEMOS) x12c.c

x12c$E:	$(PLLIBS) x12c$O
	$(LDC) $(LDC_FLAGS) $(STARTUP) x12c$O \
	       $(PLLIB_LDC) $(TO) $@ $(LDC_LIBS)

x13c$O:	$(CDEMO_HEADERS) x13c.c
	$(CC) $(CC_FLAGS_DEMOS) x13c.c

x13c$E:	$(PLLIBS) x13c$O
	$(LDC) $(LDC_FLAGS) $(STARTUP) x13c$O \
	       $(PLLIB_LDC) $(TO) $@ $(LDC_LIBS)

x14c$O:	$(CDEMO_HEADERS) x14c.c
	$(CC) $(CC_FLAGS_DEMOS) x14c.c

x14c$E:	$(PLLIBS) x14c$O
	$(LDC) $(LDC_FLAGS) $(STARTUP) x14c$O \
	       $(PLLIB_LDC) $(TO) $@ $(LDC_LIBS)

x15c$O:	$(CDEMO_HEADERS) x15c.c
	$(CC) $(CC_FLAGS_DEMOS) x15c.c

x15c$E:	$(PLLIBS) x15c$O
	$(LDC) $(LDC_FLAGS) $(STARTUP) x15c$O \
	       $(PLLIB_LDC) $(TO) $@ $(LDC_LIBS)

x16c$O:	$(CDEMO_HEADERS) x16c.c
	$(CC) $(CC_FLAGS_DEMOS) x16c.c

x16c$E:	$(PLLIBS) x16c$O
	$(LDC) $(LDC_FLAGS) $(STARTUP) x16c$O \
	       $(PLLIB_LDC) $(TO) $@ $(LDC_LIBS)

x17c$O:	$(CDEMO_HEADERS) x17c.c
	$(CC) $(CC_FLAGS_DEMOS) x17c.c

x17c$E:	$(PLLIBS) x17c$O
	$(LDC) $(LDC_FLAGS) $(STARTUP) x17c$O \
	       $(PLLIB_LDC) $(TO) $@ $(LDC_LIBS)

x18c$O:	$(CDEMO_HEADERS) x18c.c
	$(CC) $(CC_FLAGS_DEMOS) x18c.c

x18c$E:	$(PLLIBS) x18c$O
	$(LDC) $(LDC_FLAGS) $(STARTUP) x18c$O \
	       $(PLLIB_LDC) $(TO) $@ $(LDC_LIBS)

x19c$O:	$(CDEMO_HEADERS) x19c.c
	$(CC) $(CC_FLAGS_DEMOS) x19c.c

x19c$E:	$(PLLIBS) x19c$O
	$(LDC) $(LDC_FLAGS) $(STARTUP) x19c$O \
	       $(PLLIB_LDC) $(TO) $@ $(LDC_LIBS)

tutor$O:	$(CDEMO_HEADERS) tutor.c
	$(CC) $(CC_FLAGS_DEMOS) tutor.c

tutor$E:	$(PLLIBS) tutor$O
	$(LDC) $(LDC_FLAGS) $(STARTUP) tutor$O \
	       $(PLLIB_LDC) $(TO) $@ $(LDC_LIBS)

bugc$O:	$(CDEMO_HEADERS) bugc.c
	$(CC) $(CC_FLAGS_DEMOS) bugc.c

bugc$E:	$(PLLIBS) bugc$O
	$(LDC) $(LDC_FLAGS) $(STARTUP) bugc$O \
	       $(PLLIB_LDC) $(TO) $@ $(LDC_LIBS)

# -----------------------------------------------------------------------
# Example programs, in Fortran.

FDEMOS= x01f$E x02f$E x03f$E x04f$E x05f$E x06f$E x07f$E x08f$E x09f$E \
	x10f$E x11f$E x12f$E x13f$E x16f$E

fdemos:	$(FDEMOS)

x01f$O:	x01f.f
	$(F77) $(F77_FLAGS) x01f.f

x01f$E:	$(PLLIBS) x01f$O
	$(LDF) $(LDF_FLAGS) x01f$O \
	       $(PLLIB_LDF) $(TO) $@ $(LDF_PATH) $(LDF_LIBS)

x02f$O:	x02f.f
	$(F77) $(F77_FLAGS) x02f.f

x02f$E:	$(PLLIBS) x02f$O
	$(LDF) $(LDF_FLAGS) x02f$O \
	       $(PLLIB_LDF) $(TO) $@ $(LDF_PATH) $(LDF_LIBS)

x03f$O:	x03f.f
	$(F77) $(F77_FLAGS) x03f.f

x03f$E:	$(PLLIBS) x03f$O
	$(LDF) $(LDF_FLAGS) x03f$O \
	       $(PLLIB_LDF) $(TO) $@ $(LDF_PATH) $(LDF_LIBS)

x04f$O:	x04f.f
	$(F77) $(F77_FLAGS) x04f.f

x04f$E:	$(PLLIBS) x04f$O
	$(LDF) $(LDF_FLAGS) x04f$O \
	       $(PLLIB_LDF) $(TO) $@ $(LDF_PATH) $(LDF_LIBS)

x05f$O:	x05f.f
	$(F77) $(F77_FLAGS) x05f.f

x05f$E:	$(PLLIBS) x05f$O
	$(LDF) $(LDF_FLAGS) x05f$O \
	       $(PLLIB_LDF) $(TO) $@ $(LDF_PATH) $(LDF_LIBS)

x06f$O:	x06f.f
	$(F77) $(F77_FLAGS) x06f.f

x06f$E:	$(PLLIBS) x06f$O
	$(LDF) $(LDF_FLAGS) x06f$O \
	       $(PLLIB_LDF) $(TO) $@ $(LDF_PATH) $(LDF_LIBS)

x07f$O:	x07f.f
	$(F77) $(F77_FLAGS) x07f.f

x07f$E:	$(PLLIBS) x07f$O
	$(LDF) $(LDF_FLAGS) x07f$O \
	       $(PLLIB_LDF) $(TO) $@ $(LDF_PATH) $(LDF_LIBS)

x08f$O:	x08f.f
	$(F77) $(F77_FLAGS) x08f.f

x08f$E:	$(PLLIBS) x08f$O
	$(LDF) $(LDF_FLAGS) x08f$O \
	       $(PLLIB_LDF) $(TO) $@ $(LDF_PATH) $(LDF_LIBS)

x09f$O:	x09f.f
	$(F77) $(F77_FLAGS) x09f.f

x09f$E:	$(PLLIBS) x09f$O
	$(LDF) $(LDF_FLAGS) x09f$O \
	       $(PLLIB_LDF) $(TO) $@ $(LDF_PATH) $(LDF_LIBS)

x10f$O:	x10f.f
	$(F77) $(F77_FLAGS) x10f.f

x10f$E:	$(PLLIBS) x10f$O
	$(LDF) $(LDF_FLAGS) x10f$O \
	       $(PLLIB_LDF) $(TO) $@ $(LDF_PATH) $(LDF_LIBS)

x11f$O:	x11f.f
	$(F77) $(F77_FLAGS) x11f.f

x11f$E:	$(PLLIBS) x11f$O
	$(LDF) $(LDF_FLAGS) x11f$O \
	       $(PLLIB_LDF) $(TO) $@ $(LDF_PATH) $(LDF_LIBS)

x12f$O:	x12f.f
	$(F77) $(F77_FLAGS) x12f.f

x12f$E:	$(PLLIBS) x12f$O
	$(LDF) $(LDF_FLAGS) x12f$O \
	       $(PLLIB_LDF) $(TO) $@ $(LDF_PATH) $(LDF_LIBS)

x13f$O:	x13f.f
	$(F77) $(F77_FLAGS) x13f.f

x13f$E:	$(PLLIBS) x13f$O
	$(LDF) $(LDF_FLAGS) x13f$O \
	       $(PLLIB_LDF) $(TO) $@ $(LDF_PATH) $(LDF_LIBS)

x16f$O:	x16f.f
	$(F77) $(F77_FLAGS) x16f.f

x16f$E:	$(PLLIBS) x16f$O
	$(LDF) $(LDF_FLAGS) x16f$O \
	       $(PLLIB_LDF) $(TO) $@ $(LDF_PATH) $(LDF_LIBS)

bugf$O:	bugf.f
	$(F77) $(F77_FLAGS) bugf.f

bugf$E:	$(PLLIBS) bugf$O
	$(LDF) $(LDF_FLAGS) bugf$O \
	       $(PLLIB_LDF) $(TO) $@ $(LDF_PATH) $(LDF_LIBS)

# -----------------------------------------------------------------------
# TK example programs.

TKDEMOS = xtk01$E xtk02$E xtk04$E

tkdemos: $(TKDEMOS)

xtk01$O:	$(CDEMO_HEADERS) xtk01.c
	$(CC) $(CC_FLAGS_DEMOS) xtk01.c

xtk01$E:  $(PLLIBS) xtk01$O
	$(LDC) $(LDC_FLAGS) $(STARTUP) xtk01$O \
	       $(PLLIB_LDC) $(TO) $@ $(LDC_LIBS)

xtk02$O:	$(CDEMO_HEADERS) xtk02.c
	$(CC) $(CC_FLAGS_DEMOS) xtk02.c

xtk02$E:  $(PLLIBS) xtk02$O
	$(LDC) $(LDC_FLAGS) $(STARTUP) xtk02$O \
	       $(PLLIB_LDC) $(TO) $@ $(LDC_LIBS)

xtk04$O:	$(CDEMO_HEADERS) xtk04.c
	$(CC) $(CC_FLAGS_DEMOS) xtk04.c

xtk04$E:  $(PLLIBS) xtk04$O
	$(LDC) $(LDC_FLAGS) $(STARTUP) xtk04$O \
	       $(PLLIB_LDC) $(TO) $@ $(LDC_LIBS)

# -----------------------------------------------------------------------
# C++ example programs.

CXXDEMOS = x01cc$E

cxxdemos: $(CXXDEMOS)

x01cc$O: x01cc.cc
	$(CXX) $(CXX_FLAGS_DEMOS) x01cc.cc	

x01cc$E:  $(PLLIBS) x01cc$O
	$(LDCXX) $(LDCXX_FLAGS) $(STARTUP) x01cc$O \
	       	$(PLLIB_LDC) $(TO) $@ $(LDCXX_LIBS)
# -----------------------------------------------------------------------
# $Id: makefile 3186 2006-02-15 18:17:33Z slbrow $
#
# Maurice LeBrun
# IFS, University of Texas at Austin
# 14-Jul-1994
#
# Make assignments and targets for installation under Unix.
#
# It is recommended that you use a dedicated directory such as
# /usr/local/plplot (read the FAQ on this topic).  The install directory
# will be created if necessary, as will subdirectories bin, doc, examples,
# include, lib, man (eventually), and tcl.
#
# Notes: 
#	if Tk support is not included, TK_INSTALL will be null.
#	if Tcl support is not included, TCL_INSTALL will be null.
#
# -----------------------------------------------------------------------

install: install_lib install_utils install_demos

install_lib:
	-if not exist $(INCLUDE_DIR) mkdir $(INCLUDE_DIR)
	-if not exist $(LIB_DIR) mkdir $(LIB_DIR)
	-if not exist $(DOC_DIR) mkdir $(DOC_DIR)
	-if not exist $(BIN_DIR) mkdir $(BIN_DIR)
	-if not exist $(INFO_DIR) mkdir $(INFO_DIR)
	-copy $(PLLIB_DIR)\$(PLLIB_BASE)*$A $(LIB_DIR)
	-copy $(PLFNT_DIR)\*.fnt $(LIB_DIR)
	-echo $(LIB_DIR)
	-echo $(LIB_DIR)\*$A
	-$(RANLIB) $(LIB_DIRF)/$(PLLIB_BASE)*$A
#	-cd ..\..\..\..
#	-for %i in (README README.1st Changes.log COPYRIGHTS FAQ ToDo mklinks) do copy %i $(DOC_DIR)
	-copy ..\..\..\..\README	$(DOC_DIR)
	-copy ..\..\..\..\README.1st	$(DOC_DIR)
	-copy ..\..\..\..\Changes.log	$(DOC_DIR)
	-copy ..\..\..\..\COPYRIGHTS	$(DOC_DIR)
	-copy ..\..\..\..\FAQ		$(DOC_DIR)
	-copy ..\..\..\..\ToDo		$(DOC_DIR)
	-copy ..\..\..\..\mklinks	$(DOC_DIR)
	-if not exist $(DOC_DIR)\README.local do copy README.local $(DOC_DIR)
	-copy ..\..\..\..\doc\*.info $(INFO_DIR)
	-copy ..\..\..\..\scripts\pl*.* $(BIN_DIR)
#	-cd include
#	-for %i in (plplotP.h plplot.h plplotX.h plplotTK.h plevent.h plstream.h pdf.h tclMatrix.h) do copy %i $(INCLUDE_DIR)
	copy plplotP.h		$(INCLUDE_DIR)
	copy plplot.h		$(INCLUDE_DIR)
	copy plplotX.h		$(INCLUDE_DIR)
	copy plplotTK.h		$(INCLUDE_DIR)
	copy plevent.h		$(INCLUDE_DIR)
	copy plstream.h		$(INCLUDE_DIR)
	copy pdf.h		$(INCLUDE_DIR)
	copy tclMatrix.h	$(INCLUDE_DIR)

install_demos:
	-if not exist $(DEMOS_DIR) mkdir $(DEMOS_DIR)
	-copy Makedemo $(DEMOS_DIR)\Makefile
# C examples	
	-if not exist $(DEMOS_DIR)\C mkdir $(DEMOS_DIR)\C
	-copy $(DEMOS_DIR)\Makefile $(DEMOS_DIR)\C
	-copy ..\..\..\..\examples\C\*.* $(DEMOS_DIR)\C
# f77 examples
#	-if not exist $(DEMOS_DIR)\f77 mkdir $(DEMOS_DIR)\f77
#	-copy $(DEMOS_DIR)\Makefile $(DEMOS_DIR)\f77
#	-copy ..\..\..\..\examples\f77\*.* $(DEMOS_DIR)\f77

install_utils:
	$(LDC) $(LDC_FLAGS) plrender$O $(LIB_INSTALL) \
		-o plrender$E $(LDC_LIBS)
	$(LDC) $(LDC_FLAGS) pltek$O $(LIB_INSTALL) \
		-o pltek$E $(LDC_LIBS)
	-$(BIND) plrender$E $(BIND_OPTS)
	-$(STRIP) plrender$E
	-$(BIND) pltek$E $(BIND_OPTS)
	-$(STRIP) pltek$E
	-copy plrender$E $(BIN_DIR)
	-copy pltek$E $(BIN_DIR)
# -----------------------------------------------------------------------
# $Id: makefile 3186 2006-02-15 18:17:33Z slbrow $
#
# Maurice LeBrun
# IFS, University of Texas at Austin
# 22-Jul-1994
#
# Miscellaneous targets.
# -----------------------------------------------------------------------

# Cleanup

clean:
#	-rm * $(PLLIB_PATH)$(PLLIB_BASE)*

# re-bind executables with emx options (ie, Dangerous DOS options)

bind:	bind_utils

bind_utils:
	-$(BIND) plrender$E $(BIND_OPTS)
	-$(BIND) pltek$E $(BIND_OPTS)

# Autogeneration of dependencies.  Use gcc -MM because X11's "makedepend"
# includes system headers, and that really sucks.

depend:
	gcc -MM $(CORE_C) $(DRIVERS_C) $(ADD_DRIVERS_C) \
	$(UTIL_C) $(FSTUB_C) $(TCL_C) $(TK_C) \
	| ./cvtdeps 
	@echo "Rerun configure for the new dependencies to take effect"

# -----------------------------------------------------------------------
# Auto-generated dependency list follows (warning: it's ugly).

pdfutils$O: pdfutils.c plplotP.h plConfig.h plplot.h plstrm.h pdf.h pldebug.h
plargs$O: plargs.c plplotP.h plConfig.h plplot.h plstrm.h pdf.h pldebug.h
plbox$O: plbox.c plplotP.h plConfig.h plplot.h plstrm.h pdf.h pldebug.h
plcont$O: plcont.c plplotP.h plConfig.h plplot.h plstrm.h pdf.h pldebug.h
plcore$O: plcore.c plcore.h plplotP.h plConfig.h plplot.h plstrm.h pdf.h \
  drivers.h plDevs.h pldebug.h
plctrl$O: plctrl.c plplotP.h plConfig.h plplot.h plstrm.h pdf.h pldebug.h
plcvt$O: plcvt.c plplotP.h plConfig.h plplot.h plstrm.h pdf.h pldebug.h
pldtik$O: pldtik.c plplotP.h plConfig.h plplot.h plstrm.h pdf.h pldebug.h
plfill$O: plfill.c plplotP.h plConfig.h plplot.h plstrm.h pdf.h pldebug.h
plhist$O: plhist.c plplotP.h plConfig.h plplot.h plstrm.h pdf.h pldebug.h
plline$O: plline.c plplotP.h plConfig.h plplot.h plstrm.h pdf.h pldebug.h
plmap$O: plmap.c plplotP.h plConfig.h plplot.h plstrm.h pdf.h pldebug.h
plot3d$O: plot3d.c plplotP.h plConfig.h plplot.h plstrm.h pdf.h pldebug.h
plpage$O: plpage.c plplotP.h plConfig.h plplot.h plstrm.h pdf.h pldebug.h
plsdef$O: plsdef.c plplotP.h plConfig.h plplot.h plstrm.h pdf.h pldebug.h
plshade$O: plshade.c plplotP.h plConfig.h plplot.h plstrm.h pdf.h pldebug.h
plsym$O: plsym.c plplotP.h plConfig.h plplot.h plstrm.h pdf.h pldebug.h
pltick$O: pltick.c plplotP.h plConfig.h plplot.h plstrm.h pdf.h pldebug.h
plvpor$O: plvpor.c plplotP.h plConfig.h plplot.h plstrm.h pdf.h pldebug.h
plwind$O: plwind.c plplotP.h plConfig.h plplot.h plstrm.h pdf.h pldebug.h
dg300$O: dg300.c plDevs.h
hpgl$O: hpgl.c plDevs.h plplotP.h plConfig.h plplot.h plstrm.h pdf.h pldebug.h \
  drivers.h
impress$O: impress.c plDevs.h
ljiip$O: ljiip.c plDevs.h plplotP.h plConfig.h plplot.h plstrm.h pdf.h pldebug.h \
  drivers.h
ljii$O: ljii.c plDevs.h plplotP.h plConfig.h plplot.h plstrm.h pdf.h pldebug.h \
  drivers.h
ps$(): ps.c plDevs.h plplotP.h plConfig.h plplot.h plstrm.h pdf.h pldebug.h \
  drivers.h
xfig$(): xfig.c plDevs.h plplotP.h plConfig.h plplot.h plstrm.h pdf.h pldebug.h \
  drivers.h
next$O: next.c plDevs.h
null$O: null.c plDevs.h plplotP.h plConfig.h plplot.h plstrm.h pdf.h pldebug.h \
  drivers.h
plbuf$O: plbuf.c plplotP.h plConfig.h plplot.h plstrm.h pdf.h pldebug.h drivers.h \
  metadefs.h
plmeta$O: plmeta.c plDevs.h plplotP.h plConfig.h plplot.h plstrm.h pdf.h \
  pldebug.h drivers.h metadefs.h
ps$O: ps.c plDevs.h plplotP.h plConfig.h plplot.h plstrm.h pdf.h pldebug.h \
  drivers.h
tek$O: tek.c plDevs.h plplotP.h plConfig.h plplot.h plstrm.h pdf.h pldebug.h \
  drivers.h plevent.h
xfig$O: xfig.c plDevs.h plplotP.h plConfig.h plplot.h plstrm.h pdf.h pldebug.h \
  drivers.h
xwin$O: xwin.c plDevs.h plplotP.h plConfig.h plplot.h plstrm.h pdf.h pldebug.h \
  plxwd.h drivers.h plevent.h
plrender$O: plrender.c plplotP.h plConfig.h plplot.h plstrm.h pdf.h pldebug.h \
  plevent.h metadefs.h
plserver$O: plserver.c plserver.h plDevs.h pltkd.h plplotP.h plConfig.h plplot.h \
  plstrm.h pdf.h pldebug.h pltk.h pltcl.h tclMatrix.h
pltcl$O: pltcl.c plplotP.h plConfig.h plplot.h plstrm.h pdf.h pldebug.h pltcl.h \
  tclMatrix.h
sc3d$O: sc3d.c plstubs.h plplotP.h plConfig.h plplot.h plstrm.h pdf.h pldebug.h
sccont$O: sccont.c plstubs.h plplotP.h plConfig.h plplot.h plstrm.h pdf.h \
  pldebug.h
scstubs$O: scstubs.c plstubs.h plplotP.h plConfig.h plplot.h plstrm.h pdf.h \
  pldebug.h
tclAPI$O: tclAPI.c plplotP.h plConfig.h plplot.h plstrm.h pdf.h pldebug.h pltcl.h \
  tclMatrix.h
tclMain$O: tclMain.c
tclMatrix$O: tclMatrix.c tclMatrix.h
tk$O: tk.c plDevs.h pltkd.h plplotP.h plConfig.h plplot.h plstrm.h pdf.h pldebug.h \
  pltk.h pltcl.h tclMatrix.h plxwd.h tcpip.h drivers.h metadefs.h plevent.h
tkshell$O: tkshell.c plserver.h plDevs.h pltkd.h plplotP.h plConfig.h plplot.h \
  plstrm.h pdf.h pldebug.h pltk.h pltcl.h tclMatrix.h
tcpip$O: tcpip.c plDevs.h plConfig.h tcpip.h pdf.h
plframe$O: plframe.c plserver.h plDevs.h pltkd.h plplotP.h plConfig.h plplot.h \
  plstrm.h pdf.h pldebug.h pltk.h pltcl.h tclMatrix.h plxwd.h tcpip.h
tkMain$O: tkMain.c
plr$O: plr.c plserver.h plDevs.h pltkd.h plplotP.h plConfig.h plplot.h plstrm.h \
  pdf.h pldebug.h pltk.h pltcl.h tclMatrix.h plevent.h metadefs.h
