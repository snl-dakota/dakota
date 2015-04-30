EXTRA_DIST = \
header.txt\
figs/GSGA.eps\
figs/GSGA.fig\
figs/GSGA.gif\
refman/Doxyfile.html\
refman/Doxyfile.latex\
refman/footer.html\
refman/header.html\
refman/header-ref.tex\
refman/Makefile\
uguide/abstract.tex\
uguide/acknowledgements.dox\
uguide/Doxyfile.html\
uguide/Doxyfile.latex\
uguide/ea-doc.dox\
uguide/footer.html\
uguide/header.html\
uguide/header.tex\
uguide/install.dox\
uguide/intro.dox\
uguide/main.dox\
uguide/Makefile\
uguide/myoxygen.sty\
uguide/patternsearch-doc.dox\
uguide/refs.dox\
uguide/smc-doc.dox\
uguide/soliswets-doc.dox\
uguide/uguide.tex

MAINTAINERCLEANFILES = Makefile.in


##
## Makefile for documentation, using the doxygen tool
##

DIRS=acli


doc-ps:
	@(for DIR in ${DIRS}"" ; do			\
	   if [ -e "$${DIR}/Makefile" ] ; then		\
	      (cd "$${DIR}" ; $(MAKE) ps);		\
	   fi;						\
	done;)

doc-pdf:
	@(for DIR in ${DIRS}"" ; do			\
	   if [ -e "$${DIR}/Makefile" ] ; then		\
	      (cd "$${DIR}" ; $(MAKE) pdf);		\
	   fi;						\
	done;)

doc-html:
	@(for DIR in ${DIRS}"" ; do			\
	   if [ -e "$${DIR}/Makefile" ] ; then		\
	      (cd "$${DIR}" ; $(MAKE) html);		\
	   fi;						\
	done;)

clean-local:
	@(for DIR in ${DIRS}"" ; do			\
	   if [ -e "$${DIR}/Makefile" ] ; then		\
	      (cd "$${DIR}" ; $(MAKE) clean);		\
	   fi;						\
	done;)

docs: doc-html

