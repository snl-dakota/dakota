#
# CAUTION!
#
# This file should only be edited in the bootstrap/packages directory!
#

include ../config/MakeMacros.mk

#
# BUILDING THIRD PARTY LIBRARIES:
#
# 3po, soplex, coin, ampl, filib and pls:
#    Third party libraries.  An acro Makefile is used to
#    build them.
#
# trilinos, glpk, mtl and ipopt: 
#    Third party libraries.  These are already autoconf'ed
#    and we descend to those directories and use their Makefiles.
#

TOPINC=../include
TOPLIB=../lib

SUBDIRS=. $(AMPL_DIR) $(HSL_DIR) $(NPSOL_DIR) $(SNOPT_DIR) $(CONMIN_DIR) $(GLPK_DIR) $(MTL_DIR) $(TRILINOS_DIR) $(THREEPO_DIR) $(SOPLEX_DIR) $(FILIB_DIR) $(DSCPACK_DIR) $(MUMPS_DIR) $(COIN_CBC_DIR) $(MOMHLIB_DIR) $(EO_DIR) $(COIN_IPOPT_DIR) $(TINYXML_DIR) $(BOOST_DIR) $(APPSPACK_DIR) $(TEUCHOS_DIR)

DIST_SUBDIRS=. $(AMPL_DIR) $(HSL_DIR) $(NPSOL_DIR) $(SNOPT_DIR) $(CONMIN_DIR) $(GLPK_DIR) $(MTL_DIR) $(TRILINOS_DIR) $(THREEPO_DIR) $(SOPLEX_DIR) $(FILIB_DIR) $(DSCPACK_DIR) $(MUMPS_DIR) $(COIN_CBC_DIR) $(MOMHLIB_DIR) $(EO_DIR) $(COIN_IPOPT_DIR) $(TINYXML_DIR) $(BOOST_DIR) $(APPSPACK_DIR) $(TEUCHOS_DIR)

all-local:
if BUILD_APPSPACK
	cd appspack && $(MAKE)
	$(MAKE) copy_appspack
endif
if BUILD_SOPLEX
	cd soplex && $(MAKE) copy-headers
endif
if BUILD_THREEPO
	cd 3po && $(MAKE) copy-headers
endif
if BUILD_AMPL
if TARGET_MINGW
	sed 's/rm -f a\.out/rm -f a\.exe/g' ampl/Makefile > Makefile.tmp;
	sed 's/a\.out >arith/a\.exe >arith/g' Makefile.tmp > Makefile2.tmp;
	cp Makefile2.tmp ampl/Makefile;
	rm Makefile.tmp Makefile2.tmp;
	cd ampl && $(MAKE) CC="gcc -mno-cygwin" CFLAGS="-DNO_RUSAGE -O2"
else
	cd ampl && $(MAKE)
endif
	$(MAKE) copy_ampl
endif
if BUILD_COIN_CBC
	cd coin-cbc && $(MAKE) copy-headers
endif
if BUILD_COIN_IPOPT
	cd coin-ipopt && $(MAKE) copy-headers
endif
if BUILD_COIN_BONMIN
	cd coin-bonmin && $(MAKE) copy-headers
endif
if BUILD_GLPK
	cd glpk && $(MAKE) copy-headers
endif
if BUILD_DSCPACK
	cd dscpack && $(MAKE) copy-headers
endif
if BUILD_MOMHLIB
	cd momhlib && $(MAKE) copy-headers
endif
if BUILD_MUMPS
	cd mumps && $(MAKE) copy-headers
endif
if BUILD_HSL
	cd hsl && $(MAKE) copy-headers
endif
if BUILD_NPSOL
	cd npsol && $(MAKE) copy-headers
endif
if BUILD_CONMIN
	cd conmin && $(MAKE) copy-headers
endif
if BUILD_SNOPT
	cd snopt && $(MAKE) copy-headers
endif
if BUILD_EO
	cd eo && $(MAKE) copy-headers
endif
if BUILD_TINYXML
	cd tinyxml && $(MAKE) copy-headers
endif
if BUILD_BOOST_TPL
	cd boost && $(MAKE)
endif
if BUILD_TEUCHOS
	cd teuchos && $(MAKE)
endif


#clean-local:
#	$(RM) -Rf ../python

MAINTAINERCLEANFILES = Makefile.in

COPY=$(top_srcdir)/bin/cp_u

copy_ampl:
	@if [ -d ampl ]; then\
	    mkdir -p ../include/ampl;\
	    $(COPY) ampl/*h ../include/ampl;\
	    mkdir -p ../lib;\
	    $(COPY) ampl/amplsolver.a ../lib/libamplsolver.a;\
	fi

copy_appspack:
	@if [ -d appspack ]; then\
	    mkdir -p ../include/appspack;\
	    $(COPY) appspack/src/*hpp ../include/appspack;\
	    mkdir -p ../lib;\
	    $(COPY) appspack/src/libappspack.a ../lib;\
	fi


#
# Create the python.zip file as follows:
#
check-local: python

python.log: ../admin/vpy/python.zip
	rm -Rf ../python
	touch python.log

python-zip: python.log
	@if test -d ../python; then : ;\
	else\
	   PYTHON_PACKAGES=;\
	   for x in $(top_srcdir)/packages/*/python; do \
	      PYTHON_PACKAGES="$$PYTHON_PACKAGES -p %s/$$x"; \
	   done;\
	   $(top_srcdir)/tpl/vpykit/bin/install_python \
		--src $(top_srcdir) --build $(abs_top_builddir) \
		--logfile $(abs_top_builddir)/tpl/python.log \
		-c $(top_srcdir)/admin/vpy/dev.ini \
		-z $(top_srcdir)/admin/vpy/python.zip \
		-p %s/admin/acro-admin \
		-p %s/tpl/cxxtest/python \
		$$PYTHON_PACKAGES ;\
	fi

python: python-zip

update-zip:
	cd $(top_srcdir)/tpl/vpykit; \
	bin/create_zipfile -c ../../admin/vpy/dev.ini -d ../../admin/vpy

