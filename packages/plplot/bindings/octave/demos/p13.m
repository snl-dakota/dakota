## Copyright (C) 1999-2003 Joao Cardoso.
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

function p13

  t = automatic_replot;
  automatic_replot = 0;

  legend("opaque");
  title("Matrix with individual line attributes");
  fmt=["bo-;Complex;"; "-15;Trivial;"; "@-g;Normal;"];

  g = [ 1.0026965  -0.7056316   0.5318182
       -1.2488487   0.0075065   0.3397433
       -0.1075725  -0.0359792   1.3446770
       0.9289222   1.4904782   2.0537617
       0.6826819  -0.5042874  -0.7685230
       1.4587288   1.3862166   1.8868312
       0.2481350   0.5269746  -2.6129537
       0.2621972  -2.1017077  -1.3780762
       -0.5563111  -0.2584848   0.5026832
       -0.2354172  -1.6008980  -1.7502134];

  ##  plot(randn(10,3),fmt)
  plot(g,fmt)
  automatic_replot = t;

endfunction
