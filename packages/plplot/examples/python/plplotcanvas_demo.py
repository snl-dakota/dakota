#!/usr/bin/env python
"""
demo.py - Demonstrates the simplest use of the plplot canvas widget with gtk.

  Copyright (C) 2004, 2005 Thomas J. Duck
  All rights reserved.

  Thomas J. Duck <tom.duck@dal.ca>
  Department of Physics and Atmospheric Science,
  Dalhousie University, Halifax, Nova Scotia, Canada, B3H 3J5

  $Author: slbrow $
  $Revision: 3186 $
  $Date: 2006-02-15 11:17:33 -0700 (Wed, 15 Feb 2006) $
  $Name$


NOTICE

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
  MA  02111-1307  USA
"""
# Append to effective python path so that can find plplot modules.
from plplot_python_start import *

import Numeric
import plplotcanvas
import gtk

# The width and height of the plplot canvas widget
WIDTH = 1000 # 500
HEIGHT = 600 # 300

# Delete event callback
def delete_event(widget, event, data=None):
    return gtk.FALSE

# Destroy event calback
def destroy(widget, data=None):
    gtk.main_quit()

# The data to plot
x = Numeric.arange(11)
y = x**2/10.

# Create the canvas and set its size; during the creation process,
# the gcw driver is loaded into plplot, and plinit() is invoked.
canvas=plplotcanvas.Canvas(aa=True)  
canvas.set_size(WIDTH,HEIGHT)

# Determine the viewport so that everything is drawn on the canvas
xmin,xmax,ymin,ymax = canvas.get_viewport(0.12,0.95,0.15,0.88)

# Create a new window and stuff the canvas into it
window = gtk.Window(gtk.WINDOW_TOPLEVEL)
window.set_border_width(10)
window.add(canvas)

# Connect the signal handlers to the window decorations
window.connect("delete_event",delete_event)
window.connect("destroy",destroy)

# Display everything
window.show_all()

# Draw on the canvas with Plplot
canvas.pladv(0)  # Advance the page
canvas.plcol0(15) # Set color to black
canvas.plwid(2) # Set the pen width
canvas.plvpor(xmin,xmax,ymin,ymax); # Set the viewport
canvas.plwind(0.,10.,0.,10.); # Set the window
canvas.plbox("bcnst",0.,0,"bcnstv",0.,0); # Set the box
canvas.pllab("x-axis","y-axis","A Simple Plot") # Draw some labels

# Draw the line
canvas.plcol0(1) # Set the pen color
canvas.plline(x,y)

# Advancing the page finalizes this plot
canvas.pladv(0)

# Start the gtk main loop
gtk.main()
