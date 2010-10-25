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

function p12

  t = automatic_replot;
  automatic_replot = 0;

  title("War Game");
  legend("off");
  x=0:0.1:4;
  plot(x,exp(x));
  text(1,20,"Click Here -> + ");
  [x y]=ginput(1);
  if (round(x) != 2 || round(y) != 20)
    text(x,y,"You missed!")
  else
    text(x,y,"ouch! Gently!")
  endif
  legend("on");

  automatic_replot = t;

endfunction
