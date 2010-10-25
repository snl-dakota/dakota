## Process this file with automake to produce 'Makefile.in'.

## Define the programs and scripts we need to build and install.
dist_bin_SCRIPTS = dprepro
dist_noinst_SCRIPTS = dakota_cleanup simulator_script
noinst_PROGRAMS = rosenbrock_bb
rosenbrock_bb_SOURCES = rosenbrock_bb.f

## Create symlinks up to the dakota and rosenbrock binaries.
all-local: dakota$(EXEEXT) rosenbrock$(EXEEXT)

dakota$(EXEEXT): $(top_builddir)/src/dakota$(EXEEXT)
	$(LN_S) $(top_builddir)/src/dakota$(EXEEXT) dakota$(EXEEXT)

rosenbrock$(EXEEXT): $(top_builddir)/test/rosenbrock$(EXEEXT)
	$(LN_S) $(top_builddir)/test/rosenbrock$(EXEEXT) rosenbrock$(EXEEXT)

## This file should be removed on 'make maintainer-clean'.
MAINTAINERCLEANFILES = Makefile.in

## These files should be removed on 'make clean'.
CLEANFILES = dakota rosenbrock

## This file should be distributed.
EXTRA_DIST = templatedir/ros.template
