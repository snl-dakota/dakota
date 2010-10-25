## Copyright (C) 1999 Paul Kienzle
##
## This program is free software and may be used for any purpose.  This
## copyright notice must be maintained.  Paul Kienzle is not responsible
## for the consequences of using this software.

## Colormap:
##
## map = copper (number)

## Author: Paul Kienzle <pkienzle@cs.indiana.edu>
## Created: Sept 99
## Modified: jc

function map = copper (number)

  if (nargin == 0)
    number = 64;
  elseif (nargin > 1)
    usage ("copper (number)");
  endif

  x = [0:(number-1)]'/number;

  r = clip(1.25*x);
  g = clip(0.8*x);
  b = clip(0.5*x);

  map = [ r, g, b ];
  
endfunction
