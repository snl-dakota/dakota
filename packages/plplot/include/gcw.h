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
*/

#include "plDevs.h"
#include "plplotP.h"
#include "drivers.h"

#include <math.h>

#include <glib.h>
#include <gtk/gtk.h>
#include <libgnomecanvas/libgnomecanvas.h>
#include <libart_lgpl/libart.h>
#include <libgnomeprint/gnome-print.h>

typedef struct {
  GnomeCanvas* canvas;
  GnomeCanvasItem* background;
  GnomeCanvasGroup* group_background;
  GnomeCanvasGroup* group_foreground;
  GnomeCanvasGroup* group_visible;
  GnomeCanvasGroup* group_hidden;
  GnomeCanvasGroup* group_current;

  GtkWidget* window;
  GtkWidget* notebook;

  GdkColormap* colormap;
  guint32 color;

  gdouble width;
  gdouble height;

  PLINT pen_color;
  PLINT pen_width;

  gboolean zoom_is_initialized;

  PLINT pattern;
  GdkBitmap* pattern_stipple[2];

  gboolean use_text;
  gboolean use_fast_rendering;

  gboolean aa;

  gboolean use_pixmap;
  gboolean pixmap_has_data;
  GdkPixmap* pixmap;

} GcwPLdev;


/* Prototypes */
void gcw_set_canvas(PLStream* pls,GnomeCanvas* canvas);
void gcw_set_canvas_aspect(GnomeCanvas* canvas,PLFLT aspect);
void gcw_set_canvas_zoom(GnomeCanvas* canvas,PLFLT magnification);
void gcw_set_canvas_size(GnomeCanvas* canvas,PLFLT width,PLFLT height);
void gcw_get_canvas_viewport(GnomeCanvas* canvas,PLFLT xmin1,PLFLT xmax1,
			     PLFLT ymin1,PLFLT ymax1,PLFLT* xmin2,PLFLT* xmax2,
			     PLFLT* ymin2,PLFLT* ymax2);
void gcw_use_text(GnomeCanvas* canvas,PLINT use_text);
void gcw_use_fast_rendering(GnomeCanvas* canvas,PLINT use_fast_rendering);
void gcw_use_pixmap(GnomeCanvas* canvas,PLINT use_pixmap);
void gcw_use_foreground_group(GnomeCanvas* canvas);
void gcw_use_background_group(GnomeCanvas* canvas);
void gcw_use_default_group(GnomeCanvas* canvas);
