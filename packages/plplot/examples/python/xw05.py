from Numeric import *
from plplot import *

NPTS = 2047

# main
#
# Draws a histogram from sample data.

def main():

    # Fill up data points

    delta = 2.0 * pi / NPTS
    data = sin(delta*arrayrange(NPTS))

    plcol0(1)
    plhist(data, -1.1, 1.1, 44, 0)
    plcol0(2)
    pllab("#frValue", "#frFrequency",
	   "#frPLplot Example 5 - Probability function of Oscillator")

    # Restore defaults
    plcol0(1)

main()
