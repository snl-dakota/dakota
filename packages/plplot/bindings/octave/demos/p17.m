## Copyright (C) 2002-2003 Joao Cardoso.
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

function p17(fg)

  t = automatic_replot;
  automatic_replot = 0;

  title "Click and Drag button 1 to select";
  xlabel "Button 2 to restart and button 3 to finish";
  ylabel "";
  [img, map]= loadimage (file_in_loadpath ("lena.img"));
  colormap(map);
  plimage (img);
  if (!nargin)
    [x1, y1, x2, y2] = plrb(1);
    title "Lena";
    xlabel "";
    plimage (img, x1, x2, y1, y2);
  endif

  automatic_replot = t;
  
endfunction