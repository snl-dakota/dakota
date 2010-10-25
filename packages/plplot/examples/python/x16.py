#!/usr/bin/env python
#----------------------------------------------------------------------------
# PLplot Python demo #16
#
# Based heavily on the Tcl demo 16.
#
#----------------------------------------------------------------------------

# plshade demo, using color fill.

from Numeric import *
from math import *

def main(w):
    ns = 20
    nx = 35
    ny = 46

    sh_cmap = 1
    min_color = 1; min_width = 0; max_color = 0; max_width = 0

    clevel = zeros(ns,'d')
    xg1 = zeros(nx,'d')
    yg1 = zeros(ny,'d')
    xg2 = reshape( zeros(nx*ny,'d'), (nx,ny) )
    #yg2 = zeros(nx,ny)
    #zz = zeros(nx,ny)
    #ww = zeros(nx,ny)
    yg2 = reshape( zeros(nx*ny,'d'), (nx,ny) )
    zz = reshape( zeros(nx*ny,'d'), (nx,ny) )
    ww = reshape( zeros(nx*ny,'d'), (nx,ny) )

    # Set up data array

    for i in range(nx):
	x = (i - .5*nx) / (nx/2.)
	for j in range(ny):
	    y = (j - .5*ny) / (ny/2.) - 1.

	    zz[i,j] = -sin(7.*x) * cos(7.*y) + x*x - y*y
	    ww[i,j] = -cos(7.*x) * sin(7.*y) + 2.*x*y

    #zmin = min(zz.reshape())
    #zmax = max(zz.reshape())
    zmin = min(min(zz))
    zmax = max(max(zz))

    print "zmin=", zmin, " zmax=", zmax

    for i in range(ns):
	clevel[i] = zmin + (zmax - zmin) * (i+.5)/ns

    # Build the 1-d coord arrays.

    distort = .4

    for i in range(nx):
	xx = -1. + i *( 2./(nx-1))
	xg1[i] = xx + distort * cos( .5 * pi * xx )

    for j in range(ny):
	yy = -1. + j * (2./(ny-1.))
	yg1[j] = yy - distort * cos( .5 * pi * yy )

    # Build the 2-d coord arrays.

    for i in range(nx):
	xx = -1. + i * (2./(nx-1.))
	for j in range(ny):
	    yy = -1. + j * (2./(ny-1.))

	    argx = .5 * pi * xx
	    argy = .5 * pi * yy

	    xg2[i,j] = xx + distort * cos(argx) * cos(argy)
	    yg2[i,j] = yy - distort * cos(argx) * cos(argy)

    # Plot using identity transform

    w.pladv(0)
    w.plvpor( .1, .9, .1, .9 )
    w.plwind( -1., 1., -1., 1. )

    for i in range(ns):
	shade_min = zmin + (zmax - zmin) * i / ns
	shade_max = zmin + (zmax - zmin) * (i+1) / ns
	sh_color = i/(ns-1.)
	sh_width = 2
	w.plpsty(0)
	w.plshade( zz, -1., 1., -1., 1.,
		   shade_min, shade_max, sh_cmap, sh_color, sh_width,
		   min_color, min_width, max_color, max_width, 1 )

    w.plcol(1)
    w.plbox( "bcnst", 0., 0, "bcnstv", 0., 0 )
    w.plcol(2)

    #    plcont(w, nx, ny, 1, nx, 1, ny, clevel, ns, mypltr, NULL);

    w.pllab( "distance", "altitude", "Bogon density" )

    w.pleop()

    # Plot using 1d coordinate transform

    w.pladv(0)
    w.plvpor( .1, .9, .1, .9 )
    w.plwind( -1., 1., -1., 1. )

    for i in range(ns):
	shade_min = zmin + (zmax - zmin) * i / ns
	shade_max = zmin + (zmax - zmin) * (i+1) / ns
	sh_color = i/ (ns-1.)
	sh_width=2
	w.plpsty(0)
	w.plshade( zz, -1., 1., -1., 1.,
		   shade_min, shade_max, sh_cmap, sh_color, sh_width,
		   min_color, min_width, max_color, max_width,
		   1, 'pltr1', xg1, yg1 )

    w.plcol(1)
    w.plbox( "bcnst", 0.0, 0, "bcnstv", 0.0, 0 )
    w.plcol(2)
	       
    #    plcont(w, nx, ny, 1, nx, 1, ny, clevel, ns, pltr1, (void *) &cgrid1);

    w.pllab( "distance", "altitude", "Bogon density" )

    w.pleop()

    # Plot using 2d coordinate transform

    w.pladv(0)
    w.plvpor( .1, .9, .1, .9 )
    w.plwind( -1., 1., -1., 1. )

    for i in range(ns):
	shade_min = zmin + (zmax - zmin) * i / ns
	shade_max = zmin + (zmax - zmin) * (i+1) / ns
	sh_color = i/ (ns-1.)
	sh_width=2
	w.plpsty(0)
	w.plshade( zz, -1., 1., -1., 1.,
		   shade_min, shade_max, sh_cmap, sh_color, sh_width,
		   min_color, min_width, max_color, max_width,
		   0, 'pltr2', xg2, yg2 )

    w.plcol(1)
    w.plbox( "bcnst", 0.0, 0, "bcnstv", 0.0, 0 )
    w.plcol(2)

#    plcont(w, nx, ny, 1, nx, 1, ny, clevel, ns, pltr2, (void *) &cgrid2);

    w.pllab( "distance", "altitude", "Bogon density, with streamlines" )
    w.pleop()

    # Do it again, but show wrapping support.

    w.pladv(0)
    w.plvpor( .1, .9, .1, .9 )
    w.plwind( -1., 1., -1., 1. )

    # Hold perimeter
    px = zeros(100,'d'); py = zeros(100,'d')

    for i in range(100):
	t = 2.*pi*i/99.
	px[i] = cos(t)
	py[i] = sin(t)

    # We won't draw it now b/c it should be drawn /after/ the plshade stuff.

    # Now build the new coordinate matrices.

    #xg = zeros(nx,ny); yg= zeros(nx,ny); z = zeros(nx,ny)
    xg = reshape( zeros(nx*ny,'d'), (nx,ny) )
    yg = reshape( zeros(nx*ny,'d'), (nx,ny) )
    z  = reshape( zeros(nx*ny,'d'), (nx,ny) )

    for i in range(nx):
	r = i/(nx-1.)
	for j in range(ny):
	    t = 2.*pi*j/(ny-1.)
	    xg[i,j] = r * cos(t)
	    yg[i,j] = r * sin(t)
	    z [i,j] = exp(-r*r) * cos(5.*t) * cos( 5.*pi*r)

    # Need a new clevel to go along with the new data set.

    #zmin = min(z.reshape())
    #zmax = max(z.reshape())
    zmin = min(min(z))
    zmax = max(max(z))

    for i in range(ns):
	clevel[i] = zmin + (zmax - zmin) * (i+.5)/ns

    # Now we can shade the interior region.

    for i in range(ns):
	shade_min = zmin + (zmax - zmin) * i / ns
	shade_max = zmin + (zmax - zmin) * (i+1) / ns
	sh_color = i/ (ns-1.)
	sh_width=2
	w.plpsty(0)
	w.plshade( z, -1., 1., -1., 1.,
		   shade_min, shade_max, sh_cmap, sh_color, sh_width,
		   min_color, min_width, max_color, max_width,
		   0, 'pltr2', xg, yg, 2 )

    # Now we can draw the perimeter.

    w.plcol(1)
    w.plline( px, py )

    # And label the plot.

    w.plcol(2)
    w.pllab( "", "",  "Tokamak Bogon Instability" )
