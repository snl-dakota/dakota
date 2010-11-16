## Process this file with automake to produce 'Makefile.in'.

## Initialize the SUBDIRS variable; it'll be added to below.
SUBDIRS = nidr

if BUILD_TEUCHOS
SUBDIRS += teuchos
endif

## Package subdirectories to recurse into.
if WITH_AMPL
SUBDIRS += ampl
endif

if WITH_GRAPHICS
SUBDIRS += motif
endif

##if WITH_PECOS
SUBDIRS += pecos
##endif

## Need some AC_SUBST magic to handle optional directories like Surfpack.
SUBDIRS += $(MAYBE_SURFPACK)

## Need some AC_SUBST magic to handle optional directories like acro.
SUBDIRS += $(MAYBE_ACRO)

if WITH_CONMIN
SUBDIRS += CONMIN
endif

## Need some AC_SUBST magic to handle optional directories like DDACE.
SUBDIRS += $(MAYBE_DDACE)

if WITH_DOT
SUBDIRS += $(MAYBE_DOT)
endif

if WITH_FSUDACE
SUBDIRS += FSUDace
endif

## Need some AC_SUBST magic to handle optional directories like HOPSPACK.
SUBDIRS += $(MAYBE_APPSPACK)

## Need some AC_SUBST magic to handle optional directories like DDACE.
SUBDIRS += $(MAYBE_JEGA)

if WITH_NCSU
SUBDIRS += NCSUOpt
endif

if WITH_NL2SOL
SUBDIRS += NL2SOL
endif

if WITH_NLPQL
SUBDIRS += $(MAYBE_NLPQL)
endif

if WITH_NPSOL
SUBDIRS += $(MAYBE_NPSOL)
endif

## Need some AC_SUBST magic to handle optional directories like OPTPP.
SUBDIRS += $(MAYBE_OPTPP)

if WITH_PSUADE
SUBDIRS += PSUADE
endif


## Conditionally configured subdirectories should _not_ be recursed into
## during 'make distclean' or 'make maintainer-clean'.
DIST_SUBDIRS = $(SUBDIRS)

## These files should be deleted on 'make maintainer-clean'.
MAINTAINERCLEANFILES = Makefile.in

## copy Boost headers to include directory
install-data-local: ${srcdir}/boost
## No good: copies .svn directories if this is a checkout
##	cp -r $< $(DESTDIR)$(includedir)
## No good: attemped for out of source, but retains Dakota/packages/boost
##	tar c --exclude ".svn" -f - $< | (cd $(DESTDIR)$(includedir); tar xf -)
## This would be simplified by tar --strip-components or --strip-path, but the
## usage differs between tar 1.14 and 1.14.90+; also simpler with rsync
## This seems to work for in- and out of-source builds; REQUIRES GNU tar
	( cd ${srcdir}/boost && tar c --exclude ".svn" -f - boost | ( cd $(DESTDIR)$(includedir); tar xf - ) )
