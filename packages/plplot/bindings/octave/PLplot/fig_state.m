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

## current_state = fig_state([old_state])
##
## save/restore current figure axis and hold state

function cur_st = fig_state(old_st)

  cur_st.ax_set = axis_set;
  if (cur_st.ax_set)
    cur_st.ax = axis;
    axis(cur_st.ax);
  endif

  cur_st.is_hold = ishold;

  if (nargin == 1)
    if (old_st.ax_set)
      axis(old_st.ax);
    else
      axis;
    endif

    hold "off"
    if (old_st.is_hold)
      hold "on"
    endif
  endif

endfunction
