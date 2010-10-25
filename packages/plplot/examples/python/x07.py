#!/usr/bin/env python
#
#	x07c.c
#
#	Font demo.

import pl
import sys
import os

module_dir = "@MODULE_DIR@"

if module_dir[0] == '@':
	module_dir = os.getcwd ()

sys.path.insert (0, module_dir)

# main
#
# Displays the entire "plsym" symbol (font) set.

def main(w):

    base = [0, 200, 500, 600, 700, 800, 900, 2000, 2100,
	    2200, 2300, 2400, 2500, 2600, 2700, 2800, 2900]

##    # Parse and process command line arguments
##
##    pl.ParseOpts(sys.argv, pl.PARSE_FULL)
##
##    # Initialize plplot
##
##    pl.init()

    w.plfontld(1)

    for l in range(17):
	w.pladv(0)

	# Set up viewport and window

	w.plcol(2)
	w.plvpor(0.15, 0.95, 0.1, 0.9)
	w.plwind(0.0, 1.0, 0.0, 1.0)

	# Draw the grid using plbox

	w.plbox("bcg", 0.1, 0, "bcg", 0.1, 0)

	# Write the digits below the frame

	w.plcol(15)
	for i in range(10):
	    w.plmtex("b", 1.5, (0.1 * i + 0.05), 0.5, `i`)

	k = 0
	for i in range(10):

	    # Write the digits to the left of the frame

	    text = `base[l] + 10 * i`
	    w.plmtex("lv", 1.0, (0.95 - 0.1 * i), 1.0, text)

	    for j in range(10):
		x = [ 0.1 * j + 0.05 ]
		y = [ 0.95 - 0.1 * i ]

		# Display the symbols

		w.plsym(x, y, base[l] + k)
		k = k + 1

	w.plmtex("t", 1.5, 0.5, 0.5, "PLplot Example 7 - PLSYM symbols")

	w.pleop()

##	pl.end()
##
##main()
