 /* SciPlot	A generalized plotting widget
 *
 * Copyright (c) 1996 Robert W. McMullen
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 * Author: Rob McMullen <rwmcm@mail.ae.utexas.edu>
 *         http://www.ae.utexas.edu/~rwmcm
 *
 * Modified for DAKOTA by Mike Eldred (see MSE notations below).
 *
 */

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
/* NOTE:  float.h is required by POSIX */
#define SCIPLOT_SKIP_VAL (-FLT_MAX)

#include "SciPlotP.h"

/* #define DEBUG_SCIPLOT */

#define offset(field) XtOffsetOf(SciPlotRec, plot.field)
static XtResource resources[] =
{
  {XtNchartType, XtCMargin, XtRInt, sizeof(int),
    offset(ChartType), XtRImmediate, (XtPointer) XtCARTESIAN},
  {XtNdegrees, XtCBoolean, XtRBoolean, sizeof(Boolean),
    offset(Degrees), XtRImmediate, (XtPointer) True},
  {XtNdrawMajor, XtCBoolean, XtRBoolean, sizeof(Boolean),
    offset(DrawMajor), XtRImmediate, (XtPointer) True},
  {XtNdrawMajorTics, XtCBoolean, XtRBoolean, sizeof(Boolean),
    offset(DrawMajorTics), XtRImmediate, (XtPointer) True},
  {XtNdrawMinor, XtCBoolean, XtRBoolean, sizeof(Boolean),
    offset(DrawMinor), XtRImmediate, (XtPointer) True},
  {XtNdrawMinorTics, XtCBoolean, XtRBoolean, sizeof(Boolean),
    offset(DrawMinorTics), XtRImmediate, (XtPointer) True},
  {XtNmonochrome, XtCBoolean, XtRBoolean, sizeof(Boolean),
    offset(Monochrome), XtRImmediate, (XtPointer) False},
  {XtNshowLegend, XtCBoolean, XtRBoolean, sizeof(Boolean),
    offset(ShowLegend), XtRImmediate, (XtPointer) True},
  {XtNshowTitle, XtCBoolean, XtRBoolean, sizeof(Boolean),
    offset(ShowTitle), XtRImmediate, (XtPointer) True},
  {XtNshowXLabel, XtCBoolean, XtRBoolean, sizeof(Boolean),
    offset(ShowXLabel), XtRImmediate, (XtPointer) True},
  {XtNshowYLabel, XtCBoolean, XtRBoolean, sizeof(Boolean),
    offset(ShowYLabel), XtRImmediate, (XtPointer) True},
  {XtNxLabel, XtCString, XtRString, sizeof(String),
    offset(TransientXLabel), XtRString, "X Axis"},
  {XtNyLabel, XtCString, XtRString, sizeof(String),
    offset(TransientYLabel), XtRString, "Y Axis"},
  {XtNplotTitle, XtCString, XtRString, sizeof(String),
    offset(TransientPlotTitle), XtRString, "Plot"},
  {XtNmargin, XtCMargin, XtRInt, sizeof(int),
    offset(Margin), XtRImmediate, (XtPointer) 5},
  {XtNtitleMargin, XtCMargin, XtRInt, sizeof(int),
    offset(TitleMargin), XtRImmediate, (XtPointer) 16},
  {XtNlegendLineSize, XtCMargin, XtRInt, sizeof(int),
    offset(LegendLineSize), XtRImmediate, (XtPointer) 16},
  {XtNdefaultMarkerSize, XtCMargin, XtRInt, sizeof(int),
    offset(DefaultMarkerSize), XtRImmediate, (XtPointer) 3},
  {XtNlegendMargin, XtCMargin, XtRInt, sizeof(int),
    offset(LegendMargin), XtRImmediate, (XtPointer) 3},
  {XtNlegendThroughPlot, XtCBoolean, XtRBoolean, sizeof(Boolean),
    offset(LegendThroughPlot), XtRImmediate, (XtPointer) False},
  {XtNtitleFont, XtCMargin, XtRInt, sizeof(int),
    offset(TitleFont), XtRImmediate, (XtPointer) (XtFONT_HELVETICA | 24)},
  {XtNlabelFont, XtCMargin, XtRInt, sizeof(int),
    offset(LabelFont), XtRImmediate, (XtPointer) (XtFONT_TIMES | 18)},
  {XtNaxisFont, XtCMargin, XtRInt, sizeof(int),
    offset(AxisFont), XtRImmediate, (XtPointer) (XtFONT_TIMES | 10)},
  {XtNxAutoScale, XtCBoolean, XtRBoolean, sizeof(Boolean),
    offset(XAutoScale), XtRImmediate, (XtPointer) True},
  {XtNyAutoScale, XtCBoolean, XtRBoolean, sizeof(Boolean),
    offset(YAutoScale), XtRImmediate, (XtPointer) True},
  {XtNxAxisNumbers, XtCBoolean, XtRBoolean, sizeof(Boolean),
    offset(XAxisNumbers), XtRImmediate, (XtPointer) True},
  {XtNyAxisNumbers, XtCBoolean, XtRBoolean, sizeof(Boolean),
    offset(YAxisNumbers), XtRImmediate, (XtPointer) True},
  {XtNxLog, XtCBoolean, XtRBoolean, sizeof(Boolean),
    offset(XLog), XtRImmediate, (XtPointer) False},
  {XtNyLog, XtCBoolean, XtRBoolean, sizeof(Boolean),
    offset(YLog), XtRImmediate, (XtPointer) False},
  {XtNxOrigin, XtCBoolean, XtRBoolean, sizeof(Boolean),
    offset(XOrigin), XtRImmediate, (XtPointer) False},
  {XtNyOrigin, XtCBoolean, XtRBoolean, sizeof(Boolean),
    offset(YOrigin), XtRImmediate, (XtPointer) False},
  {XtNyNumbersHorizontal, XtCBoolean, XtRBoolean, sizeof(Boolean),
    offset(YNumHorz), XtRImmediate, (XtPointer) True},
};

static SciPlotFontDesc font_desc_table[] =
{
  {XtFONT_TIMES, "Times", "times", False, True},
  {XtFONT_COURIER, "Courier", "courier", True, False},
  {XtFONT_HELVETICA, "Helvetica", "helvetica", True, False},
  {XtFONT_LUCIDA, "Lucida", "lucidabright", False, False},
  {XtFONT_LUCIDASANS, "LucidaSans", "lucida", False, False},
  {XtFONT_NCSCHOOLBOOK, "NewCenturySchlbk",
    "new century schoolbook", False, True},
  {-1, NULL, NULL, False, False},
};

/*
 * Private function declarations
 */

static void Redisplay();
static void Resize();
static Boolean SetValues();
static void GetValuesHook();
static void Initialize();
static void Realize();
static void Destroy();

static void ComputeAll();
static void ComputeAllDimensions();
static void DrawAll();
static void ItemDrawAll();
static void ItemDraw();
static void EraseAll();
static void FontInit();
static int ColorStore();
static int FontStore();
static int FontnumReplace();



SciPlotClassRec sciplotClassRec =
{
  {
    /* core_class fields        */
#ifdef MOTIF
    /* superclass               */ (WidgetClass) & xmPrimitiveClassRec,
#else
    /* superclass               */ (WidgetClass) & widgetClassRec,
#endif
    /* class_name               */ "SciPlot",
    /* widget_size              */ sizeof(SciPlotRec),
    /* class_initialize         */ NULL,
    /* class_part_initialize    */ NULL,
    /* class_inited             */ False,
    /* initialize               */ Initialize,
    /* initialize_hook          */ NULL,
    /* realize                  */ Realize,
    /* actions                  */ NULL,
    /* num_actions              */ 0,
    /* resources                */ resources,
    /* num_resources            */ XtNumber(resources),
    /* xrm_class                */ NULLQUARK,
    /* compress_motion          */ True,
    /* compress_exposure        */ XtExposeCompressMultiple,
    /* compress_enterleave      */ True,
    /* visible_interest         */ True,
    /* destroy                  */ Destroy,
    /* resize                   */ Resize,
    /* expose                   */ Redisplay,
    /* set_values               */ SetValues,
    /* set_values_hook          */ NULL,
    /* set_values_almost        */ XtInheritSetValuesAlmost,
    /* get_values_hook          */ GetValuesHook,
    /* accept_focus             */ NULL,
    /* version                  */ XtVersion,
    /* callback_private         */ NULL,
    /* tm_table                 */ NULL,
    /* query_geometry           */ NULL,
    /* display_accelerator      */ XtInheritDisplayAccelerator,
    /* extension                */ NULL
  },
#ifdef MOTIF
  {
    /* primitive_class fields   */
    /* border_highlight         */ (XtWidgetProc) _XtInherit,
    /* border_unhighligh        */ (XtWidgetProc) _XtInherit,
    /* translations             */ XtInheritTranslations,
    /* arm_and_activate         */ (XtWidgetProc) _XtInherit,
    /* syn_resources            */ NULL,
    /* num_syn_resources        */ 0,
    /* extension                */ NULL
  },
#endif
  {
    /* plot_class fields        */
    /* dummy                    */ 0
    /* (some stupid compilers barf on empty structures) */
  }
};

WidgetClass sciplotWidgetClass = (WidgetClass) & sciplotClassRec;


static void
Initialize(Widget treq, Widget tnew, ArgList args, Cardinal *num)
{
  SciPlotWidget new;

  new = (SciPlotWidget) tnew;

  new->plot.plotlist = NULL;
  new->plot.alloc_plotlist = 0;
  new->plot.num_plotlist = 0;

  new->plot.alloc_drawlist = NUMPLOTITEMALLOC;
  new->plot.drawlist = (SciPlotItem *) XtCalloc(new->plot.alloc_drawlist,
    sizeof(SciPlotItem));
  new->plot.num_drawlist = 0;

  new->plot.cmap = DefaultColormap(XtDisplay(new),
    DefaultScreen(XtDisplay(new)));

  new->plot.xlabel = (char *) XtMalloc(strlen(new->plot.TransientXLabel) + 1);
  strcpy(new->plot.xlabel, new->plot.TransientXLabel);
  new->plot.ylabel = (char *) XtMalloc(strlen(new->plot.TransientYLabel) + 1);
  strcpy(new->plot.ylabel, new->plot.TransientYLabel);
  new->plot.plotTitle
    = (char *) XtMalloc(strlen(new->plot.TransientPlotTitle) + 1);
  strcpy(new->plot.plotTitle, new->plot.TransientPlotTitle);
  new->plot.TransientXLabel=NULL;
  new->plot.TransientYLabel=NULL;
  new->plot.TransientPlotTitle=NULL;

  new->plot.colors = NULL;
  new->plot.num_colors = 0;
  new->plot.fonts = NULL;
  new->plot.num_fonts = 0;

  new->plot.update = FALSE;
  new->plot.UserMin.x = new->plot.UserMin.y = 0.0;
  new->plot.UserMax.x = new->plot.UserMax.y = 10.0;

  new->plot.titleFont = FontStore(new, new->plot.TitleFont);
  new->plot.labelFont = FontStore(new, new->plot.LabelFont);
  new->plot.axisFont = FontStore(new, new->plot.AxisFont);
}

static void
GCInitialize(SciPlotWidget new)
{
  XGCValues values;
  XtGCMask mask;
  long colorsave;

  values.line_style = LineSolid;
  values.line_width = 0;
  values.fill_style = FillSolid;
  values.background = WhitePixelOfScreen(XtScreen(new));
  values.background = new->core.background_pixel;
  new->plot.BackgroundColor = ColorStore(new, values.background);
#ifdef MOTIF
  new->core.background_pixel = values.background;
#endif
  values.foreground = colorsave = BlackPixelOfScreen(XtScreen(new));
  new->plot.ForegroundColor = ColorStore(new, values.foreground);

  mask = GCLineStyle | GCLineWidth | GCFillStyle | GCForeground | GCBackground;
  new->plot.defaultGC = XCreateGC(XtDisplay(new),XtWindow(new), mask, &values);

  values.foreground = colorsave;
  values.line_style = LineOnOffDash;
  new->plot.dashGC = XCreateGC(XtDisplay(new),XtWindow(new), mask, &values);
}

static void
Realize(Widget aw, XtValueMask * value_mask, XSetWindowAttributes * attributes)
{
  SciPlotWidget w = (SciPlotWidget) aw;

#define	superclass	(&widgetClassRec)
  (*superclass->core_class.realize) (aw, value_mask, attributes);
#undef	superclass

  GCInitialize(w);
}

static void
Destroy(SciPlotWidget w)
{
  int i;
  SciPlotFont *pf;
  SciPlotList *p;

  XFreeGC(XtDisplay(w), w->plot.defaultGC);
  XFreeGC(XtDisplay(w), w->plot.dashGC);
  XtFree((char *) w->plot.xlabel);
  XtFree((char *) w->plot.ylabel);
  XtFree((char *) w->plot.plotTitle);

  for (i = 0; i < w->plot.num_fonts; i++) {
    pf = &w->plot.fonts[i];
    XFreeFont(XtDisplay((Widget) w), pf->font);
  }
  XtFree((char *) w->plot.fonts);

  XtFree((char *) w->plot.colors);

  for (i = 0; i < w->plot.alloc_plotlist; i++) {
    p = w->plot.plotlist + i;
    if (p->allocated > 0)
      XtFree((char *) p->data);
    if (p->legend)
      XtFree(p->legend);
  }
  if (w->plot.alloc_plotlist > 0)
    XtFree((char *) w->plot.plotlist);

  EraseAll(w);
  XtFree((char *) w->plot.drawlist);
}

static Boolean
SetValues(SciPlotWidget current, SciPlotWidget request, SciPlotWidget new,
  ArgList args, Cardinal nargs)
{
  Boolean redisplay = FALSE;

  if (current->plot.XLog != new->plot.XLog)
    redisplay = TRUE;
  else if (current->plot.YLog != new->plot.YLog)
    redisplay = TRUE;
  else if (current->plot.XOrigin != new->plot.XOrigin)
    redisplay = TRUE;
  else if (current->plot.YOrigin != new->plot.YOrigin)
    redisplay = TRUE;
  else if (current->plot.XAxisNumbers != new->plot.XAxisNumbers)
    redisplay = TRUE;
  else if (current->plot.YAxisNumbers != new->plot.YAxisNumbers)
    redisplay = TRUE;
  else if (current->plot.DrawMajor != new->plot.DrawMajor)
    redisplay = TRUE;
  else if (current->plot.DrawMajorTics != new->plot.DrawMajorTics)
    redisplay = TRUE;
  else if (current->plot.DrawMinor != new->plot.DrawMinor)
    redisplay = TRUE;
  else if (current->plot.DrawMinorTics != new->plot.DrawMinorTics)
    redisplay = TRUE;
  else if (current->plot.ChartType != new->plot.ChartType)
    redisplay = TRUE;
  else if (current->plot.Degrees != new->plot.Degrees)
    redisplay = TRUE;
  else if (current->plot.ShowLegend != new->plot.ShowLegend)
    redisplay = TRUE;
  else if (current->plot.ShowTitle != new->plot.ShowTitle)
    redisplay = TRUE;
  else if (current->plot.ShowXLabel != new->plot.ShowXLabel)
    redisplay = TRUE;
  else if (current->plot.ShowYLabel != new->plot.ShowYLabel)
    redisplay = TRUE;
  else if (current->plot.ShowTitle != new->plot.ShowTitle)
    redisplay = TRUE;
  else if (current->plot.Monochrome != new->plot.Monochrome)
    redisplay = TRUE;
  
  if (new->plot.TransientXLabel) {
    if (current->plot.TransientXLabel != new->plot.TransientXLabel ||
        strcmp(new->plot.TransientXLabel,current->plot.xlabel)!=0) {
      redisplay = TRUE;
      XtFree(current->plot.xlabel);
      new->plot.xlabel = (char *) XtMalloc(strlen(new->plot.TransientXLabel) + 1);
      strcpy(new->plot.xlabel, new->plot.TransientXLabel);
      new->plot.TransientXLabel=NULL;
    }
  }
  if (new->plot.TransientYLabel) {
    if (current->plot.TransientYLabel != new->plot.TransientYLabel ||
        strcmp(new->plot.TransientYLabel,current->plot.ylabel)!=0) {
      redisplay = TRUE;
      XtFree(current->plot.ylabel);
      new->plot.ylabel = (char *) XtMalloc(strlen(new->plot.TransientYLabel) + 1);
      strcpy(new->plot.ylabel, new->plot.TransientYLabel);
      new->plot.TransientYLabel=NULL;
    }
  }
  if (new->plot.TransientPlotTitle) {
    if (current->plot.TransientPlotTitle != new->plot.TransientPlotTitle ||
        strcmp(new->plot.TransientPlotTitle,current->plot.plotTitle)!=0) {
      redisplay = TRUE;
      XtFree(current->plot.plotTitle);
      new->plot.plotTitle = (char *) XtMalloc(strlen(new->plot.TransientPlotTitle) + 1);
      strcpy(new->plot.plotTitle, new->plot.TransientPlotTitle);
      new->plot.TransientPlotTitle=NULL;
    }
  }
  
  if (current->plot.AxisFont != new->plot.AxisFont) {
    redisplay = TRUE;
    FontnumReplace(new, new->plot.axisFont, new->plot.AxisFont);
  }
  if (current->plot.TitleFont != new->plot.TitleFont) {
    redisplay = TRUE;
    FontnumReplace(new, new->plot.titleFont, new->plot.TitleFont);
  }
  if (current->plot.LabelFont != new->plot.LabelFont) {
    redisplay = TRUE;
    FontnumReplace(new, new->plot.labelFont, new->plot.LabelFont);
  }

  new->plot.update = redisplay;

  return redisplay;
}

static void
GetValuesHook(SciPlotWidget w, ArgList args, Cardinal *num_args)
{
  int i;
  char **loc;

  for (i=0; i<*num_args; i++) {
    loc=(char **)args[i].value;
    if (strcmp(args[i].name,XtNplotTitle)==0)
      *loc=w->plot.plotTitle;
    else if (strcmp(args[i].name,XtNxLabel)==0)
      *loc=w->plot.xlabel;
    else if (strcmp(args[i].name,XtNyLabel)==0)
      *loc=w->plot.ylabel;
      
  }
}


static void
Redisplay(SciPlotWidget w)
{
  if (!XtIsRealized((Widget)w))
    return;
  
  if (w->plot.update) {
    Resize(w);
    w->plot.update = FALSE;
  }
  else {
    ItemDrawAll(w);
  }
}

static void
Resize(SciPlotWidget w)
{
  if (!XtIsRealized((Widget)w))
    return;
  
  EraseAll(w);
  ComputeAll(w);
  DrawAll(w);
}


/*
 * Private SciPlot utility functions
 */


static int 
ColorStore (SciPlotWidget w, Pixel color)
{
  w->plot.num_colors++;
  w->plot.colors = (Pixel *) XtRealloc((char *) w->plot.colors,
    sizeof(Pixel) * w->plot.num_colors);
  w->plot.colors[w->plot.num_colors - 1] = color;
  return w->plot.num_colors - 1;
}

static void 
FontnumStore (SciPlotWidget w, int fontnum, int flag)
{
  SciPlotFont *pf;
  int fontflag, sizeflag, attrflag;

  pf = &w->plot.fonts[fontnum];

  fontflag = flag & XtFONT_NAME_MASK;
  sizeflag = flag & XtFONT_SIZE_MASK;
  attrflag = flag & XtFONT_ATTRIBUTE_MASK;

  switch (fontflag) {
  case XtFONT_TIMES:
  case XtFONT_COURIER:
  case XtFONT_HELVETICA:
  case XtFONT_LUCIDA:
  case XtFONT_LUCIDASANS:
  case XtFONT_NCSCHOOLBOOK:
    break;
  default:
    fontflag = XtFONT_NAME_DEFAULT;
    break;
  }

  if (sizeflag < 1)
    sizeflag = XtFONT_SIZE_DEFAULT;

  switch (attrflag) {
  case XtFONT_BOLD:
  case XtFONT_ITALIC:
  case XtFONT_BOLD_ITALIC:
    break;
  default:
    attrflag = XtFONT_ATTRIBUTE_DEFAULT;
    break;
  }
  pf->id = flag;
  FontInit(w, pf);
}

static int 
FontnumReplace (SciPlotWidget w, int fontnum, int flag)
{
  SciPlotFont *pf;

  pf = &w->plot.fonts[fontnum];
  XFreeFont(XtDisplay(w), pf->font);

  FontnumStore(w, fontnum, flag);

  return fontnum;
}

static int 
FontStore (SciPlotWidget w, int flag)
{
  int fontnum;

  w->plot.num_fonts++;
  w->plot.fonts = (SciPlotFont *) XtRealloc((char *) w->plot.fonts,
    sizeof(SciPlotFont) * w->plot.num_fonts);
  fontnum = w->plot.num_fonts - 1;

  FontnumStore(w, fontnum, flag);

  return fontnum;
}

static SciPlotFontDesc *
FontDescLookup (int flag)
{
  SciPlotFontDesc *pfd;

  pfd = font_desc_table;
  while (pfd->flag >= 0) {
#ifdef DEBUG_SCIPLOT
    printf("checking if %d == %d (font %s)\n",
      flag & XtFONT_NAME_MASK, pfd->flag, pfd->PostScript);
#endif
    if ((flag & XtFONT_NAME_MASK) == pfd->flag)
      return pfd;
    pfd++;
  }
  return NULL;
}


static void 
FontnumPostScriptString (SciPlotWidget w, int fontnum, char *str)
{
  char temp[128];
  int flag, bold, italic;
  SciPlotFontDesc *pfd;

  flag = w->plot.fonts[fontnum].id;
  pfd = FontDescLookup(flag);
  if (pfd) {
    strcpy(temp, pfd->PostScript);
    bold = False;
    italic = False;
    if (flag & XtFONT_BOLD) {
      bold = True;
      strcat(temp, "-Bold");
    }
    if (flag & XtFONT_ITALIC) {
      italic = True;
      if (!bold)
	strcat(temp, "-");
      if (pfd->PSUsesOblique)
	strcat(temp, "Oblique");
      else
	strcat(temp, "Italic");
    }
    if (!bold && !italic && pfd->PSUsesRoman) {
      strcat(temp, "-Roman");
    }

    sprintf(str, "/%s findfont %d scalefont",
      temp,
      (flag & XtFONT_SIZE_MASK));
  }
  else
    sprintf(str, "/Courier findfond 10 scalefont");
}

static void 
FontX11String (int flag, char *str)
{
  SciPlotFontDesc *pfd;

  pfd = FontDescLookup(flag);
  if (pfd) {
    sprintf(str, "-*-%s-%s-%s-*-*-%d-*-*-*-*-*-*-*",
      pfd->X11,
      (flag & XtFONT_BOLD ? "bold" : "medium"),
      (flag & XtFONT_ITALIC ? (pfd->PSUsesOblique ? "o" : "i") : "r"),
      (flag & XtFONT_SIZE_MASK));
  }
  else
    sprintf(str, "fixed");
#ifdef DEBUG_SCIPLOT
  printf("font string=%s\n", str);
#endif
}

static void 
FontInit (SciPlotWidget w, SciPlotFont *pf)
{
  char str[256], **list;
  int num;

  FontX11String(pf->id, str);
  list = XListFonts(XtDisplay(w), str, 100, &num);
#ifdef DEBUG_SCIPLOT
  if (1) {
    int i;

    i = 0;
    while (i < num) {
      printf("Found font: %s\n", list[i]);
      i++;
    }
  }
#endif
  if (num <= 0) {
    pf->id &= ~XtFONT_ATTRIBUTE_MASK;
    pf->id |= XtFONT_ATTRIBUTE_DEFAULT;
    FontX11String(pf->id, str);
    list = XListFonts(XtDisplay(w), str, 100, &num);
#ifdef DEBUG_SCIPLOT
    if (1) {
      int i;

      i = 0;
      while (i < num) {
	printf("Attr reset: found: %s\n", list[i]);
	i++;
      }
    }
#endif
  }
  if (num <= 0) {
    pf->id &= ~XtFONT_NAME_MASK;
    pf->id |= XtFONT_NAME_DEFAULT;
    FontX11String(pf->id, str);
    list = XListFonts(XtDisplay(w), str, 100, &num);
#ifdef DEBUG_SCIPLOT
    if (1) {
      int i;

      i = 0;
      while (i < num) {
	printf("Name reset: found: %s\n", list[i]);
	i++;
      }
    }
#endif
  }
  if (num <= 0) {
    pf->id &= ~XtFONT_SIZE_MASK;
    pf->id |= XtFONT_SIZE_DEFAULT;
    FontX11String(pf->id, str);
    list = XListFonts(XtDisplay(w), str, 100, &num);
#ifdef DEBUG_SCIPLOT
    if (1) {
      int i;

      i = 0;
      while (i < num) {
	printf("Size reset: found: %s\n", list[i]);
	i++;
      }
    }
#endif
  }
  if (num <= 0)
    strcpy(str, "fixed");
  else
    XFreeFontNames(list);
  pf->font = XLoadQueryFont(XtDisplay(w), str);
}

static XFontStruct *
FontFromFontnum (SciPlotWidget w, int fontnum)
{
  XFontStruct *f;

  if (fontnum >= w->plot.num_fonts)
    fontnum = 0;
  f = w->plot.fonts[fontnum].font;
  return f;
}

static real
FontHeight(XFontStruct *f)
{
  return (real)(f->max_bounds.ascent + f->max_bounds.descent);
}

static real
FontnumHeight(SciPlotWidget w, int fontnum)
{
  XFontStruct *f;

  f = FontFromFontnum(w, fontnum);
  return FontHeight(f);
}

static real
FontDescent(XFontStruct *f)
{
  return (real)(f->max_bounds.descent);
}

static real
FontnumDescent(SciPlotWidget w, int fontnum)
{
  XFontStruct *f;

  f = FontFromFontnum(w, fontnum);
  return FontDescent(f);
}

static real
FontAscent(XFontStruct *f)
{
  return (real)(f->max_bounds.ascent);
}

static real
FontnumAscent(SciPlotWidget w, int fontnum)
{
  XFontStruct *f;

  f = FontFromFontnum(w, fontnum);
  return FontAscent(f);
}

static real
FontTextWidth(XFontStruct *f, char *c)
{
  return (real)XTextWidth(f, c, strlen(c));
}

static real
FontnumTextWidth(SciPlotWidget w, int fontnum, char *c)
{
  XFontStruct *f;

  f = FontFromFontnum(w, fontnum);
  return FontTextWidth(f, c);
}


/*
 * Private List functions
 */

static int 
_ListNew (SciPlotWidget w)
{
  int index;
  SciPlotList *p;
  Boolean found;

/* First check to see if there is any free space in the index */
  found = FALSE;
  for (index = 0; index < w->plot.num_plotlist; index++) {
    p = w->plot.plotlist + index;
    if (!p->used) {
      found = TRUE;
      break;
    }
  }

/* If no space is found, increase the size of the index */
  if (!found) {
    w->plot.num_plotlist++;
    if (w->plot.alloc_plotlist == 0) {
      w->plot.alloc_plotlist = NUMPLOTLINEALLOC;
      w->plot.plotlist = (SciPlotList *) XtCalloc(w->plot.alloc_plotlist,
                                                  sizeof(SciPlotList));
      if (!w->plot.plotlist) {
	printf("Can't calloc memory for SciPlotList\n");
	exit(1);
      }
      w->plot.alloc_plotlist = NUMPLOTLINEALLOC;
    }
    else if (w->plot.num_plotlist > w->plot.alloc_plotlist) {
      w->plot.alloc_plotlist += NUMPLOTLINEALLOC;
      w->plot.plotlist = (SciPlotList *) XtRealloc((char *) w->plot.plotlist,
	w->plot.alloc_plotlist * sizeof(SciPlotList));
      if (!w->plot.plotlist) {
	printf("Can't realloc memory for SciPlotList\n");
	exit(1);
      }
    }
    index = w->plot.num_plotlist - 1;
    p = w->plot.plotlist + index;
  }

  p->LineStyle = p->LineColor = p->PointStyle = p->PointColor = 0;
  p->number = p->allocated = 0;
  p->data = NULL;
  p->legend = NULL;
  p->draw = p->used = TRUE;
  p->markersize = (real)w->plot.DefaultMarkerSize;
  return index;
}

static void 
_ListDelete (SciPlotList *p)
{
  p->draw = p->used = FALSE;
  p->number = p->allocated = 0;
  if (p->data)
    XtFree((char *) p->data);
  p->data = NULL;
  if (p->legend)
    XtFree((char *) p->legend);
  p->legend = NULL;
}

static SciPlotList *
_ListFind (SciPlotWidget w, int id)
{
  SciPlotList *p;

  if ((id >= 0) && (id < w->plot.num_plotlist)) {
    p = w->plot.plotlist + id;
    if (p->used)
      return p;
  }
  return NULL;
}

static void 
_ListSetStyle (SciPlotList *p, int pcolor, int pstyle, int lcolor, int lstyle)
{
/* Note!  Do checks in here later on... */

  if (lstyle >= 0)
    p->LineStyle = lstyle;
  if (lcolor >= 0)
    p->LineColor = lcolor;
  if (pstyle >= 0)
    p->PointStyle = pstyle;
  if (pcolor >= 0)
    p->PointColor = pcolor;
}

static void 
_ListSetLegend (SciPlotList *p, const char *legend)
{
/* Note!  Do checks in here later on... */

  p->legend = (char *) XtMalloc(strlen(legend) + 1);
  strcpy(p->legend, legend);
}

static void 
_ListAllocData (SciPlotList *p, int num)
{
  if (p->data) {
    XtFree((char *) p->data);
    p->allocated = 0;
  }
  p->allocated = num + NUMPLOTDATAEXTRA;
  p->data = (realpair *) XtCalloc(p->allocated, sizeof(realpair));
  if (!p->data) {
    p->number = p->allocated = 0;
  }
}

static void 
_ListReallocData (SciPlotList *p, int more)
{
  if (!p->data) {
    _ListAllocData(p, more);
  }
  else if (p->number + more > p->allocated) {
    /* MSE, 5/16/03: changed to 2x growth for efficiency with larger arrays */
    /* p->allocated += more + NUMPLOTDATAEXTRA;   old: constant growth */
    p->allocated = 2*(p->number + more);       /* new: factor of 2 growth */
    p->data = (realpair *) XtRealloc((char *) p->data,
                                     p->allocated * sizeof(realpair));
    if (!p->data) {
      p->number = p->allocated = 0;
    }
  }

}

static void 
_ListAddReal (SciPlotList *p, int num, real *xlist, real *ylist)
{
  int i;

  _ListReallocData(p, num);
  if (p->data) {
    for (i = 0; i < num; i++) {
      p->data[i + p->number].x = xlist[i];
      p->data[i + p->number].y = ylist[i];
    }
    p->number += num;
  }
}

static void 
_ListAddFloat (SciPlotList *p, int num, float *xlist, float *ylist)
{
  int i;

  _ListReallocData(p, num);
  if (p->data) {
    for (i = 0; i < num; i++) {
      p->data[i + p->number].x = xlist[i];
      p->data[i + p->number].y = ylist[i];
    }
    p->number += num;
  }
}

static void 
_ListAddDouble (SciPlotList *p, int num, double *xlist, double *ylist)
{
  int i;

  _ListReallocData(p, num);
  if (p->data) {
    for (i = 0; i < num; i++) {
      p->data[i + p->number].x = xlist[i];
      p->data[i + p->number].y = ylist[i];
    }
    p->number += num;
  }
}

static void
_ListSetReal(SciPlotList *p, int num, real *xlist, real *ylist)
{
  if ((!p->data) || (p->allocated < num))
    _ListAllocData(p, num);
  p->number = 0;
  _ListAddReal(p, num, xlist, ylist);
}

static void 
_ListSetFloat (SciPlotList *p, int num, float *xlist, float *ylist)
{
  if ((!p->data) || (p->allocated < num))
    _ListAllocData(p, num);
  p->number = 0;
  _ListAddFloat(p, num, xlist, ylist);
}

static void 
_ListSetDouble (SciPlotList *p, int num, double *xlist, double *ylist)
{
  if ((!p->data) || (p->allocated < num))
    _ListAllocData(p, num);
  p->number = 0;
  _ListAddDouble(p, num, xlist, ylist);
}


/*
 * Private SciPlot functions
 */


/* 
 * The following vertical text drawing routine uses the "Fill Stippled" idea
 * found in xvertext-5.0, by Alan Richardson (mppa3@syma.sussex.ac.uk).
 * 
 * The following code is my interpretation of his idea, including some
 * hacked together excerpts from his source.  The credit for the clever bits
 * belongs to him.
 * 
 * To be complete, portions of the subroutine XDrawVString are
 * Copyright (c) 1993 Alan Richardson (mppa3@syma.sussex.ac.uk)
 */
static void 
XDrawVString (Display *display, Window win, GC gc, int x, int y, char *str,
              int len, XFontStruct *f)
{
  XImage *before, *after;
  char *dest, *source;
  int xloop, yloop, xdest, ydest;
  Pixmap pix, rotpix;
  int width, height;
  GC drawGC;

  width = (int) FontTextWidth(f, str);
  height = (int) FontHeight(f);

  pix = XCreatePixmap(display, win, width, height, 1);
  rotpix = XCreatePixmap(display, win, height, width, 1);

  drawGC = XCreateGC(display, pix, 0L, NULL);
  XSetBackground(display, drawGC, 0);
  XSetFont(display, drawGC, f->fid);
  XSetForeground(display, drawGC, 0);
  XFillRectangle(display, pix, drawGC, 0, 0, width, height);
  XFillRectangle(display, rotpix, drawGC, 0, 0, height, width);
  XSetForeground(display, drawGC, 1);

  XDrawImageString(display, pix, drawGC, 0, (int) FontAscent(f),
    str, strlen(str));

  source = (char *) XtCalloc((((width + 7) / 8) * height), 1);
  before = XCreateImage(display, DefaultVisual(display, DefaultScreen(display)),
    1, XYPixmap, 0, source, width, height, 8, 0);
  before->byte_order = before->bitmap_bit_order = MSBFirst;
  XGetSubImage(display, pix, 0, 0, width, height, 1L, XYPixmap, before, 0, 0);
  source = (char *) XtCalloc((((height + 7) / 8) * width), 1);
  after = XCreateImage(display, DefaultVisual(display, DefaultScreen(display)),
    1, XYPixmap, 0, source, height, width, 8, 0);
  after->byte_order = after->bitmap_bit_order = MSBFirst;

  for (yloop = 0; yloop < height; yloop++) {
    for (xloop = 0; xloop < width; xloop++) {
      source = before->data + (xloop / 8) +
	(yloop * before->bytes_per_line);
      if (*source & (128 >> (xloop % 8))) {
	dest = after->data + (yloop / 8) +
	  ((width - 1 - xloop) * after->bytes_per_line);
	*dest |= (128 >> (yloop % 8));
      }
    }
  }

#ifdef DEBUG_SCIPLOT_VTEXT
  if (1) {
    char sourcebit;

    for (yloop = 0; yloop < before->height; yloop++) {
      for (xloop = 0; xloop < before->width; xloop++) {
	source = before->data + (xloop / 8) +
	  (yloop * before->bytes_per_line);
	sourcebit = *source & (128 >> (xloop % 8));
	if (sourcebit)
	  putchar('X');
	else
	  putchar('.');
      }
      putchar('\n');
    }

    for (yloop = 0; yloop < after->height; yloop++) {
      for (xloop = 0; xloop < after->width; xloop++) {
	source = after->data + (xloop / 8) +
	  (yloop * after->bytes_per_line);
	sourcebit = *source & (128 >> (xloop % 8));
	if (sourcebit)
	  putchar('X');
	else
	  putchar('.');
      }
      putchar('\n');
    }
  }
#endif

  xdest = x - (int) FontAscent(f);
  if (xdest < 0)
    xdest = 0;
  ydest = y - width;

  XPutImage(display, rotpix, drawGC, after, 0, 0, 0, 0,
    after->width, after->height);

  XSetFillStyle(display, gc, FillStippled);
  XSetStipple(display, gc, rotpix);
  XSetTSOrigin(display, gc, xdest, ydest);
  XFillRectangle(display, win, gc, xdest, ydest, after->width, after->height);
  XSetFillStyle(display, gc, FillSolid);

  XFreeGC(display, drawGC);
  XDestroyImage(before);
  XDestroyImage(after);
  XFreePixmap(display, pix);
  XFreePixmap(display, rotpix);

/* Note that it appears that there is a memory leak here, but XDestroyImage
 * frees the image data that is XtCalloc'ed
 */

}

static char dots[] =
{2, 1, 1};
static char widedots[] =
{2, 1, 4};

static GC 
ItemGetGC (SciPlotWidget w, SciPlotItem *item)
{
  GC gc;
  short color;

  switch (item->kind.any.style) {
  case XtLINE_SOLID:
    gc = w->plot.defaultGC;
    break;
  case XtLINE_DOTTED:
    XSetDashes(XtDisplay(w), w->plot.dashGC, 0, &dots[1],
      (int) dots[0]);
    gc = w->plot.dashGC;
    break;
  case XtLINE_WIDEDOT:
    XSetDashes(XtDisplay(w), w->plot.dashGC, 0, &widedots[1],
      (int) widedots[0]);
    gc = w->plot.dashGC;
    break;
  default:
    return NULL;
    break;
  }
  if (w->plot.Monochrome)
    if (item->kind.any.color > 0)
      color = w->plot.ForegroundColor;
    else
      color = w->plot.BackgroundColor;
  else if (item->kind.any.color >= w->plot.num_colors)
    color = w->plot.ForegroundColor;
  else if (item->kind.any.color <= 0)
    color = w->plot.BackgroundColor;
  else
    color = item->kind.any.color;
  XSetForeground(XtDisplay(w), gc, w->plot.colors[color]);
  return gc;
}

static GC 
ItemGetFontGC (SciPlotWidget w, SciPlotItem *item)
{
  GC gc;
  short color, fontnum;

  gc = w->plot.dashGC;
  if (w->plot.Monochrome)
    if (item->kind.any.color > 0)
      color = w->plot.ForegroundColor;
    else
      color = w->plot.BackgroundColor;
  else if (item->kind.any.color >= w->plot.num_colors)
    color = w->plot.ForegroundColor;
  else if (item->kind.any.color <= 0)
    color = w->plot.BackgroundColor;
  else
    color = item->kind.any.color;
  XSetForeground(XtDisplay(w), gc, w->plot.colors[color]);
  if (item->kind.text.font >= w->plot.num_fonts)
    fontnum = 0;
  else
    fontnum = item->kind.text.font;

/*
 * fontnum==0 hack:  0 is supposed to be the default font, but the program
 * can't seem to get the default font ID from the GC for some reason.  So,
 * use a different GC where the default font still exists.
 */
  XSetFont(XtDisplay(w), gc, w->plot.fonts[fontnum].font->fid);
  return gc;
}

static void 
ItemDraw (SciPlotWidget w, SciPlotItem *item)
{
  XPoint point[8];
  XSegment seg;
  XRectangle rect;
  int i;
  GC gc;

  if (!XtIsRealized((Widget) w))
    return;
  if ((item->type > SciPlotStartTextTypes) &&
      (item->type < SciPlotEndTextTypes))
    gc = ItemGetFontGC(w, item);
  else
    gc = ItemGetGC(w, item);
  if (!gc)
    return;
  switch (item->type) {
  case SciPlotLine:
    seg.x1 = (short) item->kind.line.x1;
    seg.y1 = (short) item->kind.line.y1;
    seg.x2 = (short) item->kind.line.x2;
    seg.y2 = (short) item->kind.line.y2;
    XDrawSegments(XtDisplay(w), XtWindow(w), gc,
      &seg, 1);
    break;
  case SciPlotRect:
    XDrawRectangle(XtDisplay(w), XtWindow(w), gc,
      (int) (item->kind.rect.x),
      (int) (item->kind.rect.y),
      (unsigned int) (item->kind.rect.w),
      (unsigned int) (item->kind.rect.h));
    break;
  case SciPlotFRect:
    XFillRectangle(XtDisplay(w), XtWindow(w), gc,
      (int) (item->kind.rect.x),
      (int) (item->kind.rect.y),
      (unsigned int) (item->kind.rect.w),
      (unsigned int) (item->kind.rect.h));
    XDrawRectangle(XtDisplay(w), XtWindow(w), gc,
      (int) (item->kind.rect.x),
      (int) (item->kind.rect.y),
      (unsigned int) (item->kind.rect.w),
      (unsigned int) (item->kind.rect.h));
    break;
  case SciPlotPoly:
    i = 0;
    while (i < item->kind.poly.count) {
      point[i].x = (int) item->kind.poly.x[i];
      point[i].y = (int) item->kind.poly.y[i];
      i++;
    }
    point[i].x = (int) item->kind.poly.x[0];
    point[i].y = (int) item->kind.poly.y[0];
    XDrawLines(XtDisplay(w), XtWindow(w), gc,
      point, i + 1, CoordModeOrigin);
    break;
  case SciPlotFPoly:
    i = 0;
    while (i < item->kind.poly.count) {
      point[i].x = (int) item->kind.poly.x[i];
      point[i].y = (int) item->kind.poly.y[i];
      i++;
    }
    point[i].x = (int) item->kind.poly.x[0];
    point[i].y = (int) item->kind.poly.y[0];
    XFillPolygon(XtDisplay(w), XtWindow(w), gc,
      point, i + 1, Complex, CoordModeOrigin);
    XDrawLines(XtDisplay(w), XtWindow(w), gc,
      point, i + 1, CoordModeOrigin);
    break;
  case SciPlotCircle:
    XDrawArc(XtDisplay(w), XtWindow(w), gc,
      (int) (item->kind.circ.x - item->kind.circ.r),
      (int) (item->kind.circ.y - item->kind.circ.r),
      (unsigned int) (item->kind.circ.r * 2),
      (unsigned int) (item->kind.circ.r * 2),
      0 * 64, 360 * 64);
    break;
  case SciPlotFCircle:
    XFillArc(XtDisplay(w), XtWindow(w), gc,
      (int) (item->kind.circ.x - item->kind.circ.r),
      (int) (item->kind.circ.y - item->kind.circ.r),
      (unsigned int) (item->kind.circ.r * 2),
      (unsigned int) (item->kind.circ.r * 2),
      0 * 64, 360 * 64);
    break;
  case SciPlotText:
    XDrawString(XtDisplay(w), XtWindow(w), gc,
      (int) (item->kind.text.x), (int) (item->kind.text.y),
      item->kind.text.text,
      (int) item->kind.text.length);
    break;
  case SciPlotVText:
    XDrawVString(XtDisplay(w), XtWindow(w), gc,
      (int) (item->kind.text.x), (int) (item->kind.text.y),
      item->kind.text.text,
      (int) item->kind.text.length,
      FontFromFontnum(w, item->kind.text.font));
    break;
  case SciPlotClipRegion:
    rect.x = (short) item->kind.line.x1;
    rect.y = (short) item->kind.line.y1;
    rect.width = (short) item->kind.line.x2;
    rect.height = (short) item->kind.line.y2;
    XSetClipRectangles(XtDisplay(w), w->plot.dashGC, 0, 0, &rect, 1, Unsorted);
    XSetClipRectangles(XtDisplay(w), w->plot.defaultGC, 0, 0, &rect, 1,
                       Unsorted);
    break;
  case SciPlotClipClear:
    XSetClipMask(XtDisplay(w), w->plot.dashGC, None);
    XSetClipMask(XtDisplay(w), w->plot.defaultGC, None);
    break;
  default:
    break;
  }
}

static void 
ItemDrawAll (SciPlotWidget w)
{
  SciPlotItem *item;
  int i;

  if (!XtIsRealized((Widget) w))
    return;
  item = w->plot.drawlist;
  i = 0;
  while (i < w->plot.num_drawlist) {
    ItemDraw(w, item);
    i++;
    item++;
  }
}



/*
 * PostScript (r) functions ------------------------------------------------
 *
 */
typedef struct {
  char *command;
  char *prolog;
} PScommands;

static PScommands psc[] =
{
  {"ma", "moveto"},
  {"da", "lineto stroke newpath"},
  {"la", "lineto"},
  {"poly", "closepath stroke newpath"},
  {"fpoly", "closepath fill newpath"},
  {"box", "1 index 0 rlineto 0 exch rlineto neg 0 rlineto closepath stroke newpath"},
  {"fbox", "1 index 0 rlineto 0 exch rlineto neg 0 rlineto closepath fill newpath"},
  {"clipbox", "gsave 1 index 0 rlineto 0 exch rlineto neg 0 rlineto closepath clip newpath"},
  {"unclip", "grestore newpath"},
  {"cr", "0 360 arc stroke newpath"},
  {"fcr", "0 360 arc fill newpath"},
  {"vma", "gsave moveto 90 rotate"},
  {"norm", "grestore"},
  {"solid", "[] 0 setdash"},
  {"dot", "[.25 2] 0 setdash"},
  {"widedot", "[.25 8] 0 setdash"},
  {"rgb", "setrgbcolor"},
  {NULL, NULL}
};

enum PSenums {
  PSmoveto, PSlineto,
  PSpolyline, PSendpoly, PSendfill,
  PSbox, PSfbox,
  PSclipbox, PSunclip,
  PScircle, PSfcircle,
  PSvmoveto, PSnormal,
  PSsolid, PSdot, PSwidedot,
  PSrgb
};

static void 
ItemPSDrawAll (SciPlotWidget w, FILE *fd, double yflip, Boolean usecolor)
{
  int i, loopcount;
  SciPlotItem *item;
  XcmsColor currentcolor;
  int previousfont, previousline, currentfont, currentline, previouscolor;

  item = w->plot.drawlist;
  loopcount = 0;
  previousfont = 0;
  previouscolor = -1;
  previousline = XtLINE_SOLID;
  while (loopcount < w->plot.num_drawlist) {

/* 2 switch blocks:  1st sets up defaults, 2nd actually draws things. */
    currentline = previousline;
    currentfont = previousfont;
    switch (item->type) {
    case SciPlotLine:
    case SciPlotCircle:
      currentline = item->kind.any.style;
      break;
    default:
      break;
    }
    if (currentline != XtLINE_NONE) {
      if (currentline != previousline) {
	switch (item->kind.any.style) {
	case XtLINE_SOLID:
	  fprintf(fd, "%s ", psc[PSsolid].command);
	  break;
	case XtLINE_DOTTED:
	  fprintf(fd, "%s ", psc[PSdot].command);
	  break;
	case XtLINE_WIDEDOT:
	  fprintf(fd, "%s ", psc[PSwidedot].command);
	  break;
	}
	previousline = currentline;
      }

      if (usecolor && item->kind.any.color != previouscolor) {
        
          /* Get Pixel index */
        currentcolor.pixel = w->plot.colors[item->kind.any.color];
          /* Get RGBi components [0.0,1.0] */
        XcmsQueryColor( XtDisplay(w), w->plot.cmap, &currentcolor,
          XcmsRGBiFormat );
          /* output PostScript command */
        fprintf(fd, "%f %f %f %s ", currentcolor.spec.RGBi.red,
          currentcolor.spec.RGBi.green, currentcolor.spec.RGBi.blue,
          psc[PSrgb].command);

        previouscolor=item->kind.any.color;
        
      }

      switch (item->type) {
      case SciPlotLine:
	fprintf(fd, "%.2f %.2f %s %.2f %.2f %s\n",
	  item->kind.line.x1, yflip - item->kind.line.y1,
	  psc[PSmoveto].command,
	  item->kind.line.x2, yflip - item->kind.line.y2,
	  psc[PSlineto].command);
	break;
      case SciPlotRect:
	fprintf(fd, "%.2f %.2f %s %.2f %.2f %s\n",
	  item->kind.rect.x,
	  yflip - item->kind.rect.y - (item->kind.rect.h - 1.0),
	  psc[PSmoveto].command,
	  item->kind.rect.w - 1.0, item->kind.rect.h - 1.0,
	  psc[PSbox].command);
	break;
      case SciPlotFRect:
	fprintf(fd, "%.2f %.2f %s %.2f %.2f %s\n",
	  item->kind.rect.x,
	  yflip - item->kind.rect.y - (item->kind.rect.h - 1.0),
	  psc[PSmoveto].command,
	  item->kind.rect.w - 1.0, item->kind.rect.h - 1.0,
	  psc[PSfbox].command);
	break;
      case SciPlotPoly:
	fprintf(fd, "%.2f %.2f %s ",
	  item->kind.poly.x[0], yflip - item->kind.poly.y[0],
	  psc[PSmoveto].command);
	for (i = 1; i < item->kind.poly.count; i++) {
	  fprintf(fd, "%.2f %.2f %s ",
	    item->kind.poly.x[i],
	    yflip - item->kind.poly.y[i],
	    psc[PSpolyline].command);
	}
	fprintf(fd, "%s\n", psc[PSendpoly].command);
	break;
      case SciPlotFPoly:
	fprintf(fd, "%.2f %.2f %s ",
	  item->kind.poly.x[0], yflip - item->kind.poly.y[0],
	  psc[PSmoveto].command);
	for (i = 1; i < item->kind.poly.count; i++) {
	  fprintf(fd, "%.2f %.2f %s ",
	    item->kind.poly.x[i],
	    yflip - item->kind.poly.y[i],
	    psc[PSpolyline].command);
	}
	fprintf(fd, "%s\n", psc[PSendfill].command);
	break;
      case SciPlotCircle:
	fprintf(fd, "%.2f %.2f %.2f %s\n",
	  item->kind.circ.x, yflip - item->kind.circ.y,
	  item->kind.circ.r,
	  psc[PScircle].command);
	break;
      case SciPlotFCircle:
	fprintf(fd, "%.2f %.2f %.2f %s\n",
	  item->kind.circ.x, yflip - item->kind.circ.y,
	  item->kind.circ.r,
	  psc[PSfcircle].command);
	break;
      case SciPlotText:
	fprintf(fd, "font-%d %.2f %.2f %s (%s) show\n",
	  item->kind.text.font,
	  item->kind.text.x, yflip - item->kind.text.y,
	  psc[PSmoveto].command,
	  item->kind.text.text);
	break;
      case SciPlotVText:
	fprintf(fd, "font-%d %.2f %.2f %s (%s) show %s\n",
	  item->kind.text.font,
	  item->kind.text.x, yflip - item->kind.text.y,
	  psc[PSvmoveto].command,
	  item->kind.text.text,
	  psc[PSnormal].command);
	break;
      case SciPlotClipRegion:
	fprintf(fd, "%.2f %.2f %s %.2f %.2f %s\n",
	  item->kind.line.x1,
	  yflip - item->kind.line.y1 - item->kind.line.y2,
	  psc[PSmoveto].command,
	  item->kind.line.x2, item->kind.line.y2,
	  psc[PSclipbox].command);
	break;
      case SciPlotClipClear:
	fprintf(fd, "%s\n", psc[PSunclip].command);
	break;
      default:
	break;
      }
    }
    loopcount++;
    item++;
  }
}

Boolean 
SciPlotPSCreateFancy (SciPlotWidget w, char *filename, int drawborder,
                      char *titles, Boolean usecolor)
{
  FILE *fd;
  float scale, xoff, yoff, xmax, ymax, yflip, aspect, border, titlefontsize;
  int i;
  PScommands *p;
  char fontname[128];

  if (!(fd = fopen(filename, "w"))) {
    XtWarning("SciPlotPSCreate: Unable to open postscript file.");
    return False;
  }
  DrawAll(w);

  aspect = (float) w->core.width / (float) w->core.height;
  border = 36.0;
  if (aspect > (612.0 / 792.0)) {
    scale = (612.0 - (2 * border)) / (float) w->core.width;
    xoff = border;
    yoff = (792.0 - (2 * border) - scale * (float) w->core.height) / 2.0;
    xmax = xoff + scale * (float) w->core.width;
    ymax = yoff + scale * (float) w->core.height;
  }
  else {
    scale = (792.0 - (2 * border)) / (float) w->core.height;
    yoff = border;
    xoff = (612.0 - (2 * border) - scale * (float) w->core.width) / 2.0;
    xmax = xoff + scale * (float) w->core.width;
    ymax = yoff + scale * (float) w->core.height;
  }
  yflip = w->core.height;
  fprintf(fd, "%s\n%s %.2f  %s\n%s %f %f %f %f\n%s\n",
    "%!PS-ADOBE-3.0 EPSF-3.0",
    "%%Creator: SciPlot Widget",
    _SCIPLOT_WIDGET_VERSION,
    "Copyright (c) 1995 Robert W. McMullen",
    "%%BoundingBox:", xoff, yoff, xmax, ymax,
    "%%EndComments");

  p = psc;
  while (p->command) {
    fprintf(fd, "/%s {%s} bind def\n", p->command, p->prolog);
    p++;
  }

  for (i = 0; i < w->plot.num_fonts; i++) {
    FontnumPostScriptString(w, i, fontname);
    fprintf(fd, "/font-%d {%s setfont} bind def\n",
      i, fontname);
  }
  titlefontsize = 10.0;
  fprintf(fd, "/font-title {/%s findfont %f scalefont setfont} bind def\n",
    "Times-Roman", titlefontsize);
  fprintf(fd, "%f setlinewidth\n", 0.001);
  fprintf(fd, "newpath gsave\n%f %f translate %f %f scale\n",
    xoff, yoff, scale, scale);

  ItemPSDrawAll(w, fd, yflip, usecolor);

  fprintf(fd, "grestore\n");

  if (drawborder) {
    fprintf(fd, "%.2f %.2f %s %.2f %.2f %s\n",
      border, border,
      psc[PSmoveto].command,
      612.0 - 2.0 * border, 792.0 - 2.0 * border,
      psc[PSbox].command);
  }
  if (titles) {
    char *ptr;
    char buf[256];
    int len, i, j;
    float x, y;

    x = border + titlefontsize;
    y = 792.0 - border - (2.0 * titlefontsize);
    len = strlen(titles);
    ptr = titles;
    i = 0;
    while (i < len) {
      j = 0;
      while ((*ptr != '\n') && (i < len)) {
	if ((*ptr == '(') || (*ptr == ')'))
	  buf[j++] = '\\';
	buf[j++] = *ptr;
	ptr++;
	i++;
      }
      buf[j] = '\0';
      ptr++;
      i++;
      fprintf(fd, "font-title %.2f %.2f %s (%s) show\n",
	x, y, psc[PSmoveto].command, buf);
      y -= titlefontsize * 1.5;
    }
    if (border) {
      y += titlefontsize * 0.5;
      fprintf(fd, "%.2f %.2f %s %.2f %.2f %s\n",
	border, y,
	psc[PSmoveto].command,
	612.0 - border, y,
	psc[PSlineto].command);
    }
  }

  fprintf(fd, "showpage\n");
  fclose(fd);
  return True;
}

Boolean 
SciPlotPSCreate (Widget wi, char *filename)
{
  SciPlotWidget w;

  if (!XtIsSciPlot(wi)) {
    XtWarning("SciPlotPSCreate: Not a SciPlot widget.");
    return False;
  }

  w = (SciPlotWidget) wi;
  return SciPlotPSCreateFancy(w, filename, False, NULL, False);
}

Boolean 
SciPlotPSCreateColor (Widget wi, char *filename)
{
  SciPlotWidget w;

  if (!XtIsSciPlot(wi)) {
    XtWarning("SciPlotPSCreate: Not a SciPlot widget.");
    return False;
  }

  w = (SciPlotWidget) wi;
  return SciPlotPSCreateFancy(w, filename, False, NULL, True);
}


/*
 * Private device independent drawing functions
 */

static void 
EraseClassItems (SciPlotWidget w, SciPlotDrawingEnum drawing)
{
  SciPlotItem *item;
  int i;

  if (!XtIsRealized((Widget) w))
    return;
  item = w->plot.drawlist;
  i = 0;
  while (i < w->plot.num_drawlist) {
    if (item->drawing_class == drawing) {
      item->kind.any.color = 0;
      item->kind.any.style = XtLINE_SOLID;
      ItemDraw(w, item);
    }
    i++;
    item++;
  }
}

static void 
EraseAllItems (SciPlotWidget w)
{
  SciPlotItem *item;
  int i;

  item = w->plot.drawlist;
  i = 0;
  while (i < w->plot.num_drawlist) {
    if ((item->type > SciPlotStartTextTypes) &&
      (item->type < SciPlotEndTextTypes))
      XtFree(item->kind.text.text);
    i++;
    item++;
  }
  w->plot.num_drawlist = 0;
}

static void 
EraseAll (SciPlotWidget w)
{
  EraseAllItems(w);
  if (XtIsRealized((Widget) w))
    XClearWindow(XtDisplay(w), XtWindow(w));
}

static SciPlotItem *
ItemGetNew (SciPlotWidget w)
{
  SciPlotItem *item;

  w->plot.num_drawlist++;
  if (w->plot.num_drawlist >= w->plot.alloc_drawlist) {
    w->plot.alloc_drawlist += NUMPLOTITEMEXTRA;
    w->plot.drawlist = (SciPlotItem *) XtRealloc((char *) w->plot.drawlist,
      w->plot.alloc_drawlist * sizeof(SciPlotItem));
    if (!w->plot.drawlist) {
      printf("Can't realloc memory for SciPlotItem list\n");
      exit(1);
    }
#ifdef DEBUG_SCIPLOT
    printf("Alloced #%d for drawlist\n", w->plot.alloc_drawlist);
#endif
  }
  item = w->plot.drawlist + (w->plot.num_drawlist - 1);
  item->type = SciPlotFALSE;
  item->drawing_class = w->plot.current_id;
  return item;
}


static void
LineSet(SciPlotWidget w, real x1, real y1, real x2, real y2,
  int color, int style)
{
  SciPlotItem *item;

  item = ItemGetNew(w);
  item->kind.any.color = (short) color;
  item->kind.any.style = (short) style;
  item->kind.line.x1 = (real)x1;
  item->kind.line.y1 = (real)y1;
  item->kind.line.x2 = (real)x2;
  item->kind.line.y2 = (real)y2;
  item->type = SciPlotLine;
  ItemDraw(w, item);
}

static void
RectSet(SciPlotWidget w, real x1, real y1, real x2, real y2,
  int color, int style)
{
  SciPlotItem *item;
  real x, y, width, height;

  if (x1 < x2)
    x = x1, width = (x2 - x1 + 1);
  else
    x = x2, width = (x1 - x2 + 1);
  if (y1 < y2)
    y = y1, height = (y2 - y1 + 1);
  else
    y = y2, height = (y1 - y2 + 1);

  item = ItemGetNew(w);
  item->kind.any.color = (short) color;
  item->kind.any.style = (short) style;
  item->kind.rect.x = (real)x;
  item->kind.rect.y = (real)y;
  item->kind.rect.w = (real)width;
  item->kind.rect.h = (real)height;
  item->type = SciPlotRect;
  ItemDraw(w, item);
}

static void 
FilledRectSet (SciPlotWidget w, real x1, real y1, real x2, real y2, int color,
               int style)
{
  SciPlotItem *item;
  real x, y, width, height;

  if (x1 < x2)
    x = x1, width = (x2 - x1 + 1);
  else
    x = x2, width = (x1 - x2 + 1);
  if (y1 < y2)
    y = y1, height = (y2 - y1 + 1);
  else
    y = y2, height = (y1 - y2 + 1);

  item = ItemGetNew(w);
  item->kind.any.color = (short) color;
  item->kind.any.style = (short) style;
  item->kind.rect.x = (real)x;
  item->kind.rect.y = (real)y;
  item->kind.rect.w = (real)width;
  item->kind.rect.h = (real)height;
  item->type = SciPlotFRect;
  ItemDraw(w, item);
}

static void 
TriSet (SciPlotWidget w, real x1, real y1, real x2, real y2, real x3, real y3,
        int color, int style)
{
  SciPlotItem *item;

  item = ItemGetNew(w);
  item->kind.any.color = (short) color;
  item->kind.any.style = (short) style;
  item->kind.poly.count = 3;
  item->kind.poly.x[0] = (real)x1;
  item->kind.poly.y[0] = (real)y1;
  item->kind.poly.x[1] = (real)x2;
  item->kind.poly.y[1] = (real)y2;
  item->kind.poly.x[2] = (real)x3;
  item->kind.poly.y[2] = (real)y3;
  item->type = SciPlotPoly;
  ItemDraw(w, item);
}

static void 
FilledTriSet (SciPlotWidget w, real x1, real y1, real x2, real y2, real x3,
              real y3, int color, int style)
{
  SciPlotItem *item;

  item = ItemGetNew(w);
  item->kind.any.color = (short) color;
  item->kind.any.style = (short) style;
  item->kind.poly.count = 3;
  item->kind.poly.x[0] = (real)x1;
  item->kind.poly.y[0] = (real)y1;
  item->kind.poly.x[1] = (real)x2;
  item->kind.poly.y[1] = (real)y2;
  item->kind.poly.x[2] = (real)x3;
  item->kind.poly.y[2] = (real)y3;
  item->type = SciPlotFPoly;
  ItemDraw(w, item);
}

static void 
QuadSet (SciPlotWidget w, real x1, real y1, real x2, real y2, real x3, real y3,
         real x4, real y4, int color, int style)
{
  SciPlotItem *item;

  item = ItemGetNew(w);
  item->kind.any.color = (short) color;
  item->kind.any.style = (short) style;
  item->kind.poly.count = 4;
  item->kind.poly.x[0] = (real)x1;
  item->kind.poly.y[0] = (real)y1;
  item->kind.poly.x[1] = (real)x2;
  item->kind.poly.y[1] = (real)y2;
  item->kind.poly.x[2] = (real)x3;
  item->kind.poly.y[2] = (real)y3;
  item->kind.poly.x[3] = (real)x4;
  item->kind.poly.y[3] = (real)y4;
  item->type = SciPlotPoly;
  ItemDraw(w, item);
}

static void 
FilledQuadSet (SciPlotWidget w, real x1, real y1, real x2, real y2, real x3,
               real y3, real x4, real y4, int color, int style)
{
  SciPlotItem *item;

  item = ItemGetNew(w);
  item->kind.any.color = (short) color;
  item->kind.any.style = (short) style;
  item->kind.poly.count = 4;
  item->kind.poly.x[0] = (real)x1;
  item->kind.poly.y[0] = (real)y1;
  item->kind.poly.x[1] = (real)x2;
  item->kind.poly.y[1] = (real)y2;
  item->kind.poly.x[2] = (real)x3;
  item->kind.poly.y[2] = (real)y3;
  item->kind.poly.x[3] = (real)x4;
  item->kind.poly.y[3] = (real)y4;
  item->type = SciPlotFPoly;
  ItemDraw(w, item);
}

static void 
CircleSet (SciPlotWidget w, real x, real y, real r, int color, int style)
{
  SciPlotItem *item;

  item = ItemGetNew(w);
  item->kind.any.color = (short) color;
  item->kind.any.style = (short) style;
  item->kind.circ.x = (real)x;
  item->kind.circ.y = (real)y;
  item->kind.circ.r = (real)r;
  item->type = SciPlotCircle;
  ItemDraw(w, item);
}

static void 
FilledCircleSet (SciPlotWidget w, real x, real y, real r, int color, int style)
{
  SciPlotItem *item;

  item = ItemGetNew(w);
  item->kind.any.color = (short) color;
  item->kind.any.style = (short) style;
  item->kind.circ.x = (real)x;
  item->kind.circ.y = (real)y;
  item->kind.circ.r = (real)r;
  item->type = SciPlotFCircle;
  ItemDraw(w, item);
}

static void 
TextSet (SciPlotWidget w, real x, real y, char *text, int color, int font)
{
  SciPlotItem *item;

  item = ItemGetNew(w);
  item->kind.any.color = (short) color;
  item->kind.any.style = 0;
  item->kind.text.x = (real)x;
  item->kind.text.y = (real)y;
  item->kind.text.length = strlen(text);
  item->kind.text.text = XtMalloc((int) item->kind.text.length + 1);
  item->kind.text.font = font;
  strcpy(item->kind.text.text, text);
  item->type = SciPlotText;
  ItemDraw(w, item);
#ifdef DEBUG_SCIPLOT_TEXT
  if (1) {
    real x1, y1;

    y -= FontnumAscent(w, font);
    y1 = y + FontnumHeight(w, font) - 1.0;
    x1 = x + FontnumTextWidth(w, font, text) - 1.0;
    RectSet(w, x, y, x1, y1, color, XtLINE_SOLID);
  }
#endif
}

static void 
TextCenter (SciPlotWidget w, real x, real y, char *text, int color, int font)
{
  x -= FontnumTextWidth(w, font, text) / 2.0;
  y += FontnumHeight(w, font) / 2.0 - FontnumDescent(w, font);
  TextSet(w, x, y, text, color, font);
}

static void 
VTextSet (SciPlotWidget w, real x, real y, char *text, int color, int font)
{
  SciPlotItem *item;

  item = ItemGetNew(w);
  item->kind.any.color = (short) color;
  item->kind.any.style = 0;
  item->kind.text.x = (real)x;
  item->kind.text.y = (real)y;
  item->kind.text.length = strlen(text);
  item->kind.text.text = XtMalloc((int) item->kind.text.length + 1);
  item->kind.text.font = font;
  strcpy(item->kind.text.text, text);
  item->type = SciPlotVText;
  ItemDraw(w, item);
#ifdef DEBUG_SCIPLOT_TEXT
  if (1) {
    real x1, y1;

    x += FontnumDescent(w, font);
    x1 = x - FontnumHeight(w, font) - 1.0;
    y1 = y - FontnumTextWidth(w, font, text) - 1.0;
    RectSet(w, x, y, x1, y1, color, XtLINE_SOLID);
  }
#endif
}

static void 
VTextCenter (SciPlotWidget w, real x, real y, char *text, int color, int font)
{
  x += FontnumHeight(w, font) / 2.0 - FontnumDescent(w, font);
  y += FontnumTextWidth(w, font, text) / 2.0;
  VTextSet(w, x, y, text, color, font);
}

static void 
ClipSet (SciPlotWidget w)
{
  SciPlotItem *item;

  if (w->plot.ChartType == XtCARTESIAN) {
    item = ItemGetNew(w);
    item->kind.any.style = XtLINE_SOLID;
    item->kind.any.color = 1;
    item->kind.line.x1 = w->plot.x.Origin;
    item->kind.line.x2 = w->plot.x.Size;
    item->kind.line.y1 = w->plot.y.Origin;
    item->kind.line.y2 = w->plot.y.Size;
#ifdef DEBUG_SCIPLOT
    printf("clipping region: x=%f y=%f w=%f h=%f\n",
      item->kind.line.x1,
      item->kind.line.y1,
      item->kind.line.x2,
      item->kind.line.y2
      );
#endif
    item->type = SciPlotClipRegion;
    ItemDraw(w, item);
  }
}

static void 
ClipClear (SciPlotWidget w)
{
  SciPlotItem *item;

  if (w->plot.ChartType == XtCARTESIAN) {
    item = ItemGetNew(w);
    item->kind.any.style = XtLINE_SOLID;
    item->kind.any.color = 1;
    item->type = SciPlotClipClear;
    ItemDraw(w, item);
  }
}


/*
 * Private data point to screen location converters
 */

static real 
PlotX (SciPlotWidget w, real xin)
{
  real xout;

  if (w->plot.XLog)
    xout = w->plot.x.Origin + (log10(xin) - log10(w->plot.x.DrawOrigin))
         * (w->plot.x.Size / w->plot.x.DrawSize);
  else
    xout = w->plot.x.Origin + (xin - w->plot.x.DrawOrigin)
         * (w->plot.x.Size / w->plot.x.DrawSize);
  return xout;
}

static real 
PlotY (SciPlotWidget w, real yin)
{
  real yout;

  if (w->plot.YLog)
    yout = w->plot.y.Origin + w->plot.y.Size -
      (log10(yin) - log10(w->plot.y.DrawOrigin)) *
      (w->plot.y.Size / w->plot.y.DrawSize);
  else
    yout = w->plot.y.Origin + w->plot.y.Size -
      (yin - w->plot.y.DrawOrigin) * (w->plot.y.Size / w->plot.y.DrawSize);
  return yout;
}

static void 
PlotRTRadians (SciPlotWidget w, real r, real t, real *xout, real *yout)
{
  *xout = w->plot.x.Center + (r * (real)cos(t) /
    w->plot.PolarScale * w->plot.x.Size / 2.0);
  *yout = w->plot.y.Center + (-r * (real)sin(t) /
    w->plot.PolarScale * w->plot.x.Size / 2.0);
}

static void 
PlotRTDegrees (SciPlotWidget w, real r, real t, real *xout, real *yout)
{
  t *= DEG2RAD;
  PlotRTRadians(w, r, t, xout, yout);
}

static void 
PlotRT (SciPlotWidget w, real r, real t, real *xout, real *yout)
{
  if (w->plot.Degrees)
    t *= DEG2RAD;
  PlotRTRadians(w, r, t, xout, yout);
}


/*
 * Private calculation utilities for axes
 */

/* More spacing needed for horizontal x axis labels than 
   horizontal y axis labels */
/* #define MAX_MAJOR 8 */
#define MAX_MAJOR_X  6
#define MAX_MAJOR_Y  8
#define NUMBER_MINOR 8 
static real CAdeltas[8] =
{0.1, 0.2, 0.25, 0.5, 1.0, 2.0, 2.5, 5.0};
static int CAdecimals[8] =
{0, 0, 1, 0, 0, 0, 1, 0};
static int CAminors[8] =
{4, 4, 4, 5, 4, 4, 4, 5};

static void 
ComputeAxis (SciPlotAxis *axis, real min, real max, Boolean log, int max_major)
{
  real range, rel_range, rnorm, min_mag, max_mag, mag, delta, delta_order,
       calcmin, calcmax;
  int i, nexp, minornum, majornum, majordecimals, prec;

  /* MSE: reworked to fix problems observed when an actual range is
     not (yet) defined by the data set. */ 
  min_mag   = fabs(min);
  max_mag   = fabs(max);
  mag       = (max_mag > min_mag) ? max_mag : min_mag;
  range     = max - min;
  rel_range = (mag > 0.) ? range/mag : range;

  if (log) {
    if (rel_range < DBL_EPSILON) {
      calcmin = (min > 0.) ? powi(10.0, (int)floor(log10(min))) : 1;
      calcmax = 10.0*calcmin;
    }
    else {
      calcmin = powi(10.0, (int)floor(log10(min)));
      calcmax = powi(10.0, (int)ceil(log10(max)));
    }
    delta = 10.0;
#ifdef DEBUG_SCIPLOT
    printf("calcmin=%e min=%e calcmax=%e max=%e\n", calcmin, min, calcmax, max);
    printf("log calcmin=%e (int)log calcmin=%d\n", log10(calcmin),
	   (int)(log10(calcmin) * 1.0001));
#endif

    axis->DrawSize = log10(calcmax) - log10(calcmin);
    axis->MajorNum = (int)log10(calcmax / calcmin * 1.0001);
    axis->MinorNum = 10;

    prec = -(int)(log10(calcmin) * 1.0001);
  }
  else {
    if (rel_range < DBL_EPSILON) {
      /* For min == max == 0, use a +/-.5 range (common when no data yet).
         For nonzero min == max, use a range of +/-4% of max magnitude. */
      real half_range = (mag > 0.) ? mag*.02 : .5;
      range = 2.*half_range;
      min -= half_range;
      max += half_range;
    }
    nexp  = (int)floor(log10(range));
    rnorm = range / powi(10.0, nexp);
    for (i=0; i<NUMBER_MINOR; i++) {
      delta    = CAdeltas[i];
      majornum = (int)ceil(rnorm/delta);
      if (majornum <= max_major) {
        majordecimals = CAdecimals[i];
        minornum      = CAminors[i];
	break;
      }
    }
    delta *= pow(10.0, nexp);
#ifdef DEBUG_SCIPLOT
    printf("nexp=%d range=%f rnorm=%f delta=%f\n", nexp, range, rnorm, delta);
#endif

    /* MSE: simplified formulas. */
    calcmax =  ceil(max/delta) * delta;
    calcmin = floor(min/delta) * delta;

    /* NOTE: round(x) =~ floor(x+.5) =~ ceil(x-.5) where floor()/ceil() are
       portable and round()/rint()/trunc()/nearbyint() are not. */
    axis->DrawSize = calcmax - calcmin;
    axis->MajorNum = (int)floor((calcmax - calcmin)/delta + .5);
    axis->MinorNum = minornum;

    /* This code assumes %f floating point output since the magnitude of the
       values (delta) masks majordecimals (taken from the CAdecimals
       incrementing strategies) in calculating the output precision.  A better
       approach would be to calculate #decimal difference between increment and
       max value (to properly capture precision required when Inc << Total, e.g.
       a range of 1000000 to 1000005) plus majordecimals (additional precision
       required for specific increment patterns, e.g. 1000001.25, 1000002.5,
       1000003.75, 1000005). */
    delta_order = log10(delta);
    prec = (delta_order > 0.0) ? -(int)floor(delta_order) + majordecimals
	                       :  (int)ceil(-delta_order) + majordecimals;
  }
  axis->DrawOrigin = calcmin;
  axis->DrawMax    = calcmax;
  axis->MajorInc   = delta;
  axis->Precision  = (prec > 0) ? prec : 0;

#ifdef DEBUG_SCIPLOT
  printf("min=%f max=%f size=%f major inc=%f #major=%d #minor=%d prec=%d\n",
	 axis->DrawOrigin, axis->DrawMax,  axis->DrawSize, axis->MajorInc,
	 axis->MajorNum,   axis->MinorNum, axis->Precision);
#endif
}

static void 
ComputeDrawingRange (SciPlotWidget w)
{
  if (w->plot.ChartType == XtCARTESIAN) {
    ComputeAxis(&w->plot.x, w->plot.Min.x, w->plot.Max.x, w->plot.XLog,
                MAX_MAJOR_X);
    ComputeAxis(&w->plot.y, w->plot.Min.y, w->plot.Max.y, w->plot.YLog,
                MAX_MAJOR_Y);
  }
  else {
    ComputeAxis(&w->plot.x, (real)0.0, w->plot.Max.x, (Boolean)FALSE,
                MAX_MAJOR_Y);
    w->plot.PolarScale = w->plot.x.DrawMax;
  }
}

static Boolean 
CheckMinMax (SciPlotWidget w)
{
  register int i, j;
  register SciPlotList *p;
  register real val;

  if (w->plot.ChartType == XtCARTESIAN) {
    for (i = 0; i < w->plot.num_plotlist; i++) {
      p = w->plot.plotlist + i;
      if (p->draw) {
	for (j = 0; j < p->number; j++) {
          
            /* Don't count the "break in line segment" flag for Min/Max */
          if (p->data[j].x > SCIPLOT_SKIP_VAL &&
              p->data[j].y > SCIPLOT_SKIP_VAL) {
            
            val = p->data[j].x;
            if (val > w->plot.x.DrawMax || val < w->plot.x.DrawOrigin)
              return True;
            val = p->data[j].y;
            if (val > w->plot.y.DrawMax || val < w->plot.y.DrawOrigin)
              return True;
          }
        }
      }
    }
  }
  else {
    for (i = 0; i < w->plot.num_plotlist; i++) {
      p = w->plot.plotlist + i;
      if (p->draw) {
	for (j = 0; j < p->number; j++) {
	  val = p->data[j].x;
	  if (val > w->plot.Max.x || val < w->plot.Min.x)
	    return True;
	}
      }
    }
  }
  return False;
}

static void 
ComputeMinMax (SciPlotWidget w)
{
  register int i, j;
  register SciPlotList *p;
  register real val;
  Boolean firstx = True, firsty = True;

  /* MSE: these defaults are for the case of no plot data (p->number == 0 for 
     all drawn lists), but they are not seen in current DAKOTA use since the
     y min/max marker lists always have 1 data point each (exception: markers
     with zero value are ignored if log scale is used). */
  w->plot.Max.x = w->plot.Max.y = 10.0;
  w->plot.Min.x = (w->plot.XLog) ? 1.0 :   0.0;
  w->plot.Min.y = (w->plot.YLog) ? 1.0 : -10.0;

  for (i=0; i<w->plot.num_plotlist; i++) {
    p = w->plot.plotlist + i;
    if (p->draw) { /* all drawn lists (xy plot data and y min/max markers) */

      for (j=0; j<p->number; j++) {
          
        /* Don't count the "break in line segment" flag for Min/Max */
        if (p->data[j].x > SCIPLOT_SKIP_VAL &&
            p->data[j].y > SCIPLOT_SKIP_VAL) {

          val = p->data[j].x;
          if (!w->plot.XLog || (w->plot.XLog && (val > 0.0))) {
            if (firstx) {
              w->plot.Min.x = w->plot.Max.x = val;
              firstx = False;
            }
            else {
              if (val > w->plot.Max.x)
                w->plot.Max.x = val;
              else if (val < w->plot.Min.x)
                w->plot.Min.x = val;
            }
          }

          val = p->data[j].y;
          if (!w->plot.YLog || (w->plot.YLog && (val > 0.0))) {
            if (firsty) {
              w->plot.Min.y = w->plot.Max.y = val;
              firsty = False;
            }
            else {
              if (val > w->plot.Max.y)
                w->plot.Max.y = val;
              else if (val < w->plot.Min.y)
                w->plot.Min.y = val;
            }
          }
        }
      }
    }
  }

  if (w->plot.ChartType == XtCARTESIAN) {
    /* if (!w->plot.XLog) { MSE: allow range restrictions in log scale */
      if (!w->plot.XAutoScale) { /* user override of auto-scaling */
	w->plot.Min.x = w->plot.UserMin.x;
	w->plot.Max.x = w->plot.UserMax.x;
      }
      else if (!w->plot.XLog && w->plot.XOrigin) { /* MSE: add log check here */
	if (w->plot.Min.x > 0.0)
	  w->plot.Min.x = 0.0;
	if (w->plot.Max.x < 0.0)
	  w->plot.Max.x = 0.0;
      }
      /* MSE: don't corrupt the true min/max.  Corrections for zero range
              are applied in ComputeAxis().
      if (fabs(w->plot.Min.x - w->plot.Max.x) < 1.e-10) {
	w->plot.Min.x -= .5;
	w->plot.Max.x += .5;
      }
      */
    /* } */
    /* if (!w->plot.YLog) { MSE: allow range restrictions in log scale */
      if (!w->plot.YAutoScale) { /* user override of auto-scaling */
	w->plot.Min.y = w->plot.UserMin.y;
	w->plot.Max.y = w->plot.UserMax.y;
      }
      else if (!w->plot.YLog && w->plot.YOrigin) { /* MSE: add log check here */
	if (w->plot.Min.y > 0.0)
	  w->plot.Min.y = 0.0;
	if (w->plot.Max.y < 0.0)
	  w->plot.Max.y = 0.0;
      }
      /* MSE: don't corrupt the true min/max.  Corrections for zero range
              are applied in ComputeAxis().
      if (fabs(w->plot.Min.y - w->plot.Max.y) < 1.e-10) {
	w->plot.Min.y -= .5;
	w->plot.Max.y += .5;
      */
    /* } */
  }
  else { /* XtPOLAR */
    if (fabs(w->plot.Min.x) > fabs(w->plot.Max.x))
      w->plot.Max.x = fabs(w->plot.Min.x);
  }

#ifdef DEBUG_SCIPLOT
  printf("Min: (%f,%f)\tMax: (%f,%f)\n", w->plot.Min.x, w->plot.Min.y,
    w->plot.Max.x, w->plot.Max.y);
#endif
}

static void 
ComputeLegendDimensions (SciPlotWidget w)
{
  real current, xmax, ymax;
  int i;
  SciPlotList *p;

  if (w->plot.ShowLegend) {
    xmax = 0.0;
    ymax = 2.0 * (real)w->plot.LegendMargin;

    for (i = 0; i < w->plot.num_plotlist; i++) {
      p = w->plot.plotlist + i;
      if (p->draw) {
	current = (real)w->plot.Margin +
	  (real)w->plot.LegendMargin * 3.0 +
	  (real)w->plot.LegendLineSize +
	  FontnumTextWidth(w, w->plot.axisFont, p->legend);
	if (current > xmax)
	  xmax = current;
	ymax += FontnumHeight(w, w->plot.axisFont);
      }
    }

    w->plot.x.LegendSize = xmax;
    w->plot.x.LegendPos = (real)w->plot.Margin;
    w->plot.y.LegendSize = ymax;
    w->plot.y.LegendPos = 0.0;
  }
  else {
    w->plot.x.LegendSize =
      w->plot.x.LegendPos =
      w->plot.y.LegendSize =
      w->plot.y.LegendPos = 0.0;
  }
}

static void 
ComputeDimensions (SciPlotWidget w)
{
  real x, y, width, height, axisnumbersize, axisXlabelsize, axisYlabelsize;

/* x,y is the origin of the upper left corner of the drawing area inside
 * the widget.  Doesn't necessarily have to be (Margin,Margin) as it is now.
 */
  x = (real)w->plot.Margin;
  y = (real)w->plot.Margin;

/* width = (real)w->core.width - (real)w->plot.Margin - x -
 * **           legendwidth - AxisFontHeight
 */
  width = (real)w->core.width - (real)w->plot.Margin - x - w->plot.x.LegendSize;

/* height = (real)w->core.height - (real)w->plot.Margin - y
 *           - Height of axis numbers (including margin)
 *           - Height of axis label (including margin)
 *           - Height of Title (including margin)
 */
  height = (real)w->core.height - (real)w->plot.Margin - y;

  w->plot.x.Origin = x;
  w->plot.y.Origin = y;

/* Adjust the size depending upon what sorts of text are visible. */
  if (w->plot.ShowTitle)
    height -= (real)w->plot.TitleMargin + FontnumHeight(w, w->plot.titleFont);

  if (w->plot.ChartType == XtCARTESIAN) {
    axisnumbersize = (real)w->plot.Margin + FontnumHeight(w, w->plot.axisFont);
    if (w->plot.XAxisNumbers) {
      height -= axisnumbersize;
    }
    if (w->plot.YAxisNumbers) {
      width -= axisnumbersize;
      w->plot.x.Origin += axisnumbersize;
    }
    
    if (w->plot.ShowXLabel) {
      axisXlabelsize = (real)w->plot.Margin +
	FontnumHeight(w, w->plot.labelFont);
      height -= axisXlabelsize;
    }
    if (w->plot.ShowYLabel) {
      axisYlabelsize = (real)w->plot.Margin +
	FontnumHeight(w, w->plot.labelFont);
      width -= axisYlabelsize;
      w->plot.x.Origin += axisYlabelsize;
    }
  }

  w->plot.x.Size = width;
  w->plot.y.Size = height;

/* Adjust parameters for polar plot */
  if (w->plot.ChartType == XtPOLAR) {
    if (height < width)
      w->plot.x.Size = height;
  }
  w->plot.x.Center = w->plot.x.Origin + (width / 2.0);
  w->plot.y.Center = w->plot.y.Origin + (height / 2.0);

}

static void 
AdjustDimensionsCartesian (SciPlotWidget w)
{
  real xextra, yextra, val, labelval, xhorz, label_width, max_label_width,
       y_major_incr, x, y, width, height, axisnumbersize, axisXlabelsize,
       axisYlabelsize;
  char label[16]; /* , numberformat[16]; */
  int i, num_y_major; /* , precision; */

/* Compute xextra and yextra, which are the extra distances that the text
 * labels on the axes stick outside of the graph.
 */
  xextra = yextra = 0.;
  if (w->plot.XAxisNumbers) {
    val = w->plot.x.DrawMax;
    /* precision = w->plot.x.Precision;
    if (w->plot.XLog) {
      precision -= w->plot.x.MajorNum;
      if (precision < 0)
        precision = 0;
    }
    sprintf(numberformat, "%%.%df", precision); */
    sprintf(label, "%.6g", val);
    xextra = ceil(PlotX(w, val) + FontnumTextWidth(w,w->plot.axisFont,label)/2.)
           + (real)w->plot.Margin - (real)w->core.width;
    if (xextra < 0.)
      xextra = 0.;
  }
  
  /* MSE, 6/03: this section completely reworked to properly estimate y axis 
     label widths when using the new %.6g formats. These new formats avoid
     fatal problems that were occurring when y axis numbers became too large
     to print with the previous %.#f formats (xhorz left a too small/negative 
     plot region). */
  yextra = xhorz = max_label_width = 0.0;
  if (w->plot.YAxisNumbers) {
    /* sprintf(numberformat, "%%.%df", precision); */
    /* loop over each increment and save max label width */
    num_y_major  = w->plot.y.MajorNum;
    y_major_incr = w->plot.y.MajorInc;
    val = w->plot.y.DrawOrigin;
    for (i=0; i<=num_y_major; i++) { /* both ends */
      if (i)
        val = (w->plot.YLog) ? val * y_major_incr : val + y_major_incr;
      /* Make sure label is not trying to resolve more precision
         than that indicated by MajorInc (especially at y = 0.) */
      labelval = (w->plot.YLog) ? val : floor(val/y_major_incr+.5)*y_major_incr;
      if (fabs(labelval) < DBL_MIN) labelval = 0.; /* hack to remove "-0" */
      sprintf(label, "%.6g", labelval);
      label_width = FontnumTextWidth(w, w->plot.axisFont, label);
      /* printf("label = %s width = %.4g\n", label, label_width); */
      if (label_width > max_label_width)
        max_label_width = label_width;
    }
    /* printf("max label width = %.4g\n", max_label_width); */

    if (w->plot.YNumHorz) {
      yextra = FontnumHeight(w, w->plot.axisFont)/2.0;
      xhorz = max_label_width + (real)w->plot.Margin;
    }
    else {
      y = PlotY(w, val) - FontnumTextWidth(w, w->plot.axisFont, label);
      if ((int)y <= 0) {
        yextra = ceil(w->plot.Margin - y);
        if (yextra < 0.0)
          yextra = 0.0;
      }
    }
  }

/* x,y is the origin of the upper left corner of the drawing area inside
 * the widget.  Doesn't necessarily have to be (Margin,Margin) as it is now.
 */
  x = (real)w->plot.Margin + xhorz;
  y = (real)w->plot.Margin + yextra;

/* width = (real)w->core.width - (real)w->plot.Margin - x -
 *          legendwidth - AxisFontHeight
 */
  width = (real)w->core.width - (real)w->plot.Margin - x - xextra;


/* height = (real)w->core.height - (real)w->plot.Margin - y
 *           - Height of axis numbers (including margin)
 *           - Height of axis label (including margin)
 *           - Height of Title (including margin)
 */
  height = (real)w->core.height - (real)w->plot.Margin - y;

  w->plot.x.Origin = x;
  w->plot.y.Origin = y;

/* Adjust the size depending upon what sorts of text are visible. */
  if (w->plot.ShowTitle)
    height -= (real)w->plot.TitleMargin + FontnumHeight(w, w->plot.titleFont);

  axisXlabelsize = 0.0;
  axisYlabelsize = 0.0;
  axisnumbersize = (real)w->plot.Margin + FontnumHeight(w, w->plot.axisFont);
  if (w->plot.XAxisNumbers) {
    height -= axisnumbersize;
  }
  if (w->plot.YAxisNumbers && !w->plot.YNumHorz) {
    width -= axisnumbersize;
    w->plot.x.Origin += axisnumbersize;
  }
  
  if (w->plot.ShowXLabel) {
    axisXlabelsize = (real)w->plot.Margin + FontnumHeight(w, w->plot.labelFont);
    height -= axisXlabelsize;
  }
  if (w->plot.ShowYLabel) {
    axisYlabelsize = (real)w->plot.Margin + FontnumHeight(w, w->plot.labelFont);
    width -= axisYlabelsize;
    w->plot.x.Origin += axisYlabelsize;
  }

/* Move legend position to the right of the plot */
  if (w->plot.LegendThroughPlot) {
    w->plot.x.LegendPos += w->plot.x.Origin + width - w->plot.x.LegendSize;
    w->plot.y.LegendPos += w->plot.y.Origin;
  }
  else {
    width -= w->plot.x.LegendSize;
    w->plot.x.LegendPos += w->plot.x.Origin + width;
    w->plot.y.LegendPos += w->plot.y.Origin;
  }

  w->plot.x.Size = width;
  w->plot.y.Size = height;

  w->plot.y.AxisPos = w->plot.y.Origin + w->plot.y.Size + (real)w->plot.Margin +
    FontnumAscent(w, w->plot.axisFont);
  if (w->plot.YNumHorz) {
    w->plot.x.AxisPos = w->plot.x.Origin - (real)w->plot.Margin;
  }
  else {
    w->plot.x.AxisPos = w->plot.x.Origin - (real)w->plot.Margin -
      FontnumDescent(w, w->plot.axisFont);
  }
  
  w->plot.y.LabelPos = w->plot.y.Origin + w->plot.y.Size +
    (real)w->plot.Margin + (FontnumHeight(w, w->plot.labelFont) / 2.0);
  if (w->plot.XAxisNumbers)
    w->plot.y.LabelPos += axisnumbersize;
  if (w->plot.YAxisNumbers) {
    if (w->plot.YNumHorz) {
      w->plot.x.LabelPos = w->plot.x.Origin - xhorz - (real)w->plot.Margin -
        (FontnumHeight(w, w->plot.labelFont) / 2.0);
    }
    else {
      w->plot.x.LabelPos = w->plot.x.Origin - axisnumbersize -
        (real)w->plot.Margin - (FontnumHeight(w, w->plot.labelFont) / 2.0);
    }
  }
  else {
    w->plot.x.LabelPos = w->plot.x.Origin - (real)w->plot.Margin -
        (FontnumHeight(w, w->plot.labelFont) / 2.0);
  }
  
  w->plot.y.TitlePos = (real)w->core.height - (real)w->plot.Margin;
  w->plot.x.TitlePos = (real)w->plot.Margin;

#ifdef DEBUG_SCIPLOT
  printf("y.Origin:		%f\n", w->plot.y.Origin);
  printf("y.Size:			%f\n", w->plot.y.Size);
  printf("axisnumbersize:		%f\n", axisnumbersize);
  printf("y.axisLabelSize:	%f\n", axisYlabelsize);
  printf("y.TitleSize:		%f\n",
    (real)w->plot.TitleMargin + FontnumHeight(w, w->plot.titleFont));
  printf("y.Margin:		%f\n", (real)w->plot.Margin);
  printf("total-------------------%f\n", w->plot.y.Origin + w->plot.y.Size +
    axisnumbersize + axisYlabelsize + (real)w->plot.Margin +
    (real)w->plot.TitleMargin + FontnumHeight(w, w->plot.titleFont));
  printf("total should be---------%f\n", (real)w->core.height);
#endif
}

static void 
AdjustDimensionsPolar (SciPlotWidget w)
{
  real x, y, xextra, yextra, val;
  real width, height, size;
  char label[16]; /* , numberformat[16]; */

/* Compute xextra and yextra, which are the extra distances that the text
 * labels on the axes stick outside of the graph.
 */
  xextra = yextra = 0.;
  val = w->plot.PolarScale;
  PlotRTDegrees(w, val, 0.0, &x, &y);
  /* sprintf(numberformat, "%%.%df", w->plot.x.Precision); */
  sprintf(label, "%.6g", val);
  xextra = x + FontnumTextWidth(w, w->plot.axisFont, label)
         + (real)w->plot.Margin - (real)w->core.width;
  if (xextra < 0.)
    xextra = 0.;
  yextra = 0.;


/* x,y is the origin of the upper left corner of the drawing area inside
 * the widget.  Doesn't necessarily have to be (Margin,Margin) as it is now.
 */
  w->plot.x.Origin = (real)w->plot.Margin;
  w->plot.y.Origin = (real)w->plot.Margin;

/* width = (real)w->core.width - (real)w->plot.Margin - x -
 *          legendwidth - AxisFontHeight
 */
  width = (real)w->core.width - (real)w->plot.Margin - w->plot.x.Origin
        - xextra;

/* height = (real)w->core.height - (real)w->plot.Margin - y
 *           - Height of axis numbers (including margin)
 *           - Height of axis label (including margin)
 *           - Height of Title (including margin)
 */
  height = (real)w->core.height - (real)w->plot.Margin - w->plot.y.Origin
         - yextra;

/* Adjust the size depending upon what sorts of text are visible. */
  if (w->plot.ShowTitle)
    height -= (real)w->plot.TitleMargin + FontnumHeight(w, w->plot.titleFont);

/* Only need to carry one number for the size, (since it is a circle!) */
  if (height < width)
    size = height;
  else
    size = width;

/* Assign some preliminary values */
  w->plot.x.Center = w->plot.x.Origin + (width / 2.0);
  w->plot.y.Center = w->plot.y.Origin + (height / 2.0);
  w->plot.x.LegendPos += width - w->plot.x.LegendSize;
  w->plot.y.LegendPos += w->plot.y.Origin;

/*
 * Check and see if the legend can fit in the blank space in the upper right
 *
 * To fit, the legend must:
 *   1) be less than half the width/height of the plot
 *   2) hmmm.
 */
  if (!w->plot.LegendThroughPlot) {
    real radius = size / 2.0;
    real dist;

    x = w->plot.x.LegendPos - w->plot.x.Center;
    y = (w->plot.y.LegendPos + w->plot.y.LegendSize) - w->plot.y.Center;

    dist = sqrt(x * x + y * y);
/*       printf("rad=%f dist=%f: legend=(%f,%f) center=(%f,%f)\n", */
/*              radius,dist,w->plot.x.LegendPos,w->plot.y.LegendPos, */
/*              w->plot.x.Center,w->plot.y.Center); */

    /* It doesn't fit if this check is true.  Make the plot smaller */

    /* This is a first cut horrible algorithm.  My calculus is a bit
     * rusty tonight--can't seem to figure out how to maximize a circle
     * in a rectangle with a rectangular chunk out of it. */
    if (dist < radius) {
      width -= w->plot.x.LegendSize;
      height -= w->plot.y.LegendSize;

      /* readjust some parameters */
      w->plot.x.Center = w->plot.x.Origin + width / 2.0;
      w->plot.y.Center = w->plot.y.Origin + w->plot.y.LegendSize + height / 2.0;
      if (height < width)
	size = height;
      else
	size = width;
    }

  }


/* OK, customization is finished when we reach here. */
  w->plot.x.Size = w->plot.y.Size = size;

  w->plot.y.TitlePos = w->plot.y.Center + w->plot.y.Size / 2.0 +
    (real)w->plot.TitleMargin + FontnumAscent(w, w->plot.titleFont);
  w->plot.x.TitlePos = w->plot.x.Origin;
}

static void 
AdjustDimensions (SciPlotWidget w)
{
  if (w->plot.ChartType == XtCARTESIAN) {
    AdjustDimensionsCartesian(w);
  }
  else {
    AdjustDimensionsPolar(w);
  }
}

static void 
ComputeAllDimensions (SciPlotWidget w)
{
  ComputeLegendDimensions(w);
  ComputeDimensions(w);
  ComputeDrawingRange(w);
  AdjustDimensions(w);
}

static void 
ComputeAll (SciPlotWidget w)
{
  ComputeMinMax(w);
  ComputeAllDimensions(w);
}


/*
 * Private drawing routines
 */

static void 
DrawMarker (SciPlotWidget w, real xpaper, real ypaper, real size, int color,
            int style)
{
  real sizex, sizey;

  switch (style) {
  case XtMARKER_CIRCLE:
    CircleSet(w, xpaper, ypaper, size, color, XtLINE_SOLID);
    break;
  case XtMARKER_FCIRCLE:
    FilledCircleSet(w, xpaper, ypaper, size, color, XtLINE_SOLID);
    break;
  case XtMARKER_SQUARE:
    size -= .5;
    RectSet(w, xpaper - size, ypaper - size,
      xpaper + size, ypaper + size,
      color, XtLINE_SOLID);
    break;
  case XtMARKER_FSQUARE:
    size -= .5;
    FilledRectSet(w, xpaper - size, ypaper - size,
      xpaper + size, ypaper + size,
      color, XtLINE_SOLID);
    break;
  case XtMARKER_UTRIANGLE:
    sizex = size * .866;
    sizey = size / 2.0;
    TriSet(w, xpaper, ypaper - size,
      xpaper + sizex, ypaper + sizey,
      xpaper - sizex, ypaper + sizey,
      color, XtLINE_SOLID);
    break;
  case XtMARKER_FUTRIANGLE:
    sizex = size * .866;
    sizey = size / 2.0;
    FilledTriSet(w, xpaper, ypaper - size,
      xpaper + sizex, ypaper + sizey,
      xpaper - sizex, ypaper + sizey,
      color, XtLINE_SOLID);
    break;
  case XtMARKER_DTRIANGLE:
    sizex = size * .866;
    sizey = size / 2.0;
    TriSet(w, xpaper, ypaper + size,
      xpaper + sizex, ypaper - sizey,
      xpaper - sizex, ypaper - sizey,
      color, XtLINE_SOLID);
    break;
  case XtMARKER_FDTRIANGLE:
    sizex = size * .866;
    sizey = size / 2.0;
    FilledTriSet(w, xpaper, ypaper + size,
      xpaper + sizex, ypaper - sizey,
      xpaper - sizex, ypaper - sizey,
      color, XtLINE_SOLID);
    break;
  case XtMARKER_RTRIANGLE:
    sizey = size * .866;
    sizex = size / 2.0;
    TriSet(w, xpaper + size, ypaper,
      xpaper - sizex, ypaper + sizey,
      xpaper - sizex, ypaper - sizey,
      color, XtLINE_SOLID);
    break;
  case XtMARKER_FRTRIANGLE:
    sizey = size * .866;
    sizex = size / 2.0;
    FilledTriSet(w, xpaper + size, ypaper,
      xpaper - sizex, ypaper + sizey,
      xpaper - sizex, ypaper - sizey,
      color, XtLINE_SOLID);
    break;
  case XtMARKER_LTRIANGLE:
    sizey = size * .866;
    sizex = size / 2.0;
    TriSet(w, xpaper - size, ypaper,
      xpaper + sizex, ypaper + sizey,
      xpaper + sizex, ypaper - sizey,
      color, XtLINE_SOLID);
    break;
  case XtMARKER_FLTRIANGLE:
    sizey = size * .866;
    sizex = size / 2.0;
    FilledTriSet(w, xpaper - size, ypaper,
      xpaper + sizex, ypaper + sizey,
      xpaper + sizex, ypaper - sizey,
      color, XtLINE_SOLID);
    break;
  case XtMARKER_DIAMOND:
    QuadSet(w, xpaper, ypaper - size,
      xpaper + size, ypaper,
      xpaper, ypaper + size,
      xpaper - size, ypaper,
      color, XtLINE_SOLID);
    break;
  case XtMARKER_FDIAMOND:
    FilledQuadSet(w, xpaper, ypaper - size,
      xpaper + size, ypaper,
      xpaper, ypaper + size,
      xpaper - size, ypaper,
      color, XtLINE_SOLID);
    break;
  case XtMARKER_HOURGLASS:
    QuadSet(w, xpaper - size, ypaper - size,
      xpaper + size, ypaper - size,
      xpaper - size, ypaper + size,
      xpaper + size, ypaper + size,
      color, XtLINE_SOLID);
    break;
  case XtMARKER_FHOURGLASS:
    FilledQuadSet(w, xpaper - size, ypaper - size,
      xpaper + size, ypaper - size,
      xpaper - size, ypaper + size,
      xpaper + size, ypaper + size,
      color, XtLINE_SOLID);
    break;
  case XtMARKER_BOWTIE:
    QuadSet(w, xpaper - size, ypaper - size,
      xpaper - size, ypaper + size,
      xpaper + size, ypaper - size,
      xpaper + size, ypaper + size,
      color, XtLINE_SOLID);
    break;
  case XtMARKER_FBOWTIE:
    FilledQuadSet(w, xpaper - size, ypaper - size,
      xpaper - size, ypaper + size,
      xpaper + size, ypaper - size,
      xpaper + size, ypaper + size,
      color, XtLINE_SOLID);
    break;
  case XtMARKER_DOT:
    FilledCircleSet(w, xpaper, ypaper, 1.5, color, XtLINE_SOLID);
    break;

  default:
    break;
  }
}

static void 
DrawLegend (SciPlotWidget w)
{
  real x, y, len, height, height2, len2, ascent;
  int i;
  SciPlotList *p;

  w->plot.current_id = SciPlotDrawingLegend;
  if (w->plot.ShowLegend) {
    x = w->plot.x.LegendPos;
    y = w->plot.y.LegendPos;
    len = (real)w->plot.LegendLineSize;
    len2 = len / 2.0;
    height = FontnumHeight(w, w->plot.axisFont);
    height2 = height / 2.0;
    ascent = FontnumAscent(w, w->plot.axisFont);
    RectSet(w, x, y, x + w->plot.x.LegendSize - 1.0 - (real)w->plot.Margin,
      y + w->plot.y.LegendSize - 1.0, w->plot.ForegroundColor, XtLINE_SOLID);
    x += (real)w->plot.LegendMargin;
    y += (real)w->plot.LegendMargin;

    for (i = 0; i < w->plot.num_plotlist; i++) {
      p = w->plot.plotlist + i;
      if (p->draw) {
	LineSet(w, x, y + height2, x + len, y + height2,
	  p->LineColor, p->LineStyle);
	DrawMarker(w, x + len2, y + height2, p->markersize,
	  p->PointColor, p->PointStyle);
	TextSet(w, x + len + (real)w->plot.LegendMargin,
	  y + ascent,
	  p->legend, w->plot.ForegroundColor,
	  w->plot.axisFont);
	y += height;
      }
    }
  }
}

static void 
DrawCartesianAxes (SciPlotWidget w)
{
  real x, y, x1_pos, y1_pos, x2_pos, y2_pos, x1_val, y1_val, x_major_incr,
       y_major_incr, minorval, majorval, labelval, height;
  int i, j, num_x_major, num_y_major; /* precision, */
  char label[16]; /* , numberformat[16]; */

  w->plot.current_id = SciPlotDrawingAxis;
  height = FontnumHeight(w, w->plot.axisFont);
  num_x_major  = w->plot.x.MajorNum;
  num_y_major  = w->plot.y.MajorNum;
  x_major_incr = w->plot.x.MajorInc;
  y_major_incr = w->plot.y.MajorInc;
  x1_val = w->plot.x.DrawOrigin;
  y1_val = w->plot.y.DrawOrigin;
  x1_pos = PlotX(w, x1_val);
  y1_pos = PlotY(w, y1_val);
  x2_pos = PlotX(w, w->plot.x.DrawMax);
  y2_pos = PlotY(w, w->plot.y.DrawMax);

  /* draw x and y axis lines */
  LineSet(w, x1_pos, y1_pos, x2_pos, y1_pos, w->plot.ForegroundColor,
          XtLINE_SOLID); /* x axis line */
  LineSet(w, x1_pos, y1_pos, x1_pos, y2_pos, w->plot.ForegroundColor,
          XtLINE_SOLID); /* y axis line */

  /* *** BEGIN X AXIS *** */
  /* precision = w->plot.x.Precision;
  sprintf(numberformat, "%%.%df", precision);
  if (w->plot.XLog && precision>0)
    precision--; */
  if (w->plot.DrawMajorTics)
    LineSet(w, x1_pos, y1_pos+5, x1_pos, y1_pos-5, w->plot.ForegroundColor,
            XtLINE_SOLID);
  if (w->plot.XAxisNumbers) {
    sprintf(label, "%.6g", x1_val);
    TextSet(w, ceil(x1_pos - FontnumTextWidth(w, w->plot.axisFont, label)/2.),
            w->plot.y.AxisPos, label, w->plot.ForegroundColor,
            w->plot.axisFont);
  }

  /* MSE: reworked code since old while loop using numeric test failed
     for large numbers with small ranges */
  majorval = x1_val;
  for (i=0; i<num_x_major; i++) {

    /* minor x axis ticks and grid lines */
    if (w->plot.XLog) {
      if (w->plot.DrawMinor || w->plot.DrawMinorTics) {
	for (j=2; j<w->plot.x.MinorNum; j++) {
	  minorval = majorval * (real)j;
	  x = PlotX(w, minorval);
	  if (w->plot.DrawMinor)
	    LineSet(w, x, y1_pos, x, y2_pos, w->plot.ForegroundColor,
                    XtLINE_WIDEDOT);
	  if (w->plot.DrawMinorTics)
	    LineSet(w, x, y1_pos, x, y1_pos-3, w->plot.ForegroundColor,
                    XtLINE_SOLID);
	}
      }
      majorval *= x_major_incr;
      /* sprintf(numberformat, "%%.%df", precision);
      if (precision > 0)
	precision--; */
    }
    else {
      if (w->plot.DrawMinor || w->plot.DrawMinorTics) {
	for (j=1; j<w->plot.x.MinorNum; j++) {
	  minorval = majorval + x_major_incr * (real)j/w->plot.x.MinorNum;
	  x = PlotX(w, minorval);
	  if (w->plot.DrawMinor)
	    LineSet(w, x, y1_pos, x, y2_pos, w->plot.ForegroundColor,
                    XtLINE_WIDEDOT);
	  if (w->plot.DrawMinorTics)
	    LineSet(w, x, y1_pos, x, y1_pos-3, w->plot.ForegroundColor,
                    XtLINE_SOLID);
	}
      }
      majorval += x_major_incr;
    }

    /* major x axis ticks and grid lines */
    x = PlotX(w, majorval);
    if (w->plot.DrawMajor)
      LineSet(w, x, y1_pos, x, y2_pos, w->plot.ForegroundColor,	XtLINE_DOTTED);
    else if (w->plot.DrawMinor)
      LineSet(w, x, y1_pos, x, y2_pos, w->plot.ForegroundColor,	XtLINE_WIDEDOT);
    if (w->plot.DrawMajorTics)
      LineSet(w, x, y1_pos+5, x, y1_pos-5, w->plot.ForegroundColor,
              XtLINE_SOLID);
    if (w->plot.XAxisNumbers) {
      labelval = (w->plot.XLog) ? majorval : 
        floor(majorval/x_major_incr+.5)*x_major_incr;
      if (fabs(labelval) < DBL_MIN) labelval = 0.; /* hack to remove "-0" */
      /* printf("x majorval = %.6g labelval = %.6g\n", majorval, labelval); */
      sprintf(label, "%.6g", labelval);
      TextSet(w, ceil(x - FontnumTextWidth(w, w->plot.axisFont, label)/2.),
              w->plot.y.AxisPos, label, w->plot.ForegroundColor,
              w->plot.axisFont);
    }
  }

  /* *** BEGIN Y AXIS *** */
  /* precision = w->plot.y.Precision;
  sprintf(numberformat, "%%.%dg", precision);
  if (w->plot.YLog && precision>0)
    precision--; */
  if (w->plot.DrawMajorTics)
    LineSet(w, x1_pos+5, y1_pos, x1_pos-5, y1_pos, w->plot.ForegroundColor,
            XtLINE_SOLID);
  if (w->plot.YAxisNumbers) {
    sprintf(label, "%.6g", y1_val);
    if (w->plot.YNumHorz) {
      y = y1_pos + height/2.0 - FontnumDescent(w, w->plot.axisFont);
      TextSet(w, w->plot.x.AxisPos - FontnumTextWidth(w,w->plot.axisFont,label),
              y, label, w->plot.ForegroundColor, w->plot.axisFont);
    }
    else
      VTextSet(w, w->plot.x.AxisPos, y, label, w->plot.ForegroundColor,
               w->plot.axisFont);
  }

  /* MSE: reworked code since old while loop using numeric test failed
     for large numbers with small ranges */
  majorval = y1_val;
  for (i=0; i<num_y_major; i++) {

    /* minor x axis ticks and grid lines */
    if (w->plot.YLog) {
      if (w->plot.DrawMinor || w->plot.DrawMinorTics) {
	for (j=2; j<w->plot.y.MinorNum; j++) {
	  minorval = majorval * (real)j;
	  y = PlotY(w, minorval);
	  if (w->plot.DrawMinor)
	    LineSet(w, x1_pos, y, x2_pos, y, w->plot.ForegroundColor,
                    XtLINE_WIDEDOT);
	  if (w->plot.DrawMinorTics)
	    LineSet(w, x1_pos, y, x1_pos+3, y, w->plot.ForegroundColor,
                    XtLINE_SOLID);
	}
      }
      majorval *= y_major_incr;
      /* sprintf(numberformat, "%%.%df", precision);
      if (precision > 0)
	precision--; */
    }
    else {
      if (w->plot.DrawMinor || w->plot.DrawMinorTics) {
	for (j=1; j<w->plot.y.MinorNum; j++) {
	  minorval = majorval + y_major_incr * (real)j/w->plot.y.MinorNum;
	  y = PlotY(w, minorval);
	  if (w->plot.DrawMinor)
	    LineSet(w, x1_pos, y, x2_pos, y, w->plot.ForegroundColor,
                    XtLINE_WIDEDOT);
	  if (w->plot.DrawMinorTics)
	    LineSet(w, x1_pos, y, x1_pos+3, y, w->plot.ForegroundColor,
                    XtLINE_SOLID);
	}
      }
      majorval += y_major_incr;
    }

    /* major y axis ticks and grid lines */
    y = PlotY(w, majorval);
    if (w->plot.DrawMajor)
      LineSet(w, x1_pos, y, x2_pos, y, w->plot.ForegroundColor, XtLINE_DOTTED);
    else if (w->plot.DrawMinor)
      LineSet(w, x1_pos, y, x2_pos, y, w->plot.ForegroundColor,	XtLINE_WIDEDOT);
    if (w->plot.DrawMajorTics)
      LineSet(w, x1_pos-5, y, x1_pos+5, y, w->plot.ForegroundColor,
              XtLINE_SOLID);
    if (w->plot.YAxisNumbers) {
      /* Make sure label is not trying to resolve more precision
         than that indicated by MajorInc (especially at y = 0.) */
      labelval = (w->plot.YLog) ? majorval : 
        floor(majorval/y_major_incr+.5)*y_major_incr;
      if (fabs(labelval) < DBL_MIN) labelval = 0.; /* hack to remove "-0" */
      /* printf("y majorval = %.6g labelval = %.6g\n", majorval, labelval); */
      sprintf(label, "%.6g", labelval);
      if (w->plot.YNumHorz) {
        y += height/2.0 - FontnumDescent(w, w->plot.axisFont);
        TextSet(w, w->plot.x.AxisPos - 
                FontnumTextWidth(w, w->plot.axisFont, label), y, label,
                w->plot.ForegroundColor, w->plot.axisFont);
      }
      else
        VTextSet(w, w->plot.x.AxisPos, y, label, w->plot.ForegroundColor,
                 w->plot.axisFont);
    }
  }
  
  if (w->plot.ShowTitle)
    TextSet(w, w->plot.x.TitlePos, w->plot.y.TitlePos, w->plot.plotTitle,
            w->plot.ForegroundColor, w->plot.titleFont);
  if (w->plot.ShowXLabel)
    TextCenter(w, w->plot.x.Origin + w->plot.x.Size / 2.0, w->plot.y.LabelPos,
               w->plot.xlabel, w->plot.ForegroundColor, w->plot.labelFont);
  if (w->plot.ShowYLabel)
    VTextCenter(w, w->plot.x.LabelPos, w->plot.y.Origin + w->plot.y.Size / 2.0,
                w->plot.ylabel, w->plot.ForegroundColor, w->plot.labelFont);
}

          
static void 
DrawCartesianPlot (SciPlotWidget w)
{
  int i, j, jstart;
  SciPlotList *p;

  w->plot.current_id = SciPlotDrawingAny;
  ClipSet(w);
  w->plot.current_id = SciPlotDrawingLine;
  for (i = 0; i < w->plot.num_plotlist; i++) {
    p = w->plot.plotlist + i;
    if (p->draw) {
      real x1, y1, x2, y2;
      Boolean skipnext=False;
    
      jstart = 0;
      while ((jstart < p->number) &&
        (((p->data[jstart].x <= SCIPLOT_SKIP_VAL ||
           p->data[jstart].y <= SCIPLOT_SKIP_VAL) ||
          (w->plot.XLog && (p->data[jstart].x <= 0.0)) ||
          (w->plot.YLog && (p->data[jstart].y <= 0.0)))))
        jstart++;
      if (jstart < p->number) {
        x1 = PlotX(w, p->data[jstart].x);
        y1 = PlotY(w, p->data[jstart].y);
      }
      for (j = jstart; j < p->number; j++) {
        if (p->data[j].x <= SCIPLOT_SKIP_VAL ||
            p->data[j].y <= SCIPLOT_SKIP_VAL) {
          skipnext=True;
          continue;
        }
        
	if (!((w->plot.XLog && (p->data[j].x <= 0.0)) ||
	    (w->plot.YLog && (p->data[j].y <= 0.0)))) {
	  x2 = PlotX(w, p->data[j].x);
	  y2 = PlotY(w, p->data[j].y);
          if (!skipnext)
            LineSet(w, x1, y1, x2, y2, p->LineColor, p->LineStyle);
	  x1 = x2;
	  y1 = y2;
	}
        
        skipnext=False;
      }
    }
  }
  w->plot.current_id = SciPlotDrawingAny;
  ClipClear(w);
  w->plot.current_id = SciPlotDrawingLine;
  for (i = 0; i < w->plot.num_plotlist; i++) {
    p = w->plot.plotlist + i;
    if (p->draw) {
      real x2, y2;

      for (j = 0; j < p->number; j++) {
	if (!((w->plot.XLog && (p->data[j].x <= 0.0)) ||
	    (w->plot.YLog && (p->data[j].y <= 0.0)) ||
             p->data[j].x <= SCIPLOT_SKIP_VAL ||
             p->data[j].y <= SCIPLOT_SKIP_VAL )) {
	  x2 = PlotX(w, p->data[j].x);
	  y2 = PlotY(w, p->data[j].y);
	  if ((x2 >= w->plot.x.Origin) &&
	    (x2 <= w->plot.x.Origin + w->plot.x.Size) &&
	    (y2 >= w->plot.y.Origin) &&
	    (y2 <= w->plot.y.Origin + w->plot.y.Size)) {

	    DrawMarker(w, x2, y2,
	      p->markersize,
	      p->PointColor,
	      p->PointStyle);
	  }

	}
      }
    }
  }
}

static void 
DrawPolarAxes (SciPlotWidget w)
{
  real x1, y1, x2, y2, max, tic, val, height;
  int i, j;
  char label[16]; /* , numberformat[16]; */

  w->plot.current_id = SciPlotDrawingAxis;
  /* sprintf(numberformat, "%%.%df", w->plot.x.Precision); */
  height = FontnumHeight(w, w->plot.labelFont);
  max = w->plot.PolarScale;
  PlotRTDegrees(w, 0.0, 0.0, &x1, &y1);
  PlotRTDegrees(w, max, 0.0, &x2, &y2);
  LineSet(w, x1, y1, x2, y2, 1, XtLINE_SOLID);
  for (i = 45; i < 360; i += 45) {
    PlotRTDegrees(w, max, (real)i, &x2, &y2);
    LineSet(w, x1, y1, x2, y2, w->plot.ForegroundColor, XtLINE_DOTTED);
  }
  for (i = 1; i <= w->plot.x.MajorNum; i++) {
    tic = w->plot.PolarScale *
      (real)i / (real)w->plot.x.MajorNum;
    if (w->plot.DrawMinor || w->plot.DrawMinorTics) {
      for (j = 1; j < w->plot.x.MinorNum; j++) {
	val = tic - w->plot.x.MajorInc * (real)j /
	  w->plot.x.MinorNum;
	PlotRTDegrees(w, val, 0.0, &x2, &y2);
	if (w->plot.DrawMinor)
	  CircleSet(w, x1, y1, x2 - x1,
	    w->plot.ForegroundColor, XtLINE_WIDEDOT);
	if (w->plot.DrawMinorTics)
	  LineSet(w, x2, y2 - 2.5, x2, y2 + 2.5,
	    w->plot.ForegroundColor, XtLINE_SOLID);
      }
    }
    PlotRTDegrees(w, tic, 0.0, &x2, &y2);
    if (w->plot.DrawMajor)
      CircleSet(w, x1, y1, x2 - x1, w->plot.ForegroundColor, XtLINE_DOTTED);
    if (w->plot.DrawMajorTics)
      LineSet(w, x2, y2 - 5.0, x2, y2 + 5.0, w->plot.ForegroundColor,
              XtLINE_SOLID);
    if (w->plot.XAxisNumbers) {
      sprintf(label, "%.6g", tic);
      TextSet(w, x2, y2 + height, label, w->plot.ForegroundColor,
              w->plot.axisFont);
    }
  }
  
  if (w->plot.ShowTitle)
    TextSet(w, w->plot.x.TitlePos, w->plot.y.TitlePos,
      w->plot.plotTitle, w->plot.ForegroundColor, w->plot.titleFont);
}

static void 
DrawPolarPlot (SciPlotWidget w)
{
  int i, j;
  SciPlotList *p;

  w->plot.current_id = SciPlotDrawingLine;
  for (i = 0; i < w->plot.num_plotlist; i++) {
    p = w->plot.plotlist + i;
    if (p->draw) {
      int jstart;
      real x1, y1, x2, y2;
      Boolean skipnext=False;

      jstart = 0;
      while ((jstart < p->number) &&
        (p->data[jstart].x <= SCIPLOT_SKIP_VAL ||
          p->data[jstart].y <= SCIPLOT_SKIP_VAL))
        jstart++;
      if (jstart < p->number) {
        PlotRT(w, p->data[0].x, p->data[0].y, &x1, &y1);
      }
      for (j = jstart; j < p->number; j++) {
        if (p->data[j].x <= SCIPLOT_SKIP_VAL ||
            p->data[j].y <= SCIPLOT_SKIP_VAL) {
          skipnext=True;
          continue;
        }
        
	PlotRT(w, p->data[j].x, p->data[j].y, &x2, &y2);
        if (!skipnext) {
          LineSet(w, x1, y1, x2, y2,
            p->LineColor, p->LineStyle);
          DrawMarker(w, x1, y1, p->markersize,
            p->PointColor, p->PointStyle);
          DrawMarker(w, x2, y2, p->markersize,
            p->PointColor, p->PointStyle);
        }
	x1 = x2;
	y1 = y2;
        
        skipnext=False;
      }
    }
  }
}

static void 
DrawAll (SciPlotWidget w)
{
  if (w->plot.ChartType == XtCARTESIAN) {
    DrawCartesianAxes(w);
    DrawLegend(w);
    DrawCartesianPlot(w);
  }
  else {
    DrawPolarAxes(w);
    DrawLegend(w);
    DrawPolarPlot(w);
  }
}

static Boolean 
DrawQuick (SciPlotWidget w)
{
  Boolean range_check = CheckMinMax(w);
  EraseClassItems(w, SciPlotDrawingLine);
  EraseAllItems(w);
  DrawAll(w);

  return range_check;
}


/*
 * Public Plot functions
 */

int 
SciPlotAllocNamedColor (Widget wi, char *name)
{
  XColor used, exact;
  SciPlotWidget w;
  if (!XtIsSciPlot(wi))
    return -1;
  w = (SciPlotWidget) wi;

  if (!XAllocNamedColor(XtDisplay(w), w->plot.cmap, name, &used, &exact))
    return 1;
  return ColorStore(w, used.pixel);
}

int 
SciPlotAllocRGBColor (Widget wi, int r, int g, int b)
{
  XColor used;
  SciPlotWidget w;
  if (!XtIsSciPlot(wi))
    return -1;
  w = (SciPlotWidget) wi;

  used.pixel = 0;
  r *= 256;
  g *= 256;
  b *= 256;
  if (r > 65535)
    r = 65535;
  if (g > 65535)
    g = 65535;
  if (b > 65535)
    b = 65535;
  used.red = (unsigned short) r;
  used.green = (unsigned short) g;
  used.blue = (unsigned short) b;
  if (!XAllocColor(XtDisplay(w), w->plot.cmap, &used))
    return 1;
  return ColorStore(w, used.pixel);
}

void 
SciPlotSetBackgroundColor (Widget wi, int color)
{
  SciPlotWidget w;
  if (!XtIsSciPlot(wi))
    return;
  w = (SciPlotWidget) wi;

  if (color < w->plot.num_colors) {
    w->plot.BackgroundColor = color;
    w->core.background_pixel = w->plot.colors[color];
    XSetWindowBackground( XtDisplay(w), XtWindow(w), w->core.background_pixel);
  }
}

void 
SciPlotSetForegroundColor (Widget wi, int color)
{
  SciPlotWidget w;
  if (!XtIsSciPlot(wi))
    return;
  w = (SciPlotWidget) wi;

  if (color < w->plot.num_colors)
    w->plot.ForegroundColor = color;
}

void 
SciPlotListDelete (Widget wi, int idnum)
{
  SciPlotList *p;
  SciPlotWidget w;
  if (!XtIsSciPlot(wi))
    return;
  w = (SciPlotWidget) wi;

  p = _ListFind(w, idnum);
  if (p)
    _ListDelete(p);
}

int 
SciPlotListCreateFromData (Widget wi, int num, real *xlist, real *ylist,
                           char *legend, int pcolor, int pstyle, int lcolor,
                           int lstyle)
{
  int idnum;
  SciPlotList *p;
  SciPlotWidget w;
  if (!XtIsSciPlot(wi))
    return -1;
  w = (SciPlotWidget) wi;

  idnum = _ListNew(w);
  p = w->plot.plotlist + idnum;
  _ListSetReal(p, num, xlist, ylist);
  _ListSetLegend(p, legend);
  _ListSetStyle(p, pcolor, pstyle, lcolor, lstyle);
  return idnum;
}

int 
SciPlotListCreateFloat (Widget wi, int num, float *xlist, float *ylist,
                        char *legend)
{
  int idnum;
  SciPlotList *p;
  SciPlotWidget w;
  if (!XtIsSciPlot(wi))
    return -1;
  w = (SciPlotWidget) wi;

  idnum = _ListNew(w);
  p = w->plot.plotlist + idnum;
  _ListSetFloat(p, num, xlist, ylist);
  _ListSetLegend(p, legend);
  _ListSetStyle(p, 1, XtMARKER_CIRCLE, 1, XtLINE_SOLID);
  return idnum;
}

void 
SciPlotListUpdateFloat (Widget wi, int idnum, int num, float *xlist,
                        float *ylist)
{
  SciPlotList *p;
  SciPlotWidget w;
  if (!XtIsSciPlot(wi))
    return;
  w = (SciPlotWidget) wi;

  p = _ListFind(w, idnum);
  if (p)
    _ListSetFloat(p, num, xlist, ylist);
}

void 
SciPlotListAddFloat (Widget wi, int idnum, int num, float *xlist, float *ylist)
{
  SciPlotList *p;
  SciPlotWidget w;
  if (!XtIsSciPlot(wi))
    return;
  w = (SciPlotWidget) wi;

  p = _ListFind(w, idnum);
  if (p)
    _ListAddFloat(p, num, xlist, ylist);
}

int 
SciPlotListCreateDouble (Widget wi, int num, double *xlist, double *ylist,
                         const char *legend)
{
  int idnum;
  SciPlotList *p;
  SciPlotWidget w;
  if (!XtIsSciPlot(wi))
    return -1;
  w = (SciPlotWidget) wi;

  idnum = _ListNew(w);
  p = w->plot.plotlist + idnum;
  _ListSetDouble(p, num, xlist, ylist);
  _ListSetLegend(p, legend);
  _ListSetStyle(p, 1, XtMARKER_CIRCLE, 1, XtLINE_SOLID);
  return idnum;
}

void 
SciPlotListUpdateDouble (Widget wi, int idnum, int num, double *xlist,
                         double *ylist)
{
  SciPlotList *p;
  SciPlotWidget w;
  if (!XtIsSciPlot(wi))
    return;
  w = (SciPlotWidget) wi;

  p = _ListFind(w, idnum);
  if (p)
    _ListSetDouble(p, num, xlist, ylist);
}

void 
SciPlotListAddDouble (Widget wi, int idnum, int num, double *xlist,
                      double *ylist)
{
  SciPlotList *p;
  SciPlotWidget w;
  if (!XtIsSciPlot(wi))
    return;
  w = (SciPlotWidget) wi;

  p = _ListFind(w, idnum);
  if (p)
    _ListAddDouble(p, num, xlist, ylist);
}

void 
SciPlotListSetStyle (Widget wi, int idnum, int pcolor, int pstyle, int lcolor,
                     int lstyle)
{
  SciPlotList *p;
  SciPlotWidget w;
  if (!XtIsSciPlot(wi))
    return;
  w = (SciPlotWidget) wi;

  p = _ListFind(w, idnum);
  if (p)
    _ListSetStyle(p, pcolor, pstyle, lcolor, lstyle);
}


void SciPlotListSetMarkerSize(Widget wi, int idnum, float size)
{
  SciPlotList *p;
  SciPlotWidget w;
  if (!XtIsSciPlot(wi))
    return;
  w = (SciPlotWidget) wi;

  p = _ListFind(w, idnum);
  if (p)
    p->markersize=size;
}

void 
SciPlotSetXAutoScale (Widget wi)
{
  SciPlotWidget w;
  if (!XtIsSciPlot(wi))
    return;
  w = (SciPlotWidget) wi;

  w->plot.XAutoScale = True;
}

void 
SciPlotSetXUserScale (Widget wi, double min, double max)
{
  SciPlotWidget w;
  if (!XtIsSciPlot(wi))
    return;
  w = (SciPlotWidget) wi;

  if (min < max) {
    w->plot.XAutoScale = False;
    w->plot.UserMin.x = (real)min;
    w->plot.UserMax.x = (real)max;
  }
}

void 
SciPlotSetYAutoScale (Widget wi)
{
  SciPlotWidget w;
  if (!XtIsSciPlot(wi))
    return;
  w = (SciPlotWidget) wi;

  w->plot.YAutoScale = True;
}

void 
SciPlotSetYUserScale (Widget wi, double min, double max)
{
  SciPlotWidget w;
  if (!XtIsSciPlot(wi))
    return;
  w = (SciPlotWidget) wi;

  if (min < max) {
    w->plot.YAutoScale = False;
    w->plot.UserMin.y = (real)min;
    w->plot.UserMax.y = (real)max;
  }
}

void 
SciPlotPrintStatistics (Widget wi)
{
  int i, j;
  SciPlotList *p;
  SciPlotWidget w;
  if (!XtIsSciPlot(wi))
    return;
  w = (SciPlotWidget) wi;

  printf("Title=%s\nxlabel=%s\tylabel=%s\n",
    w->plot.plotTitle, w->plot.xlabel, w->plot.ylabel);
  printf("ChartType=%d\n", w->plot.ChartType);
  printf("Degrees=%d\n", w->plot.Degrees);
  printf("XLog=%d\tYLog=%d\n", w->plot.XLog, w->plot.YLog);
  printf("XAutoScale=%d\tYAutoScale=%d\n",
    w->plot.XAutoScale, w->plot.YAutoScale);
  for (i = 0; i < w->plot.num_plotlist; i++) {
    p = w->plot.plotlist + i;
    if (p->draw) {
      printf("\nLegend=%s\n", p->legend);
      printf("Styles: point=%d line=%d  Color: point=%d line=%d\n",
	p->PointStyle, p->LineStyle, p->PointColor, p->LineColor);
      for (j = 0; j < p->number; j++)
	printf("%f\t%f\n", p->data[j].x, p->data[j].y);
      printf("\n");
    }
  }
}

void 
SciPlotExportData (Widget wi, FILE *fd)
{
  int i, j;
  SciPlotList *p;
  SciPlotWidget w;
  if (!XtIsSciPlot(wi))
    return;
  w = (SciPlotWidget) wi;

  fprintf(fd, "Title=\"%s\"\n", w->plot.plotTitle);
  fprintf(fd, "Xaxis=\"%s\"\n", w->plot.xlabel);
  fprintf(fd, "Yaxis=\"%s\"\n\n", w->plot.ylabel);
  for (i = 0; i < w->plot.num_plotlist; i++) {
    p = w->plot.plotlist + i;
    if (p->draw) {
      fprintf(fd, "Line=\"%s\"\n",p->legend);
      for (j = 0; j < p->number; j++)
	fprintf(fd, "%e\t%e\n", p->data[j].x, p->data[j].y);
      fprintf(fd, "\n");
    }
  }
}

int 
SciPlotStoreAllocatedColor(Widget wi, Pixel p)
{
  SciPlotWidget w;
  if (!XtIsSciPlot(wi))
    return -1;
  w = (SciPlotWidget) wi;
  
  return ColorStore(w, p);
}

void SciPlotUpdate (Widget wi)
{
  SciPlotWidget w;
  if (!XtIsSciPlot(wi))
    return;
  w = (SciPlotWidget) wi;

  EraseAll(w);
#ifdef DEBUG_SCIPLOT
  SciPlotPrintStatistics(w);
#endif
  ComputeAll(w);
  DrawAll(w);
}

Boolean SciPlotQuickUpdate (Widget wi)
{
  SciPlotWidget w;
  if (!XtIsSciPlot(wi))
    return False;
  w = (SciPlotWidget) wi;

  return DrawQuick(w);
}

/* New functions added by MSE for additional access to internal data */

/* MSE, 6/8/03: added SciPlot list data access function to avoid maintaining
                history vectors in Graph2D. */
realpair* SciPlotListReturnPtr (Widget wi, int idnum, int* num_xy_pairs)
{
  SciPlotList *p;
  SciPlotWidget w;
  if (!XtIsSciPlot(wi))
    return;
  w = (SciPlotWidget) wi;

  p = _ListFind(w, idnum);
  if (p) {
    *num_xy_pairs = p->number;
    return p->data;
  }
  else {
    *num_xy_pairs = 0;
    return NULL;
  }
}

/* MSE, 5/7/04: added to allow return of x axis DrawOrigin/DrawMax for
                anchoring min/max markers */
void SciPlotReturnXAxis (Widget wi, real* draw_x_min, real* draw_x_max)
{
  SciPlotWidget w;
  if (!XtIsSciPlot(wi))
    return;
  w = (SciPlotWidget) wi;

  *draw_x_min = w->plot.x.DrawOrigin;
  *draw_x_max = w->plot.x.DrawMax;
}

Boolean SciPlotQuickUpdateCheck (Widget wi)
{
  SciPlotWidget w;
  if (!XtIsSciPlot(wi))
    return;
  w = (SciPlotWidget) wi;

  return CheckMinMax(w);
}

void SciPlotPrepareFullUpdate (Widget wi)
{
  SciPlotWidget w;
  if (!XtIsSciPlot(wi))
    return;
  w = (SciPlotWidget) wi;

  EraseAll(w);
  ComputeAll(w);
}

void SciPlotPrepareQuickUpdate (Widget wi)
{
  SciPlotWidget w;
  if (!XtIsSciPlot(wi))
    return;
  w = (SciPlotWidget) wi;

  EraseClassItems(w, SciPlotDrawingLine);
  EraseAllItems(w);
}

void SciPlotDrawAll (Widget wi)
{
  SciPlotWidget w;
  if (!XtIsSciPlot(wi))
    return;
  w = (SciPlotWidget) wi;

  DrawAll(w);
}
