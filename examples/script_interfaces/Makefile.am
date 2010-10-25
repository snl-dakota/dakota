## Process this file with automake to produce 'Makefile.in'.

## We need to descend into subdirectories intro and blackbox (other
## targets are handled here via dist_ and EXTRA_DIST
SUBDIRS = generic

## This file should be removed on 'make maintainer-clean'.
MAINTAINERCLEANFILES = Makefile.in

## These files are scripts and should have appropriate execute permissions.
dist_noinst_SCRIPTS = Abaqus/abaq_driver \
		      Cask/filters/cask_driver_all \
		      Cask/filters/cask_ifilter_all \
		      Cask/filters/cask_ofilter_all \
		      Cask/filters/ke_check \
		      Cask/filters/killn \
		      Nastran/cylinder/nastran_script \
		      Qtran/scripts/ch2.240 \
		      Qtran/scripts/ch2.360 \
		      Qtran/scripts/ch2.480 \
		      Qtran/scripts/killn \
		      Qtran/scripts/killone \
		      Qtran/scripts/killorphan \
		      Qtran/scripts/ll_driver \
		      Qtran/scripts/newqch_driver \
		      Qtran/scripts/qch_driver \
		      Qtran/scripts/qcheck \
		      Qtran/scripts/qcheck.ll \
		      Rsmas/rsmas_driver \
		      Salinas/salinas_script_asv

## These files should be distributed.
EXTRA_DIST = Abaqus/dakota_abaq_opt.in \
	     Abaqus/dakota_abaq_uq.in \
	     Abaqus/fe.dat \
	     Abaqus/fe.inp.app \
	     Abaqus/tpb.gif \
	     Cask/filters/createdim.C \
	     Cask/filters/fildata.f \
	     Cask/filters/getmaxtemp.f \
	     Cask/filters/ke_rebound.f \
	     Cask/filters/strip.f \
	     Cask/filters/weight.C \
	     ModelCenter/include/PHXCppApi.h \
	     ModelCenter/lib/i686-cygwin-cygwinNT/libPHXCppApi.a \
	     ModelCenter/lib/i686-cygwin-cygwinNT/PHXCppApi.dll \
	     ModelCenter/README \
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
	     Rsmas/rsmas.template \
	     Salinas/aff_all_fns.C \
	     Salinas/dakota_aff_lhs.in
