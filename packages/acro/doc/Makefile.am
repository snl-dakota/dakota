EXTRA_DIST = configure.ps \
	header.txt\
        uguide/abstract.tex\
	uguide/acknowledgements.dox\
	uguide/acro.jpg\
	uguide/acro.tif\
	uguide/configure.dox\
	uguide/Doxyfile.html\
	uguide/Doxyfile.latex\
	uguide/footer.html\
	uguide/header.html\
	uguide/header.tex\
	uguide/install.dox\
	uguide/intro.dox\
	uguide/licenseAppendix.dox\
	uguide/license.dox\
	uguide/main.dox\
	uguide/Makefile\
	uguide/packages.dox\
	uguide/refs.dox\
	uguide/software.dox\
	uguide/uguide.tex\
	uguide/userman.pdf\
	uguide/html/acknowledgements.html\
	uguide/html/configurationAppendix.html\
	uguide/html/index.html\
	uguide/html/install.html\
	uguide/html/intro.html\
	uguide/html/licensesAppendix.html\
	uguide/html/licenses.html\
	uguide/html/packages.html\
	uguide/html/pages.html\
	uguide/html/refs.html\
	uguide/html/software.html


MAINTAINERCLEANFILES = Makefile.in


##
## Makefile for documentation, using the doxygen tool
##

DIRS=uguide refman


doc-ps:
	@(for DIR in ${DIRS}"" ; do			\
	   if [ -d "$${DIR}/Makefile" ] ; then		\
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

