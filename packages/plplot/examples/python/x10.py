#!/usr/bin/env python
#
#	x10c.c
#
#	Window positioning demo.

import pl
import sys
import os

module_dir = "@MODULE_DIR@"

if module_dir[0] == '@':
	module_dir = os.getcwd ()

sys.path.insert (0, module_dir)

# main
#
# Demonstrates absolute positioning of graphs on a page.

def main(w):

##    # Parse and process command line arguments
##
##    pl.ParseOpts(sys.argv, pl.PARSE_FULL)
##
##    # Initialize plplot
##
##    pl.init()

    w.pladv(0)
    w.plvpor(0.0, 1.0, 0.0, 1.0)
    w.plwind(0.0, 1.0, 0.0, 1.0)
    w.plbox("bc", 0.0, 0, "bc", 0.0, 0)

    w.plsvpa(50.0, 150.0, 50.0, 100.0)
    w.plwind(0.0, 1.0, 0.0, 1.0)
    w.plbox("bc", 0.0, 0, "bc", 0.0, 0)
    w.plptex(0.5, 0.5, 1.0, 0.0, 0.5, "BOX at (50,150,50,100)")
    w.pleop()

##main()
