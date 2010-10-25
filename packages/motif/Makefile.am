## Process this file with automake to produce Makefile.in

## Need the m4 directory for non-native macro tests.
ACLOCAL_AMFLAGS = -I m4

## There's only one place to go; down!
SUBDIRS = src

## Headers which must be distributed.
include_HEADERS = include/Graph2D.H \
		  include/Graphics.H \
		  include/menu.h

## These files should only be removed on 'make maintainer-clean'.
MAINTAINERCLEANFILES = Makefile.in aclocal.m4 compile config.guess config.sub configure depcomp install-sh ltmain.sh missing motif_config.h.in
