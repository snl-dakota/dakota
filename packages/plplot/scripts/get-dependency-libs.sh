#!/bin/sh
#
# $Id: get-dependency-libs.sh 3186 2006-02-15 18:17:33Z slbrow $
# Generate appropriate LIBS line for pkg-config by parsing libtool *.la files
#
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


if test $# != 1 ; then
    echo 2>1 "Usage: $0 path/to/libfoo.la"
    exit 1
fi

result=

for lib in `grep ^dependency_libs= $1 \
               | sed 's/^dependency_libs=//' | sed "s/'//g"` ; do

    case $lib in

        -l* | -L* )
            result="$result $lib"
        ;;

        *.la)
            path=`echo $lib | sed 's:/*lib[^/]*.la::'`
            libname=`echo $lib | sed 's:^.*/lib::' | sed 's/\.la//'`
            result="$result -L$path"
            test -d $path/.libs && result="$result -L$path/.libs"
            result="$result -l$libname"
        ;;

    esac

done

echo $result
