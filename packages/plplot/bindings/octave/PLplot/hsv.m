## Copyright (C) 1999 Paul Kienzle
##
## This program is free software and may be used for any purpose.  This
## copyright notice must be maintained.  Paul Kienzle is not responsible
## for the consequences of using this software.

## Colormap:
##
## map = hsv (number)

## Author: Paul Kienzle <pkienzle@cs.indiana.edu>
## Created: Sept 99
## Modified: jc

function map = hsv (number)

  if (nargin == 0)
    number = 64;
  elseif (nargin > 1)
    usage ("hsv (number)");
  endif

  x = [0:(number-1)]'/number;
  r = clip(6*abs(x-1/2) - 1);
  g = clip(2 - 6*abs(x-1/3));
  b = clip(2 - 6*abs(x-2/3));

  map = [r, g, b];

endfunction
