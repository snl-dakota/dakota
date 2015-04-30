lib_LIBRARIES=libmomh.a

TOPINC=${top_builddir}/include
TOPLIB=${top_builddir}/lib
TOPBIN=${top_srcdir}/bin

SRCDIR=MOMHLib
FULLPATH_SRC=${srcdir}/$(SRCDIR)

AM_CPPFLAGS=-I${srcdir}/$(SRCDIR)/Common -I${srcdir}/$(SRCDIR)/MO


LIB_SRCS= \
  $(FULLPATH_SRC)/Common/trandomgenerator.cpp\
  $(FULLPATH_SRC)/MO/global.cpp\
  $(FULLPATH_SRC)/MO/momhconstrainedsolution.cpp\
  $(FULLPATH_SRC)/MO/nondominatedset.cpp\
  $(FULLPATH_SRC)/MO/solution.cpp\
  $(FULLPATH_SRC)/MO/solutionsset.cpp\
  $(FULLPATH_SRC)/MO/tbnode.cpp\
  $(FULLPATH_SRC)/MO/tbtree.cpp\
  $(FULLPATH_SRC)/MO/tmotools.cpp\
  $(FULLPATH_SRC)/MO/tquadnode.cpp\
  $(FULLPATH_SRC)/MO/weights.cpp\
  $(FULLPATH_SRC)/ParetoGA/tindexedpoint.cpp

LIB_H_SRC = \
  $(FULLPATH_SRC)/Common/tmatrix.cpp\
  $(FULLPATH_SRC)/HGA/hgabase.cpp\
  $(FULLPATH_SRC)/HGA/immogls.cpp\
  $(FULLPATH_SRC)/HGA/mogls.cpp\
  $(FULLPATH_SRC)/HGA/pma.cpp\
  $(FULLPATH_SRC)/MO/momethod.cpp\
  $(FULLPATH_SRC)/MOMSLS/momsls.cpp\
  $(FULLPATH_SRC)/MO/tlistset.cpp\
  $(FULLPATH_SRC)/MO/tquadtree.cpp\
  $(FULLPATH_SRC)/ParetoGA/constrainednsgaiic.cpp\
  $(FULLPATH_SRC)/ParetoGA/nsga.cpp\
  $(FULLPATH_SRC)/ParetoGA/nsgaiic.cpp\
  $(FULLPATH_SRC)/ParetoGA/nsgaii.cpp\
  $(FULLPATH_SRC)/ParetoGA/paretobase.cpp\
  $(FULLPATH_SRC)/ParetoGA/spea.cpp\
  $(FULLPATH_SRC)/SA/mosa.cpp\
  $(FULLPATH_SRC)/SA/psa.cpp\
  $(FULLPATH_SRC)/SA/sabase.cpp\
  $(FULLPATH_SRC)/SA/smosa.cpp\
  $(FULLPATH_SRC)/Common/tmatrix.h\
  $(FULLPATH_SRC)/Common/trandomgenerator.h\
  $(FULLPATH_SRC)/HGA/hgabase.h\
  $(FULLPATH_SRC)/HGA/immogls.h\
  $(FULLPATH_SRC)/HGA/mogls.h\
  $(FULLPATH_SRC)/HGA/pma.h\
  $(FULLPATH_SRC)/MO/global.h\
  $(FULLPATH_SRC)/MO/momethod.h\
  $(FULLPATH_SRC)/MO/momhconstrainedsolution.h\
  $(FULLPATH_SRC)/MO/momhsolution.h\
  $(FULLPATH_SRC)/MOMSLS/momsls.h\
  $(FULLPATH_SRC)/MO/nondominatedset.h\
  $(FULLPATH_SRC)/MO/problem.h\
  $(FULLPATH_SRC)/MO/solution.h\
  $(FULLPATH_SRC)/MO/solutionsset.h\
  $(FULLPATH_SRC)/MO/tbnode.h\
  $(FULLPATH_SRC)/MO/tbtree.h\
  $(FULLPATH_SRC)/MO/tlistset.h\
  $(FULLPATH_SRC)/MO/tmotools.h\
  $(FULLPATH_SRC)/MO/tquadnode.h\
  $(FULLPATH_SRC)/MO/tquadtree.h\
  $(FULLPATH_SRC)/MO/weights.h\
  $(FULLPATH_SRC)/ParetoGA/constrainednsgaiic.h\
  $(FULLPATH_SRC)/ParetoGA/nsga.h\
  $(FULLPATH_SRC)/ParetoGA/nsgaiic.h\
  $(FULLPATH_SRC)/ParetoGA/nsgaii.h\
  $(FULLPATH_SRC)/ParetoGA/paretobase.h\
  $(FULLPATH_SRC)/ParetoGA/spea.h\
  $(FULLPATH_SRC)/ParetoGA/tindexedpoint.h\
  $(FULLPATH_SRC)/SA/mosa.h\
  $(FULLPATH_SRC)/SA/psa.h\
  $(FULLPATH_SRC)/SA/sabase.h\
  $(FULLPATH_SRC)/SA/smosa.h

libmomh_a_SOURCES=$(LIB_SRCS)

all-local: libmomh.a
	@if test -d $(TOPLIB) ; then : ; else mkdir $(TOPLIB) ; fi
	@$(TOPBIN)/cp_u libmomh.a $(TOPLIB)

clean-local:
	@rm -f $(TOPLIB)/libmomh.a
	@$(MAKE) clean-headers

copy-headers:
	@if test -d $(TOPINC) ; then : ; else mkdir $(TOPINC) ; fi
	@if test -d $(TOPINC)/momhlib ; then : ; else mkdir $(TOPINC)/momhlib ; fi
	@$(TOPBIN)/cp_u $(LIB_H_SRC) $(TOPINC)/momhlib

clean-headers:
	@rm -rf $(TOPINC)/momhlib




