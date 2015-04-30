
include ../../../config/MakeMacros.mk

SUBDIRS=unit

noinst_PROGRAMS=

TOPINC=../../../include
TOPLIB=../../../lib

LDADD=-L. -L$(TOPLIB) $(top_builddir)/packages/pebbl/src/libpebbl.a\
                $(top_builddir)/packages/utilib/src/libutilib.la -lm

if BUILD_STATIC_EXECUTABLES
#
#A libtool flag
#
AM_LDFLAGS=@STATIC_FLAGS@
endif

#
# Paths to:
#  all headers, to config.h, and to other configuration headers.
#

AM_CPPFLAGS=-I$(top_builddir) -I$(top_srcdir)/config -I$(TOPINC) \
            $(ANSI_FLAG) $(NS_FLAG)

MAINTAINERCLEANFILES = Makefile.in
