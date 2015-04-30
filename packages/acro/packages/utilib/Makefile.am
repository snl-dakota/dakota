if BUILD_TESTS
  TEST_DIR=test
endif

include ${top_srcdir}/config/MakeMacros.mk

ACLOCAL_AMFLAGS=-I config

SUBDIRS=doc src examples $(TEST_DIR)

docs:
	cd doc && $(MAKE) docs

DISTCLEANFILES = src/utilib_config.h _configs.sed

MAINTAINERCLEANFILES = Makefile.in aclocal.m4 configure utilib/config.h.in

EXTRA_DIST=${top_srcdir}/config/MakeMacros.mk ${top_srcdir}/bin/cp_u

