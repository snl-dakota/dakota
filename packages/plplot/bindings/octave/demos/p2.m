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

function p2 

  t = automatic_replot;
  automatic_replot = 0;

  legend("opaque",0)
  title("PLplot Example 2");

  x=-pi/2:0.1:2*pi;
  y1=sinc(x);
  y2=sin(x).*cos(2*x);
  y3=x.*sin(x);
  plot(x, y1, ';sinc(x);',
       x, y2, 'b;sin(x)*cos(2*x);',
       x, y3, 'g;x*sin(x);');
  
  automatic_replot = t;
  
endfunction

