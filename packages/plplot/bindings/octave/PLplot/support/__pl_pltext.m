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

function out =__pl_pltext(str)

### str =__pl_pltext(str)
### convert (almost) an iso-8859-1 string `str' to a plplot accentuated string
### PRELIMINARY

  in = "¿¡¬√»… ÃÕ“”Ù’Ÿ⁄€‡·‚„ËÈÍÏÌÚÛÙı˘˙˚Á«";
  tr = ["#u`#d#bA"; "#u'#d#bA"; "A#b#u^#d"; "A#b#u~#d";
	"#u`#d#bE"; "#u'#d#bE"; "E#b#u^#d";
	"I#b#u`#d"; "I#b#u'#d";
	"#u`#d#bO"; "#u'#d#bO"; "#u^#d#bO"; "#u~#d#bO";
	"#u`#d#bU"; "#u'#d#bU"; "#u^#d#bU";
	"#u`#d#ba"; "#u'#d#ba"; "#u^#d#ba"; "a#b#u~#d";
	"#u`#d#be"; "#u'#d#be"; "#u^#d#be";
	"#u`#d#bi"; "#u'#d#bi";
	"#u`#d#bo"; "#u'#d#bo"; "#u^#d#bo"; "#u~#d#bo";
	"#u`#d#bu"; "#u'#d#bu"; "#u^#d#bu"; 
	"#d,#u#bc"; "#d,#u#bC"];


  out = "";
  for k=1:rows(str);
    t = "";
    for i=1:length(str(k,:))
      ix = find(str(k,i) == in);
      if (isempty(ix))
	t = [t, str(k,i)];
      else
	t = [t, tr(ix,:)]
      endif
    endfor
    out = [out; t];
  endfor

  
endfunction
