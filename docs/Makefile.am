## Process this file with automake to produce 'Makefile.in'.

## Add all the necessary m4 directories to aclocal's search path.
ACLOCAL_AMFLAGS = -I m4

## Dev and Ref at this level; recurse into latex-user and latex-theory
SUBDIRS = latex-user latex-theory

## These files should be deleted on 'make maintainer-clean'.
MAINTAINERCLEANFILES = aclocal.m4 configure install-sh missing Makefile.in


## Builds reference and developer html, latex, and man
doxygen-dev-ref: Dev_*.dox Ref_*.dox
	doxygen Doxyfile && \
	doxygen Doxyfile-ref && \
	doxygen Doxyfile && \
	doxygen Doxyfile-ref && \
	./latex_modify.perl

## Modification to prevent buffer overrun with references
## Increase save_size by a factor of 10
texmf.cnf:
	texmf_cnf_src=/usr/share/texmf/web2c/texmf.cnf ; \
	if [ ! -f $@ -a -f $$texmf_cnf_src ]; then \
	  curr_save_size=`grep -o "save_size = [0-9]*" $$texmf_cnf_src | cut -f2 -d'='` ; \
	  new_save_size=`expr 10 \* $$curr_save_size` ; \
	  sed -e "s/save_size = [0-9]\+/save_size = $$new_save_size/g" $$texmf_cnf_src > $@ ; \
	fi


## Couldn't get explicit deps on latex-ref and latex-dev working here;
## challenging due to cyclic deps in doxygen refs
pdf-local: texmf.cnf doxygen-dev-ref
	export TEXMFCNF=`pwd`:$$TEXMFCNF && \
	cd latex-dev && make && cd .. && \
	cd latex-ref && make && cd ..


## Requires latex-user/Users_Main.pdf and latex-theory/Theory_Main.pdf
SAND-shell: pdf-local
	export TEXINPUTS=`pwd`:`pwd`/SANDreport:$$TEXINPUTS && \
	cd latex-user && make pdf && cd .. && \
	$(pdflatex) SAND-shell-dev && \
	$(pdflatex) SAND-shell-ref && \
	$(pdflatex) SAND-shell-user && \
	$(pdflatex) SAND-shell-theory


clean-local:
	-rm -rf html-dev html-ref latex-dev latex-ref man-dev man-ref texmf.cnf

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
