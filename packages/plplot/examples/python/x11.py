#!/usr/bin/env python
#
#	x11c.c
#
#	Mesh plot demo.

from Numeric import *
import math
#import pl
import sys
import os

module_dir = "@MODULE_DIR@"

if module_dir[0] == '@':
	module_dir = os.getcwd ()

sys.path.insert (0, module_dir)

XPTS = 35		# Data points in x
YPTS = 46		# Data points in y

opt = [1, 2, 3, 3]

alt = [60.0, 20.0, 60.0, 60.0]

az = [30.0, 60.0, 120.0, 160.0]

title = ["#frPLplot Example 11 - Alt=60, Az=30, Opt=1",
	 "#frPLplot Example 11 - Alt=20, Az=60, Opt=2",
	 "#frPLplot Example 11 - Alt=60, Az=120, Opt=3",
	 "#frPLplot Example 11 - Alt=60, Az=160, Opt=3"]

# main
#
# Does a series of mesh plots for a given data set, with different
# viewing options in each plot.

def main(w):

##    # Parse and process command line arguments
##
##    pl.ParseOpts(sys.argv, pl.PARSE_FULL)
##
##    # Initialize plplot
##
##    pl.init()

    x = zeros(XPTS,'d'); y = zeros(YPTS,'d')
    #z = zeros(XPTS,YPTS)
    z = reshape( zeros( XPTS*YPTS, 'd' ), (XPTS, YPTS) )
##    x = []
##    y = []
##    z = []

    for i in range(XPTS):
	x[i] = float(i - (XPTS / 2)) / float(XPTS / 2)

    for i in range(YPTS):
	y[i] = float(i - (YPTS / 2)) / float(YPTS / 2)

    for i in range(XPTS):
	xx = x[i]
	zz = []
	for j in range(YPTS):
	    yy = y[j]
##	    zz.append(math.cos(2.0 * math.pi * xx) *
##		      math.sin(2.0 * math.pi * yy))
##	z.append(zz)
	    z[i,j] = math.cos(2.0 * math.pi * xx) * \
		     math.sin(2.0 * math.pi * yy)

    for k in range(4):
	w.pladv(0)
	w.plcol(1)
	w.plvpor(0.0, 1.0, 0.0, 0.8)
	w.plwind(-1.0, 1.0, -1.0, 1.5)

	w.plw3d(1.0, 1.0, 1.2, -1.0, 1.0, -1.0, 1.0, -1.5, 1.5,
	       alt[k], az[k])
	w.plbox3("bnstu", "x axis", 0.0, 0,
		"bnstu", "y axis", 0.0, 0,
		"bcdmnstuv", "z axis", 0.0, 4)

	w.plcol(2)
	w.plmesh(x, y, z, opt[k])
	w.plcol(3)
	w.plmtex("t", 1.0, 0.5, 0.5, title[k])

	w.pleop()

##main()
