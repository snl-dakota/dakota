#
# CAUTION!
#
# This file should only be edited in the bootstrap/packages directory!
#

include ../config/MakeMacros.mk

##
## utilib: Already autoconf'd (by us) subpackage, descend and
##         let it build.
##
## exact, colin, pebbl, pico, scolib, parpcx, gnlp, interfaces -
##   SNL codes, descend to directory and use our Makefile
##
## appspack: SNL code already autoconf'd (by others)
##

SUBDIRS = \
  $(UTILIB_DIR) $(COOPR_TOOLS_DIR) $(SACACHE_DIR) $(SUNDRY_DIR) \
  $(JEGA_DIR) $(COLIN_DIR) $(TMF_DIR) $(PEBBL_DIR) $(PICO_DIR) \
  $(SCOLIB_DIR) $(PARPCX_DIR) $(GNLP_DIR) $(OPTPP_DIR) $(INTERFACES_DIR) .

DIST_SUBDIRS = \
  $(UTILIB_DIR) $(COOPR_TOOLS_DIR) $(SACACHE_DIR) $(SUNDRY_DIR) \
  $(JEGA_DIR) $(COLIN_DIR) $(TMF_DIR) $(PEBBL_DIR) $(PICO_DIR) \
  $(SCOLIB_DIR) $(PARPCX_DIR) $(GNLP_DIR) $(OPTPP_DIR) $(INTERFACES_DIR)

if BUILD_OPTPP
   MAKE_OPTPP_DOC=cd optpp/docs && $(MAKE) all
   OPTPP_COPY=cd optpp/lib && ../libtool --mode=install $(INSTALL) 'liboptpp.la' '@abs_top_builddir@/lib/liboptpp.la'
   OPTPP_CLEAN=rm -f @abs_top_builddir@/lib/liboptpp*
endif
if BUILD_UTILIB
   MAKE_UTILIB_DOC=cd utilib && $(MAKE) docs
   UTILIB_COPY=cd utilib/src && ../libtool --mode=install $(INSTALL) 'libutilib.la' '@abs_top_builddir@/lib/libutilib.la'
   UTILIB_CLEAN=rm -f @abs_top_builddir@/lib/*utilib*
endif
if BUILD_INTERFACES
   MAKE_INTERFACES_DOC=cd interfaces/doc && $(MAKE) docs
endif
if BUILD_COLIN
   MAKE_COLIN_DOC=cd colin/doc && $(MAKE) docs
endif
if BUILD_SCOLIB
   MAKE_SCOLIB_DOC=cd scolib/doc && $(MAKE) docs
endif
if BUILD_PEBBL
   MAKE_PEBBL_DOC=cd pebbl/doc && $(MAKE) docs
endif
if BUILD_PICO
   MAKE_PICO_DOC=cd pico/doc && $(MAKE) docs
endif
if BUILD_PARPCX
   MAKE_PARPCX_DOC=cd parpcx/doc && $(MAKE) docs
endif
if BUILD_GNLP
   MAKE_GNLP_DOC=cd gnlp/doc && $(MAKE) docs
endif
if BUILD_TMF
   MAKE_TMF_DOC=cd tmf/doc && $(MAKE) docs
endif
if BUILD_SACACHE
   MAKE_SACACHE_DOC=cd sacache/doc && $(MAKE) docs
endif

COPY=@abs_top_srcdir@/bin/cp_u

docs:
	$(MAKE_UTILIB_DOC)
	$(MAKE_SACACHE_DOC)
	$(MAKE_COLIN_DOC)
	$(MAKE_TMF_DOC)
	$(MAKE_SCOLIB_DOC)
	$(MAKE_GNLP_DOC)
	$(MAKE_PARPCX_DOC)
	$(MAKE_PEBBL_DOC)
	$(MAKE_PICO_DOC)
	$(MAKE_OPTPP_DOC)
	$(MAKE_INTERFACES_DOC)

all-local:
	@if test -d ../lib;  then : ; else mkdir ../lib ; fi
	@$(UTILIB_COPY)
	$(OPTPP_COPY)
	$(OPTPP_COPY2)


clean-local:
	@$(UTILIB_CLEAN)
	@$(OPTPP_CLEAN)

DISTCLEANFILES= dummy/Makefile dummy/Makefile.in
MAINTAINERCLEANFILES = Makefile.in dummy/Makefile.in
EXTRA_DIST=dummy/Makefile.am
