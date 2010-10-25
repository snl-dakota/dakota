## Copyright (C) 1998, 1999, 2000 Joao Cardoso.
## 
## This program is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by the
## Free Software Foundation; either version 2 of the License, or (at your
## option) any later version.
## 
## This program is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## This file is part of plplot_octave.
## It is based on the corresponding demo function of PLplot.

## Does several contour plots using different coordinate mappings.

1;

global	XPTS=35;		## Data points in x
global	YPTS=46;		## Datat points in y

global	tr = [2/(XPTS-1); 0.0; -1.0; 0.0; 2/(YPTS-1); -1.0];	

function [tx ty] = mypltr(x, y)

  global	XPTS
  global	YPTS
  global	tr

  tx = tr(1) * x + tr(2) * y + tr(3);
  ty = tr(4) * x + tr(5) * y + tr(6);
  
endfunction


function _polar

  plenv(-1., 1., -1., 1., 0, -2);
  plcol0(1);

  ## Perimeter
  PERIMETERPTS = 100;
  RPTS = 40;
  THETAPTS = 40;

  i = 0:PERIMETERPTS-1;
  t = (2.*pi/(PERIMETERPTS-1))*i;
  px = cos(t);
  py = sin(t);
  plline(px', py');

  ## create data to be contoured
  i = 0:RPTS-1;
  r = i/(RPTS-1);
  z = r'*ones(1,RPTS);

  j = (0:THETAPTS-1)';
  theta = (2.*pi/(THETAPTS-1))*j;
  xg = cos(theta)*r;
  yg = sin(theta)*r;

  i = 0:9;
  lev = 0.05 + 0.10* i';

  plcol0(2);
  plcont2(z, 1, RPTS, 1, THETAPTS, lev, xg', yg');

  pllab("", "", "Polar Contour Plot");
endfunction

## shielded potential contour plot example.

function potential

  PPERIMETERPTS = 100;
  PRPTS = 40;
  PTHETAPTS = 64;
  PNLEVEL = 20;
  
  ## create data to be contoured

  i = 0:PRPTS-1;
  r = 0.5 + i;
  j = (0:PTHETAPTS-1)';
  theta = (2.*pi/(PTHETAPTS-1))*(0.5 + j);
  xg = (cos(theta)*r)';
  yg = (sin(theta)*r)';

  rmax = max(r);
  xmin = min(min(xg));
  xmax = max(max(xg));
  ymin = min(min(yg));
  ymax = max(max(yg));

  x0 = (xmin + xmax)/2;
  y0 = (ymin + ymax)/2;
  
  ## Expanded limits
  peps = 0.05;
  xpmin = xmin - abs(xmin)*peps;
  xpmax = xmax + abs(xmax)*peps;
  ypmin = ymin - abs(ymin)*peps;
  ypmax = ymax + abs(ymax)*peps;
  
  ## Potential inside a conducting cylinder (or sphere) by method of images.
  ## Charge 1 is placed at (d1, d1), with image charge at (d2, d2).
  ## Charge 2 is placed at (d1, -d1), with image charge at (d2, -d2).
  ## Also put in smoothing term at small distances.
  
  eeps = 2.;

  q1 = 1.;
  d1 = rmax/4.;
  
  q1i = - q1*rmax/d1;
  d1i = rmax^2/d1;
  
  q2 = -1.;
  d2 = rmax/4.;
  
  q2i = - q2*rmax/d2;
  d2i = rmax^2/d2;

  div1 = sqrt((xg-d1).^2 + (yg-d1).^2 + eeps^2);
  div1i = sqrt((xg-d1i).^2 + (yg-d1i).^2 + eeps^2);
  div2 = sqrt((xg-d2).^2 + (yg+d2).^2 + eeps^2);
  div2i = sqrt((xg-d2i).^2 + (yg+d2i).^2 + eeps^2);
  z = q1./div1 + q1i./div1i + q2./div2 + q2i./div2i;

  zmin = min(min(z));
  zmax = max(max(z));

  ## Positive and negative contour levels.
  dz = (zmax-zmin)/ PNLEVEL;

  i = 0:PNLEVEL-1;
  clevel = zmin + ( i + 0.5)*dz;
  clevelneg = clevel(clevel <= 0);
  clevelpos = clevel(clevel > 0);
  nlevelneg = columns( clevelneg);
  nlevelpos = columns( clevelpos);

  ## Colours! 
  ncollin = 11;
  ncolbox = 1;
  ncollab = 2;
  
  ## Finally start plotting this page!
  pladv(0);
  plcol0(ncolbox);

  plvpas(0.1, 0.9, 0.1, 0.9, 1.0);
  plwind(xpmin, xpmax, ypmin, ypmax);
  plbox("", 0., 0, "", 0., 0);
  
  plcol0(ncollin);
  if(nlevelneg >0)
    ## Negative contours
    pllsty(2);
    plcont2(z, 1, PRPTS, 1, PTHETAPTS, clevelneg', xg, yg);
  endif
  
  if(nlevelpos >0) 
    ## Positive contours
    pllsty(1);
    plcont2(z, 1, PRPTS, 1, PTHETAPTS, clevelpos', xg, yg);
  endif
  
  ## Draw outer boundary
  i = 0:PPERIMETERPTS-1;
  t = (2.*pi/(PPERIMETERPTS-1))*i;
  px = x0 + rmax*cos(t);
  py = y0 + rmax*sin(t);
  
  plcol0(ncolbox);
  plline(px', py');
  
  plcol0(ncollab);
  pllab("", "", "Shielded potential of charges in a conducting sphere");

endfunction
	
function ix09c

	  if (!exist("plinit"))
	    plplot_stub
	  endif

	  if(isempty(sprintf("%s",plgdev')))
	    plsdev("xwin");
	  endif

	  global	XPTS
	  global	YPTS
	  global	tr

	  clevel = linspace(-1,1,11)';

	  mark = 1500; space = 1500;

	  ## Parse and process command line arguments 

	  ##    (void) plParseOpts(&argc, argv, PL_PARSE_FULL);

	  ## Initialize plplot

	  plinit();

	  ## Set up function arrays 

	  for i=0:XPTS-1
	    xx = (i - (XPTS / 2)) /  (XPTS / 2);
	    yy = ((0:YPTS-1) - (YPTS / 2)) / (YPTS / 2) - 1.0;
	    z(i+1,:) = xx * xx - yy .* yy;
	    w(i+1,:) = 2 * xx .* yy;
	  endfor

	  ## Plot using identity transform
if (0)
	  pl_setcontlabelparam(0.006, 0.3, 0.1, 0);
	  plenv(-1.0, 1.0, -1.0, 1.0, 0, 0);
	  plcol0(2);
	  plcont(z, 1, XPTS, 1, YPTS, clevel, tr);
	  plstyl(1, mark, space);
	  plcol0(3);
	  plcont(w, 1, XPTS, 1, YPTS, clevel, tr);
	  plstyl(0, mark, space);
	  plcol0(1);
	  pllab("X Coordinate", "Y Coordinate", "Streamlines of flow");
endif
	  pl_setcontlabelparam(0.006, 0.3, 0.1, 1);
	  plenv(-1.0, 1.0, -1.0, 1.0, 0, 0);
	  plcol0(2);
	  plcont(z, 1, XPTS, 1, YPTS, clevel, tr);
	  plstyl(1, mark, space);
	  plcol0(3);
	  plcont(w, 1, XPTS, 1, YPTS, clevel, tr);
	  plstyl(0, mark, space);
	  plcol0(1);
	  pllab("X Coordinate", "Y Coordinate", "Streamlines of flow");

	  ## Set up grids

	  for i=0:XPTS-1
	    [xx yy] = mypltr(i, (0:YPTS-1));

	    argx = xx * pi/2;
	    argy = yy * pi/2;
	    distort = 0.4;

	    xg3(i+1,:) = xx .+ distort .* cos(argx);
	    yg3(i+1,:) = yy .- distort .* cos(argy);
	    
	    xg2(i+1,:) = xx .+ distort .* cos(argx) .* cos(argy);
	    yg2(i+1,:) = yy .- distort .* cos(argx) .* cos(argy);
	  endfor

	  xg1 = xg3(:,1);
	  yg1 = yg3(XPTS,:)';


	  ## Plot using 1d coordinate transform

	  pl_setcontlabelparam(0.006, 0.3, 0.1, 0);
	  plenv(-1.0, 1.0, -1.0, 1.0, 0, 0);
	  plcol0(2);
	  plcont1(z, 1, XPTS, 1, YPTS, clevel, xg1, yg1);
	  plstyl(1, mark, space);
	  plcol0(3);
	  plcont1(w, 1, XPTS, 1, YPTS, clevel, xg1, yg1);
	  plstyl(0, mark, space);
	  plcol0(1);
	  pllab("X Coordinate", "Y Coordinate", "Streamlines of flow");
if(0)
	  pl_setcontlabelparam(0.006, 0.3, 0.1, 1);
	  plenv(-1.0, 1.0, -1.0, 1.0, 0, 0);
	  plcol0(2);
	  plcont1(z, 1, XPTS, 1, YPTS, clevel, xg1, yg1);
	  plstyl(1, mark, space);
	  plcol0(3);
	  plcont1(w, 1, XPTS, 1, YPTS, clevel, xg1, yg1);
	  plstyl(0, mark, space);
	  plcol0(1);
	  pllab("X Coordinate", "Y Coordinate", "Streamlines of flow");
endif
	  ## Plot using 2d coordinate transform

	  pl_setcontlabelparam(0.006, 0.3, 0.1, 0);
	  plenv(-1.0, 1.0, -1.0, 1.0, 0, 0);
	  plcol0(2);
	  plcont2(z, 1, XPTS, 1, YPTS, clevel, xg2, yg2);
	  plstyl(1, mark, space);
	  plcol(3);
	  plcont2(w, 1, XPTS, 1, YPTS, clevel, xg2, yg2);
	  plstyl(0, mark, space);
	  plcol(1);
	  pllab("X Coordinate", "Y Coordinate", "Streamlines of flow");
if(0)
	  pl_setcontlabelparam(0.006, 0.3, 0.1, 1);
	  plenv(-1.0, 1.0, -1.0, 1.0, 0, 0);
	  plcol0(2);
	  plcont1(z, 1, XPTS, 1, YPTS, clevel, xg1, yg1);
	  plstyl(1, mark, space);
	  plcol0(3);
	  plcont1(w, 1, XPTS, 1, YPTS, clevel, xg1, yg1);
	  plstyl(0, mark, space);
	  plcol0(1);
	  pllab("X Coordinate", "Y Coordinate", "Streamlines of flow");
endif
	  pl_setcontlabelparam(0.006, 0.3, 0.1, 0);
	  _polar();
	  ## pl_setcontlabelparam(0.006, 0.3, 0.1, 1);
	  ## _polar();


    pl_setcontlabelparam(0.006, 0.3, 0.1, 0);
    potential();
    ## pl_setcontlabelparam(0.006, 0.3, 0.1, 1);
    ## potential();


	  plend1();

	endfunction

ix09c
