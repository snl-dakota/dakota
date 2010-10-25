#!/usr/bin/env python
"""
animation.py - Demonstrates the use of the plplot canvas widget with gtk.

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


DESCRIPTION

  This program demonstrates the use of the plplot canvas widget with gtk.
  Two graphs are draw in a window.  When the Execute button is pressed,
  two different waves progress through the graph in real time.  Plotting
  to the two graphs is handled in two different threads.
"""
# Append to effective python path so that can find plplot modules.
from plplot_python_start import *

import sys,threading
import Numeric
import plplotcanvas
import gtk


# The number of time steps
STEPS = 300

# The number of points and period for the first wave
NPTS = 200
PERIOD = 60

# The aspect and zoom for each plot widget
ASPECT = 2.5
ZOOM = 0.75

# Run the plots in different threads
thread0 = None
thread1 = None

# Create two different canvases
canvas0=None
canvas1=None

# Create the x array
x = Numeric.arange(NPTS)

# Lock on the gtkstate so that we don't try to plot after gtk_main_quit
GTKSTATE_CONTINUE=True
GTKSTATE_QUIT=False
gtk_state_lock = threading.Lock()
gtk_state = GTKSTATE_CONTINUE

# setup_axes - sets up plot and draws axes
def setup_axes(canvas,title):

    # Plot the axes in the foreground (for persistency)
    canvas.use_foreground_group()
    
    # Set up the viewport and window
    canvas.pllsty(1)
    canvas.plcol0(15)
    xmin,xmax,ymin,ymax = canvas.get_viewport(0.15,0.9,0.2,0.8);
    canvas.plvpor(xmin,xmax,ymin,ymax);
    canvas.plwind(x[0],x[NPTS-1],-2.,2.);
    canvas.plbox("bcnst",0.,0,"bcnstv",0.,0);
    canvas.pllab("Phase","Amplitude",title);

    # Return to the default group
    canvas.use_default_group()

    # Set the drawing color
    canvas.plcol0(canvas.get_stream_number()+8)

# plot - draws a plot on a canvas
def plot(canvas,offset,title):

    global x

    # Get the stream number
    Nstream = canvas.get_stream_number()

    # Generate the sinusoid
    y = Numeric.sin(2.*3.14*(x+offset*(Nstream+1))/PERIOD/(Nstream+1))
    
    # Draw the line
    canvas.plline(x, y)

    # Advance the page
    canvas.pladv(0)

# Delete event callback
def delete_event(widget, event, data=None):
    return gtk.FALSE

# Destroy event calback
def destroy(widget, data=None):

    global gtk_state
    
    gtk_state_lock.acquire()
    gtk_state = GTKSTATE_QUIT
    gtk_state_lock.release()

    gtk.main_quit()
    
def plot_thread(canvas,title):

    # Draw plots in succession
    for i in range(STEPS):
        gtk.threads_enter()

        # Lock the current gtk state
        gtk_state_lock.acquire()

        # Check to make sure gtk hasn't quit
        if gtk_state == GTKSTATE_QUIT:
            gtk_state_lock.release()
            gtk.threads_leave()
            return

        # Draw the plot
        plot(canvas,i,title)

        # Release the lock
        gtk_state_lock.release()
        gtk.threads_leave()

# Start threads callback from execute button
def start_threads(widget,data):

    global thread0
    global thread1

    # Ignore call if threads are currently active
    if (thread0!=None or thread1!=None) and \
       (thread0.isAlive() or thread1.isAlive()): return
    
    # Create the two plotting threads
    thread0 = threading.Thread(None,plot_thread,
                               kwargs={"canvas":canvas0,
                               "title":"A phase-progressing wave"})
    thread0.start()

    thread1 = threading.Thread(None,plot_thread,
                               kwargs={"canvas":canvas1,
                               "title":"Another phase-progressing wave"})
    thread1.start()
  
if __name__ == "__main__":

    # Initialize
    gtk.threads_init()

    # Create the first canvas, set its size, draw some axes on it, and
    # place it in a frame
    canvas0=plplotcanvas.Canvas(aa=False)
    canvas0.use_fast_rendering(True)
    canvas0.set_aspect(ASPECT)
    canvas0.set_zoom(ZOOM)
    canvas0.pladv(0)  # Advance the page
    setup_axes(canvas0,"A phase-progressing wave")
    canvas0frame=gtk.Frame()
    canvas0frame.set_shadow_type(type=gtk.SHADOW_ETCHED_OUT)
    canvas0frame.add(canvas0)

    # Create the second canvas, set its size, draw some axes on it, and
    # place it in a frame
    canvas1=plplotcanvas.Canvas(aa=False)
    canvas1.use_fast_rendering(True)
    canvas1.set_aspect(ASPECT)
    canvas1.set_zoom(ZOOM)
    canvas1.pladv(0)  # Advance the page
    setup_axes(canvas1,"Another phase-progressing wave")
    canvas1frame=gtk.Frame()
    canvas1frame.set_shadow_type(type=gtk.SHADOW_ETCHED_OUT)
    canvas1frame.add(canvas1)

    # Create a button and put it in a box
    button = gtk.Button (stock=gtk.STOCK_EXECUTE);
    button.connect("clicked", start_threads, None)   
    button.set_border_width(10)
    buttonbox = gtk.HBox()
    buttonbox.pack_start(button,True,False,0)
    
    # Create and fill the vbox with the widgets
    vbox = gtk.VBox()
    vbox.pack_start(canvas0frame,True,False,0)
    vbox.pack_start(canvas1frame,True,False,10)
    vbox.pack_start(buttonbox,True,False,0)
        
    # Create a new window
    window = gtk.Window(gtk.WINDOW_TOPLEVEL)
    
    # Set the border width of the window
    window.set_border_width(10)

    # Connect the signal handlers to the window decorations
    window.connect("delete_event",delete_event)
    window.connect("destroy",destroy)

    # Put the vbox into the window
    window.add(vbox)
    
    # Display everything
    window.show_all()

    # Start the gtk main loop
    gtk.threads_enter()
    gtk.main()
    gtk.threads_leave()
