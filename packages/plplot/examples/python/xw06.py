from Numeric import *
from plplot import *

# main
#
# Displays the entire "plpoin" symbol (font) set.

def main():

    pladv(0)

    # Set up viewport and window

    plcol0(2)
    plvpor(0.1, 1.0, 0.1, 0.9)
    plwind(0.0, 1.0, 0.0, 1.3)

    # Draw the grid using plbox

    plbox("bcg", 0.1, 0, "bcg", 0.1, 0)

    # Write the digits below the frame

    plcol0(15)
    for i in range(10):
	plmtex("b", 1.5, (0.1 * i + 0.05), 0.5, `i`)

    k = 0
    for i in range(13):

	# Write the digits to the left of the frame

	plmtex("lv", 1.0, (1.0 - (2 * i + 1) / 26.0), 1.0, `10 * i`)

	for j in range(10):
	    x = 0.1 * j + 0.05
	    y = 1.25 - 0.1 * i

	    # Display the symbol (plpoin expects that x
	    # and y are arrays so pass lists)

	    if k < 128:
		plpoin([x], [y], k)
	    k = k + 1

    plmtex("t", 1.5, 0.5, 0.5, "PLplot Example 6 - plpoin symbols")

    # Restore defaults
    plcol0(1)

main()
