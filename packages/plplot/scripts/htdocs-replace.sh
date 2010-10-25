#!/bin/bash

# WARNING: completely replaces htdocs by new version without
# backups other than the cvs repository.
# It is assumed that old versions of htdocs can be obtained using
# cvs export with a different tag or date

DOCBOOKDIR=htdocs/resources/docbook-manual
DEBIANDIR=htdocs/resources/debian
PLOTEXAMPLES=htdocs_plot_examples.tgz

echo "This script is obsolete.  In order to update the PLplot web site,"
echo "Please, use the Makefile in the www CVS module."
echo "(Read the README file for instructions)."

exit 0

cd /home/groups/p/pl/plplot

if [ ! -d $DOCBOOKDIR ] ; then
  echo "Warning: The DocBook directory is missing in the htdocs hierarchy."
  echo "  Install it from the CVS checked out sources of the docbook module."
  DOCBOOKDIR=""
else  
  mv $DOCBOOKDIR docbook-manual-$$
fi  

if [ ! -f $PLOTEXAMPLES ] ; then
    echo "Warning, $PLOTEXAMPLES does not exists."
    echo "Follow the instructions in scripts/htdocs-gen_plot-examples.sh to"
    echo "generate it."
fi

test -d $DEBIANDIR && mv $DEBIANDIR debian-$$

if [ -z "$USER" ] ; then 
  echo -n "Login name for SourceForge CVS: "
  read USER
fi
rm -rf htdocs
CVS_RSH=ssh CVSROOT=$USER@cvs1:/cvsroot/plplot \
  cvs export -r HEAD -d htdocs www
test -n "$DOCBOOKDIR" && mv docbook-manual-$$ $DOCBOOKDIR
test -n "$DEBIANDIR" && mv debian-$$ $DEBIANDIR
test -f $PLOTEXAMPLES && tar xzf htdocs_plot_examples.tgz
chmod -R g=u htdocs
