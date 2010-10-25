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

##   rgb = hls2rgb(hls)
##   
##   Hue-lightness-saturation to red-green-blue conversion.
##	all components in the range [0..1]

function rgb = hls2rgb(hls)

  [r, g, b] = plhlsrgb(hls(:,1).*360, hls(:,2), hls(:,3));
  rgb = [r, g, b];

endfunction
