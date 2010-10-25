# drivers/Makefile.am for PLplot
###
### Process this file with automake to produce Makefile.in
###
# Copyright (C) 2002, 2003, 2004  Rafael Laboissiere
# Copyright (C) 2002, 2003, 2004  Alan W. Irwin
# Copyright (C) 2003  Joao Cardoso
# Copyright (C) 2004  Andrew Roach
# Copyright (C) 2005  Thomas J. Duck
#
#
# This file is part of PLplot.
#
# PLplot file is free software; you can redistribute it and/or modify
# it under the terms of the GNU Library General Public License as published
# by the Free Software Foundation; version 2 of the License.
#
# PLplot is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public License
# along with PLplot; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

lmflag = -lm

if enable_dyndrivers

EXTRA_LTLIBRARIES = \
	aqt.la \
	cgm.la \
	dg300.la \
	gd.la \
	gnome.la \
	hpgl.la \
	impress.la \
	libgcw.la \
	linuxvga.la \
	ljii.la \
	ljiip.la \
	mem.la \
	ntk.la \
	null.la \
	pbm.la \
	plmeta.la \
	ps.la \
	pstex.la \
	tek.la \
	tk.la \
	tkwin.la \
	wingcc.la \
	xfig.la \
	xwin.la

driversdir = $(DRV_DIR)
drivers_LTLIBRARIES = $(DYNAMIC_DRIVERS)

libplplot = $(top_builddir)/src/libplplot$(LIB_TAG).la
libplplottcltk = $(top_builddir)/bindings/tcl/libplplottcltk$(LIB_TAG).la
libtclmatrix = $(top_builddir)/bindings/tcl/libtclmatrix$(LIB_TAG).la

# The variable below will force building of the dyndrivers info files
# (<driver>.rc) and their installation in the drivers directory

drivers_DATA = $(drivers_LTLIBRARIES:.la=.rc)

# In the event that drivers_DATA is being created, don't allow parallel build
.NOTPARALLEL:

# get-drv-info is a program that, at build time, will generate the driver's
# info files <driver>.rc

noinst_PROGRAMS = get-drv-info
get_drv_info_SOURCES = get-drv-info.c
get_drv_info_LDADD = $(LIBLTDL) $(libplplot)
if with_csa
get_drv_info_LDADD += $(top_builddir)/lib/csa/libcsirocsa.la
endif
if with_qhull
get_drv_info_LDADD += $(top_builddir)/lib/nn/libcsironn.la
endif
# add the following directories if either tk or tkwin devices are enabled.
if enable_tk
get_drv_info_LDADD += $(libplplottcltk) $(libtclmatrix)
else
if enable_tkwin
get_drv_info_LDADD += $(libplplottcltk) $(libtclmatrix)
endif
endif

get_drv_info_CFLAGS = $(INCLTDL)

SUFFIXES = .la .rc
.la.rc: get-drv-info
	./get-drv-info `echo $< | sed 's/.la//'` > $@

# All -I preprocessor flags apparently must be lumped together for all source
# in this directory.  Some day when automake grows up, convert these
# to xwin_la_AM_CPPFLAGS, etc.  For now, use individual CFLAGS instead
# which strictly speaking is a misuse since CFLAGS are supposed to be
# processor options, not preprocessor.

cgm_la_CFLAGS = $(CDINCCMD)
gd_la_CFLAGS = $(GDINCCMD)
aqt_la_CFLAGS = $(AQTINCCMD)
gnome_la_CFLAGS = $(GTK_CFLAGS) $(GNOME_INCLUDEDIR)
libgcw_la_CFLAGS = $(GCW_CFLAGS)
ntk_la_CFLAGS = \
	-I$(top_srcdir)/bindings/tcl \
	-I$(top_builddir)/bindings/tcl \
	-I$(top_srcdir)/bindings/tk \
	$(TCLINCCMD) \
	$(TKINCCMD)

tk_la_CFLAGS = \
	-I$(top_srcdir)/bindings/tcl \
	-I$(top_builddir)/bindings/tcl \
	-I$(top_srcdir)/bindings/tk \
	$(TCLINCCMD) \
	$(TKINCCMD)

tkwin_la_CFLAGS = \
	-I$(top_srcdir)/bindings/tcl \
	-I$(top_builddir)/bindings/tcl \
	-I$(top_srcdir)/bindings/tk \
	-I$(top_srcdir)/bindings/tk-x-plat \
	$(TCLINCCMD) \
	$(TKINCCMD)

wingcc_la_CFLAGS = $(GDI32INCCMD)

xwin_la_CFLAGS = $(X_CFLAGS)

# Set overall preprocessor options
AM_CPPFLAGS = -I$(top_srcdir)/include $(FREETYPEINCCMD) $(INCLTDL)

ldflags = -rpath $(driversdir) -no-undefined -module -avoid-version \
          $(FREETYPELIBCMD)

# Single precision versions of the dynamic drivers.
# The LDFLAGS variables have been set very carefully for each driver
# to insure all their undefined symbols are resolved by the link step

cgm_la_SOURCES = cgm.c
cgm_la_LDFLAGS = $(ldflags) $(libplplot) $(CDLIBCMD)

dg300_la_SOURCES = dg300.c
dg300_la_LDFLAGS = $(ldflags) $(libplplot)

gd_la_SOURCES = gd.c
gd_la_LDFLAGS = $(ldflags) $(libplplot) $(GDLIBCMD)

aqt_la_SOURCES = aqt.c
aqt_la_LDFLAGS = $(ldflags) $(libplplot) $(AQTLIBCMD)

wingcc_la_SOURCES = wingcc.c
wingcc_la_LDFLAGS = $(ldflags) $(libplplot) $(GDI32LIBCMD)

if enable_gnome
gnome_la_SOURCES = gnome.c
gnome_la_LDFLAGS = $(ldflags) $(libplplot) $(GNOMELIBS) $(lmflag)
endif

hpgl_la_SOURCES = hpgl.c
hpgl_la_LDFLAGS = $(ldflags) $(libplplot)

impress_la_SOURCES = impress.c
impress_la_LDFLAGS = $(ldflags) $(libplplot)

if enable_gcw
libgcw_la_SOURCES = gcw.c plplotcanvas.c plplotcanvas-hacktext.c
libgcw_la_LDFLAGS = $(ldflags) $(libplplot) $(GCW_LIBS) $(lmflag)
endif

linuxvga_la_SOURCES = linuxvga.c
linuxvga_la_LDFLAGS = $(ldflags) $(libplplot) $(LINUXVGALIBS)

ljii_la_SOURCES = ljii.c
ljii_la_LDFLAGS = $(ldflags) $(libplplot) $(lmflag)

ljiip_la_SOURCES = ljiip.c
ljiip_la_LDFLAGS = $(ldflags) $(libplplot) $(lmflag)

mem_la_SOURCES = mem.c
mem_la_LDFLAGS = $(ldflags) $(libplplot)

## ntk.c has direct calls to Tcl and Tk library functions but not X libraries.
## However, specify X and math libraries as well since libtk depends on them.

ntk_la_SOURCES = ntk.c
ntk_la_LDFLAGS = $(ldflags) $(libplplot) $(TKLIBCMD) $(TCLLIBCMD) \
	       $(X_LIBS) -lX11 -lm

null_la_SOURCES = null.c
null_la_LDFLAGS = $(ldflags) $(libplplot)

pbm_la_SOURCES = pbm.c
pbm_la_LDFLAGS = $(ldflags) $(libplplot) $(lmflag)

plmeta_la_SOURCES = plmeta.c
plmeta_la_LDFLAGS = $(ldflags) $(libplplot)

ps_la_SOURCES = ps.c
ps_la_LDFLAGS = $(ldflags) $(libplplot) $(lmflag)

pstex_la_SOURCES = ps.c pstex.c
pstex_la_LDFLAGS = $(ldflags) $(libplplot) $(lmflag)

tek_la_SOURCES = tek.c
tek_la_LDFLAGS = $(ldflags) $(libplplot)

## tk.c has direct calls to libplplot, libplplottcltk,
## Tcl and Tk library functions but not X libraries.
## However, specify X and math libraries as well since libtk depends on them.
tk_la_SOURCES = tk.c
tk_la_LDFLAGS = $(ldflags) $(libplplot) $(libplplottcltk) \
	      $(TKLIBCMD) $(TCLLIBCMD) $(X_LIBS) -lX11 -lm

tkwin_la_SOURCES = \
	tkwin.c \
	$(top_srcdir)/bindings/tk-x-plat/Plplotter_Init.c \
	$(top_srcdir)/bindings/tk-x-plat/plplotter.c

## tkwin.c has direct calls to libplplot, libplplottcltk,
## Tcl and Tk library functions but not X libraries.
## However, specify X and math libraries as well since libtk depends on them.
tkwin_la_LDFLAGS = $(ldflags) $(libplplot) $(libplplottcltk) \
		 $(TKLIBCMD) $(TCLLIBCMD) $(X_LIBS) -lX11 -lm

xfig_la_SOURCES = xfig.c
xfig_la_LDFLAGS = $(ldflags) $(libplplot) $(lmflag)

if enable_xwin
xflags = $(X_LIBS) -lX11 $(PTHREADLIBS)
xwin_la_SOURCES = xwin.c
xwin_la_LDFLAGS = $(ldflags) $(libplplot) $(xflags) $(lmflag)
endif

# end of enable_dyndrivers
else
# start of static driver alternative which is implemented using
# a convenience library to be included in libplplot.

noinst_LTLIBRARIES = libplplotdrv.la

if enable_tk
# All source that is in libplplottcltk.
tksources = \
	$(top_builddir)/bindings/tcl/tclAPI.c \
	$(top_srcdir)/bindings/tcl/tclMain.c \
	$(top_srcdir)/bindings/tk/Pltk_Init.c \
	$(top_srcdir)/bindings/tk/plframe.c \
	$(top_srcdir)/bindings/tk/plr.c \
	$(top_srcdir)/bindings/tk/tcpip.c \
	$(top_srcdir)/bindings/tk/tkMain.c

endif

$(top_builddir)/bindings/tcl/tclAPI.c: tclapi
tclapi:
	( cd $(top_builddir)/bindings/tcl ; $(MAKE) tclgen.h )
.PHONY: tclapi

if enable_tcl
# All source that is in libtclmatrix.
matrixsources = \
	$(top_srcdir)/bindings/tcl/tclMatrix.c \
	$(top_srcdir)/bindings/tcl/matrixInit.c
endif

if enable_tkwin

if enable_tk
# These additions required for tkwin driver if already have additions for
# tk driver in place.  These correspond to objects normally included in
# the tkwin driver in any case.

tkwinsources = \
	$(top_srcdir)/bindings/tk-x-plat/Plplotter_Init.c \
	$(top_srcdir)/bindings/tk-x-plat/plplotter.c
else
# tkwin driver on its own without tk driver (but tcl must be enabled so
# that in this case the matrix source is already in.  Thus, only need to
# put in the tksource plus the two extra above.

tkwinsources = \
	$(top_builddir)/bindings/tcl/tclAPI.c \
	$(top_srcdir)/bindings/tcl/tclMain.c \
	$(top_srcdir)/bindings/tk/Pltk_Init.c \
	$(top_srcdir)/bindings/tk/plframe.c \
	$(top_srcdir)/bindings/tk/plr.c \
	$(top_srcdir)/bindings/tk/tcpip.c \
	$(top_srcdir)/bindings/tk/tkMain.c \
	$(top_srcdir)/bindings/tk-x-plat/Plplotter_Init.c \
	$(top_srcdir)/bindings/tk-x-plat/plplotter.c

endif
endif

# For AM_CONDITIONALS must use SOURCES, not LIBADD
libplplotdrv_la_SOURCES = $(tksources) $(matrixsources) $(tkwinsources)
EXTRA_libplplotdrv_la_SOURCES = \
	aqt.c \
	cgm.c \
	dg300.c \
	gcw.c \
	plplotcanvas.c \
	plplotcanvas-hacktext.c \
	gd.c \
	gnome.c \
	hpgl.c \
	impress.c \
	linuxvga.c \
	ljii.c \
	ljiip.c \
	mem.c \
	ntk.c \
	null.c \
	pbm.c \
	plmeta.c \
	ps.c \
	pstex.c \
	tek.c \
	tk.c \
	tkwin.c \
	wingcc.c \
	xfig.c \
	xwin.c \
	$(extra_tksources) \
	$(extra_tkwinsources)

# All -I preprocessor flags apparently must be lumped together for all source
# in this directory.  Some day when automake grows up, convert these
# to xwin_lo_AM_CPPFLAGS, etc.  For now, use individual CFLAGS instead
# which strictly speaking is a misuse since CFLAGS are supposed to be
# processor options, not preprocessor.
# Double precision versions not needed since these flags are currently
# ignored by automake so the approach must be to concatanate them
# together to form one giant lump libplplotdrv_la_CFLAGS
# (which is going to slow the build down with the enormous number of
# gnome compiler flags, but those who use static drivers get what they
# deserve....;-))

cgm_lo_CFLAGS = $(CDINCCMD)
gd_lo_CFLAGS = $(GDINCCMD)
aqt_lo_CFLAGS = $(AQTINCCMD)
gnome_lo_CFLAGS = $(GTK_CFLAGS) $(GNOME_INCLUDEDIR)
libgcw_lo_CFLAGS = $(GCW_CFLAGS)
ntk_lo_CFLAGS = \
	-I$(top_srcdir)/bindings/tcl \
	-I$(top_builddir)/bindings/tcl \
	-I$(top_srcdir)/bindings/tk \
	$(TCLINCCMD) \
	$(TKINCCMD)

tk_lo_CFLAGS = \
	-I$(top_srcdir)/bindings/tcl \
	-I$(top_builddir)/bindings/tcl \
	-I$(top_srcdir)/bindings/tk \
	$(TCLINCCMD) \
	$(TKINCCMD)

tkwin_lo_CFLAGS = \
	-I$(top_srcdir)/bindings/tcl \
	-I$(top_builddir)/bindings/tcl \
	-I$(top_srcdir)/bindings/tk \
	-I$(top_srcdir)/bindings/tk-x-plat \
	$(TCLINCCMD) \
	$(TKINCCMD)

wingcc_lo_CFLAGS = $(GDI32INCCMD)

xwin_lo_CFLAGS = $(X_CFLAGS)

# tkwin covers ntk, tk, and tkwin
# Following line did not work so try lumped alternative with AM_CPPFLAGS
# libplplotdrv_la_CFLAGS = \
# Also add itcl compile flags just in case the tk extra objects are compiled.
AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	$(cgm_lo_CFLAGS) \
	$(gd_lo_CFLAGS) \
	$(aqt_lo_CFLAGS) \
	$(gnome_lo_CFLAGS) \
	$(libgcw_lo_CFLAGS) \
	$(tkwin_lo_CFLAGS) \
	$(wingcc_lo_CFLAGS) \
	$(xwin_lo_CFLAGS) \
	$(FREETYPEINCCMD) \
	$(ITCLINCCMD)

libplplotdrv_la_LIBADD = $(STATIC_DRIVERS)
# Concatanate all LDFLAGS for individual drivers above together

if enable_xwin
xflags = $(X_LIBS) -lX11 $(PTHREADLIBS)
endif

## Library ordering is important here
libplplotdrv_la_LDFLAGS = \
	$(CDLIBCMD) \
	$(GCW_LIBS) \
	$(GDLIBCMD) \
	$(GNOMELIBS) \
	$(LINUXVGALIBS) \
	$(ITKLIBCMD) \
	$(TKLIBCMD) \
	$(ITCLLIBCMD) \
	$(TCLLIBCMD) \
	$(FREETYPELIBCMD) \
	$(GDI32LIBCMD) \
	$(AQTLIBCMD) \
	$(xflags) $(lmflag)

# These dependencies required for some reason if using the LIBADD method.
libplplotdrv_la_DEPENDENCIES = $(libplplotdrv_la_LIBADD)

# end of static driver case.
endif

install-data-hook:
if enable_dyndrivers
	rm -f $(DESTDIR)$(driversdir)/*.a
endif

docdir = $(DOC_DIR)
doc_DATA = README.drivers

EXTRA_DIST = $(doc_DATA) next.c

CLEANFILES = *.rc
DISTCLEANFILES = $(drivers_DATA)
MAINTAINERCLEANFILES = Makefile.in
