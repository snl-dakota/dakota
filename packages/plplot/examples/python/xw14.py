#!/usr/bin/env python

# Run 14th python plplot example

# Append to effective python path so that can find plplot modules.
from plplot_python_start import *

import sys
from plplot import *

# Parse and process command line arguments
plparseopts(sys.argv, PL_PARSE_FULL)

#	Simple line plot and multiple windows demo.

from Numeric import *
from plplot import *

def main():
    
    geometry_master = "500x410+100+200"
    geometry_slave  = "500x410+650+200"
    driver = plgdev()
    print "Demo of multiple output streams via the %s driver." % driver
    print "Running with the second stream as slave to the first."
    print ""
    
    # Set up the first stream.
    plsetopt("geometry", geometry_master)
    plsdev(driver)
    plssub(2, 2)
    plinit()
    
    # Start next stream.
    plsstrm(1)
    
    # Turn off pause to make this a slave (must follow master)
    
    plsetopt("geometry", geometry_slave)
    plspause(0)
    plsdev(driver)
    plinit()
		    
    # Set up the data & plot
    # Original case

    plsstrm(0)
    
    xscale = 6.
    yscale = 1.
    xoff = 0.
    yoff = 0.
    plot1(xscale, yscale, xoff, yoff)
			
    # Set up the data & plot
			
    xscale = 1.
    yscale = 1.e+6
    plot1(xscale, yscale, xoff, yoff)
				    
    # Set up the data & plot
				    
    xscale = 1.
    yscale = 1.e-6
    digmax = 2
    plsyax(digmax, 0)
    plot1(xscale, yscale, xoff, yoff)
							
    # Set up the data & plot
							
    xscale = 1.
    yscale = 0.0014
    yoff = 0.0185
    digmax = 5
    plsyax(digmax, 0)
    plot1(xscale, yscale, xoff, yoff)
    
    # To slave
    # The pleop() ensures the eop indicator gets lit.
    
    plsstrm(1)
    plot4()
    pleop()
		
    # Back to master
		
    plsstrm(0)
    plot2()
    plot3()
			    
    # To slave
			    
    plsstrm(1)
    plot5()
    pleop()
					
    # Back to master to wait for user to advance
					
    plsstrm(0)
    pleop()
# ===============================================================

def plot1(xscale, yscale, xoff, yoff):

    x = xoff + (xscale/60.)*(1+arrayrange(60))
    y = yoff + yscale*pow(x,2.)

    xmin = x[0]
    xmax = x[59]
    ymin = y[0]
    ymax = y[59]
    
    xs = x[3::10]
    ys = y[3::10]
    
    # Set up the viewport and window using pl.env. The range in X
    # is 0.0 to 6.0, and the range in Y is 0.0 to 30.0. The axes
    # are scaled separately (just = 0), and we just draw a
    # labelled box (axis = 0).
    
    plcol0(1)
    plenv(xmin, xmax, ymin, ymax, 0, 0)
    plcol0(6)
    pllab("(x)", "(y)", "#frPLplot Example 1 - y=x#u2")
    
    # Plot the data points
    
    plcol0(9)
    plpoin(xs, ys, 9)
    
    # Draw the line through the data
    
    plcol0(4)
    plline(x, y)
    plflush()
    
# ===============================================================

def plot2():

    # Set up the viewport and window using pl.env. The range in X
    # is -2.0 to 10.0, and the range in Y is -0.4 to 2.0. The axes
    # are scaled separately (just = 0), and we draw a box with
    # axes (axis = 1).
    
    plcol0(1)
    plenv(-2.0, 10.0, -0.4, 1.2, 0, 1)
    plcol0(2)
    pllab("(x)", "sin(x)/x", "#frPLplot Example 1 - Sinc Function")

    # Fill up the arrays

    x = (arrayrange(100)-19)/6.0
    if 0.0 in x:
	#use index method on x if/when Numeric provides it.
	#replace 0.0 by small value that gives the same sinc(x) result.
	x[list(x).index(0.0)] = 1.e-30
    y = sin(x)/x

    # Draw the line

    plcol0(3)
    plline(x, y)
    plflush()

# ===============================================================

def plot3():

    # For the final graph we wish to override the default tick
    # intervals, so do not use pl.env

    pladv(0)

    # Use standard viewport, and define X range from 0 to 360
    # degrees, Y range from -1.2 to 1.2.

    plvsta()
    plwind(0.0, 360.0, -1.2, 1.2)

    # Draw a box with ticks spaced 60 degrees apart in X, and 0.2 in Y.

    plcol0(1)
    plbox("bcnst", 60.0, 2, "bcnstv", 0.2, 2)

    # Superimpose a dashed line grid, with 1.5 mm marks and spaces. 
    # plstyl expects a pointer!! 

    plstyl([1500], [1500])
    plcol0(2)
    plbox("g", 30.0, 0, "g", 0.2, 0)
    plstyl([], [])

    plcol0(3)
    pllab("Angle (degrees)", "sine", "#frPLplot Example 1 - Sine function")

    x = 3.6*arrayrange(101)
    y = sin((pi/180.)*x)

    plcol0(4)
    plline(x, y)
    plflush

# ===============================================================

def plot4():

    dtr = pi / 180.0
    x0 = cos(dtr*arrayrange(361))
    y0 = sin(dtr*arrayrange(361))

    # Set up viewport and window, but do not draw box

    plenv(-1.3, 1.3, -1.3, 1.3, 1, -2)

    i = 0.1*arrayrange(1,11)
    #outerproduct(i,x0) and outerproduct(i,y0) is what we are 
    #mocking up here since old numpy version does not have outerproduct.
    i.shape = (-1,1)
    x=i*x0
    y=i*y0
    
    # Draw circles for polar grid
    for i in range(10):
	plline(x[i], y[i])

    plcol0(2)
    for i in range(12):
	theta = 30.0 * i
	dx = cos(dtr * theta)
	dy = sin(dtr * theta)

	# Draw radial spokes for polar grid

	pljoin(0.0, 0.0, dx, dy)

	# Write labels for angle

	text = `int(theta)`
#Slightly off zero to avoid floating point logic flips at 90 and 270 deg.
	if dx >= -0.00001:
	    plptex(dx, dy, dx, dy, -0.15, text)
	else:
	    plptex(dx, dy, -dx, -dy, 1.15, text)

    # Draw the graph

    r = sin((dtr*5.)*arrayrange(361))
    x = x0*r
    y = y0*r

    plcol0(3)
    plline(x, y)

    plcol0(4)
    plmtex("t", 2.0, 0.5, 0.5, "#frPLplot Example 3 - r(#gh)=sin 5#gh")
    plflush()

# ===============================================================

XPTS = 35
YPTS = 46
XSPA = 2./(XPTS-1)
YSPA = 2./(YPTS-1)

tr = array((XSPA, 0.0, -1.0, 0.0, YSPA, -1.0))

def mypltr(x, y, data):
    result0 = data[0] * x + data[1] * y + data[2]
    result1 = data[3] * x + data[4] * y + data[5]
    return array((result0, result1))

def plot5():

    mark = 1500
    space = 1500
    
    clevel = -1. + 0.2*arange(11)

    xx = (arrayrange(XPTS) - XPTS/2) / float((XPTS/2))
    yy = (arrayrange(YPTS) - YPTS/2) / float((YPTS/2)) - 1.
    xx.shape = (-1,1)
    z = (xx*xx)-(yy*yy)
    # 2.*outerproduct(xx,yy) for new versions of Numeric which have outerproduct.
    w = 2.*xx*yy

    plenv(-1.0, 1.0, -1.0, 1.0, 0, 0)
    plcol0(2)
    plcont(z, clevel, mypltr, tr)
    plstyl([mark], [space])
    plcol0(3)
    plcont(w, clevel, mypltr, tr)
    plstyl([], [])
    plcol0(1)
    pllab("X Coordinate", "Y Coordinate", "Streamlines of flow")
    plflush()

# ===============================================================

main()
plend()
