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

## save_fig(file, [device [, reverse]])
##
## Save current plot figure in 'file' for 'device' type.
## If reverse is 1 and the background is black, reverse black and white.
## 	(This feature has been tested with the xwin and tk driver only)
##
## If the global variable FIGDIR exists, `file' is appended to FIGDIR
## if `file' does not start with any of `/~.'
##
## The file name extension determines the device type, unless it is specified.
## Defaults are "psc" (Colour Postscript), and reverse
## The special output file `|lp' send the output to the printer.
## Use `lp_setup' to setup your printer.
##
## What is saved is all that is available in the plot buffer. To see what is
## available, type `plreplot'.
## To clear the plot buffer type `plclearplot' or `plclg'
##
## *Usual* devices are:
##	
##	ps         PostScript File (monochrome)
##	psc        PostScript File (color)
##	plmeta     PLPLOT Native Meta-File (can be displayed with 'plrender')
##	xfig       Xfig file
##	pbm        PDB (PPM) Driver
##	lj_hpgl    HP Laserjet III, HPGL emulation mode
##	hp7470     HP 7470 Plotter File (HPGL Cartridge, Small Plotter)
##	hp7580     HP 7580 Plotter File (Large Plotter)

function save_fig(file, device, rev)

  global __pl FIGDIR
  strm = __pl_init;
  
  if (plglevel != 3)
    warning("Nothing to save");
    return
  endif

  if (!exist("__pl.lp_options"))
    __pl.lp_options = "lp -c";
  endif

  if (nargin < 3)
    rev = 1;
  endif

  if (nargin < 2 )
    device = "psc";	
    ix = rindex(file, '.');
    if (ix)
      device = file(ix+1:length(file));
    endif
  endif

  ## FIXME -- this should be get from plgDevs()
  vdev = [ "xwin"; "tk"; "xterm"; "tekt"; "tek4107t"; "mskermit"; "versaterm";
	  "vlt"; "conex"; "dg300"; "plmeta"; "tekf"; "tek4107f"; "ps"; "psc";
	  "xfig"; "ljiip"; "ljii"; "hp7470"; "hp7580"; "lj_hpgl"; \
	  "imp"; "pbm"; "png"; "jpeg"; "pstex"];

  dev = "";
  for i=1:rows(vdev)
    if (strcmp(device, tdeblank(vdev(i,:))))
      dev = device;
      break;
    endif
  endfor

  if (isempty(dev))
    dev = "psc";
    warning("save_fig: Using Color Postscript.");
  endif

  device = dev;

  if (nargin >= 1 && isstr(device) & isstr(file))
    
    to_prt = 0;
    if (strcmp(file,"|lp"))	# special output file, printer!
      to_prt = 1;
      file = tmpnam;
    elseif (file(1) == '~')
      file = tilde_expand(file);
    elseif (exist("FIGDIR") & file(1) != '/' & file(1) != '.')
      file = [FIGDIR, '/', file];
    endif

    ## see if it is possible to save the file
    [a, st] = system(sprintf("touch %s;", file),1);
    if (st != 0)
      error("Can't create file %s.\n", file);
      return;
    endif

    cur_fig = plgstrm;
    f = free_fig; # find new stream and make it default
    if (isempty(f))
      f = plmkstrm;
    endif
    
    plsstrm(f); # set stream type and output file
    plsfnam(file);
    plsdev(device);

    rev_done = 0;
    if (rev == 1) # exchange background before plinit(), if needed
      [r, g, b] = plgcolbg;
      if (r == 0 && g == 0 && b == 0)	# black background
	rev_done = 1;
	plSetOpt("bg", "FFFFFF");	# set it white
      endif
    endif

    plsetopt "apply"
    plinit;
    
    plcpstrm(cur_fig, 0); # copy parameters

    if (rev_done == 1) # and exchange black/white if needed, after plinit()
      plscol0(0, 255, 255, 255);
      plscol0(15, 0, 0, 0);
    endif

    plreplot;	# do the save
    plend1;
    plsstrm(cur_fig);	# return to previous stream

    if (to_prt == 1)
      system(sprintf("%s %s;", __pl.lp_options, file));
      unlink(file);
    endif
  else
    help save_fig
  endif	

endfunction
