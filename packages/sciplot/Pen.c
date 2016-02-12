#include <stdlib.h>

#include "Pen.h"
#include "StrToPmap.h"

#include <X11/Xmu/Drawing.h>
#include <X11/Xmu/CharSet.h>

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/CoreP.h>
#include <X11/Shell.h>

/* I have observed that resource converters can be called before the
   widgets initialize methods have been called.

   One of the results of this is that the window field of the core
   part is garbage.  We must therefore save the window field in a temp
   variable (in case it's NOT garbage), set it to zero, call XtGetGC
   (which will do some magic), and then restore the window field.

   It bites.

   */

int SameXfwfPen(a,b)
     XfwfPen	*a,*b;
{
  return 0 == memcmp((char*)a,(char*)b,sizeof(*a));
}

int ValidateXfwfPen(w, pen)
     Widget	w;
     XfwfPen	*pen;
{
  Drawable	win = w->core.window;

  if (pen->gc!=0)
    return 1;

  w->core.window = 0;		/* see note above */
  pen->gc = XtGetGC(w, pen->mask, &pen->xgcv);
  w->core.window = win;

  pen->user_gc = 0;

  return pen->gc != 0;
}

int FreeXfwfPen(w, pen)
     Widget	w;
     XfwfPen	*pen;
{
  Screen	*screen;
  Display	*dpy;
  if (!pen->user_gc) {
    XtReleaseGC(w, pen->gc);
    pen->gc = 0;
  }
  if (!XtIsWidget(w))
    w = XtParent(w);
  screen = XtScreen(w);
  dpy = XtDisplay(w);
  if (!pen->user_xgcv) {
    if (pen->mask&GCForeground &&
	/* these checks are a bad hack */
	pen->xgcv.foreground != WhitePixelOfScreen(screen) &&
	pen->xgcv.foreground != BlackPixelOfScreen(screen))
      XFreeColors(dpy, w->core.colormap,
		  &pen->xgcv.foreground, 1, 0L);
    if (pen->mask&GCBackground &&
	/* these checks are a bad hack */
	pen->xgcv.background != WhitePixelOfScreen(screen) &&
	pen->xgcv.background != BlackPixelOfScreen(screen))
      XFreeColors(dpy, w->core.colormap,
		  &pen->xgcv.background, 1, 0L);
    if (pen->mask&GCTile)
      XFreePixmap(dpy, pen->xgcv.tile);
    if (pen->mask&GCStipple)
      XFreePixmap(dpy, pen->xgcv.stipple);
    pen->mask &= ~XfwfPenGCBits;
  }
  if (pen->colors) {
    XFreeColors(dpy, w->core.colormap, pen->colors, pen->ncolors, 0L);
    free((char*)pen->colors);
    pen->colors = 0;
  }

  return 1;			/* no failure */
}

static int convert_pixel(dpy, screen, cmap, name, pixel)
     Display	*dpy;
     Screen	*screen;
     Colormap	cmap;
     char	*name;
     unsigned long	*pixel;
{
  if (0 == XmuCompareISOLatin1(name, XtDefaultBackground)) {
    *pixel = BlackPixelOfScreen(screen);
  } else if (0 == XmuCompareISOLatin1(name, XtDefaultForeground)) {
    *pixel = WhitePixelOfScreen(screen);
  } else {
    XColor	rval, blah;
    if (!XAllocNamedColor(dpy, cmap, name, &rval, &blah)) {
      return 0;
    }
    *pixel = rval.pixel;
  }
  return 1;
}


XtConvertArgRec  Xfwf_StringToPen_Args[] = {
  { XtWidgetBaseOffset, (XtPointer) XtOffset(Widget, core.screen),
      sizeof(Screen*) },
  { XtWidgetBaseOffset, (XtPointer) XtOffset(Widget, core.colormap),
      sizeof(Colormap) },
  { XtBaseOffset, (XtPointer) XtOffset(Object, object.self),
      sizeof(Object) },
};

int Xfwf_Num_StringToPen_Args = XtNumber(Xfwf_StringToPen_Args);

Boolean ConvertStringToPen(dpy, screen, colormap, w, src, dest)
     Display	*dpy;
     Screen	*screen;
     Colormap	colormap;
     Widget	w;		/* could be an Object */
     char	*src;
     XfwfPen	*dest;
{
  char		*scan, *s;

  /* zero all values */
  dest->gc = 0;
  dest->mask = 0;
  dest->colors = 0;
  dest->user_xgcv = 0;
  dest->user_gc = 0;

  src = XtNewString(src);

  scan = src;
  while (1) {
    s = scan;
    for (; *scan && *scan != ':'; scan++)
      ;
    if (scan==s) {
      if (scan!=src) {
	Drawable	win = w->core.window;

	w->core.window = 0;
	dest->gc = XtGetGC(w, dest->mask, &dest->xgcv);
	w->core.window = win;
      }
      break;
    }
    *scan = 0;
    scan++;

    /* s points to the first token, a code word */

    /* Draw using a solid color */
    if (0==XmuCompareISOLatin1(s, "solid")) {
      /* solid:pixel */
      dest->xgcv.fill_style = FillSolid;
      dest->mask |= GCFillStyle;
      if (!convert_pixel(dpy, screen, colormap, scan, &dest->xgcv.foreground))
	break;
      dest->mask |= GCForeground;
      scan += strlen(scan);

      /* draw using a foreground and a stipple pattern */
    } else if (0==XmuCompareISOLatin1(s, "stipple")) {
      /* stipple:pixel,bitmap */
      int	dummy;
      s = scan;
      for(; *scan && *scan != ','; scan++)
	;
      *scan = 0;
      scan++;

      dest->xgcv.fill_style = FillStippled;
      dest->mask |= GCFillStyle;

      if (!convert_pixel(dpy, screen, colormap, s, &dest->xgcv.foreground))
	break;
      dest->mask |= GCForeground;

      dest->xgcv.stipple = XmuLocateBitmapFile(screen, scan, (char*)0, 0,
					  &dummy, &dummy, &dummy, &dummy);
      if (dest->xgcv.stipple == None)
	break;
      dest->mask |= GCStipple;

      scan += strlen(scan);

      /* draw using a foreground and background and an opaque stipple pattern*/
    } else if (0==XmuCompareISOLatin1(s, "opaque-stipple")) {
      /* opaque-stipple:pixel,pixel,bitmap */
      int	dummy;
      s = scan;
      for(; *scan && *scan != ','; scan++)
	;
      *scan = 0;
      scan++;

      dest->xgcv.fill_style = FillOpaqueStippled;
      dest->mask |= GCFillStyle;

      if (!convert_pixel(dpy, screen, colormap, s, &dest->xgcv.foreground))
	break;
      dest->mask |= GCForeground;

      s = scan;
      for(; *scan && *scan != ','; scan++)
	;
      *scan = 0;
      scan++;

      if (!convert_pixel(dpy, screen, colormap, s, &dest->xgcv.background))
	break;
      dest->mask |= GCBackground;

      if (0==XmuCompareISOLatin1(scan, "XtCheckerBitmap")) {
	static char pixmap_bits[] = {
	  0x02, 0x01,
	};
	dest->xgcv.stipple = XCreateBitmapFromData
	  (dpy, RootWindowOfScreen(screen), pixmap_bits, 2, 2);
      } else {
	dest->xgcv.stipple = XmuLocateBitmapFile(screen, scan, (char*)0, 0,
						 &dummy, &dummy, &dummy,
						 &dummy);
      }

      if (dest->xgcv.stipple == None)
	break;
      dest->mask |= GCStipple;

      scan += strlen(scan);

#if 0
      /* Draw using a pixmap tile */
    } else if (0==XmuCompareISOLatin1(s, "tile")) {
      /* tile:pixmap */
      Visual	*visual;

      if (Xfwf_StringToPixmap_AssumeDefaultVisual) {
	visual = screen->root_visual;
      } else {
	Arg	arg;
	Widget	temp = w;
	visual = CopyFromParent;
	XtSetArg(arg, XtNvisual, &visual);
	while (temp && visual == CopyFromParent) {
	  XtGetValues(temp, &arg, 1);
	  temp = XtParent(temp);
	}
	if (!temp)
	  visual = screen->root_visual;
      }

      dest->xgcv.fill_style = FillTiled;
      dest->mask |= GCFillStyle;

#if 0
      {
	XrmValue	to, from;
	from.addr = scan;
	from.size = ?
	to.addr = &dest->xgcv.tile;
	to.size = sizeof(dest->xgcv.tile);
	XtConvertAndStore(w, XtRString, &from, XtRPixmap, &to);
      }
#else
      if (!XfwfLocatePixmapFile(screen, colormap, visual, scan,
				, (Pixmap*)0, (char**)0,
				(int*)0, (int*)0,
				&dest->colors, &dest->ncolors))
	break;
#endif
      dest->mask |= GCTile;

      scan += strlen(scan);
#endif
      /* if the depth is one, use one pen style, otherwise use the other */
    } else if (0==XmuCompareISOLatin1(s, "mono")) {
      /* mono:spec|spec */
      int	depth;

      s = scan;
      for(; *scan && *scan != '|'; scan++)
	;
      *scan = 0;
      scan++;

      depth = ((XtIsWidget(w)) ? w : XtParent(w) )
	->core.depth;

      if (depth<2) {
      /* if it's mono, parse the first spec */
	scan = s;
      } /* else, parse the second spec */
    } else {
      break;
    }
  }

  XtFree(src);

  if (dest->gc) {
    return 1;
  } else {
    /* clean up resources we might have allocated */
    FreeXfwfPen(w, dest);
    return 0;
  }
}
     

Boolean XfwfCvtStringToPen(dpy, args, num_args, fromVal, toVal,
			   converter_data)
     Display   *dpy;
     XrmValue  *args;
     Cardinal  *num_args;
     XrmValue  *fromVal;
     XrmValue  *toVal;
     XtPointer *converter_data;
{
  Screen	*screen;
  Colormap	colormap;
  Object	o;
  Widget	w;
  XfwfPen	*dest;
/*  char		*src;*/

  screen = *((Screen* *)args[0].addr);

  colormap = *((Colormap *)args[1].addr);

  o = *((Object *)args[2].addr);
  w = (Widget) o;

  {
    if (toVal->addr != NULL) {
      if (toVal->size < sizeof(XfwfPen*)) {
	toVal->size = sizeof(XfwfPen*);
	return False;
      }
    }
    else {
      static XfwfPen* static_val;
      toVal->addr = (XPointer)&static_val;
    }
    toVal->size = sizeof(XfwfPen*);
  }
  dest = *(XfwfPen**)toVal->addr = (XfwfPen*)XtMalloc(sizeof(XfwfPen));

  if (ConvertStringToPen(dpy, screen, colormap, w,
			 (char*)fromVal->addr, dest)) {
    return 1;
  } else {
    XtFree((XtPointer)dest);
    /* issue a warning */
    XtStringConversionWarning((char*)fromVal->addr, XtRXfwfPenPtr);
    return 0;
  }
}

void XfwfStringToPenDestructor(app, to, converter_data, args, num_args)
      XtAppContext app;
      XrmValue *to;
      XtPointer converter_data;
      XrmValue *args;
      Cardinal *num_args;
{
  XfwfPen	*pen = *(XfwfPen**)to->addr;
  FreeXfwfPen((Widget)*(Object*)args[2].addr, pen);
  XtFree((char*)pen);
}

void XfwfInstallStringToPenConverter()
{
  XtSetTypeConverter
    (XtRString, XtRXfwfPenPtr, XfwfCvtStringToPen,
     Xfwf_StringToPen_Args, Xfwf_Num_StringToPen_Args,
     XtCacheByDisplay|XtCacheRefCount, XfwfStringToPenDestructor);
}
