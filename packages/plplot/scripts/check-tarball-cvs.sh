#/bin/sh -e

# Copyright (C) 2004  Rafael Laboissiere
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

# First try on a script to check the discrepancies between the distributed
# tarball obtained with "make dist" and the CVS tree.  This script will
# send to stdout a list of files that are present in a freshly checked out
# CVS tree and absent from the tarball.
#
# Run it from the top_builddir directory.
#
# Written by Rafael Laboissiere on 2003-31-01

VERSION=`perl -ne \
           'if(/AC_INIT\(plplot, ([0-9.cvs]+)/){print "$1\n"; last}'\
           < configure.ac`
TOPDIR=plplot-$VERSION
TARBALL=$TOPDIR.tar.gz

make dist > /dev/null
cvs co plplot > /dev/null

for f in `find plplot -type f | sed 's|^plplot/||' \
          | fgrep -v CVS | fgrep -v .cvsignore` ; do
  if [ -z "`tar tfz $TARBALL $TOPDIR/$f 2>/dev/null`" ] ; then
    echo $f
  fi
done

rm -rf $TARBALL plplot
