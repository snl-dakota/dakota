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

##   [x, y, button] = ginput([n])
##
##   gets n points (default 1e6) from the current axes and returns
##       the x- and y-coordinates in length N vectors X and Y.  The cursor
##       can be positioned using a mouse (or by using the Arrow Keys on some
##       systems).  Data points are entered by pressing a mouse button
##       or any key on the keyboard.  A carriage return terminates the
##       input before N points are entered.
##
##   	button contains a vector of integers specifying which mouse button was
##       used (1,2,3 from left) or ASCII numbers if a key on the keyboard
##       was used.
##
##       A mouseclick or key press with the pointer positioned outside of
##       the viewport is ignored.
##
##	A bug makes the cross-hair remain in the plot after ginput terminates.
##	To avoid, move the cross-hair outside the window, and press <RET>
##	or issue plflush;pleop; after calling ginput

function [x, y, but] = ginput(n)

  global __pl
  __pl_init;

  if (nargin == 0)
    n = 1e6;
  endif

  keysym = str = 0;
  i = 0;

  while (i != n && keysym != 13) 
    [status, state, keysym, button, string, pX, pY, dX, dY, wX, wY] = plGetCursor;
    if (status == 0)
      continue;
    else
      i++;
      x(i) = wX;
      y(i) = wY;
      str = toascii(string);
      if (button == 0)
	but(i) = str;
      else
	but(i) = button;
      endif
      ## should output appear at output?
      ##		fprintf(stderr,"x=%f y=%f button=%d\n", x(i), y(i), but(i));
    endif
  endwhile

  ## if this is issued out of the function, in the *command line*,
  ## the cross-hair disapears!
  plflush;pleop;		

endfunction
