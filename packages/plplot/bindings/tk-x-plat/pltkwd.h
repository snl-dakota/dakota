/* $Id: pltkwd.h 3186 2006-02-15 18:17:33Z slbrow $

	Holds system header includes, prototypes of xwin driver
	utility functions, and definition of the state structure.
*/

#ifndef __PLTKWD_H__
#define __PLTKWD_H__

#include "plplot.h"
#include "plstrm.h"

/* System headers */

#ifndef MAC_TCL
#ifndef __MWERKS__
#include <sys/types.h>
#endif
#endif

#include <tk.h>
#ifdef MAC_TCL
#else
/* These are pulled in by 'tkMacPort.h' */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#endif
/* Specify max number of displays in use */

#define PLTKDISPLAYS 100

/* Set constants for dealing with colormap.  In brief:
 *
 * ccmap                When set, turns on custom color map
 *
 * See Init_CustomCmap() and  Init_DefaultCmap() for more info.
 * Set ccmap at your own risk -- still under development.
 */

static int plplot_tkwin_ccmap = 0;

/* One of these holds the display info, shared by all streams on a given */
/* display */

typedef struct {
    int		nstreams;		/* Number of streams using display */
    int		ixwd;			/* Specifies tkwDisplay number */
    char	*displayName;		/* Name of X display */
    int		screen;			/* X screen */
    Display	*display;		/* X display */
    Visual	*visual;		/* X Visual */
    GC		gcXor;			/* Graphics context for XOR draws */
    Colormap	map;			/* Colormap */
    unsigned	depth;			/* display depth */
    int		color;			/* Set to 1 if a color output device */
    int		ncol0;			/* Number of cmap 0 colors allocated */
    int		ncol1;			/* Number of cmap 1 colors allocated */
    XColor	cmap0[16];		/* Color entries for cmap 0 */
    XColor	cmap1[256];		/* Color entries for cmap 1 */
    XColor	fgcolor;		/* Foreground color (if grayscale) */
    Tk_Cursor	xhair_cursor;		/* Crosshair cursor */
} TkwDisplay;

/* One of these holds the X driver state information */

typedef struct {
    TkwDisplay	*tkwd;			/* Pointer to display info */

    Window	window;			/* X window id */
    Pixmap	pixmap;			/* Off-screen pixmap */
    GC		gc;			/* Graphics context */
    XColor	curcolor;		/* Current pen color */

    long	event_mask;		/* Event mask */
    int		flags;		        /* 1 = delete, 2 = exit event loop */
    long	init_width;		/* Initial window width */
    long	init_height;		/* Initial window height */

    unsigned	width, height, border;	/* Current window dimensions */

    double	xscale_init;		/* initial pixels/lx (virt. coords) */
    double	yscale_init;		/* initial pixels/ly (virt. coords) */
    double	xscale;			/* as above, but current value */
    double	yscale;			/*  (after possible resizing)  */

    short	xlen, ylen;		/* Lengths of device coord space */

    int		write_to_window;	/* Set if plotting direct to window */
    int		write_to_pixmap;	/* Set if plotting to pixmap */

    int		instr;			/* Instruction timer */
    int		max_instr;		/* Limit before X server is queried */

    PLGraphicsIn gin;			/* Graphics input structure */

    int		locate_mode;		/* Set while in locate mode */
    int		drawing_xhairs;		/* Set during xhair draws */
    XPoint	xhair_x[2], xhair_y[2];	/* Crosshair lines */

    void (*MasterEH) (PLStream *, XEvent *);	/* Master X event handler */
} TkwDev;

/*--------------------------------------------------------------------------*\
 *		Function Prototypes
\*--------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/* Performs basic driver initialization. */

void
plD_open_tkwin(PLStream *pls);

/* Copies the supplied PLColor to an XColor */

void
PLColor_to_TkColor(PLColor *plcolor, XColor *xcolor);

/* Copies the supplied XColor to a PLColor */

void
PLColor_from_TkColor(PLColor *plcolor, XColor *xcolor);

/* Same but also tells me if the color changed */
int
PLColor_from_TkColor_Changed(PLColor *plcolor, XColor *xcolor);

void Tkw_StoreColor(PLStream* pls, TkwDisplay* tkwd, XColor* col);

/* Set background & foreground colors.  */

void
pltkwin_setBGFG(PLStream *pls);

#ifdef __cplusplus
}
#endif

#endif	/* __PLTKWD_H__ */
