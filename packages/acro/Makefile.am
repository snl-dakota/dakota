#
# CAUTION!
#
# This file should only be edited in the bootstrap/packages directory!
#

include $(srcdir)/config/MakeMacros.mk

TOPBIN=${top_srcdir}/bin
CONFIGDIR=${top_srcdir}/config
TOPINC=include

#
# location of m4 macros
#
ACLOCAL_AMFLAGS= -I config 

SUBDIRS = . tpl packages doc

nodist_include_HEADERS = ${top_builddir}/include/acro_config.h

all-local:
	@if test -e userapps/Makefile.am ; then cd userapps && $(MAKE) ; fi
	@if test -d $(TOPINC) ; then : ; else mkdir $(TOPINC) ; fi

docs:
	@cd doc && $(MAKE) docs
	@cd packages && $(MAKE) docs

DISTCLEANFILES = _configs.sed include/acro_config.h 

MAINTAINERCLEANFILES = aclocal.m4 configure Makefile.in

EXTRA_DIST=config/MakeMacros.mk bin/cp_u\
	bin/checkCopyright\
        config \
        setup \
        bootstrap

dist-hook:
	@cd $(distdir)/config && rm -rf $$(find . -name .svn)
	@cd $(distdir)/bootstrap && rm -rf $$(find . -name .svn)

python:
	@cd tpl && $(MAKE) python

