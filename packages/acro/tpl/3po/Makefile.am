
#
# SLB: Temporary flag to prevent parallel compilation by gmake
#
.NOTPARALLEL:

include $(top_srcdir)/config/MakeMacros.mk

lib_LIBRARIES=lib3po.a

if BUILD_NSGA2
NSGA2_src=\
	nsga2/allocate.c\
	nsga2/auxiliary.c\
	nsga2/crossover.c\
	nsga2/crowddist.c\
	nsga2/decode.c\
	nsga2/display.c\
	nsga2/dominance.c\
	nsga2/eval.c\
	nsga2/fillnds.c\
	nsga2/initialize.c\
	nsga2/list.c\
	nsga2/merge.c\
	nsga2/mutation.c\
	nsga2/rand.c\
	nsga2/rank.c\
	nsga2/report.c\
	nsga2/sort.c\
	nsga2/tourselect.c
endif

lib3po_a_SOURCES=cobyla2c/cobyla.c hooke/hooke_tpl.cpp $(NSGA2_src)

TOPINC=$(top_builddir)/include
TOPLIB=$(top_builddir)/lib
TOPBIN=$(top_srcdir)/bin

FULLPATH_HFILES=$(top_srcdir)/tpl/3po/cobyla2c

#
# headers to copy to common directory
#
ALL_H_SRC=$(FULLPATH_HFILES)/cobyla.h

#
# headers to install (from common directory)
#
include3podir=$(includedir)/3po

include3po_HEADERS=cobyla2c/cobyla.h

all-local:
	@if test -d $(TOPLIB) ; then : ; else mkdir $(TOPLIB) ; fi
	@$(TOPBIN)/cp_u lib3po.a $(TOPLIB)

clean-local:
	@rm -f $(TOPLIB)/lib3po.a
	@$(MAKE) clean-headers

copy-headers:
	@if test -d $(TOPINC) ; then : ; else mkdir $(TOPINC) ; fi
	@if test -d $(TOPINC)/3po ; then : ; else mkdir $(TOPINC)/3po ; fi
	@$(TOPBIN)/cp_u $(ALL_H_SRC) $(TOPINC)/3po

clean-headers:
	@rm -rf $(TOPINC)/3po

MAINTAINERCLEANFILES = Makefile.in

EXTRA_DIST= cobyla2c/LICENSE \
            cobyla2c/README \
            cobyla2c/HISTORY
