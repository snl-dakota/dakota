#!/usr/bin/env python
#
#	x13c.c
#
#	Pie chart demo.

import math
#import pl
import sys
import os

module_dir = "@MODULE_DIR@"

if module_dir[0] == '@':
	module_dir = os.getcwd ()

sys.path.insert (0, module_dir)

text = ["Maurice",
	"Randy",
	"Mark",
	"Steve",
	"Warner"]

# main
#
# Does a simple pie chart.

def main(w):

##    # Parse and process command line arguments
##
##    pl.ParseOpts(sys.argv, pl.PARSE_FULL)
##
##    # Initialize plplot
##
##    pl.init()

    w.plenv(0., 10., 0., 10., 1, -2)
    w.plcol(2)

    per = [10, 32, 12, 30, 16]
    theta0 = 0.
    dthet = 2 * math.pi / 500

    for i in range(5):
	x = [5.]
	y = [5.]
	theta1 = theta0 + 2 * math.pi * per[i] / 100.
	if i == 4:
	    theta1 = 2 * math.pi

	theta = theta0
	while theta <= theta1:
	    x.append(5 + 3 * math.cos(theta))
	    y.append(5 + 3 * math.sin(theta))
	    theta = theta + dthet

	w.plcol(i + 1)
	w.plpsty((i + 3) % 8 + 1)
	w.plfill(x, y)
	w.plcol(1)
	w.plline(x, y)
	just = (theta0 + theta1) / 2.
	dx = .25 * math.cos(just)
	dy = .25 * math.sin(just)
	if just < math.pi / 2 or just > 3 * math.pi / 2:
	    just = 0.
	else:
	    just = 1.

	mid = len(x) / 2
	w.plptex((x[mid] + dx), (y[mid] + dy), 1.0, 0.0, just, text[i])
	theta0 = theta - dthet

    w.plfont(2)
    w.plschr(0., 1.3)
    w.plptex(5.0, 9.0, 1.0, 0.0, 0.5, "Percentage of Sales")

    # Don't forget to call PLEND to finish off!

    w.pleop()
##    pl.end()
##
##main()
