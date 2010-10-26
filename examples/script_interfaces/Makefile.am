## Process this file with automake to produce 'Makefile.in'.

## We need to descend into subdirectories intro and blackbox (other
## targets are handled here via dist_ and EXTRA_DIST
SUBDIRS = generic

## This file should be removed on 'make maintainer-clean'.
MAINTAINERCLEANFILES = Makefile.in

## These files are scripts and should have appropriate execute permissions.
dist_noinst_SCRIPTS = Abaqus/abaq_driver \
		      Nastran/cylinder/nastran_script \
		      Rsmas/rsmas_driver

## These files should be distributed.
EXTRA_DIST = Abaqus/dakota_abaq_opt.in \
	     Abaqus/dakota_abaq_uq.in \
	     Abaqus/fe.dat \
	     Abaqus/fe.inp.app \
	     Abaqus/tpb.gif \
	     Nastran/cylinder/dakota_nastran_opt.in \
	     Nastran/cylinder/dakota_nastran_ps.in \
	     Nastran/cylinder/dakota_nastran_uq.in \
	     Nastran/cylinder/modal.bdf \
	     Nastran/cylinder/modal.f06 \
	     Nastran/cylinder/modal.gif \
	     Nastran/cylinder/readme.txt \
	     Nastran/cylinder/templatedir \
	     Nastran/cylinder/templatedir/modal.bdf.template \
	     Rsmas/dakota_rsmas_ga_opt.in \
	     Rsmas/dakota_rsmas_grad_opt.in \
	     Rsmas/dakota_rsmas_pstudy.in \
	     Rsmas/rsmas.f \
	     Rsmas/rsmas.in \
	     Rsmas/rsmas.out \
	     Rsmas/rsmas.template