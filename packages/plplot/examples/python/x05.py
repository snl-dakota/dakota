#!/usr/bin/env python
#
#	x05c.c
#
#	Histogram demo.

from Numeric import *
import math
#import pl
import sys
import os

module_dir = "@MODULE_DIR@"

if module_dir[0] == '@':
	module_dir = os.getcwd ()

sys.path.insert (0, module_dir)

NPTS = 2047

# main
#
# Draws a histogram from sample data.

def main(w):

##    # Parse and process command line arguments
##
##    pl.ParseOpts(sys.argv, pl.PARSE_FULL)
##
##    # Initialize plplot
##
##    pl.init()

    # Fill up data points

    delta = 2.0 * math.pi / NPTS
    data = zeros(NPTS,'d')
##    data = []
    for i in range(NPTS):
	data[i] = math.sin(i * delta)

    w.plcol(1)
    w.plhist(data, -1.1, 1.1, 44, 0)
    w.plcol(2)
    w.pllab("#frValue", "#frFrequency",
	   "#frPLplot Example 5 - Probability function of Oscillator")

    w.pleop()
##    pl.end()
##
##main()
