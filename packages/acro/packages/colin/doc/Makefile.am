EXTRA_DIST=\
header.txt\
refman/Doxyfile.latex\
refman/Doxyfile.html\
refman/footer.html\
refman/header.html\
refman/header-ref.tex\
refman/Makefile



MAINTAINERCLEANFILES = Makefile.in


##
## Makefile for documentation, using the doxygen tool
##

DIRS=refman uguide


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

