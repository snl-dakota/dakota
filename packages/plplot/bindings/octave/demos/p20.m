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

function p20

  plclearplot;

  multiplot(4,4)

  subwindow(1, 1)
  p1
  subwindow(3 ,4)
  p2
  subwindow(1 ,4)
  p3
  subwindow(3 ,1)
  p4
  subwindow(2 ,4)
  p5
  subwindow(2 ,1)
  p6
  subwindow(2 ,3)
  p7
  subwindow(2 ,2)
  p8
  subwindow(3 ,3)
  p9
  subwindow(3 ,2)
  p11
  subwindow(1 ,3)
  p13
  subwindow(4 ,1)
  p10
  subwindow(4 ,2)
  p15
  subwindow(4 ,3)
  p14(1)
  subwindow(1 ,2)
  p12
  subwindow(4, 4)
  p17(1)
  oneplot;

endfunction
