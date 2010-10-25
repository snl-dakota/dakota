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

function __comet(x,y)

  symbol = 13;
  tail = length(x);

  delay = 10000;

  plline(x', y');
  plpoin(x(tail), y(tail), symbol);
  plflush;pleop;
  usleep(delay);
  plline(x', y');
  plpoin(x(tail), y(tail), symbol);
  plflush;pleop;
  pleop;

endfunction
