## Process this file with automake to produce Makefile.in.

## Define the library name.
lib_LTLIBRARIES = libDGraphics.la

## Define the sources the library depends on.
libDGraphics_la_SOURCES = Graph2D.C \
			  Graphics.C \
			  menu.c \
			  Widgets/Board.c \
			  Widgets/choosecol.c \
			  Widgets/Common.c \
			  Widgets/Frame.c \
			  Widgets/icon.c \
			  Widgets/long.c \
			  Widgets/Pen.c \
			  Widgets/RowCol.c \
			  Widgets/SciPlot.c \
			  Widgets/scroll.c \
			  Widgets/str2color.c \
			  Widgets/strarray.c \
			  Widgets/StrToPmap.c \
			  Widgets/ThWheel.c \
			  Widgets/ToggleBplus.c \
			  Widgets/XCC.c \
			  Widgets/Board.h \
			  Widgets/BoardP.h \
			  Widgets/choosecol.h \
			  Widgets/Common.h \
			  Widgets/CommonP.h \
			  Widgets/Converters.h \
			  Widgets/Frame.h \
			  Widgets/FrameP.h \
			  Widgets/icon.h \
			  Widgets/long.h \
			  Widgets/Pen.h \
			  Widgets/RowCol.h \
			  Widgets/RowColP.h \
			  Widgets/SciPlot.h \
			  Widgets/SciPlotP.h \
			  Widgets/scroll.h \
			  Widgets/str2color.h \
			  Widgets/strarray.h \
			  Widgets/StrToPmap.h \
			  Widgets/ThWheel.h \
			  Widgets/ThWheelP.h \
			  Widgets/ToggleBplus.h \
			  Widgets/wheels.h \
			  Widgets/XCC.h \
			  Widgets/XCCP.h

## We need the X11 headers too.
libDGraphics_la_CPPFLAGS = $(XPM_CFLAGS) $(X_CFLAGS)

## Additional places to find headers.
INCLUDES = -I$(top_srcdir)/include -I$(srcdir)/Widgets

## These files should only be removed on 'make maintainer-clean'.
MAINTAINERCLEANFILES = Makefile.in

## Extra files to be included in the distribution.
EXTRA_DIST = Widgets/darkstipple.bm \
	     Widgets/lightstipple.bm \
	     Widgets/wheel0.pm \
	     Widgets/wheel1.pm \
	     Widgets/wheel2.pm \
	     Widgets/wheel3.pm \
	     Widgets/wheel0h.pm \
	     Widgets/wheel1h.pm \
	     Widgets/wheel2h.pm \
	     Widgets/wheel3h.pm \
	     Widgets/SciPlotDocs/SciPlotDemo.gif \
	     Widgets/SciPlotDocs/SciPlotExample.gif \
	     Widgets/SciPlotDocs/SciPlotTitle.gif \
	     Widgets/SciPlotDocs/new.gif \
	     Widgets/SciPlotDocs/SciPlot.html \
	     Widgets/SciPlotDocs/SciPlotDemo.html \
	     Widgets/SciPlotDocs/SciPlotProg.html \
	     Widgets/XPM/diamond0.pm \
	     Widgets/XPM/diamond1.pm \
	     Widgets/XPM/ERROR.pm \
	     Widgets/XPM/FATAL.pm \
	     Widgets/XPM/INFO.pm \
	     Widgets/XPM/QUESTION.pm \
	     Widgets/XPM/square0.pm \
	     Widgets/XPM/square1.pm \
	     Widgets/XPM/WARNING.pm
