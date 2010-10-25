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

## hsv = rgb2hls(rgb)
##   
## red-green-blue to hue-lightness-saturation conversion.
##   all components in the range [0..1]

function hls = rgb2hls(rgb)

  [h, l, s] = plrgbhls(rgb(:,1), rgb(:,2), rgb(:,3));
  hls = [h/360, l, s];

endfunction
