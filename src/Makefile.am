## Process this file with automake to produce 'Makefile.in'.

## Define the conditional sources, includes and archives (with dependencies).
include $(srcdir)/Makefile.include
include $(srcdir)/Makefile.source

AM_CPPFLAGS = -I${top_srcdir}/packages

if ENABLE_F90
## HACK NOTICE: Libtool 1.5.X doesn't provide tagging support for F90 sources.
## Until it does (Libtool 2.0.X), we need to use the F77 tag rules.
LTFCCOMPILE = $(LIBTOOL) --tag=F77 --mode=compile $(FC) $(AM_FCFLAGS) \
	      $(FCFLAGS) $(FCFLAGS_f90)
FCLINK = $(LIBTOOL) --tag=F77 --mode=link $(FCLD) $(AM_FCFLAGS) $(FCFLAGS) \
	 $(FCFLAGS_f90) $(AM_LDFLAGS) $(LDFLAGS) -o $@
endif

## Define the DAKOTA library composition;
lib_LTLIBRARIES = libdakota.la
libdakota_la_SOURCES = $(UTIL_SRCS) \
		       $(EVALDATA_SRCS) \
		       $(DB_SRCS) \
		       $(MODEL_SRCS) \
		       $(CONSTRAINT_SRCS) \
		       $(INTERFACE_SRCS) \
		       $(STRATEGY_SRCS) \
		       $(ITERATOR_SRCS)

## as well as the build revision, date and time encoding sources.
dist_libdakota_la_SOURCES = DakotaBuildInfo.H
nodist_libdakota_la_SOURCES = DakotaBuildInfo.C

## Manage additional flags needed to link binaries with optional components
LOCALLDFLAGS = $(PTHREAD_CFLAGS)

LOCALLDADD = libdakota.la $(ARCHIVES) $(XPM_LIBS) \
              $(X_LIBS) $(X_EXTRA_LIBS) $(X_PRE_LIBS) $(PTHREAD_LIBS) \
              $(LAPACK_LIBS) $(BLAS_LIBS) $(FLIBS) $(DLOPEN_LIBS)

if WITH_PYTHON
CPPFLAGS += $(PYTHON_CPPFLAGS)
LOCALLDFLAGS += $(PYTHON_EXTRA_LDFLAGS)
endif

if ENABLE_F90
LOCALLDADD += $(FCLIBS)
endif

if WITH_MATLAB
LOCALLDADD += $(MATLAB_LIBS)
LOCALLDFLAGS += $(MATLAB_LDFLAGS)
endif

if WITH_PYTHON
LOCALLDADD += $(PYTHON_LDFLAGS) $(PYTHON_EXTRA_LIBS)
endif

## The sense of NIDR's DYNLIB flag is opposite: if no DL_SOLVER, use the 
## pre-processor to turn off NIDR's dynamic loading capability
if !WITH_DL_SOLVER
CPPFLAGS += -DNO_NIDR_DYNLIB
endif

## Define the main program binaries with their sources and dependencies.
bin_PROGRAMS = dakota dakota_restart_util

dakota_SOURCES = main.C
dakota_LDFLAGS = $(LOCALLDFLAGS)
dakota_LDADD = $(LOCALLDADD)

dakota_restart_util_SOURCES = restart_util.C
dakota_restart_util_LDFLAGS = $(LOCALLDFLAGS)
dakota_restart_util_LDADD = $(LOCALLDADD)

## Define optional binaries for library/plugin mode
if WITH_PLUGIN
bin_PROGRAMS += dakota_library_mode
dakota_library_mode_SOURCES = library_mode.C
dakota_library_mode_LDFLAGS = $(LOCALLDFLAGS)
dakota_library_mode_LDADD = $(LOCALLDADD)

if ENABLE_MPI
bin_PROGRAMS += dakota_library_split
dakota_library_split_SOURCES = library_split.C
dakota_library_split_LDFLAGS = $(LOCALLDFLAGS)
dakota_library_split_LDADD = $(LOCALLDADD)
endif

endif

if WITH_DLL
bin_PROGRAMS += dll_api
dll_api_SOURCES = dll_api.C
dll_api_LDADD = $(LOCALLDADD) 
dll_api_LDFLAGS = $(LOCALLDFLAGS) $(DLL_API_XTRALIBS)
endif

## TODO: distribute wrapper script to manage dynamic libraries
##dist_bin_SCRIPTS = dakota.sh

## The specification file should be included in the distribution as well as
## installed in the same directory where the dakota* executables go.
EXTRA_DIST = dakota.input.summary dakota.input.nspec .build-info
install-exec-local: dakota.input.summary dakota.input.nspec dakota.input.desc
	cp $^ $(DESTDIR)$(bindir)
uninstall-local: $(DESTDIR)$(bindir)/dakota.input.summary \
		 $(DESTDIR)$(bindir)/dakota.input.nspec \
		 $(DESTDIR)$(bindir)/dakota.input.desc
	rm -f $^

## These files should be deleted on 'make clean'.
CLEANFILES = S4 LHS_* fort.* *.log *.mod dakota_tabular.dat dakota.rst \
	     idr-parsed-input.txt 

## These files should be deleted on 'make maintainer-clean'.
MAINTAINERCLEANFILES = Makefile.in

## These directories should be removed on 'make clean'.
clean-local:
	-rm -fr ii_files/ SunWS_cache/ so_locations

## Conditionally enable NIDR-related targets; otherwise make will try
## to rebuild them on a clean checkout
if ENABLE_SPEC_MAINT

## Special rules for updating when the .nspec changes and to build
## Jaguar pieces; must be invoked manually
## TODO: AC_PROG_SED
## Note: when spec changes, there is a dependency on sed, perl
nidr-files: dakota.input.summary dakota.input.desc NIDR_guikeywds.h NIDR_keywds.H dakreorder dakreord

# Custom programs needed to build nidr-files
NIDRGEN = $(NIDR_BUILD)/nidrgen$(EXEEXT)
DT = $(NIDR_BUILD)/Dt$(EXEEXT)
GENDESC = ./generate_desc.pl

# This will force these to get built first when make all is invoked, assuming 
# they're associated with a target
BUILT_SOURCES += dakota.input.summary NIDR_keywds0.h dakota.input.desc \
		 NIDR_guikeywds.h NIDR_keywds0.h

dakota.input.summary: dakota.input.nspec $(NIDRGEN)
	$(NIDRGEN) -efp dakota.input.nspec > dakota.input.summary

NIDR_keywds0.h: dakota.input.nspec $(NIDRGEN) $(DT)
	$(NIDRGEN) -ftn- dakota.input.nspec > NIDR_keywds0.h
	$(DT) -8 dakota.input.nspec $(dakreorder_preq) | sed 's/.*/#define NSPEC_DATE "&"/' >> NIDR_keywds0.h

dakota.input.desc: dakota.input.nspec $(NIDRGEN) $(GENDESC) ../docs/Ref_*.dox
	$(NIDRGEN) -T dakota.tags.desc dakota.input.nspec
	$(GENDESC)
	$(RM) dakota.tags.desc

NIDR_guikeywds.h: dakota.input.nspec dakota.input.desc $(NIDRGEN)
	$(NIDRGEN) -egG dakota.input.nspec dakota.input.desc > NIDR_guikeywds.h

NIDR_keywds.H: dakota.input.nspec dakota.input.desc NIDR_initdefs.h $(NIDRGEN)
	$(NIDRGEN) dakota.input.nspec dakota.input.desc NIDR_initdefs.h NIDR_keywds.H

# TODO: make -ldl optional if possible
bin_PROGRAMS += dakreorder
dakreorder_SOURCES = dakreorder.c NIDR_keywds0.h
#dakreorder.$(OBJEXT): NIDR_keywds0.h
dakreorder_INCLUDES = $(NIDR_INCL)
dakreorder_LDADD = $(NIDR_ARCHIVE) -ldl

bin_PROGRAMS += dakreord
dakreord_SOURCES = dakreorder.c
dakreord_CFLAGS = -DNO_NIDR_keywds0
dakreord_INCLUDES = $(NIDR_INCL)
dakreord_LDADD = $(NIDR_ARCHIVE) -ldl

endif