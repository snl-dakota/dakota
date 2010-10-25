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

## set user defaults
##
## Copy this script to your own Octave directory and uncomment/add the desired defaults.
##
## The standard defaults, defined elsewhere, are
##
## plSetOpt("geometry", "400x400+800+1");
## plSetOpt("np", "");
##
## and will be override by the defaults set here.
## Also, for special needs, plsetopt() can be used to add more options
## or override the ones defined here.
##
## see also plsetopt() for a complete list of options.

function pldef
  
  ## set some options, as if they where in the command line, before plinit()

  ## The DISPLAY name. Not needed
  ## plSetOpt("display", getenv("DISPLAY"));

  ## The window geometry								
  ## plSetOpt("geometry", "400x400+800+1");

  ## bytes sent before flushing output
  ## plSetOpt("bufmax", "100000");

  ## Double buffer (smoother. Must use pleop() to flush).
  ## Fails with comet()!! (plplot plxormod() function)
  ##  plSetOpt("db", "");

  ## Don't wait for keypress to advance plots
  ## plSetOpt("np", "");

  ## Start in debug mode
  ## plSetOpt("debug", "");

  ## verbose
  ## plSetOpt("verbose", "");

  ## Dont't use pixmap (faster but stepper scrolling)
  ## must use in double buffering mode
  ## plSetOpt("nopixmap", "");

  ## Set background colour to white if you prefer. It might be important for
  ## printing from the xwin driver. Plot colors don't change.

  ## gray background
  ## plSetOpt("bg", "D8D8D8");
  ## plscol0(0, 216, 216, 216);

  ## white background
  ## plSetOpt("bg", "FFFFFF");
  ## plscol0(0, 255, 255, 255);
  ## plscol0(15, 0, 0, 0);

  ## Defaults to be done after opening a plot window.
  ## can't be set here!

  ## Change color 15 from white to back
  ## (usefull if you change background to white)
  ## plscol0(15, 0, 0, 0);

  ## Change color 0 from back to white. Background also change.
  ## (usefull if you change background to white)
  ## plscol0(0, 255, 255, 255);

  ## Don't wait for keypress to advance plots
  ## also in the "command line" options, "-np"
  ## plspause(0)


endfunction 
