from Numeric import *
from plplot import *

# main
# 
# Illustration of logarithmic axes, and redefinition of window.

def main():

    plfont(2)
    plot1(0)
    plot1(1)

    # Restore defaults
    plfont(1)
    plcol0(1)
    
def plot1(type):

    pladv(0)
    f0 = 1.0
    freql = -2.0 + arrayrange(101)/20.0
    freq = pow(10.0, freql)
    ampl = 20.0 * log10(1.0 / sqrt(1.0 + pow((freq / f0), 2.)))
    phase = -(180.0 / pi) * arctan(freq / f0)

    plvpor(0.15, 0.85, 0.1, 0.9)
    plwind(-2., 3.0, -80.0, 0.0)
    plcol0(1)
    if type == 0:
	plbox("bclnst", 0.0, 0, "bnstv", 0.0, 0)
    elif type == 1:
	plbox("bcfghlnst", 0.0, 0, "bcghnstv", 0.0, 0)
    else:
	print "error: type must be either 0 or 1"
    plcol0(2)
    plline(freql, ampl)
    plcol0(1)
    plptex(1.6, -30.0, 1.0, -20.0, 0.5, "-20 dB/decade")

    # Put labels on
    plcol0(1)
    plmtex("b", 3.2, 0.5, 0.5, "Frequency")
    plmtex("t", 2.0, 0.5, 0.5, "Single Pole Low-Pass Filter")
    plcol0(2)
    plmtex("l", 5.0, 0.5, 0.5, "Amplitude (dB)")

    # For the gridless case, put phase vs freq on same plot
    if type == 0:
        plcol0(1)
	plwind(-2.0, 3.0, -100.0, 0.0)
	plbox("", 0.0, 0, "cmstv", 30.0, 3)
	plcol0(3)
	plline(freql, phase)
	plcol0(3)
	plmtex("r", 5.0, 0.5, 0.5, "Phase shift (degrees)")

main()
