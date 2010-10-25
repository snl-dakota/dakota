## Copyright (C) 1998-2003 Joao Cardoso.
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

##	comet(Y) displays an animated comet plot of the vector Y.
##	comet(X,Y) displays an animated comet plot of vector Y vs. X.
##	comet(X,Y,p) uses a comet of length p*length(Y).  Default is p = 0.10.
##	comet, by itself, is self demonstrating.
##	Example:
##	    t = -pi:pi/200:pi;
##	    comet(t,tan(sin(t))-sin(tan(t)))

function comet(x, y, p)

  ax_s = axis "state";
  ax_s = deblank(ax_s(1,:));
  if (strcmp("auto", ax_s) == 1)
    warning("comet: FIXME: axis is 'auto', turning it to 'tight'.");
    axis "tight"
  endif

  if (nargin == 0)
    x = -pi:pi/200:pi;
    y = tan(sin(x))-sin(tan(x));
    p = 0.1;
  elseif (nargin < 2)
    y = x;
    x = 1:length(y);
  endif

  if (nargin < 3)
    p = 0.10;
  endif

  if (rows(x) != 1)
    x = x';
  endif

  if (rows(y) != 1)
    y = y';
  endif

  plot(x,y);

  [r, g, b] = plgcol0(1);
  plscol0(1, 255, 0, 255);
  plcol(1);
  pllsty(1);

  old_hold = ishold;
  hold "on";

  m = length(x);
  k = round(p*m);
  if (k == 1)
    k = 2;
  endif
  plxormod(1);
  
  unwind_protect # recover if user hit ^C
    
    ## Grow the body
    for i = 1:k
      __comet(x(1:i), y(1:i));
    endfor

    ## Primary loop
    for i = k+1:m-k
      __comet(x(i:i+k), y(i:i+k));
    endfor

    ## Clean up the tail
    for i = m-k+1:m
      __comet(x(i:m), y(i:m));
    endfor
    
  unwind_protect_cleanup
    
    plxormod(0);
    
    if (old_hold == 0)
      hold "off"
    endif

    plscol0(1, r, g, b);

  end_unwind_protect

  if (strcmp("auto", ax_s) == 1)
    axis "auto"
  endif

endfunction
