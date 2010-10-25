from Numeric import *
from plplot import *

XPTS = 35		# Data points in x
YPTS = 46		# Data points in y
opt = [3, 3]

alt = [33.0, 17.0]

az = [24.0, 115.0]

title = ["#frPLplot Example 11 - Alt=33, Az=24, Opt=3",
	 "#frPLplot Example 11 - Alt=17, Az=115, Opt=3"]

# Routine for restoring colour map1 to default.
# See static void plcmap1_def(void) in plctrl.c for reference.
def restore_cmap1():
    # For center control points, pick black or white, whichever is closer to bg 
    # Be careful to pick just short of top or bottom else hue info is lost
    vertex = sum(array(plgcolbg()))/(3.*255.)
    if vertex < 0.5:
	vertex = 0.01
	midpt = 0.10
    else:
	vertex = 0.99
	midpt = 0.90
    # Independent variable of control points.
    i = array((0., 0.44, 0.50, 0.50, 0.56, 1.))
    # Hue for control points.  Blue-violet to red
    h = array((260., 260., 260., 0., 0., 0.))
    # Lightness ranging from medium to vertex to medium
    l = array((0.5, midpt, vertex, vertex, midpt, 0.5))
    # Saturation is complete for default
    s = array((1., 1., 1., 1., 1., 1.))
    # Integer flag array is zero (no interpolation along far-side of colour
    # figure.)
    # Drop array, see plplot-devel discussion on 2004-10-27
    rev = array((0, 0, 0, 0, 0, 0))
    # Default number of cmap1 colours
    plscmap1n(128)
    # Interpolate between control points to set up default cmap1.
    plscmap1l(0, i, h, l, s, rev)

# Routine for defining a specific color map 1 in HLS space.
# if gray is true, use basic grayscale variation from half-dark to light.
# otherwise use false color variation from blue (240 deg) to red (360 deg).
def cmap1_init(gray):
    # Independent variable of control points.
    i = array((0., 1.))
    if gray:
	# Hue for control points.  Doesn't matter since saturation is zero.
	h = array((0., 0.))
	# Lightness ranging from half-dark (for interest) to light.
	l = array((0.5, 1.))
	# Gray scale has zero saturation
	s = array((0., 0.))
    else:
	# Hue ranges from blue (240 deg) to red (0 or 360 deg)
	h = array((240., 0.))
	# Lightness and saturation are constant (values taken from C example).
	l = array((0.6, 0.6))
	s = array((0.8, 0.8))

    # Integer flag array is zero (no interpolation along far-side of colour
    # figure.)
    # Drop array, see plplot-devel discussion on 2004-10-27
    rev = array((0, 0))
    # number of cmap1 colours is 256 in this case.
    plscmap1n(256)
    # Interpolate between control points to set up cmap1.
    plscmap1l(0, i, h, l, s, rev)
# main
#
# Does a series of mesh plots for a given data set, with different
# viewing options in each plot.

def main():

    x = 3.*(arrayrange(XPTS) - (XPTS / 2)) / float(XPTS / 2)
    y = 3.*(arrayrange(YPTS) - (YPTS / 2)) / float(YPTS / 2)
    x.shape = (-1,1)
    z = 3. * (1.-x)*(1.-x) * exp(-(x*x) - (y+1.)*(y+1.)) - \
        10. * (x/5. - pow(x,3.) - pow(y,5.)) * exp(-x*x-y*y) - \
        1./3. * exp(-(x+1)*(x+1) - (y*y))
#    if 0: #Jungfraujoch/Interlaken 
    # Not sure this is correct coding for truncating at -1, but
    # not activated anyway so ignore this question for now.
#	z = max(z,-1)
    x.shape = (-1,)

    zmin = min(z.flat)
    zmax = max(z.flat)
    nlevel = 10
    step = (zmax-zmin)/(nlevel+1)
    clevel = zmin + step + arange(nlevel)*step
    cmap1_init(0)
    for k in range(2):
	for i in range(4):
	    pladv(0)
	    plcol0(1)
	    plvpor(0.0, 1.0, 0.0, 0.9)
	    plwind(-1.0, 1.0, -1.0, 1.5)
	    plw3d(1.0, 1.0, 1.2, -3.0, 3.0, -3.0, 3.0, zmin, zmax, alt[k], az[k])
	    plbox3("bnstu", "x axis", 0.0, 0,
		"bnstu", "y axis", 0.0, 0,
		"bcdmnstuv", "z axis", 0.0, 4)
	    plcol0(2)

	    #wireframe plot
            if i==0:
		plmesh(x, y, z, opt[k])

	    # magnitude colored wireframe plot
	    elif i==1:
		plmesh(x, y, z, opt[k] | MAG_COLOR)

	    # magnitude colored wireframe plot with sides
	    elif i==2:
		plot3d(x, y, z, opt[k] | MAG_COLOR, 1)

	    # magnitude colored wireframe plot with base contour
	    elif i==3:
		plmeshc(x, y, z, opt[k] | MAG_COLOR | BASE_CONT, clevel)


	    plcol0(3)
	    plmtex("t", 1.0, 0.5, 0.5, title[k])

    # Restore defaults
    plcol0(1)
    restore_cmap1()
main()
