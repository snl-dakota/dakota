#!/usr/bin/env python
#
#	x03c.c
#
#	Polar plot demo.

from Numeric import *
import math
#import pl
import sys
import os

module_dir = "@MODULE_DIR@"

if module_dir[0] == '@':
	module_dir = os.getcwd ()

sys.path.insert (0, module_dir)

# main
#
# Generates polar plot, with 1-1 scaling.

def main(w):

    dtr = math.pi / 180.0
    x0 = zeros(361,'d'); y0 = zeros(361,'d')
    for i in range(361):
	x0[i] = math.cos(dtr * i)
	y0[i] = math.sin(dtr * i)

##	# Parse and process command line arguments
##
##	pl.ParseOpts(sys.argv, pl.PARSE_FULL)
##
##	# Initialize plplot
##
##	pl.init()


    w.plcol(1)

    # Set up viewport and window, but do not draw box

    w.plenv(-1.3, 1.3, -1.3, 1.3, 1, -2)

    x = zeros(11*361,'d'); y = zeros(11*361,'d')
    k=0
    for i in range(11):
	for j in range(361):
	    x[k] = 0.1 * i * x0[j]
	    y[k] = 0.1 * i * y0[j]
	    k = k + 1

    # Draw circles for polar grid

    w.plline(x, y)

    w.plcol(2)
    for i in range(12):
	theta = 30.0 * i
	dx = math.cos(dtr * theta)
	dy = math.sin(dtr * theta)

	# Draw radial spokes for polar grid

	w.pljoin(0.0, 0.0, dx, dy)

	# Write labels for angle

	text = `int(theta)`
	if dx >= 0:
	    w.plptex(dx, dy, dx, dy, -0.15, text)
	else:
	    w.plptex(dx, dy, -dx, -dy, 1.15, text)

    # Draw the graph

    x = zeros(361,'d'); y = zeros(361,'d')
    for i in range(361):
	r = math.sin(dtr * (5 * i))
	x[i] = x0[i] * r
	y[i] = y0[i] * r

    w.plcol(3)
    w.plline(x, y)

    w.plcol(4)
    w.plmtex("t", 2.0, 0.5, 0.5, "#frPLplot Example 3 - r(#gh)=sin 5#gh")

    # Close the plot at end

    w.pleop()

##
##	pl.end()
##
##main()
