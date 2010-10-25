#!/bin/sh
# Run *one* python plplot example non-interactively.
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
# ./test_single_python.sh 01
# n.b. must use two digits to specify any number between 01 and 19, to get
# any of the first, through 19th examples.

python <<EOF

# Append to effective python path so that can find plplot modules.
from plplot_python_start import *

import sys
import os

from plplot import *

plsfnam("x$1p.ps")
plsdev("psc")
# Initialize plplot
plinit()

import xw$1

# Terminate plplot
plend()
EOF
