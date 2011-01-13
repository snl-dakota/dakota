## Process this file with automake to produce 'Makefile.in'.

## Run through the conditionals, defining the interface dependencies.
include $(top_srcdir)/src/Makefile.include

## Create symlinks up to the dakota and dakota_restart_util binaries.
ALL_LOCAL_LINKS = dakota$(EXEEXT) dakota_restart_util$(EXEEXT)
if WITH_PLUGIN
ALL_LOCAL_LINKS += dakota_library_mode$(EXEEXT)
if ENABLE_MPI
ALL_LOCAL_LINKS += dakota_library_split$(EXEEXT)
endif
endif
#if WITH_GRAPHICS
#ALL_LOCAL_LINKS += plxtnd5.fnt
#endif
all-local: $(ALL_LOCAL_LINKS)

dakota$(EXEEXT): $(top_builddir)/src/dakota$(EXEEXT)
	$(LN_S) $(top_builddir)/src/dakota$(EXEEXT) dakota$(EXEEXT)

dakota_restart_util$(EXEEXT):  $(top_builddir)/src/dakota_restart_util$(EXEEXT)
	$(LN_S) $(top_builddir)/src/dakota_restart_util$(EXEEXT) dakota_restart_util$(EXEEXT)

if WITH_PLUGIN
dakota_library_mode$(EXEEXT):  $(top_builddir)/src/dakota_library_mode$(EXEEXT)
	$(LN_S) $(top_builddir)/src/dakota_library_mode$(EXEEXT) dakota_library_mode$(EXEEXT)
if ENABLE_MPI
dakota_library_split$(EXEEXT): $(top_builddir)/src/dakota_library_split$(EXEEXT)
	$(LN_S) $(top_builddir)/src/dakota_library_split$(EXEEXT) dakota_library_split$(EXEEXT)
endif
endif

#if WITH_GRAPHICS
#plxtnd5.fnt: $(top_srcdir)/packages/plplot/data/plxtnd5.fnt
#	$(LN_S) $(top_srcdir)/packages/plplot/data/plxtnd5.fnt plxtnd5.fnt
#endif

## Define the unit test sources and library dependencies.
## NOTE: Extra Dakota library linked in to resolve circular references.
LOCALCXXFLAGS = $(CPPUNIT_CFLAGS)
LOCALLDFLAGS = $(CPPUNIT_LIBS) $(PTHREAD_CFLAGS)
LOCALLDADD = $(top_builddir)/src/libdakota.la $(ARCHIVES) \
              $(XPM_LIBS) $(X_LIBS) $(X_EXTRA_LIBS) \
              $(X_PRE_LIBS) $(PTHREAD_LIBS) $(LAPACK_LIBS) \
              $(BLAS_LIBS) $(FLIBS) $(FCLIBS)

if WITH_MATLAB
LOCALLDADD += $(MATLAB_LIBS)
LOCALLDFLAGS += $(MATLAB_LDFLAGS)
endif

if WITH_PYTHON
LOCALLDADD += $(PYTHON_LDFLAGS) $(PYTHON_EXTRA_LIBS)
LOCALLDFLAGS += $(PYTHON_EXTRA_LDFLAGS)
endif

if WITH_GSL
LOCALLDADD += $(GSL_LIBS)
LOCALLDFLAGS += $(GSL_LDFLAGS)
endif

if HAVE_CPPUNIT
TESTS = container_unit_tests stream_unit_tests
endif
check_PROGRAMS = $(TESTS) 

container_unit_tests_SOURCES = container_unit_tests.H container_unit_tests.C
container_unit_tests_CXXFLAGS = $(LOCALCXXFLAGS)
container_unit_tests_LDFLAGS = $(LOCALLDFLAGS)
container_unit_tests_LDADD = $(LOCALLDADD)

stream_unit_tests_SOURCES = stream_unit_tests.H stream_unit_tests.C
stream_unit_tests_CXXFLAGS = $(LOCALCXXFLAGS)
stream_unit_tests_LDFLAGS = $(LOCALLDFLAGS)
stream_unit_tests_LDADD = $(LOCALLDADD)

## Note that the DLL tester requires run-time access to the dll_api DLL, 
## currently misnamed dll_api.exe
if WITH_DLL
dll_tester_SOURCES = dll_tester.C
## are all the following libs really necessary?
dll_tester_LDADD = $(LOCALLDADD)
dll_tester_LDFLAGS = $(LOCALLDFLAGS) -L$(top_builddir)/src -ldll_api
endif


## Define the regression test sources and library dependencies.
noinst_PROGRAMS = text_book text_book1 text_book2 text_book3 \
		  text_book_ouu \
		  cantilever mod_cantilever log_ratio short_column \
		  portal_frame steel_section \
		  steel_column_cost steel_column_perf \
		  sobol_rational sobol_g_function sobol_ishigami \
		  rosenbrock lf_rosenbrock \
		  poly_prod lf_poly_prod \
		  cyl_head container barnes barnes_lf \
		  illumination quasi_sine_fcn \
		  mogatest1 mogatest2 mogatest3 \
		  nl2func morris

if !DAKOTA_HOST_OS_AIX
noinst_PROGRAMS += text_book_par
endif
if WITH_DLL
noinst_PROGRAMS += dll_tester
endif

TESTERLDADD = $(PTHREAD_LIBS) $(LAPACK_LIBS) $(BLAS_LIBS) $(FLIBS) $(FCLIBS)

text_book_SOURCES = text_book.C
text_book_LDADD = $(TESTERLDADD)

text_book1_SOURCES = text_book1.C
text_book1_LDADD = $(TESTERLDADD)

text_book2_SOURCES = text_book2.C
text_book2_LDADD = $(TESTERLDADD)

text_book3_SOURCES = text_book3.C
text_book3_LDADD = $(TESTERLDADD)

text_book_ouu_SOURCES = text_book_ouu.C
text_book_ouu_LDADD = $(TESTERLDADD)

text_book_par_SOURCES = text_book_par.C
text_book_par_LDADD = $(TESTERLDADD)

cantilever_SOURCES = cantilever.C
cantilever_LDADD = $(TESTERLDADD)

mod_cantilever_SOURCES = mod_cantilever.C
mod_cantilever_LDADD = $(TESTERLDADD)

log_ratio_SOURCES = log_ratio.C
log_ratio_LDADD = $(TESTERLDADD)

short_column_SOURCES = short_column.C
short_column_LDADD = $(TESTERLDADD)

portal_frame_SOURCES = portal_frame.C
portal_frame_LDADD = $(TESTERLDADD)

steel_section_SOURCES = steel_section.C
steel_section_LDADD = $(TESTERLDADD)

steel_column_cost_SOURCES = steel_column_cost.C
steel_column_cost_LDADD = $(TESTERLDADD)

steel_column_perf_SOURCES = steel_column_perf.C
steel_column_perf_LDADD = $(TESTERLDADD)

sobol_rational_SOURCES = sobol_rational.C
sobol_rational_LDADD = $(TESTERLDADD)

sobol_g_function_SOURCES = sobol_g_function.C
sobol_g_function_LDADD = $(TESTERLDADD)

sobol_ishigami_SOURCES = sobol_ishigami.C
sobol_ishigami_LDADD = $(TESTERLDADD)

rosenbrock_SOURCES = rosenbrock.C
rosenbrock_LDADD = $(TESTERLDADD)

lf_rosenbrock_SOURCES = lf_rosenbrock.C
lf_rosenbrock_LDADD = $(TESTERLDADD)

poly_prod_SOURCES = poly_prod.C
poly_prod_LDADD = $(TESTERLDADD)

lf_poly_prod_SOURCES = lf_poly_prod.C
lf_poly_prod_LDADD = $(TESTERLDADD)

cyl_head_SOURCES = cyl_head.C
cyl_head_LDADD = $(TESTERLDADD)

container_SOURCES = container.C
container_LDADD = $(TESTERLDADD)

barnes_SOURCES = barnes.C
barnes_LDADD = $(TESTERLDADD) $(FLIBS)

barnes_lf_SOURCES = barnes_lf.C
barnes_lf_LDADD = $(TESTERLDADD)

illumination_SOURCES = illumination.C
illumination_LDADD = $(TESTERLDADD)

quasi_sine_fcn_SOURCES = quasi_sine_fcn.C
quasi_sine_fcn_LDADD = $(TESTERLDADD)

mogatest1_SOURCES = mogatest1.C
mogatest1_LDADD = $(TESTERLDADD)

mogatest2_SOURCES = mogatest2.C
mogatest2_LDADD = $(TESTERLDADD)

mogatest3_SOURCES = mogatest3.C
mogatest3_LDADD = $(TESTERLDADD)

nl2func_SOURCES = nl2func.C nl2test.C
nl2func_LDADD = $(TESTERLDADD)

morris_SOURCES = morris.C
morris_LDADD =  $(TESTERLDADD)


## Need to add the Dakota/src directory to our list of header directories.
INCLUDES += -I$(top_srcdir)/src

## Define what files in this directory and below are scripts.
dist_noinst_SCRIPTS = acro_test.pl \
		      container.script \
		      test_3pc_ac \
		      test_3pc_if \
		      test_3pc_of \
		      acro/dakota_test \
		      dakota_diff.perl \
		      dakota_test.perl
#		      dakota_update_input.perl 

## Define the files which should be in the distribution.
include $(srcdir)/Makefile.dist

## Remove symbolic links and test output files on 'make clean'.
CLEANFILES = $(ALL_LOCAL_LINKS) S4 SCHEME LHS_* fort.* *.log *.mod machines \
	     dakota*.out dakota*.out.[1-9]* dakota*.rst dakota*.rst.[1-9]* \
	     dakota*.tst data_test.bin dakota_tabular.dat HIFI.* LOFI.* \
	     discards.dat final*.dat

## Remove Makefile.in on 'make maintainer-clean'.
MAINTAINERCLEANFILES = Makefile.in
