## Copyright (C) 1998, 1999, 2000  Joao Cardoso
## Copyright (C) 2004  Rafael Laboissiere
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

1;

global alt = [20.0, 35.0, 50.0, 65.0];
global az = [30.0, 40.0, 50.0, 60.0];

##  Does a series of 3-d plots for a given data set, with different
##  viewing options in each plot.

function ix18c

  if (!exist("plinit"))
    plplot_stub
  endif

  if(isempty(sprintf("%s",plgdev')))
    plsdev("xwin");
  endif

  global alt;
  global az;
  global opt = [ 1, 0, 1, 0 ];
  NPTS = 1000;

  ## Parse and process command line arguments */

  ##    (void) plParseOpts(&argc, argv, PL_PARSE_FULL);

  ## Initialize plplot */
  plinit();

  for k=0:3
    test_poly(k);
  endfor

  ## From the mind of a sick and twisted physicist... */

  for i = 0:NPTS-1
    z(i+1) = -1. + 2. * i / NPTS;

    ## Pick one ... */
				#	r    = 1. - ( (float) i / (float) NPTS ); */
    r    = z(i+1);

    x(i+1) = r * cos( 2. * pi * 6. * i / NPTS );
    y(i+1) = r * sin( 2. * pi * 6. * i / NPTS );
  endfor

  for k = 0:3
    pladv(0);
    plvpor(0.0, 1.0, 0.0, 0.9);
    plwind(-1.0, 1.0, -0.9, 1.1);
    plcol(1);
    plw3d(1.0, 1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, alt(k+1), az(k+1));
    plbox3("bnstu", "x axis", 0.0, 0,
	   "bnstu", "y axis", 0.0, 0,
	   "bcdmnstuv", "z axis", 0.0, 0);

    plcol(2);

    if (opt(k+1))
      plline3( x', y', z' );
    else
      plpoin3( x', y', z', 1 );
    endif

    plcol(3);
    title=sprintf("#frPLplot Example 18 - Alt=%.0f, Az=%.0f",
		  alt(k+1), az(k+1));
    plmtex("t", 1.0, 0.5, 0.5, title);
  endfor

  plend1();
endfunction

function y = THETA(a)
 y = 2 * pi * (a) /20.;
endfunction

function y= PHI(a)
  y = pi * (a) / 20.1;
endfunction

function test_poly(k)

  global alt;
  global az;
  
  draw = [ 1, 1, 1, 1;
	  1, 0, 1, 0;
	  0, 1, 0, 1;
	  1, 1, 0, 0];

  two_pi = 2. * pi;

  pladv(0);
  plvpor(0.0, 1.0, 0.0, 0.9);
  plwind(-1.0, 1.0, -0.9, 1.1);
  plcol(1);
  plw3d(1.0, 1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, alt(k+1), az(k+1));
  plbox3("bnstu", "x axis", 0.0, 0,
	 "bnstu", "y axis", 0.0, 0,
	 "bcdmnstuv", "z axis", 0.0, 0);

  plcol(2);


  for i=0:19
    for j=0:19
      
      pj=pi*j/20.1;	pj1=pi*(j+1)/20.1;
      ti=2*pi*i/20; ti1=2*pi*(i+1)/20;
      
      x(1) = sin( pj ) * cos( ti );
      y(1) = sin( pj ) * sin( ti );
      z(1) = cos( pj );
      
      x(2) = sin( pj1 ) * cos( ti );
      y(2) = sin( pj1 ) * sin( ti );
      z(2) = cos( pj1 );
      
      x(3) = sin( pj1 ) * cos( ti1 );
      y(3) = sin( pj1 ) * sin( ti1 );
      z(3) = cos( pj1 );
      
      x(4) = sin( pj ) * cos( ti1 );
      y(4) = sin( pj ) * sin( ti1 );
      z(4) = cos( pj );
      
      x(5) = sin( pj ) * cos( ti );
      y(5) = sin( pj ) * sin( ti );
      z(5) = cos( pj );

      plpoly3(x', y', z', draw(k+1,:)', -1); ## added an extra argument, with the sign 
    endfor
  endfor

  plcol(3);
  plmtex("t", 1.0, 0.5, 0.5, "unit radius sphere" );
endfunction

ix18c
