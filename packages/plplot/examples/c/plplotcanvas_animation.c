/*
 * animation.c - Demonstrates the use of the plplot canvas widget with gtk.
 *
 *   Copyright (C) 2004, 2005 Thomas J. Duck
 *   All rights reserved.
 *
 *   Thomas J. Duck <tom.duck@dal.ca>
 *   Department of Physics and Atmospheric Science,
 *   Dalhousie University, Halifax, Nova Scotia, Canada, B3H 3J5
 *
 *   $Author: slbrow $
 *   $Revision: 3186 $
 *   $Date: 2006-02-15 11:17:33 -0700 (Wed, 15 Feb 2006) $
 *   $Name$
 *
 *
 * NOTICE
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *   MA  02111-1307  USA
 *
 *
 * DESCRIPTION
 * 
 *   This program demonstrates the use of the plplot canvas widget with gtk.
 *   Two graphs are draw in a window.  When the Execute button is pressed,
 *   two different waves progress through the graph in real time.  Plotting
 *   to the two graphs is handled in two different threads.
 * 
 */

#include <glib.h>
#include <gtk/gtk.h>

#include <plplotcanvas.h>

#include <math.h>


/* The number of time steps */
#define STEPS 300

/* The number of points and period for the first wave */
#define NPTS 200
#define PERIOD 60

/* The aspect and zoom for each plot widget */
#define ASPECT 2.5
#define ZOOM 0.75

/* Run the plots in different threads */
GThread* thread0=NULL; 
GThread* thread1=NULL;
typedef struct {
  PlplotCanvas* canvas;
  char* title;
} ThreadData;
ThreadData data0,data1;
gint Nthreads = 0; /* Count the number of threads */

/* Create two different canvases */
PlplotCanvas *canvas0=NULL;
PlplotCanvas *canvas1=NULL;

/* Create the x and y array */
static PLFLT x[NPTS], y[NPTS];

/* Lock on the gtkstate so that we don't try to plot after gtk_main_quit */
#define GTKSTATE_CONTINUE (TRUE)
#define GTKSTATE_QUIT (FALSE)
G_LOCK_DEFINE_STATIC(gtkstate);
static volatile int gtkstate = GTKSTATE_CONTINUE;

/* setup_axes - sets up plot and draws axes */
void setup_axes(PlplotCanvas *canvas,char* title)
{
  gdouble xmin,xmax,ymin,ymax;

  /* Plot the axes in the foreground (for persistency) */
  plplot_canvas_use_foreground_group(canvas);

  /* Set up the viewport and window  */
  plplot_canvas_pllsty(canvas,1);
  plplot_canvas_plcol0(canvas,15);
  plplot_canvas_get_viewport(canvas,0.15,0.9,0.2,0.8,&xmin,&xmax,&ymin,&ymax);
  plplot_canvas_plvpor(canvas,xmin,xmax,ymin,ymax);
  plplot_canvas_plwind(canvas,x[0],x[NPTS-1],-2.,2.);
  plplot_canvas_plbox(canvas,"bcnst",0.,0,"bcnstv",0.,0);
  plplot_canvas_pllab(canvas,"(x)","(y)",title);

  /* Return to the default group */
  plplot_canvas_use_default_group(canvas);

  /* Set the drawing color */
  plplot_canvas_plcol0(canvas,plplot_canvas_get_stream_number(canvas)+8); 
}

/* plot - draws a plot on a canvas */
void plot(PlplotCanvas *canvas,gdouble offset,char* title)
{
  int i;
  guint Nstream;
  gdouble xmin,xmax,ymin,ymax;

  /* Get the stream number */
  Nstream = plplot_canvas_get_stream_number(canvas);

  /* Generate the sinusoid */
  for (i = 0; i < NPTS; i++)
    y[i] = sin(2.*3.14*(x[i]+offset*(Nstream+1))/PERIOD/(float)(Nstream+1));

  /* Draw the line */
  plplot_canvas_plline(canvas,NPTS, x, y);

  plplot_canvas_pladv(canvas,0);   /* Advance the page to finalize the plot */
}

/* Delete event callback */
gint delete_event( GtkWidget *widget,GdkEvent *event,gpointer data ) {
  return FALSE;
}

/* Destroy event calback */
void destroy(GtkWidget *widget,gpointer data) {
  G_LOCK(gtkstate);
  gtkstate=GTKSTATE_QUIT;
  G_UNLOCK(gtkstate);

  gtk_main_quit ();
}

GThreadFunc plot_thread(ThreadData* data) {
  int i;

  Nthreads++;

  /* Draw STEPS plots in succession */
  for(i=0;i<STEPS;i++) {
    gdk_threads_enter();

    /* Lock the current gtk state */
    G_LOCK(gtkstate);
  
    /* Check to make sure gtk hasn't quit */
    if(gtkstate == GTKSTATE_QUIT){
      G_UNLOCK(gtkstate);
      gdk_threads_leave();
      g_thread_exit(NULL);
    }
    
    /* Draw the plot */
    plot(data->canvas,i,data->title);

    /* Release the lock */
    G_UNLOCK(gtkstate);
    gdk_threads_leave();
  }

  Nthreads--;
  g_thread_exit(NULL);
}

/* Start threads callback from execute button */
void start_threads(GtkWidget *widget,gpointer data)
{
  GError **gerror;

  /* Ignore call if threads are currently active */
  if(Nthreads) return;

  /* Create the two plotting threads */
  data0.canvas = canvas0;
  data0.title = "A phase-progressing wave";
  if((thread0=g_thread_create((GThreadFunc)plot_thread,&data0,TRUE,gerror))\
     ==NULL) {
    fprintf(stderr,"Could not create thread");
    return;
  }

  data1.canvas = canvas1;
  data1.title = "Another phase-progressing wave";
  if((thread1=g_thread_create((GThreadFunc)plot_thread,&data1,TRUE,gerror))\
     ==NULL) {
    fprintf(stderr,"Could not create thread");
    return;
  }
}

int main(int argc,char *argv[] )
{
  int i;

  GtkWidget *window;
  GtkWidget *table;

  GtkFrame *canvas0frame;
  GtkFrame *canvas1frame;

  GtkButton* button;
  GtkBox* buttonbox;

  GtkBox* vbox;

  /* Initialize */
  g_thread_init(NULL);
  gdk_threads_init();
  gtk_init(&argc, &argv);
  g_type_init();

  /* Initialize the x array */
  for(i=0;i<NPTS;i++) x[i] = (PLFLT)i;

  /* Create the first canvas, set its size, draw some axes on it, and
   *  place it in a frame 
   */
  canvas0 = plplot_canvas_new(FALSE);
  plplot_canvas_use_fast_rendering(canvas0,TRUE);
  plplot_canvas_set_aspect(canvas0,ASPECT);
  plplot_canvas_set_zoom(canvas0,ZOOM);
  plplot_canvas_pladv(canvas0,0);  /* Advance the page */
  setup_axes(canvas0,"A phase-progressing wave");
  canvas0frame = GTK_FRAME(gtk_frame_new(NULL));
  gtk_frame_set_shadow_type(canvas0frame,GTK_SHADOW_ETCHED_OUT);
  gtk_container_add(GTK_CONTAINER(canvas0frame),GTK_WIDGET(canvas0));

  /* Create the second canvas, set its size, draw some axes on it, and
   * place it in a frame
   */
  canvas1 = plplot_canvas_new(FALSE);
  plplot_canvas_use_fast_rendering(canvas1,TRUE);
  plplot_canvas_set_aspect(canvas1,ASPECT);
  plplot_canvas_set_zoom(canvas1,ZOOM);
  plplot_canvas_pladv(canvas1,0);  /* Advance the page */
  setup_axes(canvas1,"Another phase-progressing wave");
  canvas1frame = GTK_FRAME(gtk_frame_new(NULL));
  gtk_frame_set_shadow_type(canvas1frame,GTK_SHADOW_ETCHED_OUT);
  gtk_container_add(GTK_CONTAINER(canvas1frame),GTK_WIDGET(canvas1));

  /* Create a button and put it in a box */
  button = GTK_BUTTON(gtk_button_new_from_stock(GTK_STOCK_EXECUTE));
  g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(start_threads),
		     NULL);
  gtk_container_set_border_width(GTK_CONTAINER(button),10);
  buttonbox = GTK_BOX(gtk_hbox_new(FALSE,0));
  gtk_box_pack_start(buttonbox,GTK_WIDGET(button),TRUE,FALSE,0);

  /* Create and fill the vbox with the widgets */
  vbox = GTK_BOX(gtk_vbox_new(FALSE,0));
  gtk_box_pack_start(vbox,GTK_WIDGET(canvas0frame),TRUE,FALSE,0);
  gtk_box_pack_start(vbox,GTK_WIDGET(canvas1frame),TRUE,FALSE,10);
  gtk_box_pack_start(vbox,GTK_WIDGET(buttonbox),TRUE,FALSE,0);

  /* Create a new window */
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  
  /* Set the border width of the window */
  gtk_container_set_border_width(GTK_CONTAINER(window),10);
  
  /* Connect the signal handlers to the window decorations */
  g_signal_connect(G_OBJECT(window),"delete_event",
		   G_CALLBACK(delete_event),NULL);
  g_signal_connect(G_OBJECT(window),"destroy",G_CALLBACK(destroy),NULL);
  
  /* Put the vbox into the window */
  gtk_container_add(GTK_CONTAINER(window),GTK_WIDGET(vbox));
  
  /* Display everything */
  gtk_widget_show_all(window);
    
  /* Start the gtk main loop */
  gdk_threads_enter();
  gtk_main ();
  gdk_threads_leave();

  return 0;
}
