## Process this file with automake to produce 'Makefile.in'.

## Add all the necessary m4 directories to aclocal's search path.
ACLOCAL_AMFLAGS = -I m4

## Nothing to do here; recurse into latex-user.
SUBDIRS = latex-user

## These files should be deleted on 'make maintainer-clean'.
MAINTAINERCLEANFILES = aclocal.m4 configure install-sh missing Makefile.in

## This action should take place on 'make distclean'.
distclean-local:
	-rm -fr .dvips/

## These files should be distributed.
EXTRA_DIST = Dev_Recomm_Pract.dox \
	     Dev_Fn_Evals.dox \
	     Dev_Library.dox \
	     Dev_Main.dox \
	     Ref_Interface.dox \
	     Dev_Spec_Change.dox \
	     Doxyfile \
	     Doxyfile-ref \
	     README.latex \
	     Ref_Bibliography.dox \
	     Ref_Intro.dox \
	     Ref_Method.dox \
	     Ref_Main.dox \
	     Ref_Model.dox \
	     Ref_Strategy.dox \
	     Ref_Responses.dox \
	     Ref_Variables.dox \
	     header-ref.tex \
	     cfg-paper.ps \
	     configure.ps \
	     gnu_standards.ps \
	     header-dev.tex \
	     latex_modify.perl \
	     images/logo_3d_halfsize.jpg \
	     images/direct1.eps \
	     images/direct1.jpg \
	     images/logo.jpg \
	     images/logo_3d.jpg \
	     images/logo_halfsize.jpg
