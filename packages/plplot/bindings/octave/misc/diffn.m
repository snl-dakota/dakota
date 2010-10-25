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

function datal = diffn(data,step,order)

# data = diffn(data [,step [,order]])
#
# Computes the order (<=4) centered finite derivative of data, with step dx.
# Uses Taylor series expansion up to the second derivative.
# The first/last points are computed with forward/backward finite diferences.
# The step is important for correct scaling.
# step and order default to 1

if (nargin == 1)
	step = 1; order = 1;
elseif (nargin == 2)
	order = 1;
end

if (order > 4 || order < 1)
	usage("order must be:  <= 1 order <= 4 ");
endif

if (! is_vector(data))
	error("diffn only operate on row/column vectors. FIXME");
endif

[nr nc]=size(data);

row_f = 0;
if (nr > nc)
	data = data';
	row_f = 1;
endif;

n_cols=columns(data);

datal=zeros(1,n_cols);

if (order == 1)
	coef = [1;-8;0;8;-1]; div = 12*step; win = 2;	# centered difference
	h_coef = [-3;4;-1]; h_div = 2*step; h_win = 2;	# forward/backward
	t_coef = -flipud(h_coef);
elseif (order == 2)
	coef = [-1;16;-30;16;-1]; div = 12*step^2; win = 2;
	h_coef = [2;-5;4;-1]; h_div = step^2; h_win = 3;
	t_coef = flipud(h_coef);	
elseif (order == 3)
	coef = [1;-8;13;0;-13;8;-1]; div = 8*step^3; win = 3;
	h_coef = [-5;18;-24;14;-3]; h_div = 2*step^3; h_win = 4;
	t_coef = -flipud(h_coef);	
elseif (order == 4)
	coef = [-1;12;-39;56;-39;12;-1]; div = 6*step^4; win = 3;
	h_coef = [3;-14;26;-24;11;-2]; h_div = step^4; h_win = 5;
	t_coef = flipud(h_coef);	
endif

for i=win+1:n_cols-win
	datal(i) = (data(i-win:i+win) * coef)/div;
endfor

# head: use forward diferences here
for i=1:win
	datal(i) = (data(i:h_win+i) * h_coef)/h_div;
#	datal(i) = mean(datal(win+1:n_cols-win));
endfor

# tail: and backward differences here
for i=n_cols-win+1:n_cols
	datal(i) = (data(i-h_win:i) * t_coef)/h_div;
#	datal(i) = mean(datal(win+1:n_cols-win));	
endfor

if (row_f)
	datal = datal';
endif

endfunction
