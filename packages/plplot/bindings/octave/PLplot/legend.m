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

## legend (status)
##   turns the plot legend status "off", "on" == "transparent", or "opaque".
##
## st = legend
##   returns the current legend status
##
## legend ( xposition, yposition)
##   sets the left top corner legend at position (left)(bottom) 0 <= position <= 1 (right)(top)
##
## legend ("set", "legend 1", "legend 2", ...)
##   sets the legend strings, where the string must be equal to the
##   format used in the plot, e.g., "r+;legend;".
##   To disable an entry, enter one blank, e.g. legend("set","; ;")

function st = legend (x, xpos, varargin)

  global __pl
  strm = __pl_init;

  if (nargin == 0)
    ##__pl.legend(strm) = 2;
    ##__pl.legend_xpos(strm) = __pl.legend_ypos(strm) = 1;
    st = __pl.legend(strm);

  elseif (nargin == 1 && is_scalar(x) && x >=0 && x <= 2)
    __pl.legend(strm) = x;

  elseif (nargin == 2 && !isstr(x) && !isstr(xpos))
    __pl.legend_xpos(strm) = x;
    __pl.legend_ypos(strm) = xpos;

  elseif (nargin >= 1 && isstr (x))
    if (strcmp ("off", x))
      __pl.legend(strm) = 0;
    elseif (strcmp ("on", x) || strcmp ("transparent", x))
      __pl.legend(strm) = 1;
    elseif (strcmp ("opaque", x))
      __pl.legend(strm) = 2;

    elseif (strcmp ("set", x))
      if (nargin >= 2 && isstr(xpos))
	__pl.fmt{1,1} = xpos;
	i=2;
	while (--nargin > 1)
	  __pl.fmt{i++,strm} = varargin{i-1};
	endwhile
      endif
    else
      help "legend"
    endif
  else
    help "legend"
  endif

  if (automatic_replot)
    __pl_plotit;
  endif

endfunction
