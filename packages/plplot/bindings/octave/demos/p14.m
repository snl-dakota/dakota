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


function p14(fg)

  ## And now, in the Theatre near you, "Octave, the movie"

  t = automatic_replot;
  automatic_replot = 0;

  if (!nargin)
    plsetopt "db";
    fig( free_fig); #  unused plot window
    title("Octave, the movie");
  else
    title("");
  endif

  ylabel "";
  xlabel "";
  
  l = 2;
  x = y = linspace(-l, l, 30);

  [xx,yy] = meshgrid(x,y);
  r = yy .* exp(-(xx.^2 .+ yy.^2));
  
  axis([-l l -l l min(min(r)) max(max(r))]);

  colormap(bgr);

  for k = linspace(0.01, 3.5*pi, 30)
    meshc(x, y, sin(k).*r);
  endfor

  if(!nargin)
    for k = linspace(0.01, 2*pi, 15)
      surf(x, y, sin(k).*r);
  endfor

    shading "flat";
    colormap (gray);
    for k = linspace(0.01, 3*pi, 30)
      surfl(x, y, sin(k).*r);
    endfor
  endif
  axis; # reset axis to automatic mode

  if (!nargin)
    closefig; # the "db" option has negative side effects on other plots.
    plsetopt "reset"; # reset options
  endif
  automatic_replot = t;
  
endfunction


