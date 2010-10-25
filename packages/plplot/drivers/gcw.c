/* gcw - PLplot Gnome Canvas Widget device driver.

  Copyright (C) 2004, 2005  Thomas J. Duck
  Copyright (C) 2004  Rafael Laboissiere
  All rights reserved.


NOTICE

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
  USA 


DESCRIPTION

  This is the Gnome Canvas Widget driver, written by Thomas J. Duck 
  following the heritage of the plplot Gnome driver.  Like all plplot
  drivers, this operates in standalone mode by default.  However, this
  driver can also be used to write to a user-supplied GnomeCanvas.

  Please see the plplot documentation for more information.


DEVELOPMENT NOTES

  Truetype text is supplied using the PLPLOT_CANVAS_HACKTEXT item,
  which was cloned from gnome-print.  This text item was chosen because 
  it rotates and scales under a zoom correctly and easily.

  It would be better to use GNOME_CANVAS_TEXT, but currently 
  (4 March 2005) it doesn't rotate or scale under a zoom on the 
  GnomeCanvas.  GNOME_CANVAS_TEXT uses Pango, and rotations were only
  recently implemented in the Pango API (i.e., Fall 2004).  If the
  Pango API is used directly, the bounding box doesn't rotate with the 
  text on GnomeCanvas, which results in clipping.  It is likely that
  GnomeCanvas is not querying the bounding box from Pango correctly,
  and is not directing Pango to scale.  So, GnomeCanvas needs to be 
  updated to deal with Pango properly.

  Another problem is that drawing polylines on the Gnome Canvas sometimes
  results in an 'attempt to put segment in horiz list twice' error.
  The workaround here is to plot single line segments only, but this
  results in a performance hit.  This problem will need to be corrected
  in the GnomeCanvas.


KNOWN BUGS

  Dashed lines and area hatching don't work properly.  They should be 
  handled by the plplot core, but it isn't working correctly.

  Text clipping is not completely working, although it is implemented
  here.  Proper text clipping will require Pango, as it is not
  implemented for Hacktext.

  PLplot test suite problems:

    1) Example x10c does not clip the text.

    2) Example x17c, the strip chart demo, doesn't do a strip chart 
       (try the xwin driver to see how it should work).  This example
       probably isn't so important for this driver, which gives 
       alternative and more flexible ways of drawing animations.

    3) Example x20c freezes during the drawing of Lena, likely due to 
       too many items on the canvas.

*/

#include "gcw.h"
#include "plplotcanvas-hacktext.h"

#ifdef HAVE_FREETYPE

#include "plfreetype.h"
#include "plfci-truetype.h"

/* Font lookup table that is constructed in plD_FreeType_init*/
extern FCI_to_FontName_Table FontLookup[N_TrueTypeLookup];

extern void plD_FreeType_init(PLStream *pls);
extern void plD_FreeType_Destroy(PLStream *pls);

#endif /* HAVE_FREETYPE */


/* Device info */
char* plD_DEVICE_INFO_libgcw = "gcw:Gnome Canvas Widget:1:gcw:10:gcw";

/*
#define DEBUG_GCW
*/

/*
#define ASSERT_GCW
*/

#define POLYGON_GCW

#define OUTLINE_POLYGON_GCW

/* Physical dimensions */

/* Drawing units per inch; 1 DU corresponds to a pen width of 1 */
#define DU_PER_IN (360.)

/* Pixels per drawing unit: The default value of 0.2 pixes per drawing 
 * unit gives 72 pixels per inch. */
#define PIXELS_PER_DU (0.2)

/* mm per inch */
#define MM_PER_IN (25.4)

/* pixels per mm for this device */
#define PIXELS_PER_MM (PIXELS_PER_DU * DU_PER_IN / MM_PER_IN)

/* Default dimensions of the canvas (in inches) */
#define CANVAS_WIDTH (9.0)
#define CANVAS_HEIGHT (7.0)

/* The zoom factor for 100% zoom in */
#define ZOOM100 1.5
#define ZOOMSTEP 1.25

/* The scale factor for line widths */
#define WSCALE 3

/* Global driver options */

static PLINT aa = 1;

#ifdef HAVE_FREETYPE
static PLINT text = 1;
#else
static PLINT text = 0;
#endif

static PLINT hrshsym = 0;
static PLINT fast = 0; 
static PLINT pixmap = 0;


static DrvOpt gcw_options[] = 
  {
    {"aa", DRV_INT, &aa, "Use antialiased canvas (aa=0|1)"},
    {"text", DRV_INT, &text, "Use truetype fonts (text=0|1)"},
    {"hrshsym", DRV_INT, &hrshsym, "Use Hershey symbol set (hrshsym=0|1)"},
    {"fast", DRV_INT, &fast, "Use fast rendering (fast=0|1)"},
    {"pixmap", DRV_INT, &pixmap, "Use pixmap for plotting shades (pixmap=0|1)"},
    {NULL, DRV_INT, NULL, NULL}
  };

void debug (char* msg)
{
  fprintf(stderr,msg);
  fflush(stderr);
}


/*--------------------------------------------------------------------------*\
 * plcolor_to_rgba()
 *
 * Converts from the plcolor to rgba color format.
\*--------------------------------------------------------------------------*/
static guint32 plcolor_to_rgba(PLColor color, guchar alpha)
{
  return
    ((int)(color.r) << 24)
    + ((int)(color.g) << 16)
    + ((int)(color.b) << 8)
    + alpha;
}


/*--------------------------------------------------------------------------*\
 * install_canvas()
 *
 * Creates and installs a canvas into a window on the fly.  This function
 * is called if a canvas hasn't been supplied to the driver.
\*--------------------------------------------------------------------------*/

/* Delete event callback */
gint delete_event(GtkWidget *widget, GdkEvent *event, gpointer data ) {
  return FALSE;
}

/* Destroy event calback */
void destroy(GtkWidget *widget, gpointer data) {
  gtk_main_quit ();
}

/* All-purpose zoom callback */
void zoom(gpointer data, gint flag) {

  gint n;

  GtkNotebook *notebook;
  GnomeCanvas *canvas;
  GtkWidget *scrolled_window;
  GList *list;

  GcwPLdev* dev;

  gdouble curmag,dum;

  /* Get the current canvas */
  notebook = GTK_NOTEBOOK(data);
  n = gtk_notebook_get_current_page(notebook);
  scrolled_window = gtk_notebook_get_nth_page(notebook,n);
  canvas = GNOME_CANVAS(gtk_container_get_children(
	   GTK_CONTAINER(gtk_container_get_children(
           GTK_CONTAINER(scrolled_window))->data))->data);

  /* Retrieve the device */
  dev = g_object_get_data(G_OBJECT(canvas),"dev");

  /* Determine the new magnification */
  if(flag==2) /* Zoom in */
    gcw_set_canvas_zoom(canvas,ZOOMSTEP);
  else if(flag==0) /* Zoom out */
    gcw_set_canvas_zoom(canvas,1./ZOOMSTEP);
  else { /* Zoom 100 */
    /* Get the current magnification */
    if(dev->zoom_is_initialized) gnome_canvas_c2w(canvas,1,0,&curmag,&dum);
    curmag = 1./curmag;
    gcw_set_canvas_zoom(canvas,(PLFLT)(ZOOM100/curmag));
  }

  /* Set the focus on the notebook */
  gtk_window_set_focus(GTK_WINDOW(dev->window),GTK_WIDGET(dev->notebook));
}


/* Callback when zoom in button is pressed */
void zoom_in(GtkWidget *widget, gpointer data ) {
  zoom(data,2);
}

/* Callback when zoom 100 button is pressed */
void zoom_100(GtkWidget *widget, gpointer data ) {
  zoom(data,1);
}

/* Callback when zoom out button is pressed */
void zoom_out(GtkWidget *widget, gpointer data ) {
  zoom(data,0);
}

/* Callback when keys are released */
void key_release(GtkWidget *widget, GdkEventKey  *event, gpointer data ) {
  if(event->keyval == '+')  zoom(data,2);
  if(event->keyval == '=')  zoom(data,1);
  if(event->keyval == '-')  zoom(data,0);
  if(event->keyval == 'q')  destroy(widget,data);
}

void install_canvas(PLStream *pls)
{
  GcwPLdev* dev = pls->dev;
  GtkWidget *window,*vbox,*hbox,*button,*image,*scrolled_window;
  GnomeCanvas *canvas;

  gboolean flag = FALSE;

#ifdef DEBUG_GCW
  debug("<install_canvas>\n");
#endif

  if(dev->window==NULL) {

    flag = TRUE;

    /* Create a new window and prepare a notebook for it */

    /* Initialize */
    g_type_init();
    gtk_init(0,NULL);

    /* Create a new window */
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    dev->window = window;
    gtk_window_set_title(GTK_WINDOW(window),"PLplot");

    /* Connect the signal handlers to the window decorations */
    g_signal_connect(G_OBJECT(window),"delete_event",
		     G_CALLBACK(delete_event),NULL);
    g_signal_connect(G_OBJECT(window),"destroy",G_CALLBACK(destroy),NULL);

    /* Create a hbox and put it into the window */
    hbox = gtk_hbox_new(FALSE,0);
    gtk_container_add(GTK_CONTAINER(window),GTK_WIDGET(hbox));

    /* Add an vbox to the hbox */
    vbox = gtk_vbox_new(FALSE,5);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
    gtk_box_pack_start(GTK_BOX(hbox),GTK_WIDGET(vbox),FALSE,FALSE,0);

    /* Create the new notebook and add it to the hbox*/
    dev->notebook = gtk_notebook_new();
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(dev->notebook),TRUE);
    gtk_box_pack_start(GTK_BOX(hbox),GTK_WIDGET(dev->notebook),TRUE,TRUE,0);
    g_signal_connect(G_OBJECT(dev->notebook), "key_release_event",
                         G_CALLBACK(key_release), G_OBJECT(dev->notebook));

    /* Use a few labels as spacers */
    gtk_box_pack_start(GTK_BOX(vbox),GTK_WIDGET(gtk_label_new(" ")),
		       FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(vbox),GTK_WIDGET(gtk_label_new(" ")),
		       FALSE,FALSE,0);

    /* Add buttons to the vbox */

    /* Add zoom in button and create callbacks */
    image = gtk_image_new_from_stock(GTK_STOCK_ZOOM_IN,
				     GTK_ICON_SIZE_SMALL_TOOLBAR);
    button = gtk_button_new();
    gtk_container_add(GTK_CONTAINER(button), GTK_WIDGET(image));
    gtk_box_pack_start(GTK_BOX(vbox),GTK_WIDGET(button),FALSE,FALSE,0);
    g_signal_connect (G_OBJECT(button), "clicked",
		      G_CALLBACK(zoom_in), G_OBJECT(dev->notebook));
    g_signal_connect(G_OBJECT(button), "key_release_event",
                         G_CALLBACK(key_release), G_OBJECT(dev->notebook));

    /* Add zoom100 button and create callbacks */
    image = gtk_image_new_from_stock(GTK_STOCK_ZOOM_100,
				     GTK_ICON_SIZE_SMALL_TOOLBAR);
    button = gtk_button_new();
    gtk_container_add(GTK_CONTAINER(button), GTK_WIDGET(image));
    gtk_box_pack_start(GTK_BOX(vbox),GTK_WIDGET(button),FALSE,FALSE,0);
    g_signal_connect (G_OBJECT(button), "clicked",
		      G_CALLBACK(zoom_100), G_OBJECT(dev->notebook));

    /* Add zoom out button and create callbacks */
    image = gtk_image_new_from_stock(GTK_STOCK_ZOOM_OUT,
				     GTK_ICON_SIZE_SMALL_TOOLBAR);
    button = gtk_button_new();
    gtk_container_add(GTK_CONTAINER(button), GTK_WIDGET(image));
    gtk_box_pack_start(GTK_BOX(vbox),GTK_WIDGET(button),FALSE,FALSE,0);
    g_signal_connect (G_OBJECT(button), "clicked",
		      G_CALLBACK(zoom_out), G_OBJECT(dev->notebook));
    g_signal_connect(G_OBJECT(button), "key_release_event",
                         G_CALLBACK(key_release), G_OBJECT(dev->notebook));
  }

  /* Create a new canvas */
  if(aa) {
    canvas = GNOME_CANVAS(gnome_canvas_new_aa());
    dev->aa = TRUE;
  }
  else {
    canvas = GNOME_CANVAS(gnome_canvas_new());
    dev->aa = FALSE;
  }

  gcw_set_canvas(pls,canvas);
  gcw_set_canvas_zoom(canvas,ZOOM100);

  /* Put the canvas in a scrolled window */
  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled_window), 
					GTK_WIDGET(canvas));

  /* Install the scrolled window in the notebook */
  gtk_notebook_append_page(GTK_NOTEBOOK(dev->notebook),
			   GTK_WIDGET(scrolled_window), NULL);

  if(flag) {

    /* Set the focus on the notebook */
    gtk_window_set_focus(GTK_WINDOW(window),GTK_WIDGET(dev->notebook));

    /* Size the window */
    gtk_window_resize(GTK_WINDOW(window),dev->width*ZOOM100+50,
 		      dev->height*ZOOM100+50);
  }

  /* Display everything */
  gtk_widget_show_all(dev->window);
}


/*--------------------------------------------------------------------------*\
 * gcw_set_canvas()
 *
 * Adds a canvas to the newly created stream.  
 *
 * The associated device is attached to the canvas as the property "dev".
\*--------------------------------------------------------------------------*/
void gcw_set_canvas(PLStream* pls,GnomeCanvas* canvas)
{
  GcwPLdev* dev = pls->dev;

#ifdef DEBUG_GCW
  debug("<gcw_set_canvas>\n");
#endif

  /* Add the canvas to the device */
  dev->canvas=canvas;

  if(!GNOME_IS_CANVAS(canvas)) {
    fprintf(stderr,"\n\n*** GCW driver error: canvas not found.\n");
    return;
  }

  /* Attach the device to the canvas widget */
  g_object_set_data(G_OBJECT(canvas),"dev",(gpointer)dev);

  /* Determine if the canvas is antialiased */
  g_object_get(G_OBJECT(canvas),"aa",&(dev->aa),NULL);


  /* Add the background to the canvas and move it to the back */
  if(!GNOME_IS_CANVAS_ITEM(
    dev->group_background = (GnomeCanvasGroup*)gnome_canvas_item_new(
					  gnome_canvas_root(canvas),
					  gnome_canvas_clipgroup_get_type(),
					  "x",0.,
					  "y",0.,
					  NULL)
    )) {
    fprintf(stderr,"\n\n*** GCW driver error: item not created.\n");
    return;
  }

  /* Set the clip to NULL */
  g_object_set(G_OBJECT(dev->group_background),"path",NULL,NULL);

  if(!GNOME_IS_CANVAS_ITEM(
    dev->background = gnome_canvas_item_new(
		        dev->group_background,
			GNOME_TYPE_CANVAS_RECT,
			"x1", 0.0,
			"y1", -dev->height,
			"x2", dev->width,
			"y2", 0.0,
			"fill-color-rgba",
			plcolor_to_rgba(pls->cmap0[0],0xFF),
			"width-units", 0.0,
			NULL)
    )) {
    fprintf(stderr,"\n\n*** GCW driver error: item not created.\n");
    return;
  }

  gnome_canvas_item_lower_to_bottom(GNOME_CANVAS_ITEM(dev->group_background));

  /* Add the foreground to the canvas and move it to the front */
  if(!GNOME_IS_CANVAS_ITEM(
    dev->group_foreground = (GnomeCanvasGroup*)gnome_canvas_item_new(
					  gnome_canvas_root(canvas),
					  gnome_canvas_clipgroup_get_type(),
					  "x",0.,
					  "y",0.,
					  NULL)
    )) {
    fprintf(stderr,"\n\n*** GCW driver error: item not created.\n");
    return;
  }

  /* Set the clip to NULL */
  g_object_set(G_OBJECT(dev->group_foreground),"path",NULL,NULL);

  gnome_canvas_item_raise_to_top(GNOME_CANVAS_ITEM(dev->group_foreground));

  /* Set the canvas width and height */
  gcw_set_canvas_aspect(canvas,CANVAS_WIDTH/CANVAS_HEIGHT);

  /* Get the colormap */
  dev->colormap = gtk_widget_get_colormap(GTK_WIDGET(dev->canvas));
}


/*--------------------------------------------------------------------------*\
 * gcw_set_canvas_aspect()
 *
 * Sets the aspect (the canvas width/height) by changing the region on the 
 * canvas that is visible. Note that the user will have to correctly set the 
 * plplot viewport to take this into account.
\*--------------------------------------------------------------------------*/
void gcw_set_canvas_aspect(GnomeCanvas* canvas,PLFLT aspect)
{
  GcwPLdev* dev;
  gdouble default_aspect = CANVAS_WIDTH/CANVAS_HEIGHT;

#ifdef DEBUG_GCW
  debug("<gcw_set_canvas_aspect>\n");
#endif

  if(!GNOME_IS_CANVAS(canvas)) {
    fprintf(stderr,"\n\n*** GCW driver error: canvas not found.\n");
    return;
  }

  /* Retrieve the device */
  dev = g_object_get_data(G_OBJECT(canvas),"dev");

  /* Set the new width and height */
  if(aspect < default_aspect) {
    dev->width = PIXELS_PER_DU*DU_PER_IN*CANVAS_WIDTH *
      (gdouble)(aspect)/default_aspect;
    dev->height = PIXELS_PER_DU * DU_PER_IN * CANVAS_HEIGHT;
  }
  else {
    dev->height = PIXELS_PER_DU*DU_PER_IN*CANVAS_HEIGHT*default_aspect / 
      (gdouble)(aspect);
    dev->width = PIXELS_PER_DU * DU_PER_IN * CANVAS_WIDTH;
  }

  /* Resize the background */
  g_object_set(G_OBJECT(dev->background),"x1",0.,"y1",-dev->height,
	       "x2",dev->width,"y2",0.,NULL);
}

/*--------------------------------------------------------------------------*\
 * gcw_set_canvas_zoom()
 *
 * Sets the zoom magnification on the canvas and resizes the widget
 * appropriately.
\*--------------------------------------------------------------------------*/
void gcw_set_canvas_zoom(GnomeCanvas* canvas,PLFLT magnification)
{
  GcwPLdev* dev;

  gdouble curmag=1.,dum;

#ifdef DEBUG_GCW
  debug("<gcw_set_canvas_zoom>\n");
#endif

  if(!GNOME_IS_CANVAS(canvas)) {
    fprintf(stderr,"\n\n*** GCW driver error: canvas not found.\n");
    return;
  }

  /* Retrieve the device */
  dev = g_object_get_data(G_OBJECT(canvas),"dev");

  /* Get the current magnification */
  if(dev->zoom_is_initialized) gnome_canvas_c2w(canvas,1,0,&curmag,&dum);
  curmag = 1./curmag;

  gnome_canvas_set_pixels_per_unit(canvas,magnification*curmag);

  gtk_widget_set_size_request(GTK_WIDGET(canvas),
			      (dev->width+1)*magnification*curmag,
			      (dev->height+2)*magnification*curmag);

  gnome_canvas_set_scroll_region(canvas,0,-dev->height,
				 dev->width+1,1);

  dev->zoom_is_initialized = TRUE;
}


/*--------------------------------------------------------------------------*\
 * gcw_set_canvas_size()
 *
 * Sets the canvas size by setting the aspect and zoom
\*--------------------------------------------------------------------------*/
void gcw_set_canvas_size(GnomeCanvas* canvas,PLFLT width,PLFLT height)
{
  PLFLT magnification;
  GcwPLdev* dev;

#ifdef DEBUG_GCW
  debug("<gcw_set_canvas_size>\n");
#endif

  if(!GNOME_IS_CANVAS(canvas)) {
    fprintf(stderr,"\n\n*** GCW driver error: canvas not found.\n");
    return;
  }

  /* Retrieve the device */
  dev = g_object_get_data(G_OBJECT(canvas),"dev");

  gcw_set_canvas_aspect(canvas,width/height);

  magnification = width/(PLFLT)(dev->width);

  gcw_set_canvas_zoom(canvas,magnification);
}


/*--------------------------------------------------------------------------*\
 * gcw_get_canvas_viewport()
 *
 * Given a plplot viewport, this routine adjusts it taking the device
 * aspect into account.
\*--------------------------------------------------------------------------*/
void gcw_get_canvas_viewport(GnomeCanvas* canvas,PLFLT xmin1,PLFLT xmax1,
			     PLFLT ymin1,PLFLT ymax1,PLFLT* xmin2,PLFLT* xmax2,
			     PLFLT* ymin2,PLFLT* ymax2)
{
  GcwPLdev* dev;

#ifdef DEBUG_GCW
  debug("<gcw_set_canvas_viewport>\n");
#endif

  if(!GNOME_IS_CANVAS(canvas)) {
    fprintf(stderr,"\n\n*** GCW driver error: canvas not found.\n");
    return;
  }

  /* Retrieve the device */
  dev = g_object_get_data(G_OBJECT(canvas),"dev");

  *xmin2 = xmin1*(PLFLT)(dev->width)/(PIXELS_PER_DU*DU_PER_IN*CANVAS_WIDTH);
  *xmax2 = xmax1*(PLFLT)(dev->width)/(PIXELS_PER_DU*DU_PER_IN*CANVAS_WIDTH);
  *ymin2 = ymin1*(PLFLT)(dev->height)/(PIXELS_PER_DU*DU_PER_IN*CANVAS_HEIGHT);
  *ymax2 = ymax1*(PLFLT)(dev->height)/(PIXELS_PER_DU*DU_PER_IN*CANVAS_HEIGHT);
}


/*--------------------------------------------------------------------------*\
 * gcw_use_text()
 *
 * Used to turn text usage on and off.
\*--------------------------------------------------------------------------*/
void gcw_use_text(GnomeCanvas* canvas,PLINT use_text)
{
  GcwPLdev* dev;

#ifdef DEBUG_GCW
  debug("<gcw_use_text>\n");
#endif

  if(!GNOME_IS_CANVAS(canvas)) {
    fprintf(stderr,"\n\n*** GCW driver error: canvas not found.\n");
    return;
  }

  /* Retrieve the device */
  dev = g_object_get_data(G_OBJECT(canvas),"dev");

#ifdef HAVE_FREETYPE
  dev->use_text = (gboolean)use_text;

  /* Use a hack to the plplot escape mechanism to update text handling */
  plP_esc(PLESC_HAS_TEXT,NULL);
#endif

}


/*--------------------------------------------------------------------------*\
 * gcw_use_fast_rendering()
 *
 * Used to turn fast rendering on and off.  This matters in 
 * plD_polyline_gcw, where fast rendering can cause errors on the
 * GnomeCanvas.
\*--------------------------------------------------------------------------*/
void gcw_use_fast_rendering(GnomeCanvas* canvas,PLINT use_fast_rendering)
{
  GcwPLdev* dev;

#ifdef DEBUG_GCW
  debug("<gcw_use_fast_rendering>\n");
#endif

  if(!GNOME_IS_CANVAS(canvas)) {
    fprintf(stderr,"\n\n*** GCW driver error: canvas not found.\n");
    return;
  }

  /* Retrieve the device */
  dev = g_object_get_data(G_OBJECT(canvas),"dev");

  dev->use_fast_rendering = (gboolean)use_fast_rendering;
}


/*--------------------------------------------------------------------------*\
 * gcw_use_pixmap()
 *
 * Used to turn pixmap usage on and off for polygon fills (used during
 * shading calls).
\*--------------------------------------------------------------------------*/

void clear_pixmap(PLStream* pls)
{
  GcwPLdev* dev=pls->dev;
  GdkGC* gc;
  GdkColor color;
  PLColor plcolor = pls->cmap0[0];

  /* Allocate the background color*/
  color.red=(guint16)(plcolor.r/255.*65535); 
  color.green=(guint16)(plcolor.g/255.*65535); 
  color.blue=(guint16)(plcolor.b/255.*65535);
  gdk_colormap_alloc_color(dev->colormap,&color,FALSE,TRUE);

  /* Clear the pixmap with the background color */
  gc = gdk_gc_new(dev->pixmap);
  gdk_gc_set_foreground(gc,&color);
  gdk_draw_rectangle(dev->pixmap,gc,TRUE,0,0,dev->width,dev->height);
  gdk_gc_unref(gc);
}

void gcw_use_pixmap(GnomeCanvas* canvas,PLINT use_pixmap)
{
  GcwPLdev* dev;

  if(!GNOME_IS_CANVAS(canvas)) {
    fprintf(stderr,"\n\n*** GCW driver error: canvas not found.\n");
    return;
  }

  /* Retrieve the device */
  dev = g_object_get_data(G_OBJECT(canvas),"dev");

  dev->use_pixmap=(gboolean)use_pixmap;

  /* Allocate the pixmap */
  if(use_pixmap) {
    dev->pixmap = gdk_pixmap_new(NULL,dev->width,
				 dev->height,gdk_visual_get_best_depth());
  }

  dev->pixmap_has_data = FALSE;
}


/*--------------------------------------------------------------------------*\
 * gcw_use_*_group()
 *
 * Used to switch which groups plplot is writing to.  The choices are:
 *
 *  background - persistent (never clears)
 *  foreground - persistent (never clears)
 *  default - shown when the page is advanced, cleared for next page
 *
\*--------------------------------------------------------------------------*/
void gcw_use_background_group(GnomeCanvas* canvas)
{
  GcwPLdev* dev;

  if(!GNOME_IS_CANVAS(canvas)) {
    fprintf(stderr,"\n\n*** GCW driver error: canvas not found.\n");
    return;
  }

  /* Retrieve the device */
  dev = g_object_get_data(G_OBJECT(canvas),"dev");

  dev->group_current = dev->group_background;
}
void gcw_use_foreground_group(GnomeCanvas* canvas)
{
  GcwPLdev* dev;

  if(!GNOME_IS_CANVAS(canvas)) {
    fprintf(stderr,"\n\n*** GCW driver error: canvas not found.\n");
    return;
  }

  /* Retrieve the device */
  dev = g_object_get_data(G_OBJECT(canvas),"dev");

  dev->group_current = dev->group_foreground;
}
void gcw_use_default_group(GnomeCanvas* canvas)
{
  GcwPLdev* dev;

  if(!GNOME_IS_CANVAS(canvas)) {
    fprintf(stderr,"\n\n*** GCW driver error: canvas not found.\n");
    return;
  }

  /* Retrieve the device */
  dev = g_object_get_data(G_OBJECT(canvas),"dev");

  dev->group_current = dev->group_hidden;
}


/*--------------------------------------------------------------------------*\
 * plD_dispatch_init_gcw()
 *
 * Initializes the dispatch table.
\*--------------------------------------------------------------------------*/

void plD_open_gcw(PLStream *pls);
void plD_init_gcw(PLStream *);
void plD_line_gcw(PLStream *, short, short, short, short);
void plD_polyline_gcw(PLStream *, short *, short *, PLINT);
void plD_eop_gcw(PLStream *);
void plD_bop_gcw(PLStream *);
void plD_tidy_gcw(PLStream *);
void plD_state_gcw(PLStream *, PLINT);
void plD_esc_gcw(PLStream *, PLINT, void *);

void plD_dispatch_init_gcw( PLDispatchTable *pdt )
{

#ifdef DEBUG_GCW
  debug("<plD_dispatch_init_gcw>\n");
#endif

#ifndef ENABLE_DYNDRIVERS
  pdt->pl_MenuStr  = "Gnome Canvas Widget";
  pdt->pl_DevName  = "gcw";
#endif
  pdt->pl_type     = plDevType_Interactive;
  pdt->pl_seq      = 1;
  pdt->pl_init     = (plD_init_fp)     plD_init_gcw;
  pdt->pl_line     = (plD_line_fp)     plD_line_gcw;
  pdt->pl_polyline = (plD_polyline_fp) plD_polyline_gcw;
  pdt->pl_eop      = (plD_eop_fp)      plD_eop_gcw;
  pdt->pl_bop      = (plD_bop_fp)      plD_bop_gcw;
  pdt->pl_tidy     = (plD_tidy_fp)     plD_tidy_gcw;
  pdt->pl_state    = (plD_state_fp)    plD_state_gcw;
  pdt->pl_esc      = (plD_esc_fp)      plD_esc_gcw;
}


/*--------------------------------------------------------------------------*\
 * plD_init_gcw()
 *
 * Initializes the device.
 *
 * This routine is invoked by a call to plinit, along with others:
 *
 *   <plD_init_gcw>
 *   <plD_bop_gcw>
 *   <plD_esc_gcw>: PLESC_GRAPH
 *   <plD_state_gcw>: PLSTATE_COLOR0
 *
\*--------------------------------------------------------------------------*/

void plD_init_gcw(PLStream *pls)
{
  GcwPLdev* dev;

#ifdef DEBUG_GCW
  debug("<plD_init_gcw>\n");
#endif

  /* Parse the driver options */
  plParseDrvOpts(gcw_options);

  /* Set up the stream */
  pls->termin = 1;      /* Is an interactive terminal */
  pls->dev_flush = 1;   /* Handle our own flushes */
  pls->plbuf_write = 1; /* Use plot buffer to replot to another device */
  pls->width = 1;
  pls->dev_clear = 1;   /* Handle plclear() */
  pls->dev_fill0 = 1;	/* Handle solid fills */
  pls->dev_dash = 0;	/* Handle dashed lines */

  /* Create the device */
  if((dev = g_malloc(sizeof(GcwPLdev))) == NULL) {
    plabort("GCW driver: Insufficient memory");
  }
  pls->dev = dev;

  /* Set the number of pixels per mm */
  plP_setpxl((PLFLT)PIXELS_PER_MM,(PLFLT)PIXELS_PER_MM);

  /* Set up physical limits of plotting device (in drawing units) */
  plP_setphy((PLINT)0,(PLINT)CANVAS_WIDTH*DU_PER_IN,
	     (PLINT)0,(PLINT)CANVAS_HEIGHT*DU_PER_IN);

  /* Set the width and height for the device */
  dev->width = PIXELS_PER_DU * DU_PER_IN * CANVAS_WIDTH;
  dev->height = PIXELS_PER_DU * DU_PER_IN * CANVAS_HEIGHT;

  /* Set text handling */
#ifdef HAVE_FREETYPE
  if(aa && text) {
    pls->dev_text = TRUE;
    pls->dev_unicode = TRUE;
    dev->use_text = TRUE;
    if(hrshsym) pls->dev_hrshsym = 1;

    /* Initialize freetype */
    plD_FreeType_init(pls);
  }
  else {
    pls->dev_text = FALSE;
    pls->dev_unicode = FALSE;
    dev->use_text = FALSE;
  }
#else
  pls->dev_text = FALSE;
  pls->dev_unicode = FALSE;
  dev->use_text = FALSE;
#endif

  /* Set fast rendering for polylines */
  dev->use_fast_rendering = (gboolean)fast;

  /* Set up the pixmap support */
  dev->use_pixmap = (gboolean)pixmap;
  dev->pixmap = NULL;
  dev->pixmap_has_data = FALSE;

  /* Only initialize the zoom after all other initialization is complete */
  dev->zoom_is_initialized = FALSE;

  /* Set the device canvas pointer and window */
  dev->canvas = NULL;
  dev->window = NULL;

  /* Initialize the visible and hidden groups.  All of the plplot plotting
   * commands are drawn to the hidden group.  When the page is finalized,
   * the group is made visible, and the old group destroyed. */
  dev->group_visible=NULL;
  dev->group_hidden=NULL;
  dev->group_background=NULL;
  dev->group_foreground=NULL;
  dev->group_current=NULL;
}


/*--------------------------------------------------------------------------*\
 * plD_polyline_gcw()
 *
 * Draw a polyline in the current color.
\*--------------------------------------------------------------------------*/

void plD_polyline_gcw(PLStream *pls, short *x, short *y, PLINT npts)
{
  GcwPLdev* dev = pls->dev;
  GnomeCanvasPoints* points;
  GnomeCanvasPoints pts;
  GnomeCanvasGroup* group;
  GnomeCanvasItem* item;
  GnomeCanvas* canvas;

  PLINT i;

  gdouble width;
  guint32 color;

#ifdef DEBUG_GCW
  debug("<plD_polyline_gcw>\n");
#endif

  if(dev->canvas==NULL) install_canvas(pls);
  canvas = dev->canvas;

  if(dev->group_hidden==NULL) plD_bop_gcw(pls);

#ifdef ASSERT_GCW
  if(!GNOME_IS_CANVAS_ITEM(
    group = dev->group_current
    )) {
    fprintf(stderr,"\n\n*** GCW driver error: group is NULL.\n");
    return;
  }
#else
  group = dev->group_current;
#endif

  /* Put the data in a points structure */
  points = gnome_canvas_points_new(npts);
  for ( i = 0; i < npts; i++ ) {
    points->coords[2*i] = ((gdouble) x[i]) * PIXELS_PER_DU;
    points->coords[2*i + 1] = ((gdouble) -y[i]) * PIXELS_PER_DU;
  }

  /* Get the width and color */
  width = pls->width*PIXELS_PER_DU*WSCALE;
  color = dev->color;

  if(dev->use_fast_rendering) {
    if(!GNOME_IS_CANVAS_ITEM(
      item=gnome_canvas_item_new(group,
				 GNOME_TYPE_CANVAS_LINE,
				 "cap_style", GDK_CAP_ROUND,
				 "join-style", GDK_JOIN_ROUND,
				 "points", points,
				 "fill-color-rgba", color,
				 "width-units",width,
				 NULL)
      )) {
      fprintf(stderr,"\n\n*** GCW driver error: item not created.\n");
      return;
    }

    /* Free the points structure */
    gnome_canvas_points_free(points);
  }
  else {

    /* Workaround for the 'attempt to put segment in horiz list twice'
     * from libgnomecanvas:
     *
     *   Plot a series of line segments rather than a single polyline.
     *
     * This slows rendering down a considerable amount.  However, it is 
     * unclear what else can be done.  Libgnomecanvas should be able to 
     * deal with all valid data; bizarre plotting errors happen along with
     * this error.
     *
     * Note that instead of allocating a series of points structures, 
     * we just refer to the original one from a separate struct 
     * (GnomeCanvas does not hold a reference to the points structure).
     */

    pts.num_points = 2;
    pts.ref_count = 1;
    pts.coords = points->coords;

    for(i=0;i<npts-1;i++) {
      pts.coords=&(points->coords[2*i]);

      if(!GNOME_IS_CANVAS_ITEM(
        item=gnome_canvas_item_new(group,
				 GNOME_TYPE_CANVAS_LINE,
				 "cap_style", GDK_CAP_ROUND,
				 "join-style", GDK_JOIN_ROUND,
				 "points", &pts,
				 "fill-color-rgba", color,
				 "width-units", width,
				 NULL)
	)) {
	fprintf(stderr,"\n\n*** GCW driver error: item not created.\n");
	return;
      }
    }

    /* Free the points structure */
    gnome_canvas_points_free(points);
  }
}


/*--------------------------------------------------------------------------*\
 * plD_line_gcw()
 *
 * Draw a line in the current color from (x1,y1) to (x2,y2).
\*--------------------------------------------------------------------------*/

void plD_line_gcw(PLStream *pls, short x1, short y1, short x2, short y2)
{
  short x[2];
  short y[2];

#ifdef DEBUG_GCW
  debug("<plD_line_gcw>\n");
#endif

  x[0] = x1;
  x[1] = x2;
  y[0] = y1;
  y[1] = y2;

  plD_polyline_gcw(pls, x, y, (PLINT) 2);
}


/*--------------------------------------------------------------------------*\
 * plD_eop_gcw()
 *
 * End of page.
\*--------------------------------------------------------------------------*/

void plD_eop_gcw(PLStream *pls)
{
  GcwPLdev* dev = pls->dev;
  GnomeCanvas* canvas;

  GdkPixbuf* pixbuf;
  GnomeCanvasItem* item;

  gdouble dx, dy;

#ifdef DEBUG_GCW
  debug("<plD_eop_gcw>\n");
#endif

  if(dev->canvas!=NULL) {

    canvas = dev->canvas;

    /* Render the pixmap to a pixbuf on the canvas and move it to the back
     * of the current group */
    if(dev->pixmap_has_data) {

      if((pixbuf=gdk_pixbuf_get_from_drawable(NULL,
					      dev->pixmap,
					      dev->colormap,
					      0,0,
					      0,0,
					      dev->width,dev->height))==NULL) {
	fprintf(stderr,"\n\nGCW driver error: Can't draw pixmap into pixbuf\n\n");
	fflush(stderr);
	return;
      }

      /* Different offsets depending on the type of canvas */
      if(dev->aa) { dx=0.; dy=0.; }
      else { dx=1.; dy=1.; }

      item = gnome_canvas_item_new (dev->group_current,
				    GNOME_TYPE_CANVAS_PIXBUF,
				    "pixbuf",pixbuf,
				    "x", dx,
				    "y", -dev->height+dy,
				    "width", dev->width,
				    "height", dev->height,
				    NULL);
      gnome_canvas_item_lower_to_bottom(item);
      gnome_canvas_item_lower_to_bottom (dev->background);

      /* Set the pixmap as unused */
      dev->pixmap_has_data = FALSE;

      /* Free the pixbuf */
      gdk_pixbuf_unref(pixbuf);
    }

    /* Make the hidden group visible */
    gnome_canvas_item_show((GnomeCanvasItem*)(dev->group_hidden));

    /* Destroy the current visible group */
    if(dev->group_visible!=NULL) {
      gtk_object_destroy((GtkObject*)(dev->group_visible));
      dev->group_visible = NULL;
    }

    /* Name the hidden group as visible */
    dev->group_visible = dev->group_hidden;
    dev->group_hidden=NULL;

    /* Update the canvas */
    canvas->need_update = 1;
    gnome_canvas_update_now (canvas);
    
    /* If the driver is creating its own canvasses, set dev->canvas to be
     * NULL now in order to force creation of a new canvas when the next
     * drawing call is made.
     */
    if(dev->window!=NULL) {
      dev->canvas=NULL;
      dev->group_visible=NULL;
      dev->group_hidden=NULL;
      dev->group_background=NULL;
      dev->group_foreground=NULL;
      dev->group_current=NULL;
    }
  }
}


/*--------------------------------------------------------------------------*\
 * plD_bop_gcw()
 *
 * Set up for the next page.
 *
 * This routine is invoked by a call to plinit, along with others:
 *
 *   <plD_init_gcw>
 *   <plD_bop_gcw>
 *   <plD_esc_gcw>: PLESC_GRAPH
 *   <plD_state_gcw>: PLSTATE_COLOR0
 *
\*--------------------------------------------------------------------------*/

void plD_bop_gcw(PLStream *pls)
{
  GcwPLdev* dev = pls->dev;
  GnomeCanvas* canvas;

#ifdef DEBUG_GCW
  debug("<plD_bop_gcw>\n");
#endif

  if(dev->canvas!=NULL) { 
    canvas = dev->canvas;

    /* Make sure the zoom is initialized */
    if(!dev->zoom_is_initialized) gcw_set_canvas_zoom(canvas,1.);

    /* Creat a new hidden group; all new drawing will be to this group */
    if(!GNOME_IS_CANVAS_ITEM(
      dev->group_hidden = (GnomeCanvasGroup*)gnome_canvas_item_new(
					  gnome_canvas_root(canvas),
					  gnome_canvas_clipgroup_get_type(),
					  "x",0.,
					  "y",0.,
					  NULL)
      )) {
      fprintf(stderr,"\n\n*** GCW driver error: item not created.\n");
      return;
    }

    /* Set the clip to NULL */
    g_object_set(G_OBJECT(dev->group_hidden),"path",NULL,NULL);

    /* Hide this group until drawing is done */
    gnome_canvas_item_hide((GnomeCanvasItem*)(dev->group_hidden));

    /* Set the hidden group as current unless it is fore or background */
    if( (dev->group_current != dev->group_foreground) &&
	(dev->group_current != dev->group_background) )
      dev->group_current = dev->group_hidden;

    /* Make sure the foreground group is at the front, and the background
     * group is at the back 
     */
    gnome_canvas_item_raise_to_top(
			    GNOME_CANVAS_ITEM(dev->group_foreground));
    gnome_canvas_item_lower_to_bottom(
			    GNOME_CANVAS_ITEM(dev->group_background));
  }
}


/*--------------------------------------------------------------------------*\
 * plD_tidy_gcw()
 *
 * Close graphics file
\*--------------------------------------------------------------------------*/

void plD_tidy_gcw(PLStream *pls)
{
  GcwPLdev* dev = pls->dev;

#ifdef DEBUG_GCW
  debug("<plD_tidy_gcw>\n");
#endif

#ifdef HAVE_FREETYPE
  if (pls->dev_text) {
    FT_Data *FT=(FT_Data *)pls->FT;
    plscmap0n(FT->ncol0_org);
    plD_FreeType_Destroy(pls);
  }
#endif

  if(dev->window!=NULL) {
    gtk_main ();
  }
}


/*--------------------------------------------------------------------------*\
 * plD_state_gcw()
 *
 * Handle change in PLStream state (color, pen width, fill attribute, etc).
 *
 * This routine is invoked by a call to plinit, along with others:
 *
 *   <plD_init_gcw>
 *   <plD_bop_gcw>
 *   <plD_esc_gcw>: PLESC_GRAPH
 *   <plD_state_gcw>: PLSTATE_COLOR0
 *
\*--------------------------------------------------------------------------*/

void plD_state_gcw(PLStream *pls, PLINT op)
{
  GcwPLdev* dev = pls->dev;
  char msg[100];

#ifdef DEBUG_GCW
  sprintf(msg,"<plD_state_gcw>: %d\n",op);
  debug(msg);
#endif

  switch (op) {
    case (1): /* PLSTATE_WIDTH */
      if(dev->canvas==NULL) install_canvas(pls);
      break;
    case (2): /* PLSTATE_COLOR0 */
      dev->color = plcolor_to_rgba(pls->cmap0[pls->icol0],0xFF);
      break;
    case (3): /* PLSTATE_COLOR1 */
      if(dev->canvas==NULL) install_canvas(pls);
      dev->color = plcolor_to_rgba(pls->cmap1[pls->icol1],0xFF);
      break;
    case (4): /* PLSTATE_FILL */
      if(dev->canvas==NULL) install_canvas(pls);
      break;
    case (5): /* PLSTATE_CMAP0 */
/*       if(dev->canvas==NULL) install_canvas(pls); */
      break;
    case (6): /* PLSTATE_CMAP1 */
/*      if(dev->canvas==NULL) install_canvas(pls); */
      break;
    default: 
      if(dev->canvas==NULL) install_canvas(pls);
      break;
  }
}


/*--------------------------------------------------------------------------*\
 * fill_polygon()
 *
 * Fills the polygon defined by the given points.  Used for shade
 * plotting.  Only solid fills are allowed.
\*--------------------------------------------------------------------------*/

static void fill_polygon (PLStream* pls)
{
  GnomeCanvasPoints* points;
  GnomeCanvasGroup* group;
  GnomeCanvasItem* item;
  GdkGC* gc;
  GcwPLdev* dev = pls->dev;
  GnomeCanvas* canvas;
  
  PLINT i;

  GdkColor color;
  GdkPoint* gdkpoints;

  PLINT tmp;

#ifdef DEBUG_GCW
  debug("<fill_polygon>\n");
#endif

  if(dev->canvas==NULL) install_canvas(pls);
  canvas = dev->canvas;

  if(dev->group_hidden==NULL) plD_bop_gcw(pls);

#ifdef ASSERT_GCW
  if(!GNOME_IS_CANVAS_ITEM(
    group = dev->group_current
    )) {
    fprintf(stderr,"\n\n*** GCW driver error: group is NULL.\n");
    return;
  }
#else
  group = dev->group_current;
#endif
  points = gnome_canvas_points_new (pls->dev_npts);

  if(dev->use_pixmap) { /* Write to a pixmap */

    /* Allocate a new pixmap if needed */
    if(dev->pixmap == NULL) gcw_use_pixmap(canvas,TRUE);

    /* Clear the pixmap if required */
    if(! dev->pixmap_has_data) clear_pixmap(pls);

    gc = gdk_gc_new(dev->pixmap);

    color.red=(guint16)(pls->curcolor.r/255.*65535); 
    color.green=(guint16)(pls->curcolor.g/255.*65535); 
    color.blue=(guint16)(pls->curcolor.b/255.*65535);

    gdk_colormap_alloc_color(gtk_widget_get_colormap(GTK_WIDGET(dev->canvas)),
			     &color,FALSE,TRUE);
    gdk_gc_set_foreground(gc,&color);

    if((gdkpoints = (GdkPoint*)malloc(pls->dev_npts*sizeof(GdkPoint)))==NULL) {
      fprintf(stderr,"\n\nGCW driver error: Insufficient memory\n\n");
      fflush(stderr);
      return;
    }

    for(i=0;i<pls->dev_npts;i++) {
      gdkpoints[i].x = pls->dev_x[i] * PIXELS_PER_DU;
      gdkpoints[i].y = dev->height-pls->dev_y[i] * PIXELS_PER_DU;
    }

    gdk_draw_polygon(dev->pixmap,gc,TRUE,gdkpoints,pls->dev_npts);

    dev->pixmap_has_data = TRUE;

    gdk_gc_unref(gc);
    free(gdkpoints);
  }
  else { /* Use Gnome Canvas polygons */

    for (i=0; i<pls->dev_npts; i++) {
      points->coords[2*i] = ((gdouble) pls->dev_x[i]) * PIXELS_PER_DU;
      points->coords[2*i + 1] = ((gdouble) -pls->dev_y[i]) * PIXELS_PER_DU;
    }

    if(!GNOME_IS_CANVAS_ITEM(
      item = gnome_canvas_item_new (group,
				    GNOME_TYPE_CANVAS_POLYGON,
				    "points", points,
				    "fill-color-rgba",dev->color,
				    /* "outline-color-rgba",dev->color, */
				    NULL)
      )) {
      fprintf(stderr,"\n\n*** GCW driver error: item not created.\n");
      return;
    }
  
    gnome_canvas_points_free(points);


    /* Draw a thin outline for each polygon */
#ifdef OUTLINE_POLYGON_GCW
    tmp = pls->width;
    pls->width=1;
    plD_polyline_gcw(pls,pls->dev_x,pls->dev_y,pls->dev_npts);
    pls->width = tmp;
#endif
  }
}


/*--------------------------------------------------------------------------*\
 * dashed_line()
 *
 * Handles call for dashed line.  Currently there is no facility for
 * setting dash lengths on the Gdk-type GnomeCanvas; there is no dash
 * facility whatsoever for antialiased GnomeCanvas objects.
\*--------------------------------------------------------------------------*/

static void dashed_line(PLStream* pls)
{
  GcwPLdev* dev = pls->dev;
  GnomeCanvasPoints* points;
  GnomeCanvasGroup* group;
  GnomeCanvasItem* item;
  GnomeCanvas* canvas;

/*   gchar* dash_list; */
/*   gint dash_list_len; */

  guint i;

#ifdef DEBUG_GCW
  debug("<dashed_line>\n");
#endif

  if(dev->canvas==NULL) install_canvas(pls);
  canvas = dev->canvas;

  if(dev->group_hidden==NULL) plD_bop_gcw(pls);

#ifdef ASSERT_GCW
  if(!GNOME_IS_CANVAS_ITEM(
    group = dev->group_current
    )) {
    fprintf(stderr,"\n\n*** GCW driver error: group is NULL.\n");
    return;
  }
#else
  group = dev->group_current;
#endif

/*   /\* Save the dash list in a handy construct *\/ */
/*   dash_list_len = 2 * pls->nms; */
/*   dash_list = g_malloc (dash_list_len * sizeof(gchar)); */
/*   for (i = 0; i < pls->nms; i++) { */
/*     dash_list[2*i] = (gchar) ceil ((pls->mark[i]/1e3) * PIXELS_PER_MM); */
/*     dash_list[2*i+1] = (gchar) floor ((pls->space[i]/1e3) * PIXELS_PER_MM); */
/*   } */


  /* Save the points list */
  points = gnome_canvas_points_new (pls->dev_npts);
  for (i = 0; i < pls->dev_npts; i++) {
    points->coords[2*i] = ((double) pls->dev_x[i]) * PIXELS_PER_DU;
    points->coords[2*i+1] = ((double) -pls->dev_y[i]) * PIXELS_PER_DU;
  }

  if(!GNOME_IS_CANVAS_ITEM(
    item = gnome_canvas_item_new (group,
				GNOME_TYPE_CANVAS_LINE,
				"cap_style", GDK_CAP_BUTT,
				"join_style", GDK_JOIN_ROUND,
                                "points", points,
 				"line-style", GDK_LINE_ON_OFF_DASH,
				"fill-color-rgba",dev->color,
                                "width-units",pls->width*PIXELS_PER_DU*WSCALE,
                                NULL)
    )) {
    fprintf(stderr,"\n\n*** GCW driver error: item not created.\n");
    return;
  }

  gnome_canvas_points_free (points);
}


/*--------------------------------------------------------------------------*\
 * clear()
 *
 * Handles call to clear the canvas.
\*--------------------------------------------------------------------------*/

static void clear (PLStream* pls)
{
  GcwPLdev* dev = pls->dev;

#ifdef DEBUG_GCW
  debug("<clear>\n");
#endif

  if(dev->canvas==NULL) install_canvas(pls);

  if(dev->group_visible!=NULL) {
    gtk_object_destroy((GtkObject*)(dev->group_visible));
    dev->group_visible = NULL;
  }

  if(dev->group_hidden!=NULL){
    gtk_object_destroy((GtkObject*)(dev->group_hidden));
    dev->group_hidden = NULL;
  }
}


/*--------------------------------------------------------------------------*\
 * proc_str()
 *
 * Handles call to draw text on the canvas when the HAS_TEXT escape funtion
 * case is invoked.
 *
 * This routine is unicode enabled, and requires freetype.
\*--------------------------------------------------------------------------*/

void proc_str(PLStream *pls, EscText *args)
{
  PLFLT *t = args->xform; /* Transform matrix for string */
  FT_Data *FT=(FT_Data *)pls->FT; /* Freetype information */

  GnomeCanvasGroup* group;
  GcwPLdev* dev = pls->dev;
  GnomeCanvas* canvas;

  PLUNICODE fci; /* The unicode font characterization integer */
  guchar *fontname = NULL;
  gint font_size;
  GnomeFont *font;
  GnomeFontFace *face;
  GnomeGlyphList *glyphlist;
  guint Nglyphs;

  gdouble affine_baseline[6] = {0.,0.,0.,0.,0.,0.}; /* Affine transforms */
  gdouble affine_translate[6] = {0.,0.,0.,0.,0.,0.};
  gdouble affine_plplot[6] = {0.,0.,0.,0.,0.,0.};

  PLINT clxmin, clxmax, clymin, clymax; /* Clip limits */
  ArtBpath* clip;
  GnomeCanvasPathDef* path;

  GnomeCanvasItem* item[200]; /* List of string segments */
  gdouble width[200],height[200]; /* Height and width of string segment */
  gdouble up_list[200]; /* Indicates sub/sup position of string segment */
  gdouble up=0,scale=1; /* Used to create superscripts and subscripts */

  ArtDRect bbox; /* Bounding box for each segment to get width & height */

  const PLUNICODE *text; /* The text and pointers to it */
  guint i=0,Ntext; /* The text index and maximum length */

  char esc; /* The escape character */

  guint N=0; /* The number of text segments */
  gdouble total_width=0,sum_width=0;

  guint symbol;


#ifdef DEBUG_GCW
  debug("<proc_str>\n");
#endif

  if(dev->canvas==NULL) install_canvas(pls);
  canvas = dev->canvas;

  if(dev->group_hidden==NULL) plD_bop_gcw(pls);

#ifdef ASSERT_GCW
  if(!GNOME_IS_CANVAS_ITEM(
    group = dev->group_current
    )) {
    fprintf(stderr,"\n\n*** GCW driver error: group is NULL.\n");
    return;
  }
#else
  group = dev->group_current;
#endif

  /* Retrieve the escape character */
  plgesc(&esc);

  /* Put the transform matrix values in the order expected by libart.
   * Note that the plplot transform matrix only has a rotation and shear;
   * plplot's rotation direction and shear are opposite from that expected 
   * by libart, hence the negative signs below.
   */
  affine_plplot[0] = t[0];  /* cos(theta) */
  affine_plplot[1] = -t[2]; /* sin(theta) */
  affine_plplot[2] = -t[1]; /* a cos(theta) - sin(theta) */
  affine_plplot[3] = t[3];  /* a sin(theta) + cos(theta) */

  /* Apply plplot difilt transformations; i.e., determine the string
   * position and clip limits.
   */
  difilt(&args->x, &args->y, 1, &clxmin, &clxmax, &clymin, &clymax);

  /* Set the clip in this clipgroup */
  path = gnome_canvas_path_def_new();
  gnome_canvas_path_def_ensure_space(path,6);
  gnome_canvas_path_def_moveto(path,clxmin*PIXELS_PER_DU,
			       -clymin*PIXELS_PER_DU);
  gnome_canvas_path_def_lineto(path,clxmin*PIXELS_PER_DU,
			       -clymax*PIXELS_PER_DU);
  gnome_canvas_path_def_lineto(path,clxmax*PIXELS_PER_DU,
			       -clymax*PIXELS_PER_DU);
  gnome_canvas_path_def_lineto(path,clxmax*PIXELS_PER_DU,
			       -clymin*PIXELS_PER_DU);
  gnome_canvas_path_def_closepath(path);
  g_object_set(G_OBJECT(group),"path",path,NULL);

  /* Font size: size is in pixels but chrht is in mm.
   * The factor at the end matches the font size to plplot's native
   * font size.
   */
  font_size = (gint)(pls->chrht/MM_PER_IN*DU_PER_IN*PIXELS_PER_DU/2.75);

  /* Determine the default font */
  plgfci(&fci);
  fontname = plP_FCI2FontName(fci, FontLookup, N_TrueTypeLookup);
  if (fontname == NULL) {
    fprintf(stderr, "fci = 0x%x, font name pointer = NULL \n", fci);
    plabort("PLplot GCW driver (proc_str): FCI inconsistent with"
	    "TrueTypeLookup; internal PLplot error");
    return;
  }

  /* Retrieve the font face */
  face = gnome_font_face_find_from_filename(fontname,0);

  /* Get the unicode string */
  text = args->unicode_array;
  Ntext = (guint)(args->unicode_array_len);

  /* Process the string: Break it into segments of constant font and size,
   * making sure we process control characters as we come to them.  Save
   * the extra information that will allow us to place the text on the
   * canvas.
   */
  while(i<Ntext) {

    /* Process the next character */

    if(text[i] & PL_FCI_MARK) { /* Is it a font characterization index? */

      /* Determine the font name */
      fontname = plP_FCI2FontName(text[i], FontLookup, N_TrueTypeLookup);
      if (fontname == NULL) {
	fprintf(stderr, "fci = 0x%x, font name pointer = NULL \n", fci);
	plabort("PLplot GCW driver (proc_str): FCI inconsistent with"
		"TrueTypeLookup; internal PLplot error");
	return;
      }

      /* Retrieve the font face */
      gnome_font_unref(face); /* We already have a face */
      face = gnome_font_face_find_from_filename(fontname,0);

      i++; /* Move ahead to the next character */

    }
    else {

      if(text[i] == esc) { /* Check for escape sequences */

	/* Process escape sequence */

	i++; /* Move on to next character */
	if(i>=Ntext) {
	  fprintf(stderr,"\n\nPLplot GCW driver error: "
		  "invalid escape sequence.\n\n");
	  return;
	}

	switch(text[i]) {

	case '#': /* <esc><esc>; this should translate to a hash */
	  break;  /* Watch out for it later */

	/* Move to lower sub/sup position */
	case 'd':
	case 'D':
	  if(up>0.) scale *= 1.25;  /* Subscript scaling parameter */
	  else scale *= 0.8;  /* Subscript scaling parameter */
	  up -= font_size / 2.;
	  break;

	/* Move to higher sub/sup position */
	case 'u':
	case 'U':
	  if(up<0.) scale *= 1.25;  /* Subscript scaling parameter */
	  else scale *= 0.8;  /* Subscript scaling parameter */
	  up += font_size / 2.;
	  break;

	/* Ignore the next sequences */

        /* Overline */
	case '+':

	/* Underline */
	case '-':

	/* Backspace */
	case 'b':
	case 'B':
	  plwarn("'+', '-', and 'b' text escape sequences not processed.");
	  break;

	} /* switch(text[i]) */

	if(text[i]!='#') i++; /* Move ahead to the next character */

      } /* if(text[i] == esc) */
    } /* if(text[i] & PL_FCI_MARK) */


    if(i==Ntext) continue; /* End of string */

    /* Save the sub/sup position */
    up_list[N] = up;

    /* Get the font */
    font = gnome_font_face_get_font_default(face,font_size*scale);
    /* printf("\n\nfont name = %s\n\n",gnome_font_get_name(font)); */

    /* Create the glyphlist for this text segment */
    glyphlist = gnome_glyphlist_new ();
    gnome_glyphlist_font(glyphlist, font);
    gnome_glyphlist_color(glyphlist,dev->color);
    gnome_glyphlist_advance(glyphlist, TRUE);
    gnome_glyphlist_kerning(glyphlist, 0.);
    gnome_glyphlist_letterspace(glyphlist, 0.);

    /* Free the font */
    gnome_font_unref(font);

    /* Move along to the next escape or FCI character, stuffing 
     * everything else into the glyphlist.
     */
    Nglyphs=0;
    while(i<Ntext && !(text[i] & PL_FCI_MARK)) {

      /* Differentiate between ## and escape sequences */
      if(text[i]==esc) {
	if( !(i>0 && text[i-1]==esc) ) break;
      }

      gnome_glyphlist_glyph(glyphlist, 
			    gnome_font_lookup_default(font,text[i]));
      i++; Nglyphs++;
    }

    if(Nglyphs) {

      /* Determine the bounding box of the text */
      gnome_glyphlist_bbox(glyphlist,NULL,0,&bbox);
      width[N] = bbox.x1-bbox.x0;
      height[N] = bbox.y1-bbox.y0;

      /* Keep track of the total string width so that we can justify it */
      total_width += width[N];
      if(N!=0) total_width += 2; /* Add a little extra space */

      /* Create the canvas text item */
      if(!GNOME_IS_CANVAS_ITEM(
        item[N] = gnome_canvas_item_new (group,
				       PLPLOT_TYPE_CANVAS_HACKTEXT,
				       "glyphlist",glyphlist,
				       "fill-color-rgba",dev->color,
				       "x",0.,
				       "y",0.,
				       NULL)
	)) {
	fprintf(stderr,"\n\n*** GCW driver error: item not created.\n");
	return;
      }

      /* Free the glyphlist */
      gnome_glyphlist_unref(glyphlist);
      
      /* Advance to next string segment */
      N++;
    } /* if(Nglyphs) */


    /* Don't overflow buffer */
    if(N==200 && i<Ntext) {
      fprintf(stderr,"\n\nPLplot GCW driver internal error: " \
	             "too many text segments.\n\n");
      break;
    }
  } /* while(i<Ntext) */

  /* We have all of the string segments.  Place each on the canvas 
   * appropriately.
   */
  for(i=0;i<N;i++) {

    /* Calculate and apply the affine transforms */
    art_affine_translate(affine_baseline,
			 -total_width*args->just + sum_width,
			 height[0]/2.5-up_list[i]);
    art_affine_translate(affine_translate,
			 args->x*PIXELS_PER_DU,-args->y*PIXELS_PER_DU);
    gnome_canvas_item_affine_relative(item[i],affine_translate);
    gnome_canvas_item_affine_relative(item[i],affine_plplot);
    gnome_canvas_item_affine_relative(item[i],affine_baseline);

    /* Keep track of the position in the string */
    sum_width += width[i];
    if(i!=N-1) sum_width += 2; /* Add a little extra space */
  }
}


/*--------------------------------------------------------------------------*\
 * plD_esc_gcw()
 *
 * Escape functions.
 *
 * This routine is invoked by a call to plinit, along with others:
 *
 *   <plD_init_gcw>
 *   <plD_bop_gcw>
 *   <plD_esc_gcw>: PLESC_GRAPH
 *   <plD_state_gcw>: PLSTATE_COLOR0

\*--------------------------------------------------------------------------*/

void plD_esc_gcw(PLStream *pls, PLINT op, void *ptr)
{

#ifdef DEBUG_GCW
  char msg[100];

  sprintf(msg,"<plD_esc_gcw>: %d\n",op);
  debug(msg);
#endif

  switch(op) {

  case PLESC_DEVINIT:
    gcw_set_canvas(pls,GNOME_CANVAS(ptr));
    break;

  case PLESC_CLEAR:
    /*    clear(pls); */
    break;

  case PLESC_DASH:
    dashed_line(pls);
    break;

  case PLESC_FILL:
#ifdef POLYGON_GCW
    fill_polygon(pls);
#endif
    break;

  case PLESC_HAS_TEXT:
    if(ptr!=NULL) {
      proc_str(pls, ptr); /* Draw the text */
    }
    else { 

      /* Assume this was a request to change the text handling,
       * which is a special hack for this driver.  This is done 
       * through the escape function so that we can get easy access
       * to pls.
       */
      if(((GcwPLdev*)(pls->dev))->use_text) 
	pls->dev_text = 1; /* Allow text handling */
      else pls->dev_text = 0; /* Disallow text handling */
    }
    break;

  case PLESC_GRAPH:
    break;

  default:
    break;
  }
}
