## Process this file with automake to produce 'Makefile.in'.

## Need the config directory for its m4 files.
ACLOCAL_AMFLAGS = -I config

## Source subdirectories to be recursed into during the build.
SUBDIRS = packages src
if ENABLE_TESTS
SUBDIRS += test
SUBDIRS += examples
endif
if ENABLE_DOCS
SUBDIRS += docs
endif

## Conditionally configured subdirectories should _not_ be recursed into
## during 'make distclean' or 'make maintainer-clean'.
DIST_SUBDIRS = $(SUBDIRS)

## These files should be deleted on 'make maintainer-clean'.
MAINTAINERCLEANFILES = aclocal.m4 compile config.guess config.sub configure \
                       dakota_config.h.in depcomp install-sh ltmain.sh \
                       missing Makefile.in ylwrap

## These directories should be removed on 'make clean'.
clean-local:
	-rm -fr ii_files/ SunWS_cache/ .dvips/ so_locations

## These files are scripts and should have appropriate execute permissions.
## previously has platform/config.* here

## These files should be included in the distribution.
EXTRA_DIST = LICENSE

## The generated config.h should be installed, but not distributed
nodist_include_HEADERS = ${top_builddir}/dakota_config.h
