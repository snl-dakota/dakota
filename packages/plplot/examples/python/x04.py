#!/usr/bin/env python
#
#	x04c.c
#
#	Log plot demo.

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
# Illustration of logarithmic axes, and redefinition of window.

def main(w):

##    # Parse and process command line arguments
##
##    pl.ParseOpts(sys.argv, pl.PARSE_FULL)
##
##    # Initialize plplot
##
##    pl.init()

    w.pladv(0)
    w.plfont(2)

    f0 = 1000.0
    freql = zeros(101,'d'); ampl = zeros(101,'d'); phase=zeros(101,'d')
##    freql = []
##    ampl = []
##    phase = []
    for i in range(101):
##	freql.append(1.0 + i / 20.0)
	freql[i] = 1.0 + i / 20.0
##	freq = pow(10.0, freql[i])
	freq = pow( 10.0, freql[i] )
##	ampl.append(
##	    20.0 * math.log10(1.0 / math.sqrt(1.0 + pow((freq / f0), 2.)))
##	    )
	ampl[i] = 20.0 * math.log10(1.0 / math.sqrt(1.0 + pow((freq / f0), 2.)))
##	phase.append(-(180.0 / math.pi) * math.atan(freq / f0))
	phase[i] = -(180.0 / math.pi) * math.atan(freq / f0)

    w.plvpor(0.15, 0.85, 0.1, 0.9)
    w.plwind(1.0, 6.0, -80.0, 0.0)
    w.plcol(1)
    w.plbox("bclnst", 0.0, 0, "bnstv", 0.0, 0)
    w.plcol(2)
    w.plline(freql, ampl)
    w.plcol(1)
    w.plptex(5.0, -30.0, 1.0, -20.0, 0.5, "-20 dB/decade")

    w.plwind(1.0, 6.0, -100.0, 0.0)
    w.plbox("", 0.0, 0, "cmstv", 30.0, 3)
    w.plcol(3)
    w.plline(freql, phase)

    w.plcol(1)
    w.plmtex("b", 3.2, 0.5, 0.5, "Frequency")
    w.plmtex("t", 2.0, 0.5, 0.5, "Single Pole Low-Pass Filter")
    w.plcol(2)
    w.plmtex("l", 5.0, 0.5, 0.5, "Amplitude (dB)")
    w.plcol(3)
    w.plmtex("r", 5.0, 0.5, 0.5, "Phase shift (degrees)")

    w.pleop()

##main()
