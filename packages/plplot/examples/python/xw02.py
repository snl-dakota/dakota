from Numeric import *
from plplot import *

# Demonstrates multiple windows and default color map 0 palette.

def main():

    # For starting from scratch this call to pladv increments cursub, but 
    # then the following plssub sets it to zero so the whole thing is 
    # essentially a nop.  However, for the case when other examples are run 
    # first, this call to pladv is absolutely essential to finish the 
    # preceding page.
    pladv(0)
    # Divide screen into 16 regions
    plssub(4, 4)

    plschr(0.0, 3.5)
    plfont(4)

    for i in range(16):
	plcol0(i)
	pladv(0)
	vmin = 0.1
	vmax = 0.9
	for j in range(3):
	    plwid(j + 1)
	    plvpor(vmin, vmax, vmin, vmax)
	    plwind(0.0, 1.0, 0.0, 1.0)
	    plbox("bc", 0.0, 0, "bc", 0.0, 0)
	    vmin = vmin + 0.1
	    vmax = vmax - 0.1
	plwid(1)
	plptex(0.5, 0.5, 1.0, 0.0, 0.5, `i`)

    # Restore defaults
    plssub(1, 1)
    plfont(1)
    plcol0(1)
    pleop()

main()
