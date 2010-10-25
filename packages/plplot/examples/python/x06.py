#!/usr/bin/env python
#
#	x06c.c
#
#	Font demo.

#import pl
import sys
import os

module_dir = "@MODULE_DIR@"

if module_dir[0] == '@':
	module_dir = os.getcwd ()

sys.path.insert (0, module_dir)

# main
#
# Displays the entire "plpoin" symbol (font) set.

def main(w):

##    # Parse and process command line arguments
##
##    pl.ParseOpts(sys.argv, pl.PARSE_FULL)
##
##    # Initialize plplot
##
##    pl.init()

    w.pladv(0)

    # Set up viewport and window

    w.plcol(2)
    w.plvpor(0.1, 1.0, 0.1, 0.9)
    w.plwind(0.0, 1.0, 0.0, 1.3)

    # Draw the grid using plbox

    w.plbox("bcg", 0.1, 0, "bcg", 0.1, 0)

    # Write the digits below the frame

    w.plcol(15)
    for i in range(10):
	w.plmtex("b", 1.5, (0.1 * i + 0.05), 0.5, `i`)

    k = 0
    for i in range(13):

	# Write the digits to the left of the frame

	w.plmtex("lv", 1.0, (1.0 - (2 * i + 1) / 26.0), 1.0, `10 * i`)

	for j in range(10):
	    x = 0.1 * j + 0.05
	    y = 1.25 - 0.1 * i

	    # Display the symbol (plpoin expects that x
	    # and y are arrays so pass lists)

	    if k < 128:
		w.plpoin([x], [y], k)
	    k = k + 1

    w.plmtex("t", 1.5, 0.5, 0.5, "PLplot Example 6 - plpoin symbols")
    w.pleop()
##	w.plend()
##
##main()
