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

function x17c

  if (!exist("plinit"))
    plplot_stub
  endif

  if(isempty(sprintf("%s",plgdev')))
    plsdev("xwin");
  endif

  ## If db is used the plot is much more smooth. However, because of the
  ## async X behaviour, one does not have a real-time scripcharter.
  ##    plSetOpt("db", ""); 

  plSetOpt("np", "");

  ## Specify some reasonable defaults for ymin and ymax */
  ## The plot will grow automatically if needed (but not shrink) */

  ymin = -1.0;
  ymax =  1.0;

  ## Specify initial tmin and tmax -- this determines length of window. */
  ## Also specify maximum jump in t */
  ## This can accomodate adaptive timesteps */

  tmin = 0.;
  tmax = 10.;
  tjump = 0.3;	## percentage of plot to jump

  ## Axes options same as plbox. */
  ## Only automatic tick generation and label placement allowed */
  ## Eventually I'll make this fancier */

  colbox = 1;
  collab = 3;
  styline(1) = colline(1) = 2;	## pens color and line style
  styline(2) = colline(2) = 3;
  styline(3) = colline(3) = 4;
  styline(4) = colline(4) = 5;    

  ##    legline = ["sum"; "sin"; "sin*noi"; "sin+noi";];


  xlab = 0.; ylab = 0.25;	## legend position 
  
  autoy = 0;	## autoscale y
  acc = 1;	## dont strip, accumulate

  ## Initialize plplot */
  plinit();
  pladv(0);    
  plvsta();    
  id1= plstripc("bcnst", "bcnstv",
		tmin, tmax, tjump, ymin, ymax,
		xlab, ylab,
		autoy, acc,
		colbox, collab,
		colline', styline', "sum", "sin", "sin*noi", "sin+noi",
		"t", "pois", "Strip chart demo");

  ## This is to represent a loop over time */
  ## Let's try a random walk process */

  y1 = y2 = y3 = y4 = 0.0;
  dt = 0.1;

  for n = 0:1000
    t = n * dt;
    noise = randn/2;
    y1 = y1 + noise;
    y2 = sin(t*pi/18.);
    y3 = y2 * noise;
    y4 = y2 + noise;
    
    ## there is no need for all pens to have the same number of points
    ## or beeing equally time spaced.
    
    if (rem(n,2))	
      plstripa(id1, 0, t, y1);
    endif
    if rem(n,3)
      plstripa(id1, 1, t, y2);
    endif
    if rem(n,4)
      plstripa(id1, 2, t, y3);
    endif
    if rem(n,5)
      plstripa(id1, 3, t, y4);
    endif
  endfor

  ## Destroy strip chart and it's memory */

  plstripd(id1);
  plend1();
  
endfunction
