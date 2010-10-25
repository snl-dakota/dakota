## Copyright (C) 1998, 1999, 2000 Joao Cardoso.
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

function [gx gy] = gradn(z, dx, dy, order)

# [gx gy] = gradn(z, [dx, dy [,order]])
#
# gx and gy are the x and y components of the gradient of matrix z.
# dx and dy are scalars giving the x and y step of z, default to 1
# order is the order of the "gradient", default 1, max 4

if (nargin == 1)
	dx = dy = 1;
endif

if (nargin == 3)
	order = 1;
endif

if (nargin == 2 || nargin > 4)
	help grad; return
endif

[nr nc] = size(z);

if (nr == 1 || nc == 1)
	help gradn; return
endif

gx = gy = zeros(nr,nc);

for i=1:nr
	gx(i,:) = diffn(z(i,:), dx, order);  
endfor

for i=1:nc
	gy(:,i) = diffn(z(:,i), dy, order);  
endfor
	
endfunction
