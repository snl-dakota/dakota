#! /bin/bash
#
# Autootols bootstrap script for PLplot
#
# Copyright (C) 2003, 2004  Alan W. Irwin
# Copyright (C) 2003  Joao Cardoso
# Copyright (C) 2003, 2004  Rafael Laboissiere
#
# This file is part of PLplot.
#
# PLplot is free software; you can redistribute it and/or modify
# it under the terms of the GNU Library General Public License as published
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

# Filter "underquoted definitions" warnings from aclocal output.  This
# should not be necessary in teh distant future (today is 2004-02),
# when all the third party m4 macros will be properly quoted.

function filter {
  fgrep -v underquoted | fgrep -v "run info" | fgrep -v sources.redhat
  return 0
}

function run {
  echo -n Running `$1 --version | sed 1q`...
  $* 2>&1 | filter
  echo " done"
}

function usage {
  cat <<EOF
Usage: $0 [OPTIONS]
Options:
  --version=VER
  --date-version
  --aclocal-incdir=DIR
  --help
Note:
  Option --aclocal-incdir can be used more than one time.
EOF
  exit 0
}

# Check for Automake version >= 1.8.2
automake --version | sed 1q \
  | perl -ne '/((\d+)\.(\d+)(\.(\d+))?)/; \
      die "'$0': Automake version is $1.  Version 1.8.2 or later is needed\n"
        if $2<1 or ($2==1 and $3<8) or ($2==1 and $3==8 and $5<2); \
      exit 0' || exit 1

version=""
date_version=no
set_date=no
aclocal_incdir="-I cf"

while test $# -gt 0 ; do
  case $1 in
  --version=*)
    version=`echo "$1" | sed 's/[-_a-zA-Z0-9]*=//'`
    set_date=yes
    ;;
  --date-version)
    date_version=yes
    set_date=yes
    ;;
  --aclocal-incdir=*)
    val=`echo "$1" | sed 's/[-_a-zA-Z0-9]*=//'`
    test -z "$val" || aclocal_incdir="$aclocal_incdir -I $val"
    ;;
  --help)
    usage
    ;;
  *)
    ;;
  esac
  shift
done

curver=`grep ^AC_INIT configure.ac \
        | perl -ne 'if (/plplot, (\d+\.\d+\.\d+)/) {print $1}'`

if [ $date_version = yes ] ; then
  version=$curver.cvs.`date '+%Y%m%d'`
fi

if [ -n "$version" ] ; then
  echo -n "Patching configure.ac (version $version)... "
  perl -pi -e \
    's/^(AC_INIT\(plplot, )([^,)]+)/${1}'$version'/' \
    configure.ac
  echo done
fi

if [ $set_date = yes ] ; then
  date=`date +%Y-%m-%d`
  echo -n "Patching configure.ac (release date $date)... "
  perl -pi -e 's/^(RELEASE_DATE=).*/${1}'$date'/' configure.ac
  echo done
fi

# The following lines give redundant informations and are commented out
# for now, until we decide the best way to present the program versions.
#
# echo "Autotools versions are the following:"
# autoconf --version |sed 1q
# automake --version |sed 1q
# libtool  --version |sed 1q

# We use the specific commands below rather than the recommended autoreconf
# because the particular way we use libltdl demands the
# --ltdl option for libtoolize.  We may change the way we use libltdl in
# the future in which case we should be able to replace the commands below
# with "run autoreconf -vfi -I cf".

run aclocal $aclocal_incdir \
  && run autoheader \
  && rm -rf libltdl \
  && run libtoolize --force --copy --ltdl --automake \
  && run automake --add-missing --copy \
  && run autoconf \
  && ( echo -n "Regenerating libltdl/aclocal+configure..."; \
       cd libltdl ; \
       aclocal 2>&1 | filter && \
       automake ; \
       if [ ! -e configure.ac ] ; then \
           cp configure.in configure.ac ; \
           autoconf 2>/dev/null ; \
           rm -f configure.ac ; \
       else \
           autoconf 2>/dev/null ; \
       fi && \
       echo " done" )
