#!/bin/bash

# This will prepare a distribution tarbal directly from the CVS sources of
# the PLplot project.
# CAVEAT: this script silently and completely deletes a directory called
# plplot-cvs-tarball existent in the current working directory.

# Copyright (C) 2003, 2004  Rafael Laboissiere
#
# This file is part of PLplot.
#
# PLplot is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Library Public License as published
# by the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# PLplot is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public License
# along with PLplot; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

# $Id: make-cvs-tarball.sh 3186 2006-02-15 18:17:33Z slbrow $

usage () {
  local prog=`basename $0`
  echo "Usage: $prog [-n] [-u user] [-t tmpdir] [-w remote dir] \\"
  echo "          [-r branch] [-v version] [-c] [-i prefix] \\"
  echo "          [-p config_prefix] [-o config_opts]"
  echo "       $prog -d"
  echo "       $prog -h"
  echo
  echo "Option -n prevents building of the DocBook manual."
  echo "Option -d prints the default values."
  echo "Option -o allows specifying additional configuration options."
  echo "Option -p allows specifying the prefix during preparation."
  echo "When option -v is not given, a tarball is produced with version and"
  echo "  label containing today's date string."
  echo "When option -c is given, the tarball is unpacked and make check"
  echo "  is run.  For exercising make install, use option -i."
  exit $1
}

DOC_ARG=${DOC_ARG:---enable-builddoc}
WWW_USER=${WWW_USER:-tomduck}
CVSROOTDIR=${CVSROOTDIR:-cvs.sourceforge.net:/cvsroot/plplot}
VERSION=${VERSION:+--version=$1}
BRANCH=${BRANCH:--D now}
CVSTMPDIR=${CVSTMPDIR:-plplot-cvs-tarball}

# Put here extra configure options that should always be used when
# generating a tarball
config_opt=""

print_defaults () {
  local v
  for v in DOC_ARG WWW_USER CVSROOTDIR VERSION BRANCH CVSTMPDIR ; do
    eval "echo $v=\\\"\$$v\\\""
  done
  exit 0
}

bypass_rtag=no
do_check=no
prefix=""
preparation_prefix=""

while getopts "bcdhi:no:p:r:t:u:v:w:" option
do
  case $option in
    b) bypass_rtag=yes ;;
    c) do_check=yes ;;
    d) print_defaults ;;
    h) usage 0 ;;
    i) test -n "$OPTARG" || usage 1 ; prefix=$OPTARG ;;
    n) DOC_ARG= ;;
    o) config_opt="$config_opt $OPTARG" ;;
    p) test -n "$OPTARG" || usage 1 ; preparation_prefix="--prefix $OPTARG" ;;
    r) test -n "$OPTARG" || usage 1 ; BRANCH="-r $OPTARG" ;;
    t) test -n "$OPTARG" || usage 1 ; CVSTMPDIR=$OPTARG ;;
    u) test -n "$OPTARG" || usage 1 ; WWW_USER=$OPTARG ;;
    v) test -n "$OPTARG" || usage 1 ; VERSION="--version=$OPTARG" ;;
    w) test -n "$OPTARG" || usage 1 ; CVSROOTDIR=$OPTARG ;;
    *) usage 1 ;;
  esac
done

clean_prefix=yes
test -d "$prefix" && clean_prefix=no

cleanup ( ) {
    rm -rf $CVSTMPDIR
}

trap "cleanup" 0 HUP INT QUIT PIPE TERM

cleanup

test $bypass_rtag = yes \
  || date_tag=cvs-tarball_`date --utc +%Y-%m-%d-%H-%M-%S` \
  && echo "Tagging repository with $date_tag" \
  && cvs -d${WWW_USER}@$CVSROOTDIR rtag $BRANCH $date_tag plplot \
  && BRANCH="-r $date_tag"

cvs -d${WWW_USER}@$CVSROOTDIR export -d$CVSTMPDIR $BRANCH plplot \
  && cd $CVSTMPDIR \
  && cf/bootstrap.sh ${VERSION:---date-version} \
  && ./configure $preparation_prefix $DOC_ARG $config_opt \
  && echo "Making distribution." \
  && make dist \
  && TARBALL=`ls plplot-*.tar.gz` \
  && DISTDIR=`echo $TARBALL | sed s/.tar.gz//` \
  && mv $TARBALL .. \
  && cd .. \
  && echo "CVS distribution tarball: $TARBALL" \
  && test "$do_check" = yes \
  && rm -rf $DISTDIR \
  && tar xfz $TARBALL \
  && ( cd $DISTDIR \
       && ./configure ${prefix:+--prefix=$prefix} $config_opt  \
       && make check \
       && test -n "$prefix" \
       && make install ) ; \
  rm -rf $DISTDIR \
  && test "$clean_prefix" = yes \
  && rm -rf "$prefix"
