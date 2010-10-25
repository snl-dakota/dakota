from plplot import *
from Numeric import *

# Pairs of points making the line segments used to plot the user defined arrow
arrow_x = [-0.5, 0.5, 0.3, 0.5, 0.3, 0.5]
arrow_y = [0.0, 0.0, 0.2, 0.0, -0.2, 0.0]
arrow2_x = [-0.5, 0.3, 0.3, 0.5, 0.3, 0.3]
arrow2_y = [0.0, 0.0,   0.2, 0.0, -0.2, 0.0]


def circulation():

    nx = 20
    ny = 20

    dx = 1.0
    dy = 1.0

    xmin = -nx/2*dx
    xmax = nx/2*dx
    ymin = -ny/2*dy
    ymax = ny/2*dy

    # Create data - circulation around the origin.
    ix = ones(nx)
    iy = ones(ny)
    x = arange(nx)+0.5-nx/2.0
    y = arange(ny)+0.5-ny/2.0
    xg = multiply.outer(x,iy)
    yg = multiply.outer(ix,y)
    u = yg
    v = -xg

    # Plot vectors with default arrows
    plenv(xmin, xmax, ymin, ymax, 0, 0)
    pllab("(x)", "(y)", "#frPLplot Example 22 - circulation")
    plcol0(2)
    scaling = 0.0
    plvect(u,v,scaling,pltr2,xg,yg)
    plcol0(1)


# Vector plot of flow through a constricted pipe
def constriction():

    nx = 20
    ny = 20

    dx = 1.0
    dy = 1.0

    xmin = -nx/2*dx
    xmax = nx/2*dx
    ymin = -ny/2*dy
    ymax = ny/2*dy

    Q = 2.0
    ix = ones(nx)
    iy = ones(ny)
    x = (arange(nx)-nx/2+0.5)*dx
    y = (arange(ny)-ny/2+0.5)*dy
    xg = multiply.outer(x,iy)
    yg = multiply.outer(ix,y)
    b = ymax/4.0*(3-cos(pi*x/xmax))
    b2 = multiply.outer(b,iy)
    mask = greater.outer(b,abs(y))
    dbdx = ymax/4.0*(sin(pi*xg/xmax)*yg/b2)
    u = Q*ymax/b2*mask
    v = dbdx*u

    plenv(xmin, xmax, ymin, ymax, 0, 0)
    pllab("(x)", "(y)", "#frPLplot Example 22 - constriction")
    plcol0(2)
    scaling=-0.5
    plvect(u,v,scaling,pltr2,xg,yg)
    plcol0(1)


# Vector plot of the gradient of a shielded potential (see example 9)
def potential():
    nper = 100
    nlevel = 10
    nr = 20
    ntheta = 20
  
    # Create data to be contoured
    r = 0.5+arange(nr)
    r.shape = (-1,1)
    theta = (2.*pi/float(ntheta-1))*(0.5+arange(ntheta))
    xg = r*cos(theta)
    yg = r*sin(theta)

    rmax = nr
    xmin = min(xg.flat)
    xmax = max(xg.flat)
    ymin = min(yg.flat)
    ymax = max(yg.flat)

    x = xg
    y = yg

    # Potential inside a conducting cylinder (or sphere) by method of images.
    # Charge 1 is placed at (d1, d1), with image charge at (d2, d2).
    # Charge 2 is placed at (d1, -d1), with image charge at (d2, -d2).
    # Also put in smoothing term at small distances.

    eps = 2.

    q1 = 1.
    d1 = rmax/4.

    q1i = - q1*rmax/d1
    d1i = rmax**2/d1

    q2 = -1.
    d2 = rmax/4.

    q2i = - q2*rmax/d2
    d2i = rmax**2/d2

    div1 = sqrt((x-d1)**2 + (y-d1)**2 + eps**2)
    div1i = sqrt((x-d1i)**2 + (y-d1i)**2 + eps**2)
    div2 = sqrt((x-d2)**2 + (y+d2)**2 + eps**2)
    div2i = sqrt((x-d2i)**2 + (y+d2i)**2 + eps**2)
    zg = q1/div1 + q1i/div1i + q2/div2 + q2i/div2i
    zmin = min(zg.flat)
    zmax = max(zg.flat)

    ug = -q1*(x-d1)/div1**3 - q1i*(x-d1i)/div1i**3 \
	- q2*(x-d2)/div2**3 - q2i*(x-d2i)/div2i**3
    vg = -q1*(y-d1)/div1**3 - q1i*(y-d1i)/div1i**3 \
	- q2*(y+d2)/div2**3 - q2i*(y+d2i)/div2i**3

    umin = min(ug.flat)
    umax = max(ug.flat)
    vmin = min(vg.flat)
    vmax = max(vg.flat)

    plenv(xmin, xmax, ymin, ymax, 0, 0)
    pllab("(x)", "(y)", "#frPLplot Example 22 - potential gradient vector plot")
    # Plot contours of the potential
    dz = (zmax-zmin)/float(nlevel)
    clevel = zmin + (arange(nlevel)+0.5)*dz
    du = (umax-umin)/float(nlevel)
    clevelu = umin + (arange(nlevel)+0.5)*du
    dv = (vmax-vmin)/float(nlevel)
    clevelv = vmin + (arange(nlevel)+0.5)*dv

    plcol0(3)
    pllsty(2)
    plcont(zg,clevel,pltr2,xg,yg)
    pllsty(1)
    plcol0(1)
    
    # Plot the vectors of the gradient of the potential
    plcol0(2)
    scaling = 25.0
    plvect(ug,vg,scaling,pltr2,xg,yg)
    plcol0(1)

    # Perimeter
    t = (2.*pi/(nper-1))*arange(nper)
    px = rmax*cos(t)
    py = rmax*sin(t)
    plline(px,py)
    
# main
#
# Does a series of vector plots
#
def main():

    circulation()

    narr = 6
    fill = 0

# Set arrow style using arrow_x and arrow_y then 
# plot using these arrows.
    plsvect(arrow_x, arrow_y, fill)
    constriction()

# Set arrow style using arrow2_x and arrow2_y then 
# plot using these filled arrows.
    fill = 1
    plsvect(arrow2_x, arrow2_y, fill)
    constriction()

    potential()


# Vector plot of the circulation about the origin 
main()
