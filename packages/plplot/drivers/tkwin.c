/* $Id: tkwin.c 3186 2006-02-15 18:17:33Z slbrow $
 *
 * PLplot Tk device driver.
 *
 * Copyright (C) 2004  Maurice LeBrun
 * Copyright (C) 2004  Joao Cardoso
 *
 * This file is part of PLplot.
 *
 * PLplot is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Library Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * PLplot is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with PLplot; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * This device driver is designed to be used by a PlPlotter, and in fact requires
 * the existence of an enclosing PlPlotter.
 *
 * The idea is that this should develop into a completely cross-platform driver
 * for use by the cross platform Tk system.
 *
*/

#include "plDevs.h"

#define DEBUG

#ifdef PLD_tkwin


#define NEED_PLDEBUG
#include "plplotP.h"
#include "pltkwd.h"
#include "drivers.h"
#include "plevent.h"

#define _TCLINT
#ifdef USE_TCL_STUBS
/* Unfortunately, tkInt.h ends up loading Malloc.h under Windows */
/* So we have to deal with this mess */
    #undef malloc
    #undef free
    #undef realloc
    #undef calloc
#if defined(__WIN32__) || defined (MAC_TCL)
#include <tkInt.h>
#else
#include <tk.h>
#endif
    #define malloc ckalloc
    #define free(m) ckfree((char*)m)
    #define realloc ckrealloc
    #define calloc ckcalloc
#else
#if defined(__WIN32__) || defined (MAC_TCL)
#include <tkInt.h>
#else
#include <tk.h>
#endif
#endif

#ifdef ckalloc
#undef ckalloc
#define ckalloc malloc
#endif
#ifdef ckfree
#undef ckfree
#define ckfree free
#endif
#ifdef free
#undef free
#endif

/* Device info */
char* plD_DEVICE_INFO_tkwin = "tkwin:New tk driver:1:tkwin:45:tkwin";


void *	ckcalloc(size_t nmemb, size_t size);

/*
 * We want to use the 'pure Tk' interface.  On Unix we can use
 * some direct calls to X instead of Tk, if we want, although
 * that code hasn't been tested for some time.  So this define
 * is required on Windows/MacOS and perhaps optional on Unix.
 */
#define USE_TK

#ifdef __WIN32__
#define XSynchronize(display, bool) {display->request++;}
#define XSync(display, bool) {display->request++;}
#define XFlush(display)
#endif

/* Dummy definition of PlPlotter containing first few fields */
typedef struct PlPlotter {
  Tk_Window tkwin;  /* Window that embodies the frame. NULL
		     * means that the window has been destroyed
		     * but the data structures haven't yet been
		     * cleaned up.
		     */
  Display *display;  /* Display containing widget. Used, among
		      * other things, so that resources can be
		      * freed even after tkwin has gone away.
		      */
  Tcl_Interp *interp;  /* Interpreter associated with
			* widget. Used to delete widget
			* command.
			*/
} PlPlotter;

void CopyColour(XColor* from, XColor* to);
void Tkw_StoreColor(PLStream* pls, TkwDisplay* tkwd, XColor* col);
static int  pltk_AreWeGrayscale(PlPlotter *plf);
void PlplotterAtEop(Tcl_Interp *interp, register PlPlotter *plPlotterPtr);
void PlplotterAtBop(Tcl_Interp *interp, register PlPlotter *plPlotterPtr);

static int synchronize = 0; /* change to 1 for synchronized operation */
/* for debugging only */

/* Number of instructions to skip between querying the X server for events */

#define MAX_INSTR 20

/* Pixels/mm */

#define PHYSICAL 0  /* Enables physical scaling.. */

/* Set constants for dealing with colormap. In brief:
 *
 * ccmap  When set, turns on custom color map
 *
 * XWM_COLORS  Number of low "pixel" values to copy.
 * CMAP0_COLORS  Color map 0 entries.
 * CMAP1_COLORS  Color map 1 entries.
 * MAX_COLORS  Maximum colors period.
 *
 * See Init_CustomCmap() and Init_DefaultCmap() for more info.
 * Set ccmap at your own risk -- still under development.
 */

/* plplot_tkwin_ccmap is statically defined in pltkwd.h.  Note that
 * plplotter.c also includes that header and uses that variable. */

#define XWM_COLORS 70
#define CMAP0_COLORS 16
#define CMAP1_COLORS 50
#define MAX_COLORS 256

#ifndef USE_TK
/* Variables to hold RGB components of given colormap. */
/* Used in an ugly hack to get past some X11R5 and TK limitations. */

static int sxwm_colors_set;
static XColor sxwm_colors[MAX_COLORS];
#endif

/* Keep pointers to all the displays in use */

static TkwDisplay *tkwDisplay[PLTKDISPLAYS];

#if !defined(MAC_TCL) && !defined(__WIN32__)
static unsigned char CreatePixmapStatus;
static int CreatePixmapErrorHandler(Display *display, XErrorEvent *error);
#endif

/* Function prototypes */
/* Initialization */

static void Init  (PLStream *pls);
static void InitColors  (PLStream *pls);
static void AllocCustomMap (PLStream *pls);
static void AllocCmap0  (PLStream *pls);
static void AllocCmap1  (PLStream *pls);
static void CreatePixmap (PLStream *pls);
static void GetVisual  (PLStream *pls);
static void AllocBGFG  (PLStream *pls);

/* Escape function commands */

static void ExposeCmd  (PLStream *pls, PLDisplay *ptr);
static void RedrawCmd  (PLStream *pls);
static void ResizeCmd  (PLStream *pls, PLDisplay *ptr);
#ifndef USE_TK
static void GetCursorCmd (PLStream *pls, PLGraphicsIn *ptr);
#endif
static void FillPolygonCmd (PLStream *pls);
#ifdef USING_PLESC_COPY
static void CopyCommand (PLStream *pls);
#endif

/* Miscellaneous */

static void StoreCmap0  (PLStream *pls);
static void StoreCmap1  (PLStream *pls);
static void WaitForPage(PLStream *pls);

void plD_dispatch_init_tkwin	( PLDispatchTable *pdt );

void plD_init_tkwin		(PLStream *);
void plD_line_tkwin		(PLStream *, short, short, short, short);
void plD_polyline_tkwin		(PLStream *, short *, short *, PLINT);
void plD_eop_tkwin		(PLStream *);
void plD_bop_tkwin		(PLStream *);
void plD_tidy_tkwin		(PLStream *);
void plD_state_tkwin		(PLStream *, PLINT);
void plD_esc_tkwin		(PLStream *, PLINT, void *);
void plD_open_tkwin             (PLStream *pls);

void plD_dispatch_init_tkwin( PLDispatchTable *pdt )
{
#ifndef ENABLE_DYNDRIVERS
    pdt->pl_MenuStr  = "PLplot Tk plotter";
    pdt->pl_DevName  = "tkwin";
#endif
    pdt->pl_type     = plDevType_Interactive;
    pdt->pl_seq      = 45;
    pdt->pl_init     = (plD_init_fp)     plD_init_tkwin;
    pdt->pl_line     = (plD_line_fp)     plD_line_tkwin;
    pdt->pl_polyline = (plD_polyline_fp) plD_polyline_tkwin;
    pdt->pl_eop      = (plD_eop_fp)      plD_eop_tkwin;
    pdt->pl_bop      = (plD_bop_fp)      plD_bop_tkwin;
    pdt->pl_tidy     = (plD_tidy_fp)     plD_tidy_tkwin;
    pdt->pl_state    = (plD_state_fp)    plD_state_tkwin;
    pdt->pl_esc      = (plD_esc_fp)      plD_esc_tkwin;
}

/*--------------------------------------------------------------------------*\
 * plD_init_tkwin()
 *
 * Initialize device.
 * Tk-dependent stuff done in plD_open_tkwin() and Init().
\*--------------------------------------------------------------------------*/

void
plD_init_tkwin(PLStream *pls)
{
    TkwDev *dev;
    float pxlx, pxly;
    int xmin = 0;
    int xmax = PIXELS_X - 1;
    int ymin = 0;
    int ymax = PIXELS_Y - 1;

    dbug_enter("plD_init_tkw");

    pls->termin = 1;  /* Is an interactive terminal */
    pls->dev_flush = 1;  /* Handle our own flushes */
    pls->dev_fill0 = 1;  /* Handle solid fills */
    pls->plbuf_write = 1; /* Activate plot buffer */

    /* The real meat of the initialization done here */

    if (pls->dev == NULL)
	plD_open_tkwin(pls);

    dev = (TkwDev *) pls->dev;

    Init(pls);

    /* Get ready for plotting */

    dev->xlen = xmax - xmin;
    dev->ylen = ymax - ymin;

    dev->xscale_init = dev->init_width / (double) dev->xlen;
    dev->yscale_init = dev->init_height / (double) dev->ylen;

    dev->xscale = dev->xscale_init;
    dev->yscale = dev->yscale_init;

#if PHYSICAL
    pxlx = (PLFLT) ((double) PIXELS_X / dev->width * DPMM);
    pxly = (PLFLT) ((double) PIXELS_Y / dev->height * DPMM);
#else
    pxlx = (PLFLT) ((double) PIXELS_X / LPAGE_X);
    pxly = (PLFLT) ((double) PIXELS_Y / LPAGE_Y);
#endif

    plP_setpxl(pxlx, pxly);
    plP_setphy(xmin, xmax, ymin, ymax);
}

/*--------------------------------------------------------------------------*\
 * plD_open_tkwin()
 *
 * Performs basic driver initialization, without actually opening or
 * modifying a window. May be called by the outside world before plinit
 * in case the caller needs early access to the driver internals (not
 * very common -- currently only used externally by plplotter).
\*--------------------------------------------------------------------------*/

void
plD_open_tkwin(PLStream *pls)
{
    TkwDev *dev;
    TkwDisplay *tkwd;
    int i;

    dbug_enter("plD_open_tkw");

    /* Allocate and initialize device-specific data */

    if (pls->dev != NULL)
	plwarn("plD_open_tkw: device pointer is already set");

    pls->dev = (TkwDev*) calloc(1, (size_t) sizeof(TkwDev));
    if (pls->dev == NULL)
	plexit("plD_init_tkw: Out of memory.");

    dev = (TkwDev *) pls->dev;

    /* Variables used in querying the X server for events */

    dev->instr = 0;
    dev->max_instr = MAX_INSTR;

    /* See if display matches any already in use, and if so use that */

    dev->tkwd = NULL;
    for (i = 0; i < PLTKDISPLAYS; i++) {
	if (tkwDisplay[i] == NULL) {
	    continue;
	}
	else if (pls->FileName == NULL && tkwDisplay[i]->displayName == NULL) {
	    dev->tkwd = tkwDisplay[i];
	    break;
	}
	else if (pls->FileName == NULL || tkwDisplay[i]->displayName == NULL) {
	    continue;
	}
	else if (strcmp(tkwDisplay[i]->displayName, pls->FileName) == 0) {
	    dev->tkwd = tkwDisplay[i];
	    break;
	}
    }

    /* If no display matched, create a new one */

    if (dev->tkwd == NULL) {
	dev->tkwd = (TkwDisplay *) calloc(1, (size_t) sizeof(TkwDisplay));
	if (dev->tkwd == NULL)
	    plexit("Init: Out of memory.");

	for (i = 0; i < PLTKDISPLAYS; i++) {
	    if (tkwDisplay[i] == NULL)
		break;
	}
	if (i == PLTKDISPLAYS)
	    plexit("Init: Out of tkwDisplay's.");

	tkwDisplay[i] = tkwd = (TkwDisplay *) dev->tkwd;
	tkwd->nstreams = 1;

	/*
	 * If we don't have a tk widget we're being called on, then
	 * abort operations now
	 */
	if (pls->plPlotterPtr == NULL) {
	    fprintf(stderr, "No tk plframe widget to connect to\n");
	    exit(1);
	}
	/* Old version for MacOS Tk8.0 */
	/*
	 * char deflt[] = "Macintosh:0";
	 * pls->FileName = deflt;
	 * tkwd->display = (Display*) TkpOpenDisplay(pls->FileName);
	 */

	/* Open display */
#if defined(MAC_TCL) || defined(__WIN32__)
	if(!pls->FileName) {
	    /*
	     * Need to strdup because Tk has allocated the screen name,
	     * but we will actually 'free' it later ourselves, and therefore
	     * need to own the memory.
	     */
	    pls->FileName = strdup(TkGetDefaultScreenName(NULL,NULL));
	}
	tkwd->display = pls->plPlotterPtr->display;
#else
	tkwd->display = XOpenDisplay(pls->FileName);
#endif
	if (tkwd->display == NULL) {
	    fprintf(stderr, "Can't open display\n");
	    exit(1);
	}
	tkwd->displayName = pls->FileName;
	tkwd->screen = DefaultScreen(tkwd->display);
	if (synchronize) {
	    XSynchronize(tkwd->display, 1);
	}
	/* Get colormap and visual */

	tkwd->map = Tk_Colormap(pls->plPlotterPtr->tkwin);
	GetVisual(pls);

	/*
	 * Figure out if we have a color display or not.
	 * Default is color IF the user hasn't specified and IF the output device is
	 * not grayscale.
	 */

	if (pls->colorset)
	    tkwd->color = pls->color;
	else {
	    pls->color = 1;
	    tkwd->color = ! pltk_AreWeGrayscale(pls->plPlotterPtr);
	}

	/* Allocate & set background and foreground colors */

	AllocBGFG(pls);
	pltkwin_setBGFG(pls);
    }

    /* Display matched, so use existing display data */

    else {
	tkwd = (TkwDisplay *) dev->tkwd;
	tkwd->nstreams++;
    }
    tkwd->ixwd = i;
}

/*--------------------------------------------------------------------------*\
 * plD_line_tkwin()
 *
 * Draw a line in the current color from (x1,y1) to (x2,y2).
\*--------------------------------------------------------------------------*/

void
plD_line_tkwin(PLStream *pls, short x1a, short y1a, short x2a, short y2a)
{
    TkwDev *dev = (TkwDev *) pls->dev;
    TkwDisplay *tkwd = (TkwDisplay *) dev->tkwd;

    int x1 = x1a, y1 = y1a, x2 = x2a, y2 = y2a;

    if (dev->flags & 1) return;

    y1 = dev->ylen - y1;
    y2 = dev->ylen - y2;

    x1 = (int) (x1 * dev->xscale);
    x2 = (int) (x2 * dev->xscale);
    y1 = (int) (y1 * dev->yscale);
    y2 = (int) (y2 * dev->yscale);

    if (dev->write_to_window)
	XDrawLine(tkwd->display, dev->window, dev->gc, x1, y1, x2, y2);

    if (dev->write_to_pixmap)
	XDrawLine(tkwd->display, dev->pixmap, dev->gc, x1, y1, x2, y2);
}

/*--------------------------------------------------------------------------*\
 * plD_polyline_tkwin()
 *
 * Draw a polyline in the current color from (x1,y1) to (x2,y2).
\*--------------------------------------------------------------------------*/

void
plD_polyline_tkwin(PLStream *pls, short *xa, short *ya, PLINT npts)
{
    TkwDev *dev = (TkwDev *) pls->dev;
    TkwDisplay *tkwd = (TkwDisplay *) dev->tkwd;

    PLINT i;
    XPoint pts[PL_MAXPOLY];

    if (dev->flags & 1) return;

    if (npts > PL_MAXPOLY)
	plexit("plD_polyline_tkw: Too many points in polyline\n");

    for (i = 0; i < npts; i++) {
	pts[i].x = (short) (dev->xscale * xa[i]);
	pts[i].y = (short) (dev->yscale * (dev->ylen - ya[i]));
    }

    if (dev->write_to_window)
	XDrawLines(tkwd->display, dev->window, dev->gc, pts, npts,
		   CoordModeOrigin);

    if (dev->write_to_pixmap)
	XDrawLines(tkwd->display, dev->pixmap, dev->gc, pts, npts,
		   CoordModeOrigin);
}

/*--------------------------------------------------------------------------*\
 * plD_eop_tkwin()
 *
 * End of page. User must hit return (or third mouse button) to continue.
\*--------------------------------------------------------------------------*/

void
plD_eop_tkwin(PLStream *pls)
{
    TkwDev *dev = (TkwDev *) pls->dev;
    TkwDisplay *tkwd = (TkwDisplay *) dev->tkwd;

    dbug_enter("plD_eop_tkw");
    if (dev->flags & 1) return;

    XFlush(tkwd->display);
    if (pls->db)
	ExposeCmd(pls, NULL);

    if (!pls->nopause)
	WaitForPage(pls);

}

/*--------------------------------------------------------------------------*\
 * WaitForPage()
 *
 * This routine waits for the user to advance the plot, while handling
 * all other events.
\*--------------------------------------------------------------------------*/

static void
WaitForPage(PLStream *pls)
{
    PlPlotter *plf = pls->plPlotterPtr;
    TkwDev *dev = (TkwDev *) pls->dev;

    dbug_enter("WaitForPage");

    dev->flags &= 1;
    if (plf == NULL) {
	plwarn("WaitForPage: Illegal call --- driver can't find enclosing PlPlotter");
	return;
    }
    PlplotterAtEop(plf->interp, plf);

    while (!(dev->flags) && !Tcl_InterpDeleted(plf->interp) && (Tk_GetNumMainWindows() > 0)) {
    	Tcl_DoOneEvent(0);
    }

    if (Tcl_InterpDeleted(plf->interp) || (Tk_GetNumMainWindows() <= 0)) {
	dev->flags |= 1;
    }

    dev->flags &= 1;
}

/*--------------------------------------------------------------------------*\
 * plD_bop_tkwin()
 *
 * Set up for the next page.
\*--------------------------------------------------------------------------*/

void
plD_bop_tkwin(PLStream *pls)
{
    PlPlotter *plf = pls->plPlotterPtr;
    TkwDev *dev = (TkwDev *) pls->dev;
    TkwDisplay *tkwd = (TkwDisplay *) dev->tkwd;

    XRectangle xrect;
    xrect.x = 0; xrect.y = 0;
    xrect.width = dev->width; xrect.height = dev->height;

    dbug_enter("plD_bop_tkw");
    if (dev->flags & 1) return;

    if (dev->write_to_window) {
#ifdef MAC_TCL
	/* MacTk only has these X calls */
	XSetForeground(tkwd->display, dev->gc, tkwd->cmap0[0].pixel);
	XFillRectangles(tkwd->display, dev->window, dev->gc, &xrect, 1);
	XSetForeground(tkwd->display, dev->gc, dev->curcolor.pixel);
#else
	XClearWindow(tkwd->display, dev->window);
#endif
    }
    if (dev->write_to_pixmap) {
	XSetForeground(tkwd->display, dev->gc, tkwd->cmap0[0].pixel);
	XFillRectangles(tkwd->display, dev->pixmap, dev->gc, &xrect, 1);
	XSetForeground(tkwd->display, dev->gc, dev->curcolor.pixel);
    }
    XSync(tkwd->display, 0);
    pls->page++;
	PlplotterAtBop(plf->interp, plf);
}

/*--------------------------------------------------------------------------*\
 * plD_tidy_tkwin()
 *
 * Close graphics file
\*--------------------------------------------------------------------------*/

void
plD_tidy_tkwin(PLStream *pls)
{
    TkwDev *dev = (TkwDev *) pls->dev;
    TkwDisplay *tkwd = (TkwDisplay *) dev->tkwd;

    dbug_enter("plD_tidy_tkw");

    tkwd->nstreams--;
    if (tkwd->nstreams == 0) {
	int ixwd = tkwd->ixwd;
	XFreeGC(tkwd->display, dev->gc);
#if !defined(MAC_TCL) && !defined(__WIN32__)
	XCloseDisplay(tkwd->display);
#endif
	free_mem(tkwDisplay[ixwd]);
    }
    /*
     * Vince removed this November 1999.  It seems as if a simple
     * 'plframe .p ; destroy .p' leaves a temporary buf file open
     * if we clear this flag here.  It should be checked and then
     * cleared by whoever called us.  An alternative fix would
     * be to carry out the check/tidy here.  The plframe widget
     * handles this stuff for us.
     */
    /* pls->plbuf_write = 0; */
}

/*--------------------------------------------------------------------------*\
 * plD_state_tkwin()
 *
 * Handle change in PLStream state (color, pen width, fill attribute, etc).
\*--------------------------------------------------------------------------*/

void
plD_state_tkwin(PLStream *pls, PLINT op)
{
    TkwDev *dev = (TkwDev *) pls->dev;
    TkwDisplay *tkwd = (TkwDisplay *) dev->tkwd;
    dbug_enter("plD_state_tkw");
    if (dev->flags & 1) return;

    switch (op) {

	case PLSTATE_WIDTH:
	    break;

	case PLSTATE_COLOR0:{
	    int icol0 = pls->icol0;
	    if (tkwd->color) {
		if (icol0 == PL_RGB_COLOR) {
		    PLColor_to_TkColor(&pls->curcolor, &dev->curcolor);
		    Tkw_StoreColor(pls, tkwd, &dev->curcolor);
		} else {
		    dev->curcolor = tkwd->cmap0[icol0];
		}
		XSetForeground(tkwd->display, dev->gc, dev->curcolor.pixel);
	    } else {
		dev->curcolor = tkwd->fgcolor;
		XSetForeground(tkwd->display, dev->gc, dev->curcolor.pixel);
	    }
	    break;
	}

	case PLSTATE_COLOR1:{
	    int icol1;

	    if (tkwd->ncol1 == 0)
		AllocCmap1(pls);

	    if (tkwd->ncol1 < 2)
		break;

	    icol1 = (pls->icol1 * (tkwd->ncol1-1)) / (pls->ncol1-1);
	    if (tkwd->color)
		dev->curcolor = tkwd->cmap1[icol1];
	    else
		dev->curcolor = tkwd->fgcolor;

	    XSetForeground(tkwd->display, dev->gc, dev->curcolor.pixel);
	    break;
	}

	case PLSTATE_CMAP0:
	    pltkwin_setBGFG(pls);
	    StoreCmap0(pls);
	    break;

	case PLSTATE_CMAP1:
	    StoreCmap1(pls);
	    break;
    }
}

/*--------------------------------------------------------------------------*\
 * plD_esc_tkwin()
 *
 * Escape function.
 *
 * Functions:
 *
 * PLESC_EH Handle pending events
 * PLESC_EXPOSE Force an expose
 * PLESC_FILL Fill polygon
 * PLESC_FLUSH Flush X event buffer
 * PLESC_GETC Get coordinates upon mouse click
 * PLESC_REDRAW Force a redraw
 * PLESC_RESIZE Force a resize
\*--------------------------------------------------------------------------*/

void
plD_esc_tkwin(PLStream *pls, PLINT op, void *ptr)
{
    TkwDev *dev = (TkwDev *) pls->dev;
#ifndef USE_TK
    TkwDisplay *tkwd = (TkwDisplay *) dev->tkwd;
#endif
    dbug_enter("plD_esc_tkw");
    if (dev->flags & 1) return;

    switch (op) {
	case PLESC_EH:
#ifndef USE_TK
	    HandleEvents(pls);
#endif
	    break;

	case PLESC_EXPOSE:
	    ExposeCmd(pls, (PLDisplay *) ptr);
	    break;

	case PLESC_FILL:
	    FillPolygonCmd(pls);
	    break;

	case PLESC_FLUSH:
#ifndef USE_TK
	    HandleEvents(pls);
	    XFlush(tkwd->display);
#endif
	    break;

	case PLESC_GETC:
#ifndef USE_TK
	    GetCursorCmd(pls, (PLGraphicsIn *) ptr);
#endif
	    break;

	case PLESC_REDRAW:
	    RedrawCmd(pls);
	    break;

	case PLESC_RESIZE:
	    ResizeCmd(pls, (PLDisplay *) ptr);
	    break;

/* Added by Vince, disabled by default since we want a minimal patch */
#ifdef USING_PLESC_COPY
	case PLESC_COPY:
	    CopyCommand(pls);
	    break;
#endif
    }
}

#ifdef USING_PLESC_COPY
/*--------------------------------------------------------------------------*\
 * CopyCommand()
 *
 * Copy a rectangle to a new part of the image.
 * Points described in first 3 elements of pls->dev_x[] and pls->dev_y[].
\*--------------------------------------------------------------------------*/

static void
CopyCommand (PLStream *pls)
{
    int x0,w,x1,y0,h,y1;
    TkwDev *dev = (TkwDev *) pls->dev;
    TkwDisplay *tkwd = (TkwDisplay *) dev->tkwd;

    x0 = (int) (dev->xscale * pls->dev_x[0]);
    x1 = (int) (dev->xscale * pls->dev_x[2]);
    y0 = (int) (dev->yscale * (dev->ylen - pls->dev_y[0]));
    y1 = (int) (dev->yscale * (dev->ylen - pls->dev_y[2]));
    w = (int) (dev->xscale * (pls->dev_x[1]-pls->dev_x[0]));
    h = (int) (-dev->yscale * (pls->dev_y[1]-pls->dev_y[0]));

    if (dev->write_to_window)
	XCopyArea(tkwd->display, dev->window, dev->window, dev->gc,
		  x0,y0,w,h,x1,y1);

    if (dev->write_to_pixmap)
	XCopyArea(tkwd->display, dev->pixmap, dev->pixmap, dev->gc,
		  x0,y0,w,h,x1,y1);
}
#endif

/*--------------------------------------------------------------------------*\
 * FillPolygonCmd()
 *
 * Fill polygon described in points pls->dev_x[] and pls->dev_y[].
 * Only solid color fill supported.
\*--------------------------------------------------------------------------*/

static void
FillPolygonCmd(PLStream *pls)
{
    TkwDev *dev = (TkwDev *) pls->dev;
    TkwDisplay *tkwd = (TkwDisplay *) dev->tkwd;
    XPoint pts[PL_MAXPOLY];
    int i;

    if (pls->dev_npts > PL_MAXPOLY)
	plexit("FillPolygonCmd: Too many points in polygon\n");


    for (i = 0; i < pls->dev_npts; i++) {
	pts[i].x = (short) (dev->xscale * pls->dev_x[i]);
	pts[i].y = (short) (dev->yscale * (dev->ylen - pls->dev_y[i]));
    }

/* Fill polygons */

    if (dev->write_to_window)
	XFillPolygon(tkwd->display, dev->window, dev->gc,
		     pts, pls->dev_npts, Nonconvex, CoordModeOrigin);

    if (dev->write_to_pixmap)
	XFillPolygon(tkwd->display, dev->pixmap, dev->gc,
		     pts, pls->dev_npts, Nonconvex, CoordModeOrigin);

/* If in debug mode, draw outline of boxes being filled */

#ifdef DEBUG
    if (pls->debug) {
	XSetForeground(tkwd->display, dev->gc, tkwd->fgcolor.pixel);
	if (dev->write_to_window)
	    XDrawLines(tkwd->display, dev->window, dev->gc, pts, pls->dev_npts,
		       CoordModeOrigin);

	if (dev->write_to_pixmap)
	    XDrawLines(tkwd->display, dev->pixmap, dev->gc, pts, pls->dev_npts,
		       CoordModeOrigin);

	XSetForeground(tkwd->display, dev->gc, dev->curcolor.pixel);
    }
#endif
}

/*--------------------------------------------------------------------------*\
 * Init()
 *
 * Xlib initialization routine.
 *
 * Controlling routine for X window creation and/or initialization.
 * The user may customize the window in the following ways:
 *
 * display: pls->OutFile (use plsfnam() or -display option)
 * size: pls->xlength, pls->ylength (use plspage() or -geo option)
 * bg color: pls->cmap0[0] (use plscolbg() or -bg option)
\*--------------------------------------------------------------------------*/

static void
Init(PLStream *pls)
{
    PlPlotter *plf;
    TkwDev *dev = (TkwDev *) pls->dev;
    TkwDisplay *tkwd = (TkwDisplay *) dev->tkwd;

    dbug_enter("Init");

    dev->window = pls->window_id;

    plf = pls->plPlotterPtr;
    if (plf == NULL) {
	plwarn("Init: Illegal call --- driver can't find enclosing PlPlotter");
	return;
    }

/* Initialize colors */
    InitColors(pls);
#ifndef MAC_TCL
    XSetWindowColormap( tkwd->display, dev->window, tkwd->map );
#else
#endif

/* Set up GC for ordinary draws */
    if ( ! dev->gc)
	dev->gc = XCreateGC(tkwd->display, dev->window, 0, 0);

/* Set up GC for rubber-band draws */
    if ( ! tkwd->gcXor) {
	XGCValues gcValues;
	unsigned long mask;

	gcValues.background = tkwd->cmap0[0].pixel;
	gcValues.foreground = 0xFF;
	gcValues.function = GXxor;
	mask = GCForeground | GCBackground | GCFunction;

	tkwd->gcXor = XCreateGC(tkwd->display, dev->window, mask, &gcValues);
    }

/* Get initial drawing area dimensions */
    dev->width = Tk_Width(plf->tkwin);
    dev->height = Tk_Height(plf->tkwin);
    dev->border = Tk_InternalBorderWidth(plf->tkwin);
    tkwd->depth = Tk_Depth(plf->tkwin);

    dev->init_width = dev->width;
    dev->init_height = dev->height;

    /* Set up flags that determine what we are writing to */
    /* If nopixmap is set, ignore db */

    if (pls->nopixmap) {
	dev->write_to_pixmap = 0;
	pls->db = 0;
    } else {
	dev->write_to_pixmap = 1;
    }
    dev->write_to_window = ! pls->db;

    /* Create pixmap for holding plot image (for expose events). */

    if (dev->write_to_pixmap)
	CreatePixmap(pls);

    /* Set drawing color */

    plD_state_tkwin(pls, PLSTATE_COLOR0);

    XSetWindowBackground(tkwd->display, dev->window, tkwd->cmap0[0].pixel);
    XSetBackground(tkwd->display, dev->gc, tkwd->cmap0[0].pixel);

}

/*--------------------------------------------------------------------------*\
 * ExposeCmd()
 *
 * Event handler routine for expose events.
 * These are "pure" exposures (no resize), so don't need to clear window.
\*--------------------------------------------------------------------------*/

static void
ExposeCmd(PLStream *pls, PLDisplay *pldis)
{
    TkwDev *dev = (TkwDev *) pls->dev;
    TkwDisplay *tkwd = (TkwDisplay *) dev->tkwd;
    int x, y, width, height;

    dbug_enter("ExposeCmd");

    /* Return if plD_init_tkw hasn't been called yet */

    if (dev == NULL) {
	plwarn("ExposeCmd: Illegal call -- driver uninitialized");
	return;
    }

    /* Exposed area. If unspecified, the entire window is used. */

    if (pldis == NULL) {
	x = 0;
	y = 0;
	width = dev->width;
	height = dev->height;
    } else {
	x = pldis->x;
	y = pldis->y;
	width = pldis->width;
	height = pldis->height;
    }

    /* Usual case: refresh window from pixmap */
    /* DEBUG option: draws rectangle around refreshed region */

    XSync(tkwd->display, 0);
    if (dev->write_to_pixmap) {
	XCopyArea(tkwd->display, dev->pixmap, dev->window, dev->gc,
		  x, y, width, height, x, y);
	XSync(tkwd->display, 0);
#ifdef DEBUG
	if (pls->debug) {
	    XPoint pts[5];
	    int x0 = x, x1 = x+width, y0 = y, y1 = y+height;
	    pts[0].x = x0; pts[0].y = y0;
	    pts[1].x = x1; pts[1].y = y0;
	    pts[2].x = x1; pts[2].y = y1;
	    pts[3].x = x0; pts[3].y = y1;
	    pts[4].x = x0; pts[4].y = y0;

	    XDrawLines(tkwd->display, dev->window, dev->gc, pts, 5,
		       CoordModeOrigin);
	}
#endif
    } else {
	plRemakePlot(pls);
	XFlush(tkwd->display);
    }
}

/*--------------------------------------------------------------------------*\
 * ResizeCmd()
 *
 * Event handler routine for resize events.
\*--------------------------------------------------------------------------*/

static void
ResizeCmd(PLStream *pls, PLDisplay *pldis)
{
    TkwDev *dev = (TkwDev *) pls->dev;
    TkwDisplay *tkwd = (TkwDisplay *) dev->tkwd;
    int write_to_window = dev->write_to_window;

    dbug_enter("ResizeCmd");

    /* Return if plD_init_tkw hasn't been called yet */

    if (dev == NULL) {
	plwarn("ResizeCmd: Illegal call -- driver uninitialized");
	return;
    }

    /* Return if pointer to window not specified. */

    if (pldis == NULL) {
	plwarn("ResizeCmd: Illegal call -- window pointer uninitialized");
	return;
    }

    /* Reset current window bounds */

    dev->width = pldis->width;
    dev->height = pldis->height;

    dev->xscale = dev->width / (double) dev->init_width;
    dev->yscale = dev->height / (double) dev->init_height;

    dev->xscale = dev->xscale * dev->xscale_init;
    dev->yscale = dev->yscale * dev->yscale_init;

#if PHYSICAL
    {
	float pxlx = (double) PIXELS_X / dev->width * DPMM;
	float pxly = (double) PIXELS_Y / dev->height * DPMM;
	plP_setpxl(pxlx, pxly);
    }
#endif

    /* Note: the following order MUST be obeyed -- if you instead redraw into
     * the window and then copy it to the pixmap, off-screen parts of the window
     * may contain garbage which is then transferred to the pixmap (and thus
     * will not go away after an expose).
     */

    /* Resize pixmap using new dimensions */

    if (dev->write_to_pixmap) {
	dev->write_to_window = 0;
#if defined(__WIN32__) || defined(MAC_TCL)
	Tk_FreePixmap(tkwd->display, dev->pixmap);
#else
	/* Vince's original driver code used
	 * Tk_FreePixmap(tkwd->display, dev->pixmap);
	 *which is defined in tk-8.3 (and 8.2?) source as
	 *void
	 * Tk_FreePixmap(display, pixmap)
	 *     Display *display;
	 *     Pixmap pixmap;
	 * {
	 *    XFreePixmap(display, pixmap);
	 *    Tk_FreeXId(display, (XID) pixmap);
	 * }
	 * But that bombed under Linux and tcl/tk8.2 so now just call
	 * XFreePixmap directly.  (Not recommended as permanent solution
	 * because you eventually run out of resources according to man
	 * page if you don't call Tk_FreeXId.)  Vince is still looking into
	 * how to resolve this problem.
	 */
	XFreePixmap(tkwd->display, dev->pixmap);
#endif
	CreatePixmap(pls);
    }

    /* Initialize & redraw (to pixmap, if available). */

    plD_bop_tkwin(pls);
    plRemakePlot(pls);
    XSync(tkwd->display, 0);

    /* If pixmap available, fake an expose */

    if (dev->write_to_pixmap) {
	dev->write_to_window = write_to_window;
	XCopyArea(tkwd->display, dev->pixmap, dev->window, dev->gc, 0, 0,
		  dev->width, dev->height, 0, 0);
	XSync(tkwd->display, 0);
    }
}

/*--------------------------------------------------------------------------*\
 * RedrawCmd()
 *
 * Handles page redraw without resize (pixmap does not get reallocated).
 * Calling this makes sure all necessary housekeeping gets done.
\*--------------------------------------------------------------------------*/

static void
RedrawCmd(PLStream *pls)
{
    TkwDev *dev = (TkwDev *) pls->dev;
    TkwDisplay *tkwd = (TkwDisplay *) dev->tkwd;
    int write_to_window = dev->write_to_window;

    dbug_enter("RedrawCmd");

    /* Return if plD_init_tkw hasn't been called yet */

    if (dev == NULL) {
	plwarn("RedrawCmd: Illegal call -- driver uninitialized");
	return;
    }

    /* Initialize & redraw (to pixmap, if available). */

    if (dev->write_to_pixmap)
	dev->write_to_window = 0;

    plD_bop_tkwin(pls);
    plRemakePlot(pls);
    XSync(tkwd->display, 0);

    dev->write_to_window = write_to_window;

    /* If pixmap available, fake an expose */

    if (dev->write_to_pixmap) {
	XCopyArea(tkwd->display, dev->pixmap, dev->window, dev->gc, 0, 0,
		  dev->width, dev->height, 0, 0);
	XSync(tkwd->display, 0);
    }
}

/*--------------------------------------------------------------------------*\
 * CreatePixmap()
 *
 * This routine creates a pixmap, doing error trapping in case there
 * isn't enough memory on the server.
\*--------------------------------------------------------------------------*/

static void
CreatePixmap(PLStream *pls)
{
    TkwDev *dev = (TkwDev *) pls->dev;
    TkwDisplay *tkwd = (TkwDisplay *) dev->tkwd;
    Tk_Window tkwin = pls->plPlotterPtr->tkwin;

#if !defined(MAC_TCL) && !defined(__WIN32__)
    int (*oldErrorHandler)();
    oldErrorHandler = XSetErrorHandler(CreatePixmapErrorHandler);
    CreatePixmapStatus = Success;
#endif

#ifdef MAC_TCL
    /* MAC_TCL's version of XCreatePixmap doesn't like 0 by 0 maps */
    if(dev->width == 0) { dev->width = 10; }
    if(dev->height == 0) { dev->height = 10; }
#endif
    pldebug("CreatePixmap",
	    "creating pixmap: width = %d, height = %d, depth = %d\n",
	    dev->width, dev->height, tkwd->depth);
/*
dev->pixmap = Tk_GetPixmap(tkwd->display, dev->window,
			       dev->width, dev->height, tkwd->depth);
*/
/*
 * Vince's original driver code used Tk_Display(tkwin) for first argument,
 * but that bombed on an Linux tcl/tk 8.2 machine.  Something was wrong
 * with that value.  Thus, we now use tkwd->display, and that works well.
 * Vince is looking into why Tk_Display(tkwin) is badly defined under 8.2.
 * old code:

    dev->pixmap = Tk_GetPixmap(Tk_Display(tkwin), Tk_WindowId(tkwin),
	    Tk_Width(tkwin), Tk_Height(tkwin),
	    DefaultDepthOfScreen(Tk_Screen(tkwin)));
 */
    dev->pixmap = Tk_GetPixmap(tkwd->display, Tk_WindowId(tkwin),
	    Tk_Width(tkwin), Tk_Height(tkwin),
	    DefaultDepthOfScreen(Tk_Screen(tkwin)));
    XSync(tkwd->display, 0);
#if !defined(MAC_TCL) && !defined(__WIN32__)
    if (CreatePixmapStatus != Success) {
	dev->write_to_pixmap = 0;
	dev->write_to_window = 1;
	pls->db = 0;
	fprintf(stderr, "\n\
      Warning: pixmap could not be allocated (insufficient memory on server).\n\
      Driver will redraw the entire plot to handle expose events.\n");
    }

    XSetErrorHandler(oldErrorHandler);
#endif
}

/*--------------------------------------------------------------------------*\
 * GetVisual()
 *
 * Get visual info. In order to safely use a visual other than that of
 * the parent (which hopefully is that returned by DefaultVisual), you
 * must first find (using XGetRGBColormaps) or create a colormap matching
 * this visual and then set the colormap window attribute in the
 * XCreateWindow attributes and valuemask arguments. I don't do this
 * right now, so this is turned off by default.
\*--------------------------------------------------------------------------*/

static void
GetVisual(PLStream *pls)
{
    int depth;
    TkwDev *dev = (TkwDev *) pls->dev;
    TkwDisplay *tkwd = (TkwDisplay *) dev->tkwd;

    dbug_enter("GetVisual");

    tkwd->visual = Tk_GetVisual(pls->plPlotterPtr->interp,
				pls->plPlotterPtr->tkwin,
				"best",
				&depth, NULL);
    tkwd->depth = depth;

}

/*--------------------------------------------------------------------------*\
 * AllocBGFG()
 *
 * Allocate background & foreground colors. If possible, I choose pixel
 * values such that the fg pixel is the xor of the bg pixel, to make
 * rubber-banding easy to see.
\*--------------------------------------------------------------------------*/

static void
AllocBGFG(PLStream *pls)
{
    TkwDev *dev = (TkwDev *) pls->dev;
    TkwDisplay *tkwd = (TkwDisplay *) dev->tkwd;

#ifndef USE_TK
    int i, j, npixels;
    unsigned long plane_masks[1], pixels[MAX_COLORS];
#endif

    dbug_enter("AllocBGFG");

    /* If not on a color system, just return */

    if ( ! tkwd->color)
	return;
#ifndef USE_TK
    /* Allocate r/w color cell for background */

    if (XAllocColorCells(tkwd->display, tkwd->map, False,
			 plane_masks, 0, pixels, 1)) {
	tkwd->cmap0[0].pixel = pixels[0];
    } else {
	plexit("couldn't allocate background color cell");
    }

    /* Allocate as many colors as we can */

    npixels = MAX_COLORS;
    for (;;) {
	if (XAllocColorCells(tkwd->display, tkwd->map, False,
			     plane_masks, 0, pixels, npixels))
	break;
	npixels--;
	if (npixels == 0)
	    break;
    }

    /* Find the color with pixel = xor of the bg color pixel. */
    /* If a match isn't found, the last pixel allocated is used. */

    for (i = 0; i < npixels-1; i++) {
	if (pixels[i] == (~tkwd->cmap0[0].pixel & 0xFF))
	    break;
    }

    /* Use this color cell for our foreground color. Then free the rest. */

    tkwd->fgcolor.pixel = pixels[i];
    for (j = 0; j < npixels; j++) {
	if (j != i)
	    XFreeColors(tkwd->display, tkwd->map, &pixels[j], 1, 0);
    }
#endif
}

/*--------------------------------------------------------------------------*\
 * pltkwin_setBGFG()
 *
 * Set background & foreground colors. Foreground over background should
 * have high contrast.
\*--------------------------------------------------------------------------*/

void
pltkwin_setBGFG(PLStream *pls)
{
    TkwDev *dev = (TkwDev *) pls->dev;
    TkwDisplay *tkwd = (TkwDisplay *) dev->tkwd;
    PLColor fgcolor;
    int gslevbg, gslevfg;

    dbug_enter("pltkwin_setBGFG");

    /*
     * Set background color.
     *
     * Background defaults to black on color screens, white on grayscale (many
     * grayscale monitors have poor contrast, and black-on-white looks better).
     */

    if ( ! tkwd->color) {
	pls->cmap0[0].r = pls->cmap0[0].g = pls->cmap0[0].b = 0xFF;
    }
    gslevbg = ((long) pls->cmap0[0].r +
	       (long) pls->cmap0[0].g +
	       (long) pls->cmap0[0].b) / 3;

    PLColor_to_TkColor(&pls->cmap0[0], &tkwd->cmap0[0]);

    /*
     * Set foreground color.
     *
     * Used for grayscale output, since otherwise the plots can become nearly
     * unreadable (i.e. if colors get mapped onto grayscale values). In this
     * case it becomes the grayscale level for all draws, and is taken to be
     * black if the background is light, and white if the background is dark.
     * Note that white/black allocations never fail.
     */

    if (gslevbg > 0x7F)
	gslevfg = 0;
    else
	gslevfg = 0xFF;

    fgcolor.r = fgcolor.g = fgcolor.b = gslevfg;

    PLColor_to_TkColor(&fgcolor, &tkwd->fgcolor);

    /* Now store */
#ifndef USE_TK
    if (tkwd->color) {
	XStoreColor(tkwd->display, tkwd->map, &tkwd->fgcolor);
	XStoreColor(tkwd->display, tkwd->map, &tkwd->cmap0[0]);
    } else {
	XAllocColor(tkwd->display, tkwd->map, &tkwd->cmap0[0]);
	XAllocColor(tkwd->display, tkwd->map, &tkwd->fgcolor);
    }
#else
    Tkw_StoreColor(pls, tkwd, &tkwd->cmap0[0]);
    Tkw_StoreColor(pls, tkwd, &tkwd->fgcolor);
#endif
}

/*--------------------------------------------------------------------------*\
 * InitColors()
 *
 * Does all color initialization.
\*--------------------------------------------------------------------------*/

static void
InitColors(PLStream *pls)
{
    TkwDev *dev = (TkwDev *) pls->dev;
    TkwDisplay *tkwd = (TkwDisplay *) dev->tkwd;

    dbug_enter("InitColors");

    /* Allocate and initialize color maps. */
    /* Defer cmap1 allocation until it's actually used */

    if (tkwd->color) {
	if (plplot_tkwin_ccmap) {
	    AllocCustomMap(pls);
	} else {
	    AllocCmap0(pls);
	}
    }
}

/*--------------------------------------------------------------------------*\
 * AllocCustomMap()
 *
 * Initializes custom color map and all the cruft that goes with it.
 *
 * Assuming all color X displays do 256 colors, the breakdown is as follows:
 *
 * XWM_COLORS Number of low "pixel" values to copy. These are typically
 *  allocated first, thus are in use by the window manager. I
 *  copy them to reduce flicker.
 *
 * CMAP0_COLORS Color map 0 entries. I allocate these both in the default
 *  colormap and the custom colormap to reduce flicker.
 *
 * CMAP1_COLORS Color map 1 entries. There should be as many as practical
 *  available for smooth shading. On the order of 50-100 is
 *  pretty reasonable. You don't really need all 256,
 *  especially if all you're going to do is to print it to
 *  postscript (which doesn't have any intrinsic limitation on
 *  the number of colors).
 *
 * It's important to leave some extra colors unallocated for Tk. In
 * particular the palette tools require a fair amount. I recommend leaving
 * at least 40 or so free.
\*--------------------------------------------------------------------------*/

static void
AllocCustomMap(PLStream *pls)
{
    TkwDev *dev = (TkwDev *) pls->dev;
    TkwDisplay *tkwd = (TkwDisplay *) dev->tkwd;

    XColor xwm_colors[MAX_COLORS];
    int i;
#ifndef USE_TK
    int npixels;
    unsigned long plane_masks[1], pixels[MAX_COLORS];
#endif

    dbug_enter("AllocCustomMap");

    /* Determine current default colors */

    for (i = 0; i < MAX_COLORS; i++) {
	xwm_colors[i].pixel = i;
    }
#ifndef MAC_TCL
    XQueryColors(tkwd->display, tkwd->map, xwm_colors, MAX_COLORS);
#endif

  /* Allocate cmap0 colors in the default colormap.
   * The custom cmap0 colors are later stored at the same pixel values.
   * This is a really cool trick to reduce the flicker when changing colormaps.
   */

    AllocCmap0(pls);
    XAllocColor(tkwd->display, tkwd->map, &tkwd->fgcolor);

    /* Create new color map */

    tkwd->map = XCreateColormap( tkwd->display, DefaultRootWindow(tkwd->display),
				tkwd->visual, AllocNone );

    /* Now allocate all colors so we can fill the ones we want to copy */

#ifndef USE_TK
    npixels = MAX_COLORS;
    for (;;) {
	if (XAllocColorCells(tkwd->display, tkwd->map, False,
			     plane_masks, 0, pixels, npixels))
	break;
	npixels--;
	if (npixels == 0)
	    plexit("couldn't allocate any colors");
    }

    /* Fill the low colors since those are in use by the window manager */

    for (i = 0; i < XWM_COLORS; i++) {
	XStoreColor(tkwd->display, tkwd->map, &xwm_colors[i]);
	pixels[xwm_colors[i].pixel] = 0;
    }

    /* Fill the ones we will use in cmap0 */

    for (i = 0; i < tkwd->ncol0; i++) {
	XStoreColor(tkwd->display, tkwd->map, &tkwd->cmap0[i]);
	pixels[tkwd->cmap0[i].pixel] = 0;
    }

  /* Finally, if the colormap was saved by an external agent, see if there are
   * any differences from the current default map and save those! A very cool
   * (or sick, depending on how you look at it) trick to get over some X and
   * Tk limitations.
   */

    if (sxwm_colors_set) {
	for (i = 0; i < MAX_COLORS; i++) {
	    if ((xwm_colors[i].red != sxwm_colors[i].red) ||
		(xwm_colors[i].green != sxwm_colors[i].green) ||
		(xwm_colors[i].blue != sxwm_colors[i].blue) ) {

		if (pixels[i] != 0) {
		    XStoreColor(tkwd->display, tkwd->map, &xwm_colors[i]);
		    pixels[i] = 0;
		}
	    }
	}
    }

    /* Now free the ones we're not interested in */

    for (i = 0; i < npixels; i++) {
	if (pixels[i] != 0)
	    XFreeColors(tkwd->display, tkwd->map, &pixels[i], 1, 0);
    }
#endif
    /* Allocate colors in cmap 1 */

    AllocCmap1(pls);
}

/*--------------------------------------------------------------------------*\
 * AllocCmap0()
 *
 * Allocate & initialize cmap0 entries.
\*--------------------------------------------------------------------------*/

static void
AllocCmap0(PLStream *pls)
{
    TkwDev *dev = (TkwDev *) pls->dev;
    TkwDisplay *tkwd = (TkwDisplay *) dev->tkwd;

#ifndef USE_TK
    int npixels;
    int i;
    unsigned long plane_masks[1], pixels[MAX_COLORS];
#endif

    dbug_enter("AllocCmap0");

    /* Allocate and assign colors in cmap 0 */

#ifndef USE_TK
    npixels = pls->ncol0-1;
    for (;;) {
	if (XAllocColorCells(tkwd->display, tkwd->map, False,
			     plane_masks, 0, &pixels[1], npixels))
	break;
	npixels--;
	if (npixels == 0)
	    plexit("couldn't allocate any colors");
    }

    tkwd->ncol0 = npixels+1;
    for (i = 1; i < tkwd->ncol0; i++) {
	tkwd->cmap0[i].pixel = pixels[i];
    }
#else
    /* We use the Tk color scheme */
    tkwd->ncol0 = pls->ncol0;
#endif
    StoreCmap0(pls);
}

/*--------------------------------------------------------------------------*\
 * AllocCmap1()
 *
 * Allocate & initialize cmap1 entries. If using the default color map,
 * must severely limit number of colors otherwise TK won't have enough.
\*--------------------------------------------------------------------------*/

static void
AllocCmap1(PLStream *pls)
{
    TkwDev *dev = (TkwDev *) pls->dev;
    TkwDisplay *tkwd = (TkwDisplay *) dev->tkwd;

    int npixels;
#ifndef USE_TK
    int i, j;
    unsigned long plane_masks[1], pixels[MAX_COLORS];
#endif

    dbug_enter("AllocCmap1");

    /* Allocate colors in cmap 1 */

    npixels = MAX(2, MIN(CMAP1_COLORS, pls->ncol1));
#ifndef USE_TK
    for (;;) {
	if (XAllocColorCells(tkwd->display, tkwd->map, False,
			     plane_masks, 0, pixels, npixels))
	break;
	npixels--;
	if (npixels == 0)
	    break;
    }

    if (npixels < 2) {
	tkwd->ncol1 = -1;
	fprintf(stderr,
		"Warning: unable to allocate sufficient colors in cmap1\n");
	return;
    } else {
	tkwd->ncol1 = npixels;
	if (pls->verbose)
	    fprintf(stderr, "AllocCmap1 (xwin.c): Allocated %d colors in cmap1\n", npixels);
    }

    /* Don't assign pixels sequentially, to avoid strange problems with xor GC's */
    /* Skipping by 2 seems to do the job best */

    for (j = i = 0; i < tkwd->ncol1; i++) {
	while (pixels[j] == 0)
	    j++;

	tkwd->cmap1[i].pixel = pixels[j];
	pixels[j] = 0;

	j += 2;
	if (j >= tkwd->ncol1)
	    j = 0;
    }
#else
    tkwd->ncol1 = npixels;
#endif
    StoreCmap1(pls);
}

/*--------------------------------------------------------------------------*\
 * StoreCmap0()
 *
 * Stores cmap 0 entries in X-server colormap.
\*--------------------------------------------------------------------------*/

static void
StoreCmap0(PLStream *pls)
{
    TkwDev *dev = (TkwDev *) pls->dev;
    TkwDisplay *tkwd = (TkwDisplay *) dev->tkwd;
    int i;

    if ( ! tkwd->color)
	return;

    for (i = 1; i < tkwd->ncol0; i++) {
	PLColor_to_TkColor(&pls->cmap0[i], &tkwd->cmap0[i]);
#ifndef USE_TK
	XStoreColor(tkwd->display, tkwd->map, &tkwd->cmap0[i]);
#else
	Tkw_StoreColor(pls, tkwd, &tkwd->cmap0[i]);
#endif
    }
}

void CopyColour(XColor* from, XColor* to) {
    to->pixel = from->pixel;
    to->red = from->red;
    to->blue = from->blue;
    to->green = from->green;
    to->flags = from->flags;
}

/*--------------------------------------------------------------------------*\
 * StoreCmap1()
 *
 * Stores cmap 1 entries in X-server colormap.
\*--------------------------------------------------------------------------*/

static void
StoreCmap1(PLStream *pls)
{
    TkwDev *dev = (TkwDev *) pls->dev;
    TkwDisplay *tkwd = (TkwDisplay *) dev->tkwd;

    PLColor cmap1color;
    int i;

    if ( ! tkwd->color)
	return;

    for (i = 0; i < tkwd->ncol1; i++) {
	plcol_interp(pls, &cmap1color, i, tkwd->ncol1);
	PLColor_to_TkColor(&cmap1color, &tkwd->cmap1[i]);
#ifndef USE_TK
	XStoreColor(tkwd->display, tkwd->map, &tkwd->cmap1[i]);
#else
	Tkw_StoreColor(pls, tkwd, &tkwd->cmap1[i]);
#endif
    }
}

void Tkw_StoreColor(PLStream* pls, TkwDisplay* tkwd, XColor* col) {
    XColor *xc;
#ifndef USE_TK
    XStoreColor(tkwd->display, tkwd->map, col);
#else
    /* We're probably losing memory here */
    xc = Tk_GetColorByValue(pls->plPlotterPtr->tkwin,col);
    CopyColour(xc,col);
#endif
}

/*--------------------------------------------------------------------------*\
 * void PLColor_to_TkColor()
 *
 * Copies the supplied PLColor to an XColor, padding with bits as necessary
 * (a PLColor uses 8 bits for color storage, while an XColor uses 16 bits).
 * The argument types follow the same order as in the function name.
\*--------------------------------------------------------------------------*/

#define ToXColor(a) (((0xFF & (a)) << 8) | (a))
#define ToPLColor(a) (((U_LONG) a) >> 8)

void
PLColor_to_TkColor(PLColor *plcolor, XColor *xcolor)
{
    xcolor->red = ToXColor(plcolor->r);
    xcolor->green = ToXColor(plcolor->g);
    xcolor->blue = ToXColor(plcolor->b);
    xcolor->flags = DoRed | DoGreen | DoBlue;
}

/*--------------------------------------------------------------------------*\
 * void PLColor_from_TkColor()
 *
 * Copies the supplied XColor to a PLColor, stripping off bits as
 * necessary. See the previous routine for more info.
\*--------------------------------------------------------------------------*/

void
PLColor_from_TkColor(PLColor *plcolor, XColor *xcolor)
{
    plcolor->r = (unsigned char) ToPLColor(xcolor->red);
    plcolor->g = (unsigned char) ToPLColor(xcolor->green);
    plcolor->b = (unsigned char) ToPLColor(xcolor->blue);
}

/*--------------------------------------------------------------------------*\
 * void PLColor_from_TkColor_Changed()
 *
 * Copies the supplied XColor to a PLColor, stripping off bits as
 * necessary. See the previous routine for more info.
 *
 * Returns 1 if the color was different from the old one.
\*--------------------------------------------------------------------------*/

int
PLColor_from_TkColor_Changed(PLColor *plcolor, XColor *xcolor)
{
    int changed = 0;
    int color;
    color = ToPLColor(xcolor->red);

    if(plcolor->r != color) {
        changed = 1;
        plcolor->r = color;
    }
    color = ToPLColor(xcolor->green);
    if(plcolor->g != color) {
        changed = 1;
        plcolor->g = color;
    }
    color = ToPLColor(xcolor->blue);
    if(plcolor->b != color) {
        changed = 1;
        plcolor->b = color;
    }
    return changed;
}

/*--------------------------------------------------------------------------*\
 * int pltk_AreWeGrayscale(PlPlotter *plf)
 *
 * Determines if we're using a monochrome or grayscale device.
 * gmf 11-8-91; Courtesy of Paul Martz of Evans and Sutherland.
 * Changed July 1996 by Vince: now uses Tk to check the enclosing PlPlotter
\*--------------------------------------------------------------------------*/

static int
pltk_AreWeGrayscale(PlPlotter *plf)
{
#if defined(__cplusplus) || defined(c_plusplus)
#define THING c_class
#else
#define THING class
#endif

    Visual* visual;
    /* get the window's Visual */
    visual = Tk_Visual(plf->tkwin);
    if ((visual->THING != GrayScale) && (visual->THING != StaticGray))
	return (0);
    /* if we got this far, only StaticGray and GrayScale classes available */
    return (1);
}

#if !defined(MAC_TCL) && !defined(__WIN32__)
/*--------------------------------------------------------------------------*\
 * CreatePixmapErrorHandler()
 *
 * Error handler used in CreatePixmap() to catch errors in allocating
 * storage for pixmap. This way we can nicely substitute redraws for
 * pixmap copies if the server has insufficient memory.
\*--------------------------------------------------------------------------*/

static int
CreatePixmapErrorHandler(Display *display, XErrorEvent *error)
{
    if (error->error_code == BadAlloc) {
	CreatePixmapStatus = error->error_code;
    } else {
	char buffer[256];
	XGetErrorText(display, error->error_code, buffer, 256);
	fprintf(stderr, "Error in XCreatePixmap: %s.\n", buffer);
    }
    return 1;
}
#endif

#else
int
pldummy_tkwin()
{
    return 0;
}

#endif    /* PLD_tkwin */

void *	ckcalloc(size_t nmemb, size_t size) {
    long *ptr;
    long *p;
    size *= nmemb;
    ptr = (long*) malloc(size);
    if (!ptr)
	return(0);

#if !__POWERPC__

    for (size = (size / sizeof(long)) + 1, p = ptr; --size;)
	*p++ = 0;

#else

    for (size = (size / sizeof(long)) + 1, p = ptr - 1; --size;)
	*++p = 0;

#endif

    return(ptr);
}
