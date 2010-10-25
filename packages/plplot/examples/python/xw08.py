# xw08.py PLplot demo for Python
#
# Copyright (C) 2004  Alan W. Irwin
#
# This file is part of PLplot.
#
# PLplot is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Library Public License as published
# by the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# PLplot is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public License
# along with PLplot; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

from Numeric import *
from plplot import *

XPTS = 35		# Data points in x
YPTS = 46		# Data points in y

alt = [60.0, 20.0]

az = [30.0, 60.0]

title = ["#frPLplot Example 8 - Alt=60, Az=30",
	 "#frPLplot Example 8 - Alt=20, Az=60"]

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
# Does a series of 3-d plots for a given data set, with different
# viewing options in each plot.

def main():

    rosen = 1
    x = (arrayrange(XPTS) - (XPTS / 2)) / float(XPTS / 2)
    y = (arrayrange(YPTS) - (YPTS / 2)) / float(YPTS / 2)
    if rosen == 1:
	x = 1.5*x
	y = 0.5 + y
    x.shape = (-1,1)
    r2 = (x*x) + (y*y)
    if rosen == 1:
	z = (1. - x)*(1. - x) + 100 * (x*x - y)*(x*x - y)
	# The log argument may be zero for just the right grid.  */
	z = log(choose(greater(z,0.), (exp(-5.), z)))
    else:
	z = exp(-r2)*cos((2.0*pi)*sqrt(r2))

    x.shape = (-1,)
    zmin = min(z.flat)
    zmax = max(z.flat)
    nlevel = 10
    step = (zmax-zmin)/(nlevel+1)
    clevel = zmin + step + arange(nlevel)*step
    pllightsource(1., 1., 1.)
    for k in range(2):
	for ifshade in range(4):
	    pladv(0)
	    plvpor(0.0, 1.0, 0.0, 0.9)
	    plwind(-1.0, 1.0, -0.9, 1.1)
	    plcol0(3)
	    plmtex("t", 1.0, 0.5, 0.5, title[k])
	    plcol0(1)
	    if rosen == 1:
		plw3d(1.0, 1.0, 1.0, -1.5, 1.5, -0.5, 1.5, zmin, zmax,
		alt[k], az[k])
	    else:
		plw3d(1.0, 1.0, 1.0, -1.0, 1.0, -1.0, 1.0, zmin, zmax,
		alt[k], az[k])
	    plbox3("bnstu", "x axis", 0.0, 0,
	    "bnstu", "y axis", 0.0, 0,
	    "bcdmnstuv", "z axis", 0.0, 0)

	    plcol0(2)
	    if ifshade == 0:
		# diffuse light surface plot.
		# set up modified gray scale cmap1.
		cmap1_init(1)
		plsurf3d(x, y, z, 0, ())
	    elif ifshade == 1:
		# magnitude colored plot.
		cmap1_init(0)
		plsurf3d(x, y, z, MAG_COLOR, ())
	    elif ifshade == 2:
		# magnitude colored plot with faceted squares
		cmap1_init(0)
		plsurf3d(x, y, z, MAG_COLOR | FACETED, ())
	    elif ifshade == 3:
		# magnitude colored plot with contours
		cmap1_init(0)
		plsurf3d(x, y, z, MAG_COLOR | SURF_CONT | BASE_CONT, clevel)

    # Restore defaults
    plcol0(1)
    restore_cmap1()

main()
