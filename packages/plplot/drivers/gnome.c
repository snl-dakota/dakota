/* $Id: gnome.c 3186 2006-02-15 18:17:33Z slbrow $

        PLplot GnomeCanvas device driver.

   Copyright (C) 2004  Rafael Laboissiere

   This file is part of PLplot.

   PLplot is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Library Public License as published
   by the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   PLplot is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with PLplot; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

*/

#include "plDevs.h"

#ifdef PLD_gnome

#include "plplotP.h"
#include "drivers.h"

#include <gtk/gtk.h>
#include <glib.h>
#include <pthread.h>
#include <math.h>

#include <gnome.h>

#include <unistd.h>

/* Device info */
char* plD_DEVICE_INFO_gnome = "gnome:Gnome Canvas:1:gnome:6:gnome";

/*#undef DEBUG*/
#define DEBUG

#undef ANTIALISED_CANVAS

/* Physical dimensions */

/* Unit of drawing (1/360 in).  Corresponds to the pen widht = 1 */
#define DRAWING_UNIT (0.0027777778)

/* How many pixels in a drawing unit (defualt value) */
#define PIXELS_PER_DU (0.2)

/* mm per inch */
#define MM_PER_IN (25.4)

/* pixels per mm */
#define PIXELS_PER_MM (PIXELS_PER_DU / DRAWING_UNIT / MM_PER_IN)

/* Default dimensions of the canvas (in inches) */
#define WIDTH (9)
#define HEIGHT (6.5)

/* Magnification factor for the PLplot physical dimensions */
#define MAG_FACTOR 10

#ifndef ABS
#define ABS(x) ((x < 0) ? (-(x)) : (x))
#endif

void
debug (char* msg)
{
#ifdef DEBUG
  printf (msg);
  fflush (stdout);
#endif
}

G_LOCK_DEFINE_STATIC (canvas);

static pthread_t tid;

static guint gnome_is_initialized = FALSE;

/*// FIXME : Terrible global variable hack*/
static GtkStatusbar* sb;

typedef enum {
  GNOME_PLDEV_LOCATE_MODE,
  GNOME_PLDEV_ZOOM_MODE,
  GNOME_PLDEV_NONE_MODE
} GnomePLdevCanvasMode;

typedef struct {
  guint cmap;
  gdouble color;
} ItemColor;

typedef struct {
  GtkScrolledWindow* sw;
  GnomeCanvas* canvas;
  GnomePLdevCanvasMode mode;
  GnomeCanvasItem* hlocline;
  double hpos;
  GnomeCanvasItem* vlocline;
  double vpos;
  GnomeCanvasItem* zoomrect;
  PLGraphicsIn gin;
  guint context;
  GtkAdjustment* hadj;
  GtkAdjustment* vadj;
  double width;
  double height;
  double ppu;
  short bufx[1024];
  short bufy[1024];
  guint bufc;
  PLINT curcolor;
  PLINT curwidth;
} GnomePLdevPage;

typedef struct {
  GtkWidget* parent;
  GtkWidget* root;
  GtkNotebook* notebook;
  GtkStatusbar* statusbar;
  guint npages;
  GnomePLdevPage** page;
  guint parent_is_from_driver;
  PLINT pattern;
  GdkBitmap* pattern_stipple[2];
} GnomePLdev;

void
gnome_pldev_adopt (PLStream* pls, GtkContainer* parent)
{
  GnomePLdev* dev = pls->dev;
  GtkWidget* root;

  if (dev->parent_is_from_driver) {
    root = dev->root;
    gtk_object_ref (GTK_OBJECT (root));
    gtk_container_remove (GTK_CONTAINER (dev->parent), root);
    gtk_widget_destroy (dev->parent);
    dev->parent_is_from_driver = FALSE;
  }
  gtk_container_add (parent, root);
  dev->parent = GTK_WIDGET (parent);
  gtk_object_unref (GTK_OBJECT (root));
}

static guint timeout_id;

static gint
canvas_timeout (gpointer dev)
{
  guint i;

  for (i = 0; i < ((GnomePLdev*) dev)->npages ; i++)
    ((GnomePLdev*) dev)->page[i]->canvas->need_update = TRUE;

  return FALSE;
}

static void
timeout_register (GnomeCanvas* canvas, GnomePLdev* dev)
{
  /*//  gtk_timeout_remove (timeout_id);*/
  /*//  canvas->need_update = FALSE;*/
  /*//  timeout_id = gtk_timeout_add (100, canvas_timeout, dev);*/
}

static
void change_mode (GnomePLdevPage* page, GnomePLdevCanvasMode mode)
{
  guint context = page->context;

  page->mode = mode;

  switch (mode) {
  case GNOME_PLDEV_LOCATE_MODE:
    /*// FIXME : Terrible global variable hack*/
    gtk_statusbar_pop (sb, context);
    /*// FIXME : Terrible global variable hack*/
    gtk_statusbar_push (sb, context, "Locate Mode");
    break;
  case GNOME_PLDEV_ZOOM_MODE:
    /*// FIXME : Terrible global variable hack*/
    gtk_statusbar_pop (sb, context);
    /*// FIXME : Terrible global variable hack*/
    gtk_statusbar_push (sb, context, "Zoom Mode");
    break;
  default:
    break;
  }
}

static
void *init(void *args)
{
  gdk_threads_enter();
  gtk_main();
  gdk_threads_leave ();
  pthread_exit(NULL);
}

static gboolean
quit_dialog (void)
{
  GtkWidget* dialog;
  gint answer;

  fflush (stderr);

  dialog = gnome_dialog_new ("PLplot Gnome driver",
			     GNOME_STOCK_BUTTON_OK,
			     GNOME_STOCK_BUTTON_CANCEL,
			     NULL);

  gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (dialog)->vbox),
		      gtk_label_new ("Quit PLplot?"),
		      TRUE,
		      TRUE,
		      0);

  gtk_widget_show_all (GTK_WIDGET (GNOME_DIALOG (dialog)->vbox));

  gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
  gnome_dialog_set_default (GNOME_DIALOG (dialog), 0);

  answer = gnome_dialog_run_and_close (GNOME_DIALOG (dialog));

  if (answer == 0) {
    return TRUE;
  }
  else
    return FALSE;
}

static gint
canvas_pressed_cb(GnomeCanvasItem *item, GdkEvent *event,
		  GnomePLdevPage* page)
{
  static guint dragging;
  guint move;
  GdkCursor *cursor;
  char buffer[128];
  PLGraphicsIn* gin = &(page->gin);
  ItemColor* color;
  GnomeCanvasItem* item_at_cursor;

  move = FALSE;

  switch (event->type) {

  case GDK_2BUTTON_PRESS:

  case GDK_3BUTTON_PRESS:

  case GDK_BUTTON_PRESS:

    if (event->button.button == 1) {

      gnome_canvas_item_raise_to_top (page->hlocline);
      gnome_canvas_item_raise_to_top (page->vlocline);

      move = TRUE;

      cursor = gdk_cursor_new (GDK_CROSSHAIR);
      gnome_canvas_item_grab(item,
			     GDK_POINTER_MOTION_MASK |
			     GDK_BUTTON_RELEASE_MASK,
			     cursor,
			     event->button.time);
      gdk_cursor_destroy(cursor);

      /*// FIXME : Terrible global variable hack*/
      gtk_statusbar_push (sb, page->context, "");

      dragging = TRUE;
    }
    break;

  case GDK_MOTION_NOTIFY:
    if (dragging && (event->motion.state & GDK_BUTTON1_MASK))
      move = TRUE;
    break;

  case GDK_BUTTON_RELEASE:
    if (dragging && (event->motion.state & GDK_BUTTON1_MASK)) {
      gnome_canvas_item_ungrab(item, event->button.time);
      dragging = FALSE;
      gnome_canvas_item_hide (page->hlocline);
      gnome_canvas_item_hide (page->vlocline);
      /*// FIXME : Terrible global variable hack*/
      gtk_statusbar_pop (sb, page->context);
      /*//      printf ("Button release\n");*/
      /*//      fflush (stdout);*/
    }
    break;

  default:
    /*//    printf ("Other event\n");*/
    /*//    fflush (stdout);*/
    break;

  }

  if (move) {

    gnome_canvas_item_hide (page->hlocline);
    gnome_canvas_item_hide (page->vlocline);

    item_at_cursor = gnome_canvas_get_item_at (page->canvas,
					       event->button.x,
					       event->button.y);

    if (item_at_cursor != NULL)
      color = (ItemColor *) gtk_object_get_data (GTK_OBJECT (item_at_cursor),
						 "color");
    else
      color = NULL;

    gnome_canvas_item_show (page->hlocline);
    gnome_canvas_item_show (page->vlocline);

    gnome_canvas_item_move (page->hlocline,
			    0.0, event->button.y - page->vpos);
    page->vpos = event->button.y;

    gnome_canvas_item_move (page->vlocline,
			    event->button.x - page->hpos, 0.0);
    page->hpos = event->button.x;

    gin->dX = page->hpos / page->width;
    gin->dY = page->vpos / (- page->height);

    plTranslateCursor (gin);

    if (color == NULL)
      sprintf (buffer, "");
    else
      if (color->cmap == 0)
	sprintf (buffer, "   x = %f   y = %f   color = %d (cmap0)",
		 gin->wX, gin->wY, (int) color->color);
      else
	sprintf (buffer, "   x = %f   y = %f   color = %f (cmap1)",
		 gin->wX, gin->wY, color->color);

    /*// FIXME : Terrible global variable hack*/
    gtk_statusbar_pop (sb, page->context);
    /*// FIXME : Terrible global variable hack*/
    gtk_statusbar_push (sb, page->context, buffer);

  }

  return TRUE;
}


static gint
delete_event_cb (GtkWidget* widget, GdkEventAny* e, gpointer data)
{
  if (quit_dialog () == TRUE) {
    gtk_widget_destroy (widget);
    gtk_main_quit ();
  }
  return TRUE;
}


static gint
item_event(GnomeCanvasItem *item, GdkEvent *event, gpointer data)
{
  switch (event->type) {
  case GDK_BUTTON_PRESS:
    break;
  }

  return TRUE;
}

static void
setup_item(GnomeCanvasItem *item)
{
  gtk_signal_connect (GTK_OBJECT (item), "event",
                      (GtkSignalFunc) canvas_pressed_cb,
                      NULL);
}

static void
add_to_adj (GtkAdjustment* adj, gfloat add)
{
  gfloat value;

  value = adj->value + add;
  gtk_adjustment_set_value (adj, value);
}

static gint
key_cb (GtkWidget* widget, GdkEventKey* event, PLStream* pls)
{
  GnomePLdev* dev = pls->dev;
  GnomePLdevPage* page;
  GtkNotebook* notebook = dev->notebook;
  GtkStatusbar* statusbar = dev->statusbar;
  guint curpage;

  if (notebook != NULL)
    curpage = gtk_notebook_get_current_page (notebook);
  else
    curpage = 0;

  page = dev->page[curpage];

  switch (event->keyval) {
  case GDK_minus:
  case GDK_plus:
  case GDK_equal:
    switch (event->keyval) {
    case GDK_minus:
      page->ppu /= 1.4142;
      break;
    case GDK_plus:
    case GDK_equal:
      page->ppu *= 1.4142;
      break;
    }
    gnome_canvas_set_pixels_per_unit (GNOME_CANVAS (widget), page->ppu);
    break;
  case GDK_Q:
  case GDK_Return:
    if (quit_dialog () == TRUE)
      gtk_main_quit ();
    break;
  case GDK_l:
    /*//gdk_threads_enter ();*/
    change_mode (page, GNOME_PLDEV_LOCATE_MODE);
    /*//gdk_threads_leave ();*/
    break;
  case GDK_z:
    /*//gdk_threads_enter ();*/
    change_mode (page, GNOME_PLDEV_ZOOM_MODE);
    /*//gdk_threads_leave ();*/
    break;
  case GDK_Page_Up:
    if (curpage != 0)
      gtk_notebook_set_page (notebook, curpage - 1);
    break;
  case GDK_Page_Down:
    gtk_notebook_set_page (notebook, curpage + 1);
    break;
  case GDK_Right:
  case GDK_Left:
  case GDK_Up:
  case GDK_Down:
    switch (event->keyval) {
    case GDK_Right:
      add_to_adj (page->hadj, page->ppu);
      break;
    case GDK_Left:
      add_to_adj (page->hadj, -page->ppu);
      break;
    case GDK_Up:
      add_to_adj (page->vadj, page->ppu);
      break;
    case GDK_Down:
      add_to_adj (page->vadj, -page->ppu);
      break;
    }
    page->hadj->step_increment = page->ppu;
    page->hadj->step_increment = page->ppu;
    gtk_adjustment_changed (page->hadj);
    gtk_adjustment_changed (page->vadj);
    break;
  default:
    break;
  }

  return TRUE;
}

static void
page_switch (GtkNotebook *notebook, GtkNotebookPage *page, gint page_num,
	     gpointer dev)
{
  guint curpage;
  GnomeCanvas* canvas;

  canvas = ((GnomePLdev*)(dev))->page[page_num]->canvas;
  if (canvas != NULL)
    gtk_widget_grab_focus (GTK_WIDGET (canvas));
}

static guint32
plcolor_to_rgba (PLColor color, guchar alpha)
{
  return
    ((int)(color.r) << 24)
    + ((int)(color.g) << 16)
    + ((int)(color.b) << 8)
    + alpha;
}

static guint32
plcolor_to_rgba_inv (PLColor color, guchar alpha)
{
  return
    ((int)(255 - color.r) << 24)
    + ((int)(255 - color.g) << 16)
    + ((int)(255 - color.b) << 8)
    + alpha;
}

static void
set_color (GnomeCanvasItem* item, guint cmap, gdouble color)
{
  ItemColor* colorp;

  colorp = g_malloc (sizeof (ItemColor));
  colorp->cmap = cmap;
  colorp->color = color;

  gtk_object_set_data (GTK_OBJECT (item), "color", colorp);
}

static void
new_page (PLStream* pls)
{
  GtkAdjustment* adj;
  GnomeCanvasGroup* group;
  GnomeCanvasItem* background;
  GnomeCanvas* canvas;
  GnomePLdev* dev;
  GnomePLdevPage* page;
  GnomeCanvasPoints* points;
  guint np;
  guint32 loclinecolor;
  char buffer[32];

  dev = pls->dev;
  page = g_malloc (sizeof (GnomePLdevPage));
  page->mode = GNOME_PLDEV_LOCATE_MODE;
  page->ppu = 1.0;

  page->bufc = 0;

  np = dev->npages;

  /*//  gdk_threads_enter ();*/

#ifdef ANTIALISED_CANVAS

  gtk_widget_push_visual(gdk_rgb_get_visual());
  gtk_widget_push_colormap(gdk_rgb_get_cmap());
  canvas = GNOME_CANVAS (gnome_canvas_new_aa ());

#else

  gtk_widget_push_visual(gdk_imlib_get_visual());
  gtk_widget_push_colormap(gdk_imlib_get_colormap());
  canvas = GNOME_CANVAS (gnome_canvas_new ());

#endif

  timeout_id = gtk_timeout_add (1000, canvas_timeout, dev);

  page->canvas = canvas;

  page->width = PIXELS_PER_DU * WIDTH / DRAWING_UNIT;
  page->height = PIXELS_PER_DU * HEIGHT / DRAWING_UNIT;

  gnome_canvas_set_scroll_region(GNOME_CANVAS(canvas),
				 0, -page->height, page->width, 0);

  gtk_widget_pop_visual();
  gtk_widget_pop_colormap();

  group = gnome_canvas_root (GNOME_CANVAS (canvas));

  background = gnome_canvas_item_new (group,
				      gnome_canvas_rect_get_type(),
				      "x1", 0.0,
				      "y1", -page->height,
				      "x2", page->width,
				      "y2", 0.0,
				      "fill_color", (pls->cmap0[0]).name,
				      "width_units", 0.0,
				      NULL);

  timeout_register (canvas, dev);

  set_color (background, 0, 0.0);

  points = gnome_canvas_points_new (2);

  points->coords[0] = 0.0;
  points->coords[1] = 0.0;
  points->coords[2] = page->width;
  points->coords[3] = 0.0;

  loclinecolor = plcolor_to_rgba_inv (pls->cmap0[0], 0xF0),
  page->hlocline =
    gnome_canvas_item_new (group,
			   gnome_canvas_line_get_type(),
			   "points", points,
			   "fill_color_rgba", loclinecolor,
			   "width_pixels", 1,
			   NULL);
  gnome_canvas_item_hide (page->hlocline);

  timeout_register (canvas, dev);

  page->hpos = 0.0;

  points->coords[0] = 0.0;
  points->coords[1] = -page->height;
  points->coords[2] = 0.0;
  points->coords[3] = 0.0;

  page->vlocline =
    gnome_canvas_item_new (group,
			   gnome_canvas_line_get_type(),
			   "points", points,
			   "fill_color_rgba", loclinecolor,
			   "width_pixels", 1,
			   NULL);
  gnome_canvas_item_hide (page->vlocline);

  timeout_register (canvas, dev);

  page->vpos = 0.0;

  gnome_canvas_points_unref (points);
  points = gnome_canvas_points_new (5);

  points->coords[0] = 0.0;
  points->coords[1] = 0.0;
  points->coords[2] = 0.0;
  points->coords[3] = 0.0;
  points->coords[4] = 0.0;
  points->coords[5] = 0.0;
  points->coords[6] = 0.0;
  points->coords[7] = 0.0;
  points->coords[8] = 0.0;
  points->coords[9] = 0.0;

  page->zoomrect =
    gnome_canvas_item_new (group,
			   gnome_canvas_line_get_type(),
			   "points", points,
			   "fill_color_rgba",
			   plcolor_to_rgba_inv (pls->cmap0[0], 0xF0),
			   "width_units", 1.0,
			   NULL);

  gnome_canvas_item_hide (page->zoomrect);

  timeout_register (canvas, dev);

  page->hpos = 0.0;

  gnome_canvas_points_unref (points);

  page->context = np+1;

  change_mode (page, GNOME_PLDEV_LOCATE_MODE);

  gtk_signal_connect (GTK_OBJECT (background), "event",
                      (GtkSignalFunc) canvas_pressed_cb,
                      page);

  gtk_signal_connect (GTK_OBJECT (canvas), "key_press_event",
		      GTK_SIGNAL_FUNC (key_cb), pls);

  page->hadj = GTK_ADJUSTMENT (GTK_LAYOUT (canvas)->hadjustment);
  page->vadj = GTK_ADJUSTMENT (GTK_LAYOUT (canvas)->vadjustment);

  page->sw = GTK_SCROLLED_WINDOW (gtk_scrolled_window_new (page->hadj,
							   page->vadj));

  gtk_scrolled_window_set_policy (page->sw,
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);

  gtk_container_add (GTK_CONTAINER (page->sw), GTK_WIDGET (canvas));

  if (np == 0)
    dev->page = g_malloc (sizeof (GnomePLdevPage*));
  else
    dev->page = g_realloc (dev->page,
			   (np+1) * sizeof (GnomePLdevPage*));


  dev->page[np] = page;

  gtk_notebook_set_show_tabs (dev->notebook, (np > 0));

  sprintf (buffer, "Page %d", np+1);
  gtk_notebook_append_page (dev->notebook, GTK_WIDGET (page->sw),
			    gtk_label_new (buffer));

  /*//  adj = gtk_scrolled_window_get_vadjustment (page->hadj);*/
  /*//  page->hadj->value = 0.0;*/
  /*//page->hadj->step_increment = 1.0;*/
  /*//  page->vadj = adj;*/
  /*//gtk_scrolled_window_set_vadjustment (page->sw, adj);*/

  /*//adj = gtk_scrolled_window_get_hadjustment (page->sw);*/
  /*//page->vadj->value = 0.0;*/
  /*//page->vadj->step_increment = 1.0;*/
  /*//  page->hadj = adj;*/
  /*//gtk_scrolled_window_set_hadjustment (page->sw, adj);*/

  gtk_widget_show_all (dev->parent);

  gtk_notebook_set_page (dev->notebook, -1);

  /*//  gdk_threads_leave ();*/

  dev->npages++;

}

void
gnome_pldev_create (PLStream* pls)
{
  GnomePLdev* dev;
  GtkWidget* vbox;

  dev = g_malloc (sizeof (GnomePLdev));
  pls->dev = dev;
  dev->npages = 0;
  dev->pattern = 0;
  dev->pattern_stipple[0] = NULL;
  dev->pattern_stipple[1] = NULL;
  vbox = gtk_vbox_new (FALSE, 0);
  dev->root = vbox;

  dev->statusbar = GTK_STATUSBAR (gtk_statusbar_new ());
  sb = dev->statusbar;

  gtk_box_pack_end (GTK_BOX (dev->root), GTK_WIDGET (dev->statusbar),
		    FALSE, FALSE, 0);

  dev->notebook = GTK_NOTEBOOK (gtk_notebook_new ());

  gtk_signal_connect (GTK_OBJECT (dev->notebook), "switch_page",
    		      GTK_SIGNAL_FUNC (page_switch), dev);

  gtk_notebook_set_scrollable (dev->notebook, TRUE);

  gtk_box_pack_start (GTK_BOX (dev->root), GTK_WIDGET (dev->notebook),
		      TRUE, TRUE, 0);

  gtk_widget_show_all (GTK_WIDGET (dev->notebook));

  dev->parent_is_from_driver = FALSE;
}

void plD_open_gnome(PLStream *pls);

void plD_init_gnome		(PLStream *);
void plD_line_gnome		(PLStream *, short, short, short, short);
void plD_polyline_gnome		(PLStream *, short *, short *, PLINT);
void plD_eop_gnome			(PLStream *);
void plD_bop_gnome			(PLStream *);
void plD_tidy_gnome		(PLStream *);
void plD_state_gnome		(PLStream *, PLINT);
void plD_esc_gnome			(PLStream *, PLINT, void *);

void plD_dispatch_init_gnome( PLDispatchTable *pdt )
{
#ifndef ENABLE_DYNDRIVERS
    pdt->pl_MenuStr  = "Gnome Canvas";
    pdt->pl_DevName  = "gnome";
#endif
    pdt->pl_type     = plDevType_Interactive;
    pdt->pl_seq      = 1;
    pdt->pl_init     = (plD_init_fp)     plD_init_gnome;
    pdt->pl_line     = (plD_line_fp)     plD_line_gnome;
    pdt->pl_polyline = (plD_polyline_fp) plD_polyline_gnome;
    pdt->pl_eop      = (plD_eop_fp)      plD_eop_gnome;
    pdt->pl_bop      = (plD_bop_fp)      plD_bop_gnome;
    pdt->pl_tidy     = (plD_tidy_fp)     plD_tidy_gnome;
    pdt->pl_state    = (plD_state_fp)    plD_state_gnome;
    pdt->pl_esc      = (plD_esc_fp)      plD_esc_gnome;
}

/*--------------------------------------------------------------------------*\
 * plD_init_gnome()
 *
 * Initialize device.
 * X-dependent stuff done in plD_open_gnome() and Init().
\*--------------------------------------------------------------------------*/

void
plD_init_gnome (PLStream *pls)
{
  char* argv[] = { "" };
  GnomePLdev* dev;
  GtkWidget* window;
  double phys2canvas = MAG_FACTOR * PIXELS_PER_MM;

  pls->termin = 1;		/* Is an interactive terminal */
  pls->dev_flush = 1;		/* Handle our own flushes */
  pls->dev_fill0 = 1;		/* Handle solid fills */
  pls->dev_fill1 = 1;		/* Handle pattern fills */
  pls->dev_dash = 1;		/* Handle dashed lines */
  pls->plbuf_write = 1;	        /* Use plot buffer to replot to another device */
  pls->width = 1;
  pls->dev_clear = 1;           /* Handle plclear() */


  /* The real meat of the initialization done here */

  /*//  atexit (do_quit);*/

  /* init threads */
  g_thread_init (NULL);

  if (pls->dev == NULL) {

    if (! gnome_is_initialized ) {

      gnome_init ("GnomePLplotDriver", "0.0.1", 1, argv);
      gnome_sound_shutdown ();

    }

    gdk_rgb_init ();

    gnome_pldev_create (pls);

    dev = pls->dev;

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    dev->parent = window;

    gtk_signal_connect (GTK_OBJECT (window),
			"delete_event",
			GTK_SIGNAL_FUNC (delete_event_cb),
			NULL);

    gtk_window_set_title (GTK_WINDOW (window), "Gnome PLplot Driver");

    gtk_window_set_policy (GTK_WINDOW (window), TRUE, TRUE, TRUE);

    gtk_window_set_default_size (GTK_WINDOW (window), 700, 565);

    gtk_container_add (GTK_CONTAINER (window), dev->root);

    gtk_widget_show_all (window);

  }

  plP_setpxl ((PLFLT) phys2canvas, (PLFLT) phys2canvas);
  plP_setphy ((PLINT) 0, (PLINT) MAG_FACTOR * WIDTH/DRAWING_UNIT,
	      (PLINT) 0, (PLINT) MAG_FACTOR * HEIGHT/DRAWING_UNIT);

  gnome_is_initialized = TRUE;

#ifdef HAVE_PTHREADS

  pthread_create (&tid, NULL, init, NULL);

#endif

}


/*--------------------------------------------------------------------------*\
 * plD_polyline_gnome()
 *
 * Draw a polyline in the current color from (x1,y1) to (x2,y2).
\*--------------------------------------------------------------------------*/
static
int count[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

void
plD_polyline_gnome(PLStream *pls, short *x, short *y, PLINT npts)
{
  GnomePLdev* dev;
  GnomePLdevPage* page;
  GnomeCanvasPoints* points;
  GnomeCanvasGroup* group;
  GnomeCanvasItem* item;
  GnomeCanvas* canvas;
  guint i;
  short* xp;
  short* yp;

  /*//  gdk_threads_enter ();*/

  dev = pls->dev;

  page = dev->page[dev->npages-1];

  /*//  debug ("Before if\n");*/

  if ( page->bufc > 0
       && ((npts == 0)
	   || (page->curcolor != pls->icol0)
	   || (page->curwidth != pls->width)
	   || (*x != page->bufx[page->bufc - 1])
	   || (*y != page->bufy[page->bufc - 1])) ) {

    /*//       printf("bufc = %d\tnpts = %d\n", page->bufc, npts);*/
    /*//q        fflush(stdout);*/

  canvas = page->canvas;

  group = gnome_canvas_root (canvas);

  points = gnome_canvas_points_new (page->bufc);

  for ( i = 0; i < page->bufc; i++ ) {
    points->coords[2*i] = ((double) page->bufx[i]/MAG_FACTOR) * PIXELS_PER_DU;
    points->coords[2*i + 1] = ((double) -page->bufy[i]/MAG_FACTOR) * PIXELS_PER_DU;;
  }

  item = gnome_canvas_item_new (group,
                                gnome_canvas_line_get_type (),
				"cap_style", GDK_CAP_ROUND,
				"join_style", GDK_JOIN_ROUND,
                                "points", points,
				"fill_color_rgba",
				plcolor_to_rgba (pls->cmap0[page->curcolor], 0xFF),
                                "width_units",
				MAX ((double) page->curwidth, 3.0) * PIXELS_PER_DU,
                                NULL);

  timeout_register (canvas, dev);

  set_color (item, 0, page->curcolor);

  gtk_signal_connect (GTK_OBJECT (item), "event",
                      (GtkSignalFunc) canvas_pressed_cb,
                      page);

  gnome_canvas_points_unref (points);

  count[pls->icol0]++;

    page->bufc = 0;

  }

  /*//  debug ("Got here\n");*/

  xp = &(page->bufx[page->bufc]);
  yp = &(page->bufy[page->bufc]);

  page->bufc += npts;

  while (npts--) {
    *(xp++) = *(x++);
    *(yp++) = *(y++);
  }

  page->curcolor = pls->icol0;
  page->curwidth = pls->width;

  /*//  gdk_threads_leave ();*/

}

/*--------------------------------------------------------------------------*\
 * plD_line_gnome()
 *
 * Draw a line in the current color from (x1,y1) to (x2,y2).
\*--------------------------------------------------------------------------*/

void
plD_line_gnome(PLStream *pls, short x1, short y1, short x2, short y2)
{
  short x[2];
  short y[2];

  x[0] = x1;
  x[1] = x2;
  y[0] = y1;
  y[1] = y2;

  plD_polyline_gnome(pls, x, y, (PLINT) 2);

}

/*--------------------------------------------------------------------------*\
 * plD_eop_gnome()
 *
 * End of page.  User must hit return (or third mouse button) to continue.
\*--------------------------------------------------------------------------*/

void
plD_eop_gnome(PLStream *pls)
{
  GnomePLdev* dev;
  GnomePLdevPage* page;
  GnomeCanvas* canvas;
  int i;
  short x, y;

  /*//  static int i;*/

  dev = pls->dev;

  plD_polyline_gnome(pls, NULL, NULL, 0);

  /*//  gdk_threads_enter ();*/

  page = dev->page[dev->npages-1];
  canvas = page->canvas;

  canvas->need_update = 1;
  gnome_canvas_update_now (canvas);

  gdk_threads_leave ();

  /*//  for (i=0;i<16;i++)*/
  /*//    printf("count[%d] = %d\n", i, count[i]);*/
  /*//  fflush (stdout);*/

  /*//  printf("eop #%d\n", i++);*/
  /*//  fflush (stdout);*/

  /*//  getchar();*/
}

/*--------------------------------------------------------------------------*\
 * plD_bop_gnome()
 *
 * Set up for the next page.
\*--------------------------------------------------------------------------*/

void
plD_bop_gnome(PLStream *pls)
{
  GnomePLdev* dev;
  GnomePLdevPage* page;
  GnomeCanvas* canvas;

  gdk_threads_enter ();

  new_page (pls);

  dev = pls->dev;

  /*//  printf("npages = %d\n", dev->npages);*/
  /*//  fflush (stdout);*/

  page = dev->page[dev->npages-1];
  canvas = page->canvas;

  canvas->need_update = 1;
  gnome_canvas_update_now (canvas);

  /*//  gdk_threads_leave ();*/

  pls->page++;
}

/*--------------------------------------------------------------------------*\
 * plD_tidy_gnome()
 *
 * Close graphics file
\*--------------------------------------------------------------------------*/

void
plD_tidy_gnome(PLStream *pls)
{

#ifdef HAVE_PTHREADS

  pthread_join (tid, NULL);

#else

  if (pls->nopause) {
    gtk_main_quit();
  } else
    gtk_main();

#endif

}

/*--------------------------------------------------------------------------*\
 * plD_state_gnome()
 *
 * Handle change in PLStream state (color, pen width, fill attribute, etc).
\*--------------------------------------------------------------------------*/

void
plD_state_gnome(PLStream *pls, PLINT op)
{
  switch (op) {

  case PLSTATE_WIDTH:
    break;

  case PLSTATE_COLOR0:
    break;

  case PLSTATE_COLOR1:
    break;

  case PLSTATE_CMAP0:
    break;

  case PLSTATE_CMAP1:
    break;
  }
}

static void
generate_pattern_fill (PLStream* pls)
{
  GnomePLdev* dev = pls->dev;
  int i;

  if (dev->pattern == pls->patt)
    return;

  dev->pattern = pls->patt;

  for (i = 0; i < pls->nps; i++) {
    int j;
    PLINT incl;
    double tani;
    double dist; /* in pixels */
    int width;
    int height;
    int cols;
    double inclr;
    double sini;
    double cosi;
    GdkGC* gc;
    GdkColor black;
    GdkColor white;
    GdkColormap* colormap;

    dist = (pls->delta[i]/1e3) * PIXELS_PER_MM;

    incl = pls->inclin[i] % 3600;
    if (incl > 900)
      incl = incl - 900;
    else if (incl < -900)
      incl = incl + 900;

    inclr = PI * incl / 1800.0;
    sini = sin (inclr);
    cosi = cos (inclr);

    if (ABS (sini) < cosi) {
      if (sini == 0.0) {
	width = 1;
	height = (int) dist;;
      }
      else {
	width = MIN (ABS (dist / sini), 64);
	height = MAX (dist / cosi, 2);
      }
    }
    else {
      if (ABS(incl) == 900) {
	width = (int) dist;
	height = 1;
      }
      else {
	width = MAX (ABS (dist / sini), 2);
	height = MIN (dist / cosi, 64);
      }
    }

    colormap = gtk_widget_get_colormap (GTK_WIDGET (dev->page[0]->canvas));

    gdk_color_parse ("white", &white);
    gdk_color_alloc (colormap, &white);
    gdk_color_parse ("black", &black);
    gdk_color_alloc (colormap, &black);

    if (dev->pattern_stipple[i] != NULL)
      gdk_pixmap_unref (dev->pattern_stipple[i]);

    dev->pattern_stipple[i] = gdk_pixmap_new (NULL, width, height, 1);

    gc = gdk_gc_new (dev->pattern_stipple[i]);

    gdk_gc_set_foreground (gc, &black);

    gdk_draw_rectangle (dev->pattern_stipple[i], gc, TRUE,
			0, 0, width, height);

    gdk_gc_set_foreground (gc, &white);
    gdk_gc_set_line_attributes (gc, 1, GDK_LINE_SOLID, 0, 0);

    if (incl == 0 || ABS (incl) == 900)
      gdk_draw_point (dev->pattern_stipple[i], gc, 0, 0);
    else if (sini < 0.0) {
      gdk_draw_line (dev->pattern_stipple[i], gc, 0, 0, width, height);
      gdk_draw_line (dev->pattern_stipple[i], gc, 0, -height,
		     2*width, height);
      gdk_draw_line (dev->pattern_stipple[i], gc, -width, 0,
		     width, 2*height);
    }
    else {
      gdk_draw_line (dev->pattern_stipple[i], gc, width, 0, 0, height);
      gdk_draw_line (dev->pattern_stipple[i], gc, 2*width, 0, 0, 2*height);
      gdk_draw_line (dev->pattern_stipple[i], gc,
		     width, -height, -width, height);
    }

    gdk_gc_unref (gc);

  }
}


static void
fill_polygon (PLStream* pls)
{
  GnomePLdev* dev;
  GnomePLdevPage* page;
  GnomeCanvasPoints* points;
  GnomeCanvasGroup* group;
  GnomeCanvasItem* item;
  GnomeCanvas* canvas;
  guint i;

  dev = pls->dev;

  /*//  gdk_threads_enter ();*/

  page = dev->page[dev->npages-1];

  canvas = page->canvas;

  group = gnome_canvas_root (canvas);

  points = gnome_canvas_points_new (pls->dev_npts);

  for ( i = 0; i < pls->dev_npts; i++ ) {
    points->coords[2*i] =
      ((double) pls->dev_x[i]/MAG_FACTOR) * PIXELS_PER_DU;
    points->coords[2*i + 1] =
      ((double) -pls->dev_y[i]/MAG_FACTOR) * PIXELS_PER_DU;
  }

  /* Experimental stuff for pattern fill */
  if (pls->patt < 0) {
    int i;

    generate_pattern_fill (pls);

    for (i = 0; i < pls->nps; i++)
      item = gnome_canvas_item_new (group,
				    gnome_canvas_polygon_get_type (),
				    "points", points,
				    "fill_stipple", dev->pattern_stipple[i],
				    "fill_color_rgba",
				    plcolor_to_rgba (pls->curcolor, 0xFF),
				    "width_units", 0.0,
				    NULL);
  }
  else
    item = gnome_canvas_item_new (group,
				  gnome_canvas_polygon_get_type (),
				  "points", points,
				  "fill_color_rgba",
				  plcolor_to_rgba (pls->curcolor, 0xFF),
				  "width_units", 0.0,
				  NULL);

  timeout_register (canvas, dev);

  set_color (item, 1, (double) pls->icol1);

  gtk_signal_connect (GTK_OBJECT (item), "event",
                      (GtkSignalFunc) canvas_pressed_cb,
                      page);

  gnome_canvas_points_unref (points);

  /*//  gdk_threads_leave ();*/

}

static void
dashed_line (PLStream* pls)
{
  GnomePLdev* dev;
  GnomePLdevPage* page;
  GnomeCanvasPoints* points;
  GnomeCanvasGroup* group;
  GnomeCanvasItem* item;
  GnomeCanvas* canvas;
  guint i;
  gchar* dash_list;

  dev = pls->dev;

  /*//  gdk_threads_enter ();*/

  page = dev->page[dev->npages-1];

  canvas = page->canvas;

  group = gnome_canvas_root (canvas);

  points = gnome_canvas_points_new (pls->dev_npts);

  for (i = 0; i < pls->dev_npts; i++) {
    points->coords[2*i] =
      ((double) pls->dev_x[i]/MAG_FACTOR) * PIXELS_PER_DU;
    points->coords[2*i + 1] =
      ((double) -pls->dev_y[i]/MAG_FACTOR) * PIXELS_PER_DU;
  }

  dash_list = g_malloc (sizeof (gchar) * 2 * pls->nms);
  for (i = 0; i < pls->nms; i++) {
    dash_list[2*i] = (gchar) ceil ((pls->mark[i]/1e3) * PIXELS_PER_MM);
    dash_list[2*i+1] = (gchar) floor ((pls->space[i]/1e3) * PIXELS_PER_MM);
    fflush(stdout);
  }

  gdk_gc_set_dashes (canvas->pixmap_gc, 0, dash_list, 2*pls->nms);

  g_free (dash_list);

  item = gnome_canvas_item_new (group,
                                gnome_canvas_line_get_type (),
				"cap_style", GDK_CAP_BUTT,
				"join_style", GDK_JOIN_ROUND,
				"line_style", GDK_LINE_ON_OFF_DASH,
                                "points", points,
				"fill_color_rgba",
				plcolor_to_rgba (pls->curcolor, 0xFF),
                                "width_units",
				MAX ((double) pls->width, 3.0) * PIXELS_PER_DU,
                                NULL);

  timeout_register (canvas, dev);

  set_color (item, 0, (double) pls->icol0);

  gtk_signal_connect (GTK_OBJECT (item), "event",
                      (GtkSignalFunc) canvas_pressed_cb,
                      page);

  gnome_canvas_points_unref (points);

  /*//  gdk_threads_leave ();*/

}

/*--------------------------------------------------------------------------*\
 * plD_esc_gnome()
 *
 * Escape function.
 *
 * Functions:
 *
 *	PLESC_EH	Handle pending events
 *	PLESC_EXPOSE	Force an expose
 *	PLESC_FILL	Fill polygon
 *	PLESC_FLUSH	Flush X event buffer
 *	PLESC_GETC	Get coordinates upon mouse click
 *	PLESC_REDRAW	Force a redraw
 *	PLESC_RESIZE	Force a resize
 * 	PLESC_XORMOD 	set/reset xor mode
\*--------------------------------------------------------------------------*/

static void
clear (PLStream* pls)
{
  GnomePLdev* dev = pls->dev;
  GnomePLdevPage* page;
  GnomeCanvas* canvas;
  GnomeCanvasGroup* group;
  GnomeCanvasItem* rect;

  fflush(stdout);

  page = dev->page[pls->page-1];
  canvas = page->canvas;
  group = gnome_canvas_root (canvas);

  rect = gnome_canvas_item_new (group,
				gnome_canvas_rect_get_type(),
				"x1", (double) (pls->sppxmi) * PIXELS_PER_DU,
				"y1", (double) -(pls->sppyma) * PIXELS_PER_DU,
				"x2", (double) (pls->sppxma) * PIXELS_PER_DU,
				"y2", (double) -(pls->sppymi) * PIXELS_PER_DU,
				"fill_color", (pls->cmap0[0]).name,
				"width_units", 0.0,
				NULL);
  gnome_canvas_item_raise_to_top (rect);

  gtk_signal_connect (GTK_OBJECT (rect), "event",
                      (GtkSignalFunc) canvas_pressed_cb,
                      page);

}

void
plD_esc_gnome(PLStream *pls, PLINT op, void *ptr)
{
  dbug_enter("plD_esc_gnome");

  switch (op) {

  case PLESC_CLEAR:
    clear (pls);
    break;

  case PLESC_DASH:
    dashed_line (pls);
    break;

  case PLESC_FILL:
    fill_polygon(pls);
    break;

  }
}

#else
int
pldummy_gnome()
{
    return 0;
}

#endif				/* PLD_gnome */
