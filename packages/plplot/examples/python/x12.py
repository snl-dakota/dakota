#!/usr/bin/env python
#
#	x12c.c
#
#	Bar chart demo.

import pl
import sys
import os

module_dir = "@MODULE_DIR@"

if module_dir[0] == '@':
	module_dir = os.getcwd ()

sys.path.insert (0, module_dir)

# main
#
# Does a simple bar chart, using color fill.  If color fill is
# unavailable, pattern fill is used instead (automatic).

def main(w):

##    # Parse and process command line arguments
##
##    pl.ParseOpts(sys.argv, pl.PARSE_FULL)
##
##    # Initialize plplot
##
##    pl.init()

    w.pladv(0)
    w.plvsta()
    w.plwind(1980.0, 1990.0, 0.0, 35.0)
    w.plbox("bc", 1.0, 0, "bcnv", 10.0, 0)
    w.plcol(2)
    w.pllab("Year", "Widget Sales (millions)", "#frPLplot Example 12")

    y0 = [5, 15, 12, 24, 28, 30, 20, 8, 12, 3]

    for i in range(10):
	w.plcol(i + 1)
	w.plpsty(0)
	fbox( w, (1980. + i), y0[i] )
	string = `y0[i]`
	w.plptex((1980. + i + .5), (y0[i] + 1.), 1.0, 0.0, .5, string)
	string = `1980 + i`
	w.plmtex("b", 1.0, ((i + 1) * .1 - .05), 0.5, string)

    # Don't forget to call PLEND to finish off!

    w.pleop()

def fbox( w, x0, y0 ):

    x = [x0, x0, x0 + 1., x0 + 1.]
    y = [0., y0, y0, 0.]
    w.plfill(x, y)
    w.plcol(1)
    w.pllsty(1)
    w.plline(x, y)

##main()
