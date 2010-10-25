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

## set_view (alt, az)
## set_view (script, arg1, ...)
## set_view ()
##
## If called with two numeric arguments, sets up the altitude and azimute
## in degrees for posterior 3d plots.
##
## If called without arguments, rotates iteratively a cube using the mouse.
##
## If called with one string as the first argument, instead of a cube the string
## is evaluated as a function and should do a 3D plot, using the remaining arguments.
##
##     example of callback `script':
##      [x, y, z] = peaks;
##      set_view("surfl", x, y, z);
##
## To control the view angle, the following mouse buttons can be used:
##
##    button 1 press: marks new origin
##    button 1,3 drag: rotate cube
##    button 2 press: finish
##    button 3 press: return to default position

function set_view (alt, az, varargin)

  global __pl 
  strm = __pl_init;

  callback = 0;

  if (nargin == 2 && !isstr(alt))
    __pl.az(strm) = az;
    __pl.alt(strm) = alt;
    return
  elseif (nargin >= 2 && isstr(alt))
    cmd = alt;
    arg1 = az;	
    callback = 1;
  endif

  az = __pl.az(strm);
  alt =  __pl.alt(strm);

  if (!callback)
    x = y = [0; 1];
    z = ones(2);
    xm = ym = zm = 0;
    xM = yM = zM = 1;

    plcol(15);

    __pl_plenv(-1.5, 1.5, -1.5, 2.5, 0, -2);
    plw3d(2, 2, 2, xm, xM, ym, yM, zm, zM, alt, az)

    plmtex("t", 3, 0.5, 0.5, sprintf("Alt=%d   Az=%d", alt, az));

    plbox3("bnstu", "X axis", 0.0, 0,"bnstu", "Y axis", 0.0, 0,"bdcmnstuv",
	   "Z axis", 0.0, 0)

    plot3d(x,y,z,1,1);
    plflush;

    xm = ym = zm = 0;
    xM = yM = zM = 1;
  else
    title(sprintf("Alt=%d   Az=%d", alt, az));
    feval(cmd, arg1, varargin{:});
  endif
  
  odx = ody = 0;
  c_alt = c_az = 0;
  
  while (1)
    [status, state, keysym, button, string, pX, pY, dX, dY, wX, wY, subwin] = plGetCursor;

    if (button == 3) # default position
      az = -60; alt = 30;
      ox = dX; oy = dY;
      c_alt = c_az = 0;
    elseif(button == 2) # stop
      break;
    elseif (button == 1) # mark position
      ox = dX; oy = dY;
      alt = alt + c_alt; az = az + c_az;
      c_alt = c_az = 0;
    elseif (button == 0) # drag
      c_az = (dX-ox)*100;
      c_alt = (oy-dY)*100;
    endif
    
    if (alt + c_alt > 90)
      c_alt = 90 - alt;
    elseif (alt + c_alt < 0)
      c_alt = -alt;
    endif

    if (!callback)
      __pl_plenv(-1.5, 1.5, -1.5, 2.5, 0, -2);

      plw3d(2, 2, 2, xm, xM, ym, yM, zm, zM, alt+c_alt, az+c_az)

      plbox3("bnstu", "x axis", 0.0, 0,"bnstu", "y axis", 0.0,
	     0,"bdcmnstuv", "z axis", 0.0, 0)

      plmtex("t", 3, 0.5, 0.5, sprintf("Alt=%d   Az=%d", alt+c_alt, az+c_az));

      plot3d(x,y,z,1,1);
      plflush;
    else
      __pl.az(strm) = az + c_az; 
      __pl.alt(strm) = alt + c_alt;
      title(sprintf("Alt=%d   Az=%d", alt+c_alt, az+c_az));
      feval(cmd, arg1, varargin{:});
    endif
    
  endwhile

  __pl.az(strm) = az + c_az; 
  __pl.alt(strm) = alt + c_alt;

  
endfunction
