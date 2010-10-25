#!/bin/sh
#
# Copyright (C) 2004  Alan W. Irwin
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

# usage:
# ./test_single_tcl.sh 01
# n.b. must use two digits to specify any number between 01 and 18, to get
# any of the first, through 18th examples.
number=$1
shortnumber=$number
if test ${number:0:1} == "0"; then shortnumber=${number:1:1}; fi
./pltcl -dev psc -o x$number\t.ps <<EOF
plinit
source tcldemos.tcl
[format "%d" $shortnumber]
exit
EOF
