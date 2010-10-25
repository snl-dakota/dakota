#!/usr/bin/env python
#
#	x01c.c
#
#	Simple line plot and multiple windows demo.

import math
#import pl
import sys
import os

module_dir = "@MODULE_DIR@"

if module_dir[0] == '@':
	module_dir = os.getcwd ()

sys.path.insert (0, module_dir)

from Numeric import *
from pl import *

# main
#
# Generates several simple line plots.  Demonstrates:
#   - subwindow capability
#   - setting up the window, drawing plot, and labelling
#   - changing the color
#   - automatic axis rescaling to exponential notation
#   - placing the axes in the middle of the box
#   - gridded coordinate axes

def main(w):

	global xscale, yscale, xoff, yoff

	# plplot initialization
	# Divide page into 2x2 plots unless user overrides

##	pl.ssub(2, 2)
##
##	# Parse and process command line arguments
##
##	pl.ParseOpts(sys.argv, pl.PARSE_FULL)
##
##	# Get version number, just for kicks
##
##	print "PLplot library version:", pl.gver()
##
##	# Initialize plplot
##
##	pl.init()

	# Set up the data
	# Original case

	xscale = 6.
	yscale = 1.
	xoff = 0.
	yoff = 0.

	# Do a plot

	plot1(w)
 
	# Set up the data

	xscale = 1.
	yscale = 0.0014
	yoff = 0.0185

	# Do a plot

	digmax = 5
	plsyax(digmax, 0)
	plot1(w)

	plot2(w)

	plot3(w)

	# Let's get some user input

##	while 1:
##		plc = pl.GetCursor()
##		if len(plc) == 4:
##			pl.text()
##			print " wx =", plc[2]
##			print " wy =", plc[3]
##			print " vx =", plc[0]
##			print " vy =", plc[1]
##			pl.gra()
##			continue
##		else:
##			break
			

	# Don't forget to call pl.end to finish off!

	# Uh, I don't think  so.

# ===============================================================

def plot1(w):

	global x, y, xscale, yscale, xoff, yoff, xs, ys

##	x = []
##	y = []
##	for i in range(60):
##		x.append(xoff + xscale * (i + 1) / 60.0)
##		y.append(yoff + yscale * pow(x[i], 2.))

	x = zeros(60,'d'); y = zeros(60,'d')
	for i in range(60):
	    x[i] = xoff + xscale * (i + 1) / 60.0
	    y[i] = yoff + yscale * pow(x[i], 2.)

	xmin = x[0]
	xmax = x[59]
	ymin = y[0]
	ymax = y[59]

##	xs = []
##	ys = []
##	for i in range(6):
##		xs.append(x[i * 10 + 3])
##		ys.append(y[i * 10 + 3])

	xs = zeros(6,'d'); ys = zeros(6,'d')
	for i in range(6):
	    xs[i] = x[i * 10 + 3]
	    ys[i] = y[i * 10 + 3]

	# Set up the viewport and window using pl.env. The range in X
	# is 0.0 to 6.0, and the range in Y is 0.0 to 30.0. The axes
	# are scaled separately (just = 0), and we just draw a
	# labelled box (axis = 0).

	w.plcol(1)
	w.plenv(xmin, xmax, ymin, ymax, 0, 0)
	w.plcol(2)
	w.pllab("(x)", "(y)", "#frPLplot Example 1 - y=x#u2")

	# Plot the data points

	w.plcol(4)
	w.plpoin(xs, ys, 9)

	# Draw the line through the data

	w.plcol(3)
	w.plline(x, y)

	w.pleop()

# ===============================================================

def plot2(w):

	global x, y, xscale, yscale, xoff, yoff, xs, ys

	# Set up the viewport and window using pl.env. The range in X
	# is -2.0 to 10.0, and the range in Y is -0.4 to 2.0. The axes
	# are scaled separately (just = 0), and we draw a box with
	# axes (axis = 1).

##	pl.col(1)
##	pl.env(-2.0, 10.0, -0.4, 1.2, 0, 1)
##	pl.col(2)
##	pl.lab("(x)", "sin(x)/x", "#frPLplot Example 1 - Sinc Function")
	w.plcol(1)
	w.plenv(-2.0, 10.0, -0.4, 1.2, 0, 1)
	w.plcol(2)
	w.pllab("(x)", "sin(x)/x", "#frPLplot Example 1 - Sinc Function")

	# Fill up the arrays

##	x = []
##	y = []
##	for i in range(100):
##		x.append((i - 19.0) / 6.0)
##		if x[i] == 0.0:
##			y.append(1.0)
##		else:
##			y.append(math.sin(x[i]) / x[i])

	x = zeros(100,'d'); y = zeros(100,'d')
	for i in range(100):
	    x[i] = (i - 19.0) / 6.0
	    if x[i] == 0.0:
		y[i] = 1.
	    else:
		y[i] = math.sin(x[i]) / x[i]

	# Draw the line

##	pl.col(3)
##	pl.line(x, y)
	w.plcol(3)
	w.plline(x, y)

	w.pleop()

# ===============================================================

def plot3(w):

	global x, y, xscale, yscale, xoff, yoff, xs, ys

	# For the final graph we wish to override the default tick
	# intervals, so do not use pl.env

	w.pladv(0)

	# Use standard viewport, and define X range from 0 to 360
	# degrees, Y range from -1.2 to 1.2.

	w.plvsta()
	w.plwind(0.0, 360.0, -1.2, 1.2)

	# Draw a box with ticks spaced 60 degrees apart in X, and 0.2 in Y.

	w.plcol(1)
	w.plbox("bcnst", 60.0, 2, "bcnstv", 0.2, 2)

	# Superimpose a dashed line grid, with 1.5 mm marks and spaces. 
	# plstyl expects a pointer!! 

	#w.plstyl([1500], [1500])
	w.plcol(2)
	w.plbox("g", 30.0, 0, "g", 0.2, 0)
	#w.plstyl([], [])

	w.plcol(3)
	w.pllab("Angle (degrees)", "sine",
	       "#frPLplot Example 1 - Sine function")

##	x = []
##	y = []
##	for i in range(101):
##		x.append(3.6 * i)
##		y.append(math.sin(x[i] * math.pi / 180.0))
	x = zeros(101,'d'); y = zeros(101,'d')
	for i in range(101):
	    x[i] = 3.6 * i
	    y[i] = math.sin(x[i] * math.pi / 180.0)

	w.plcol(4)
	w.plline(x, y)

	w.pleop()

## Do something to make the demo run if not under the Tk demo gui.
## Something about the if __name__ == __main__ business, but need to
## cobble together a "loopback" widget or some such, so it will work.  
