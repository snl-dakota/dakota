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

##   gtext(string [,just [,orient]])
##
##       Place text on a 2-D graph using a mouse.
##   gtext('string') displays the graph window, puts up a
##       cross-hair, and waits for a mouse button or keyboard key to be
##       pressed.  The cross-hair can be positioned with the mouse (or
##       with the arrow keys on some computers).  Pressing a mouse button
##       or any key writes the text string onto the graph at the selected
##       location.
##   just controls justification of text;
##       just=0, left justified; just=1, right justified. In between values
##       justify acordingly. Default, center justified.
##   orient control orientation of text, measured in degrees. Default horizontal.

function gtext(string, just, orient)

  if (nargin < 1 || nargin > 3)
    help gtext
    return
  endif

  __pl_init;

  if (nargin == 1)
    just = 0.5;
    orient = 0;
  endif

  if (nargin == 2)
    orient = 0;
  endif

  orient = orient*pi/180;
  dx = cos(orient);
  dy = sin(orient);

  status = 0;
  while (status == 0)
    [status, state, key, but, st, pX, pY, dX, dY, wX, wY] = plGetCursor;
  endwhile

  ## try to get world coordinates of a device point away (0.01, 0.01),
  ## to enable computing proper values of dx dy (orientation)

  [st, dwx, dwy] = plTranslateCursor(dX+0.01, dY+0.01); # this may fail
  xfact = (dwx - wX);
  yfact = (dwy - wY);

  plptex(wX, wY, dx*xfact, dy*yfact, just, string)
  plflush;pleop;

endfunction

