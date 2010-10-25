## Copyright (C) 1999 Paul Kienzle
##
## This program is free software and may be used for any purpose.  This
## copyright notice must be maintained.  Paul Kienzle is not responsible
## for the consequences of using this software.

## Clip values outside the range to the value at the boundary of the
## range.
##
## X = clip(X)
##   Clip to range [0 1]
##
## X = clip(X, hi)
##   Clip to range [0 hi]
##
## X = clip(X, [lo hi])
##   Clip to range [lo hi]

function x = clip (x, range)

  if (nargin == 2)
    if (length(range) == 1)
      range = [0, range];
    end
  elseif (nargin == 1)
    range = [0, 1];
  else
    usage("X = clip(X [, range])");
  end
  [i, j] = size(x);
  x = reshape(x, i*j, 1);
  clip = find(x>range(2));
  if clip != []
    x(clip) = range(2);
  end
  clip = find(x<range(1));
  if clip != []
    x(clip) = range(1);
  end
  x = reshape(x, i, j);

endfunction
