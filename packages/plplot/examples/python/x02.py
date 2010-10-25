#!/usr/bin/env python
#
#	x02c.c
#
#	Multiple window and color map 0 demo.

#import pl
import sys
import os

module_dir = "@MODULE_DIR@"

if module_dir[0] == '@':
	module_dir = os.getcwd ()

sys.path.insert (0, module_dir)

# Demonstrates multiple windows and default color map 0 palette.

def main(w):

    # Divide screen into 16 regions

    w.plssub(4, 4)

##    # Parse and process command line arguments
##
##    pl.ParseOpts(sys.argv, pl.PARSE_FULL)
##
##    # Initialize plplot
##
##    pl.init()

    w.plschr(0.0, 3.5)
    w.plfont(4)

    for i in range(16):
	w.plcol(i)
	w.pladv(0)
	vmin = 0.1
	vmax = 0.9
	for j in range(3):
	    w.plwid(j + 3)
	    w.plvpor(vmin, vmax, vmin, vmax)
	    w.plwind(0.0, 1.0, 0.0, 1.0)
	    w.plbox("bc", 0.0, 0, "bc", 0.0, 0)
	    vmin = vmin + 0.1
	    vmax = vmax - 0.1
	    w.plwid(1)
	    w.plptex(0.5, 0.5, 1.0, 0.0, 0.5, `i`)

    # Now finish up.

    w.pleop()

    # And revert the screen to one plot per page.

    w.plssub(1,1)

##    pl.end()
##
##main()
