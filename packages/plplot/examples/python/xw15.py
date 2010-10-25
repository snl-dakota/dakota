from Numeric import *
from plplot import *

XPTS = 35		# Data points in x
YPTS = 46		# Data points in y


def main():
# Does a variety of shade plots with discrete colours and patterns.

    # Do not fiddle with cmap1 since this example actually uses cmap0.
    # Furthermore, if you do fiddle with cmap1, it will screw up other
    # plots (unless you return to default cmap1 like is done with eighth
    # example.
    x = (arrayrange(XPTS) - (XPTS / 2)) / float(XPTS / 2)
    y = ((arrayrange(YPTS) - (YPTS / 2)) / float(YPTS / 2)) - 1.
    x.shape = (-1,1)
    z = x*x - y*y + (x - y)/(x*x+y*y + 0.1)
    x.shape = (-1,)
    zmin = min(z.flat)
    zmax = max(z.flat)
    
    plot1(z, zmin, zmax)
    plot2(z, zmin, zmax)
    
    # Restore defaults
    plcol0(1)
	
def plot1(z, zmin, zmax):
# Illustrates a single shaded region

    pladv(0)
    plvpor(0.1, 0.9, 0.1, 0.9)
    plwind(-1.0, 1.0, -1.0, 1.0)
    
    shade_min = zmin + (zmax-zmin)*0.4
    shade_max = zmin + (zmax-zmin)*0.6
    sh_cmap = 0
    sh_color = 7
    sh_width = 2
    min_color = 9
    min_width = 2
    max_color = 2
    max_width = 2
    
    plpsty(8)
    
    # Just use identity transform on indices of z mapped to -1, 1 range
    # in X and Y coordinates
    plshade( z, -1., 1., -1., 1.,
    shade_min, shade_max, sh_cmap, sh_color, sh_width,
    min_color, min_width, max_color, max_width, 1)
							
    plcol0(1)
    plbox("bcnst", 0.0, 0, "bcnstv", 0.0, 0)
    plcol0(2)
    pllab("distance", "altitude", "Bogon flux")

def plot2(z, zmin, zmax):
# Illustrates multiple adjacent shaded regions, using different fill
# patterns for each region.

    pladv(0)
    plvpor(0.1, 0.9, 0.1, 0.9)
    plwind(-1.0, 1.0, -1.0, 1.0)
    
    sh_cmap = 0
    sh_width = 2
    min_color = 0
    min_width = 0
    max_color = 0
    max_width = 0
    
    for i in range(10):
	shade_min = zmin + (zmax - zmin) * i / 10.0
	shade_max = zmin + (zmax - zmin) * (i +1) / 10.0
	sh_color = i+6
	plpsty((i + 2) % 8 + 1)
    
	# Just use identity transform on indices of z mapped to -1, 1 range
	# in X and Y coordinates
	plshade( z, -1., 1., -1., 1.,
	shade_min, shade_max, sh_cmap, sh_color, sh_width,
	min_color, min_width, max_color, max_width, 1)

    plcol0(1)
    plbox("bcnst", 0.0, 0, "bcnstv", 0.0, 0)
    plcol0(2)
    pllab("distance", "altitude", "Bogon flux")

main()
