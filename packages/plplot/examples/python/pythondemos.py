#!/usr/bin/env python

# Copyright (C) 2004  Alan W. Irwin
# Copyright (C) 2004  Andrew Ross
#
# This file is part of PLplot.
#
# PLplot is free software; you can redistribute it and/or modify
# it under the terms of the GNU Library General Public License as published
# by the Free Software Foundation; version 2 of the License.
#
# PLplot is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public License
# along with the file PLplot; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA


# Run all python plplot examples non-interactively.

# Append to effective python path so that can find plplot modules.
from plplot_python_start import *

import sys
from plplot import *

# Parse and process command line arguments
plparseopts(sys.argv, PL_PARSE_FULL)

# Initialize plplot
plinit()

# xw14.py must be standalone so cannot be run from pythondemos.py
# xw17.py must be standalone and interactive so cannot be run from pythondemos.py

failed = []
for i in range(1, 14) + [15, 16, 18, 19, 22]:
    script = 'xw' + '%02d' % i
    try:
	__import__(script, globals(), locals(), [])
    except:
	failed.append(script)

# Terminate plplot
plend()

if len(failed) != 0:
    import string
    print "Failed script(s): " + string.join(failed,", ")
    sys.exit(1)

