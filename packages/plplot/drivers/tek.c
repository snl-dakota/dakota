/* $Id: tek.c 3186 2006-02-15 18:17:33Z slbrow $

	PLplot tektronix device & emulators driver.
*/
#include "plDevs.h"

#if defined(PLD_xterm) ||		/* xterm */ \
    defined(PLD_tek4010) ||		/* TEK 4010 */ \
    defined(PLD_tek4010f) ||		/* ditto, file */ \
    defined(PLD_tek4107) ||		/* TEK 4107 */ \
    defined(PLD_tek4107f) ||		/* ditto, file */ \
    defined(PLD_mskermit) ||		/* MS-kermit emulator */ \
    defined(PLD_versaterm) ||		/* Versaterm emulator */ \
    defined(PLD_vlt) ||			/* VLT emulator */ \
    defined(PLD_conex)			/* conex emulator 4010/4014/4105 */

#define NEED_PLDEBUG
#include "plplotP.h"
#include "drivers.h"
#include "plevent.h"

#include <ctype.h>

/* Device info */

char* plD_DEVICE_INFO_tek = 
  "conex:Conex vt320/tek emulator:1:tek:24:conex\n"
  "mskermit:MS-Kermit emulator:1:tek:21:mskermit\n"
  "tek4107t:Tektronix Terminal (4105/4107):1:tek:20:tek4107t\n"
  "tek4107f:Tektronix File (4105/4107):0:tek:28:tek4107f\n"
  "tekt:Tektronix Terminal (4010):1:tek:19:tekt\n"
  "tekf:Tektronix File (4010):0:tek:27:tekf\n"
  "versaterm:Versaterm vt100/tek emulator:1:tek:22:versaterm\n"
  "vlt:VLT vt100/tek emulator:1:tek:23:vlt\n"
  "xterm:Xterm Window:1:tek:18:xterm";

/* Prototype the driver entry points that will be used to initialize the
   dispatch table entries. */

void plD_init_xterm		(PLStream *);
void plD_init_tekt		(PLStream *);
void plD_init_tekf		(PLStream *);
void plD_init_tek4107t		(PLStream *);
void plD_init_tek4107f		(PLStream *);
void plD_init_mskermit		(PLStream *);
void plD_init_versaterm		(PLStream *);
void plD_init_vlt		(PLStream *);
void plD_init_conex		(PLStream *);

/* External generic entry points */

void plD_line_tek		(PLStream *, short, short, short, short);
void plD_polyline_tek		(PLStream *, short *, short *, PLINT);
void plD_eop_tek		(PLStream *);
void plD_bop_tek		(PLStream *);
void plD_tidy_tek		(PLStream *);
void plD_state_tek		(PLStream *, PLINT);
void plD_esc_tek		(PLStream *, PLINT, void *);

/* Static function prototypes */

static void  WaitForPage	(PLStream *pls);
static void  tek_init		(PLStream *pls);
static void  tek_text		(PLStream *pls);
static void  tek_graph		(PLStream *pls);
static void  fill_polygon	(PLStream *pls);
static void  GetCursor		(PLStream *pls, PLGraphicsIn *ptr);
static void  encode_int		(char *c, int i);
static void  encode_vector	(char *c, int x, int y);
static void  decode_gin		(char *c, PLGraphicsIn *gin);
static void  tek_vector		(PLStream *pls, int x, int y);
static void  scolor		(PLStream *pls, int icol, int r, int g, int b);
static void  setcmap		(PLStream *pls);

static void  LookupEvent	(PLStream *pls);
static void  InputEH		(PLStream *pls);
static void  LocateEH		(PLStream *pls);

/* Stuff for handling tty cbreak mode */

#ifdef HAVE_TERMIOS_H
#include <termios.h>
#include <unistd.h>
static struct termios	termios_cbreak, termios_reset;
static enum { RESET, CBREAK } ttystate = RESET;
static void tty_setup	(void);
static int  tty_cbreak	(void);
static int  tty_reset	(void);
static void tty_atexit	(void);
#else
static void tty_setup	(void) {}
static int  tty_cbreak	(void) {return 0;}
static int  tty_reset	(void) {return 0;}
static void tty_atexit	(void) {}
#endif

/* Pixel settings */

#define TEKX   1023
#define TEKY    779

/* Graphics control characters. */

#define RING_BELL	"\007"		/* ^G = 7 */
#define CLEAR_VIEW	"\033\f"	/* clear the view = ESC FF */

#define ALPHA_MODE	"\037"		/* Enter Alpha  mode:  US */
#define VECTOR_MODE	"\035"		/* Enter Vector mode:  GS */
#define GIN_MODE	"\033\032"	/* Enter GIN    mode:  ESC SUB */
#define BYPASS_MODE	"\033\030"	/* Enter Bypass mode:  ESC CAN */
#define XTERM_VTMODE	"\033\003"	/* End xterm-Tek mode: ESC ETX */
#define CANCEL		"\033KC"	/* Cancel */	

/* Static vars */

enum {tek4010, tek4105, tek4107, xterm, mskermit, vlt, versaterm};

/* One of these holds the tek driver state information */

typedef struct {
    PLINT	xold, yold;	/* Coordinates of last point plotted */
    int		exit_eventloop;	/* Break out of event loop */
    int		locate_mode;	/* Set while in locate (pick) mode */
    int		curcolor;	/* Current color index */
    PLGraphicsIn gin;		/* Graphics input structure */
} TekDev;

/* color for MS-DOS Kermit v2.31 (and up) tektronix emulator 
 *	0 = normal, 1 = bright 
 *	foreground color (30-37) = 30 + colors
 *		where colors are   1=red, 2=green, 4=blue 
 */
#ifdef PLD_mskermit
static char *kermit_color[15]= {
   "0;30","0;37",
   "0;32","0;36","0;31","0;35",
   "1;34","1;33","1;31","1;37",
   "1;35","1;32","1;36","0;34",
   "0;33"};
#endif

static void tek_dispatch_init_helper( PLDispatchTable *pdt,
                                      char *menustr, char *devnam,
                                      int type, int seq, plD_init_fp init )
{
#ifndef ENABLE_DYNDRIVERS
    pdt->pl_MenuStr = menustr;
    pdt->pl_DevName = devnam;
#endif
    pdt->pl_type = type;
    pdt->pl_seq = seq;
    pdt->pl_init     = init;
    pdt->pl_line     = (plD_line_fp)     plD_line_tek;
    pdt->pl_polyline = (plD_polyline_fp) plD_polyline_tek;
    pdt->pl_eop      = (plD_eop_fp)      plD_eop_tek;
    pdt->pl_bop      = (plD_bop_fp)      plD_bop_tek;
    pdt->pl_tidy     = (plD_tidy_fp)     plD_tidy_tek;
    pdt->pl_state    = (plD_state_fp)    plD_state_tek;
    pdt->pl_esc      = (plD_esc_fp)      plD_esc_tek;
}

void plD_dispatch_init_xterm	( PLDispatchTable *pdt )
{
    tek_dispatch_init_helper( pdt,
                              "Xterm Window", "xterm",
                              plDevType_Interactive, 18,
                              (plD_init_fp) plD_init_xterm );
}

void plD_dispatch_init_tekt	( PLDispatchTable *pdt )
{
    tek_dispatch_init_helper( pdt,
                              "Tektronix Terminal (4010)", "tekt",
                              plDevType_Interactive, 19,
                              (plD_init_fp) plD_init_tekt );
}

void plD_dispatch_init_tek4107t	( PLDispatchTable *pdt )
{
    tek_dispatch_init_helper( pdt,
                              "Tektronix Terminal (4105/4107)", "tek4107t",
                              plDevType_Interactive, 20,
                              (plD_init_fp) plD_init_tek4107t );
}

void plD_dispatch_init_mskermit	( PLDispatchTable *pdt )
{
    tek_dispatch_init_helper( pdt,
                              "MS-Kermit emulator", "mskermit",
                              plDevType_Interactive, 21,
                              (plD_init_fp) plD_init_mskermit );
}

void plD_dispatch_init_versaterm( PLDispatchTable *pdt )
{
    tek_dispatch_init_helper( pdt,
                              "Versaterm vt100/tek emulator", "versaterm",
                              plDevType_Interactive, 22,
                              (plD_init_fp) plD_init_versaterm );
}

void plD_dispatch_init_vlt	( PLDispatchTable *pdt )
{
    tek_dispatch_init_helper( pdt,
                              "VLT vt100/tek emulator", "vlt",
                              plDevType_Interactive, 23,
                              (plD_init_fp) plD_init_vlt );
}

void plD_dispatch_init_conex	( PLDispatchTable *pdt )
{
    tek_dispatch_init_helper( pdt,
                              "Conex vt320/tek emulator", "conex",
                              plDevType_Interactive, 24,
                              (plD_init_fp) plD_init_conex );
}

void plD_dispatch_init_tekf	( PLDispatchTable *pdt )
{
    tek_dispatch_init_helper( pdt,
                              "Tektronix File (4010)", "tekf",
                              plDevType_FileOriented, 27,
                              (plD_init_fp) plD_init_tekf );
}

void plD_dispatch_init_tek4107f	( PLDispatchTable *pdt )
{
    tek_dispatch_init_helper( pdt,
                              "Tektronix File (4105/4107)", "tek4107f",
                              plDevType_FileOriented, 28,
                              (plD_init_fp) plD_init_tek4107f );
}

/*--------------------------------------------------------------------------*\
 * plD_init_xterm()	xterm 
 * plD_init_tekt()	Tek 4010 terminal
 * plD_init_tekf()	Tek 4010 file
 * plD_init_tek4107t()	Tek 4105/4107 terminal
 * plD_init_tek4107f()	Tek 4105/4107 file
 * plD_init_mskermit()	MS-Kermit emulator (DOS)
 * plD_init_vlt()	VLT emulator (Amiga)
 * plD_init_versaterm()	VersaTerm emulator (Mac)
 * plD_init_conex()	Conex vt320/Tek 4105 emulator (DOS)
 *
 * These just set attributes for the particular tektronix device, then call
 * tek_init().  The following attributes can be set:
 *
 * pls->termin		if a terminal device
 * pls->color		if color (1), if only fixed colors (2)
 * pls->dev_fill0	if can handle solid area fill
 * pls->dev_fill1	if can handle pattern area fill
\*--------------------------------------------------------------------------*/

void 
plD_init_xterm(PLStream *pls)
{
    pls->dev_minor = xterm;
    pls->termin = 1;
    tek_init(pls);
}

void
plD_init_tekt(PLStream *pls)
{
    pls->termin = 1;
    plD_init_tekf(pls);
}

void
plD_init_tekf(PLStream *pls)
{
    pls->dev_minor = tek4010;
    tek_init(pls);
}

void
plD_init_tek4107t(PLStream *pls)
{
    pls->termin = 1;
    plD_init_tek4107f(pls);
}

void
plD_init_tek4107f(PLStream *pls)
{
    pls->dev_minor = tek4107;
    pls->color = 1;
    pls->dev_fill0 = 1;
    tek_init(pls);
}

void
plD_init_mskermit(PLStream *pls)
{
    pls->dev_minor = mskermit;
    pls->termin = 1;
    pls->color = 1;
    pls->dev_fill0 = 1;
    tek_init(pls);
}

void
plD_init_vlt(PLStream *pls)
{
    pls->dev_minor = vlt;
    pls->termin = 1;
    pls->color = 1;
    pls->dev_fill0 = 1;
    tek_init(pls);
}

void
plD_init_versaterm(PLStream *pls)
{
    pls->dev_minor = versaterm;
    pls->termin = 1;
    pls->color = 1;
    pls->dev_fill0 = 1;
    tek_init(pls);
}

void
plD_init_conex(PLStream *pls)
{
    pls->dev_minor = xterm;		/* responds to xterm escape codes */
    pls->termin = 1;
    pls->color = 2;			/* only fixed colours */
    tek_init(pls);
}

/*--------------------------------------------------------------------------*\
 * tek_init()
 *
 * Generic tektronix device initialization.
\*--------------------------------------------------------------------------*/

static void
tek_init(PLStream *pls)
{
    TekDev *dev;
    int xmin = 0;
    int xmax = TEKX;
    int ymin = 0;
    int ymax = TEKY;

    PLFLT pxlx = 4.771;
    PLFLT pxly = 4.653;

    pls->graphx = TEXT_MODE;

/* Allocate and initialize device-specific data */

    pls->dev = calloc(1, (size_t) sizeof(TekDev));
    if (pls->dev == NULL)
	plexit("tek_init: Out of memory.");

    dev = (TekDev *) pls->dev;

    dev->curcolor = 1;
    dev->xold = PL_UNDEFINED;
    dev->yold = PL_UNDEFINED;

    plP_setpxl(pxlx, pxly);
    plP_setphy(xmin, xmax, ymin, ymax);

/* Terminal/file initialization */

    if (pls->termin) {
	pls->OutFile = stdout;
	tty_setup();
    }
    else {
	plFamInit(pls);
	plOpenFile(pls);
    }

    switch (pls->dev_minor) {
#ifdef PLD_tek4107
    case tek4107:
	pls->graphx = GRAPHICS_MODE;
	fprintf(pls->OutFile, "\033%%!0");	/* set tek mode */
	fprintf(pls->OutFile, "\033KN1");	/* clear the view */
	fprintf(pls->OutFile, "\033LZ");	/* clear dialog buffer */
	fprintf(pls->OutFile, "\033ML1");	/* set default color */
	break;
#endif	/* PLD_tek4107 */

/* A sneaky hack: VLT sometimes has leftover panel information, causing 
 * garbage at the beginning of a sequence of color fills.  Since 
 * there is no clear panel command, instead I set the fill color to the
 * same as background and issue an end panel command.
 */
#ifdef PLD_vlt
    case vlt:{
	char fillcol[4];
	tek_graph(pls);
	encode_int(fillcol, 0);
	fprintf(pls->OutFile, "\033MP%s\033LE", fillcol);
	break;
    }
#endif	/* PLD_vlt */

    default:
	tek_graph(pls);
    }

/* Initialize palette */

   if ( pls->color & 0x01 ) {
       printf("\033TM111");	/* Switch to RGB colors */
       setcmap(pls);
   }

/* Finish initialization */

    fprintf(pls->OutFile, VECTOR_MODE);	/* Enter vector mode */
    if (pls->termin)
	fprintf(pls->OutFile, CLEAR_VIEW);/* erase and home */

    fflush(pls->OutFile);
}

/*--------------------------------------------------------------------------*\
 * plD_line_tek()
 *
 * Draw a line from (x1,y1) to (x2,y2).
\*--------------------------------------------------------------------------*/

void
plD_line_tek(PLStream *pls, short x1, short y1, short x2, short y2)
{
    TekDev *dev = (TekDev *) pls->dev;

    tek_graph(pls);

/* If not continuation of previous line, begin a new one */

    if (x1 != dev->xold || y1 != dev->yold) {
	pls->bytecnt += fprintf(pls->OutFile, VECTOR_MODE);
	tek_vector(pls, x1, y1);
    }

/* Now send following point to complete line draw */

    tek_vector(pls, x2, y2);

    dev->xold = x2;
    dev->yold = y2;
}

/*--------------------------------------------------------------------------*\
 * plD_polyline_tek()
 *
 * Draw a polyline in the current color.
\*--------------------------------------------------------------------------*/

void
plD_polyline_tek(PLStream *pls, short *xa, short *ya, PLINT npts)
{
    PLINT i;
    TekDev *dev = (TekDev *) pls->dev;
    short x = xa[0], y = ya[0];

    tek_graph(pls);

/* If not continuation of previous line, begin a new one */

    if ( x != dev->xold || y != dev->yold ) {
	pls->bytecnt += fprintf(pls->OutFile, VECTOR_MODE);
	tek_vector(pls, x, y);
    }

/* Now send following points to complete polyline draw */

    for (i = 1; i < npts; i++) 
	tek_vector(pls, xa[i], ya[i]);

    dev->xold = xa[npts-1];
    dev->yold = ya[npts-1];
}

/*--------------------------------------------------------------------------*\
 * plD_eop_tek()
 *
 * End of page.  User must hit a <CR> to continue (terminal output).
\*--------------------------------------------------------------------------*/

void
plD_eop_tek(PLStream *pls)
{
    tek_graph(pls);

    if (pls->termin) {
	if ( ! pls->nopause) 
	    WaitForPage(pls);
    }
    fprintf(pls->OutFile, CLEAR_VIEW);		/* erase and home */
}

/*--------------------------------------------------------------------------*\
 * plD_bop_tek()
 *
 * Set up for the next page.  Advance to next family file if necessary
 * (file output).  Devices that share graphics/alpha screens need a page
 * clear.
\*--------------------------------------------------------------------------*/

void
plD_bop_tek(PLStream *pls)
{
   TekDev *dev = (TekDev *) pls->dev;

   dev->xold = PL_UNDEFINED;
   dev->yold = PL_UNDEFINED;

   if (pls->termin) {
       switch (pls->dev_minor) {
       case mskermit:
	   fprintf(pls->OutFile, CLEAR_VIEW);	/* erase and home */
	   break;
       }
   } else {
       plGetFam(pls);
   }
   pls->page++;

/* Initialize palette */

   if ( pls->color & 0x01 )
       setcmap(pls);
}

/*--------------------------------------------------------------------------*\
 * plD_tidy_tek()
 *
 * Close graphics file or otherwise clean up.
\*--------------------------------------------------------------------------*/

void
plD_tidy_tek(PLStream *pls)
{
    if ( ! pls->termin) {
	fclose(pls->OutFile);
    } else {
	tek_text(pls);
	fflush(pls->OutFile);
    }
}

/*--------------------------------------------------------------------------*\
 * tek_color()
 *
 * Change to specified color index.
\*--------------------------------------------------------------------------*/

static void 
tek_color(PLStream *pls, int icol)
{
    switch (pls->dev_minor) {
#ifdef PLD_mskermit			/* Is this really necessary? */
    case mskermit:
	printf("\033[%sm", kermit_color[icol % 14] );
	break;
#endif
    default:
	pls->bytecnt += fprintf(pls->OutFile, "\033ML%c", icol + '0');
    }
}

/*--------------------------------------------------------------------------*\
 * plD_state_tek()
 *
 * Handle change in PLStream state (color, pen width, fill attribute,
 * etc).
\*--------------------------------------------------------------------------*/

void 
plD_state_tek(PLStream *pls, PLINT op)
{
    TekDev *dev = (TekDev *) pls->dev;

    switch (op) {

    case PLSTATE_WIDTH:
	break;

    case PLSTATE_COLOR0:
	if (pls->color) {
	    int icol0 = pls->icol0;
	    tek_graph(pls);
	    if (icol0 != PL_RGB_COLOR) {
		dev->curcolor = icol0;
		tek_color(pls, icol0);
	    }
	}
	break;

    case PLSTATE_COLOR1:
	if (pls->color) {
	    int icol1, ncol1;
	    tek_graph(pls);
	    if ((ncol1 = MIN(16 - pls->ncol0, pls->ncol1)) < 1)
		break;

	    icol1 = pls->ncol0 + (pls->icol1 * (ncol1-1)) / (pls->ncol1-1);
	    dev->curcolor = icol1;
	    tek_color(pls, icol1);
	}
	break;

    case PLSTATE_CMAP0:
    case PLSTATE_CMAP1:
	if ( pls->color & 0x01 )
	    setcmap(pls);    

	break;
    }
}

/*--------------------------------------------------------------------------*\
 * plD_esc_tek()
 *
 * Escape function.
\*--------------------------------------------------------------------------*/

void 
plD_esc_tek(PLStream *pls, PLINT op, void *ptr)
{
    switch (op) {

    case PLESC_TEXT:
	tek_text(pls);
	break;

    case PLESC_GRAPH:
	tek_graph(pls);
	break;

    case PLESC_FILL:
	fill_polygon(pls);
	break;

    case PLESC_GETC:
	GetCursor(pls, (PLGraphicsIn *) ptr);
	break;
    }
}

/*--------------------------------------------------------------------------*\
 * GetCursor()
 *
 * Waits for a left button mouse event and returns coordinates.
 * xterm doesn't handle GIN. I think all the rest do.
\*--------------------------------------------------------------------------*/

static void
GetCursor(PLStream *pls, PLGraphicsIn *ptr)
{
#define MAX_GIN 10
    char input_string[MAX_GIN];
    int i = 0;

    plGinInit(ptr);

    if (pls->termin && pls->dev_minor != xterm) {
	tek_graph(pls);

    /* Enter GIN mode */

	printf(GIN_MODE);
	fflush(stdout);

    /* Read & decode report */

	while (++i < MAX_GIN && (input_string[i-1] = getchar()) != '\n')
	    ;

	input_string[i-1] = '\0';
	ptr->keysym = input_string[0];
	decode_gin(&input_string[1], ptr);

    /* Switch out of GIN mode */

	printf(VECTOR_MODE);
    }
}

/*--------------------------------------------------------------------------*\
 * fill_polygon()
 *
 * Fill polygon described in points pls->dev_x[] and pls->dev_y[].
\*--------------------------------------------------------------------------*/

static void
fill_polygon(PLStream *pls)
{
    TekDev *dev = (TekDev *) pls->dev;
    int i;
    char fillcol[4], firstpoint[5];

    if (pls->dev_npts < 1)
	return;

    tek_graph(pls);

    encode_int(fillcol, -dev->curcolor);
    encode_vector(firstpoint, pls->dev_x[0], pls->dev_y[0]);

/* Select the fill pattern */

    pls->bytecnt += fprintf(pls->OutFile, "\033MP%s", fillcol);

/* Begin panel boundary */
/* Set pls->debug to see the boundary of each fill box -- cool! */

    if (pls->debug)
	pls->bytecnt += fprintf(pls->OutFile, "\033LP%s1", firstpoint);
    else
	pls->bytecnt += fprintf(pls->OutFile, "\033LP%s0", firstpoint);

/* Specify boundary (in vector mode) */

    pls->bytecnt += fprintf(pls->OutFile, VECTOR_MODE);
    for (i = 1; i < pls->dev_npts; i++) 
	tek_vector(pls, pls->dev_x[i], pls->dev_y[i]);

/* End panel */

    pls->bytecnt += fprintf(pls->OutFile, "\033LE");
}

/*--------------------------------------------------------------------------*\
 * tek_text()
 *
 * Switch to text screen (or alpha mode, for vanilla tek's).  Restore
 * terminal to its original state, to better handle user input if
 * necessary.
 *
 * Note: xterm behaves strangely in the following circumstance: switch to
 * the text screen, print a string, and switch to the graphics screen, all
 * done in quick succession.  The first character of the printed string
 * usually comes out blank -- but only apparently so, because if you force
 * a refresh of the screen in this area it will reappear.  This is a
 * reproducible bug on the HP 720 under X11R5.  If you insert a sleep(1)
 * after the switch to text screen or before the switch to graphics
 * screen, the string is printed correctly.  I've been unable to find a
 * workaround for this problem (and I've tried, you can believe eet man).
\*--------------------------------------------------------------------------*/

static void 
tek_text(PLStream *pls)
{
    if (pls->termin && (pls->graphx == GRAPHICS_MODE)) {
	tty_reset();
	pls->graphx = TEXT_MODE;
	switch (pls->dev_minor) {
	case xterm:
	    printf("\033\003");		/* vt100 mode (xterm) = ESC ETX */
	    break;

	case mskermit:
	case vlt:
	    printf("\033[?38l");	/* vt100 screen */
	    break;

	case versaterm:
	    printf("\033%%!2");		/* vt100 screen */
	    break;

	case tek4107:
	    printf("\033LV1");		/* set dialog visible */
	    printf("\033%%!1");		/* set ansi mode */
	    break;

	default:
	    printf(ALPHA_MODE);		/* enter alpha mode */
	}
	fflush(stdout);
    }
}

/*--------------------------------------------------------------------------*\
 * tek_graph()
 *
 * Switch to graphics screen.  Also switch terminal to cbreak mode, to allow
 * single keystrokes to govern actions at end of page.
\*--------------------------------------------------------------------------*/

static void 
tek_graph(PLStream *pls)
{
    if (pls->termin && (pls->graphx == TEXT_MODE)) {
	tty_cbreak();
	pls->graphx = GRAPHICS_MODE;
	switch (pls->dev_minor) {
	case xterm:
	case mskermit:
	case vlt:
	    printf("\033[?38h");	/* switch to tek screen */
	    break;

	case versaterm:
	    printf("\033%%!0");		/* switch to tek4107 screen */
	    break;

	case tek4107:
	    printf("\033%%!0");		/* set tek mode */
   	    printf(CLEAR_VIEW);		/* clear screen */
	    printf("\033LV0");		/* set dialog invisible */
	    break;
	}
    }
}

/*--------------------------------------------------------------------------*\
 * encode_int()
 *
 * Encodes a single int into standard tek integer format, storing into a
 * NULL-terminated character string (must be length 4 or greater).  This
 * scheme does not work for negative integers less than 15.
\*--------------------------------------------------------------------------*/

static void
encode_int(char *c, int i)
{
    int negative = 0;

    if (i > 0) {
	if (i & 0x7C00)				/* are any of bits 10-14 set?*/
	    *c++ = ((i >> 10) & 0x1F) | 0x40;
	if (i & 0x03F0)				/* are any of bits 4-9 set? */
	    *c++ = ((i >> 4) & 0x3F) | 0x40;
    } else {
	i = -i;
	negative = 1;
    }

    if (i & 0x000F)				/* are any of bits 0-3 set? */
	*c = (i & 0x0F) | 0x20;
    else					/* if not, put in a space */
	*c = 0x20;
	
    if ( ! negative)				/* if positive, set sign bit */
	*c |= 0x10;

    c++; *c = '\0';				/* NULL-terminate */
    return;
}

/*--------------------------------------------------------------------------*\
 * decode_gin()
 *
 * Decodes a GIN tek vector string into an xy pair of relative device
 * coordinates.  It's best to not use absolute device coordinates since the
 * coordinate bounds are different depending on the report encoding used.
 *
 * Standard:	<HiX><LoX><HiY><LoY>
 * Extended:	<HiY><Extra><LoY><HiX><LoX>
 * 
 * where <Extra> holds the two low order bits for each coordinate.
\*--------------------------------------------------------------------------*/

static void
decode_gin(char *c, PLGraphicsIn *gin)
{
    int x, y, lc = strlen(c);

    if (lc == 4) {
	x = ((c[0] & 0x1f) << 5) +
	    ((c[1] & 0x1f)     );

	y = ((c[2] & 0x1f) << 5) +
	    ((c[3] & 0x1f)     );

	gin->pX = x;
	gin->pY = y;
	gin->dX = x / (double) TEKX;
	gin->dY = y / (double) TEKY;
    }
    else if (lc == 5) {
	y = ((c[0] & 0x1f) << 7) +
	    ((c[2] & 0x1f) << 2) +
	    ((c[1] & 0x06) >> 2);

	x = ((c[3] & 0x1f) << 7) +
	    ((c[4] & 0x1f) << 2) +
	    ((c[1] & 0x03)     );

	gin->pX = x;
	gin->pY = y;
	gin->dX = x / (double) (TEKX << 2);
	gin->dY = y / (double) (TEKY << 2);
    }
    else {			/* Illegal encoding */
	gin->pX = 0;
	gin->pY = 0;
	gin->dY = 0;
	gin->dX = 0;
    }
}

/*--------------------------------------------------------------------------*\
 * encode_vector()
 *
 * Encodes an xy vector (2 ints) into standard tek vector format, storing
 * into a NULL-terminated character string of length 5.  Note that the y
 * coordinate always comes first.
\*--------------------------------------------------------------------------*/

static void
encode_vector(char *c, int x, int y)
{
    c[0] = (y >> 5)   + 0x20;		/* hy */
    c[1] = (y & 0x1f) + 0x60;		/* ly */
    c[2] = (x >> 5)   + 0x20;		/* hx */
    c[3] = (x & 0x1f) + 0x40;		/* lx */
    c[4] = '\0';			/* NULL */
}

/*--------------------------------------------------------------------------*\
 * tek_vector()
 *
 * Issues a vector draw command, assuming we are in vector plot mode.  XY
 * coordinates are encoded according to the standard xy encoding scheme.
\*--------------------------------------------------------------------------*/

static void
tek_vector(PLStream *pls, int x, int y)
{
    char c[5];

    encode_vector(c, x, y);
    pls->bytecnt += fprintf( pls->OutFile, "%s", c );
}

/*--------------------------------------------------------------------------*\
 * scolor()
 *
 * Sets a color by tek-encoded RGB values.  Need to convert PLplot RGB
 * color range (0 to 255) to Tek RGB color range (0 to 100).
\*--------------------------------------------------------------------------*/

static void
scolor(PLStream *pls, int icol, int r, int g, int b)
{
    char tek_col[4], tek_r[4], tek_g[4], tek_b[4];

    encode_int(tek_col, icol);
    encode_int(tek_r, (100*r)/255);
    encode_int(tek_g, (100*g)/255);
    encode_int(tek_b, (100*b)/255);

    pls->bytecnt += fprintf(pls->OutFile, "\033TG14%s%s%s%s",
			    tek_col, tek_r, tek_g, tek_b);
}

/*--------------------------------------------------------------------------*\
 * setcmap()
 *
 * Sets up color palette.
\*--------------------------------------------------------------------------*/

static void
setcmap(PLStream *pls)
{
    int i, ncol1 = MIN(16 - pls->ncol0, pls->ncol1);
    PLColor cmap1col;

    tek_graph(pls);

/* Initialize cmap 0 colors */

    for (i = 0; i < pls->ncol0; i++) 
	scolor(pls, i, pls->cmap0[i].r, pls->cmap0[i].g, pls->cmap0[i].b);

/* Initialize any remaining slots for cmap1 */

    for (i = 0; i < ncol1; i++) {
	plcol_interp(pls, &cmap1col, i, ncol1);
	scolor(pls, i + pls->ncol0, cmap1col.r, cmap1col.g, cmap1col.b);
    }
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
    TekDev *dev = (TekDev *) pls->dev;

    printf(ALPHA_MODE);		/* Switch to alpha mode (necessary) */
    printf(RING_BELL);		/* and ring bell */
    printf(VECTOR_MODE);	/* Switch out of alpha mode */
    fflush(stdout);

    while ( ! dev->exit_eventloop) {
	LookupEvent(pls);
	if (dev->locate_mode)
	    LocateEH(pls);
	else
	    InputEH(pls);
    }
    dev->exit_eventloop = FALSE;
}

/*--------------------------------------------------------------------------*\
 * LookupEvent()
 *
 * Fills in the PLGraphicsIn from an input event.  
\*--------------------------------------------------------------------------*/

static void
LookupEvent(PLStream *pls)
{
    TekDev *dev = (TekDev *) pls->dev;
    PLGraphicsIn *gin = &(dev->gin);

    if (dev->locate_mode) {
	GetCursor(pls, gin);
    }
    else {
	plGinInit(gin);
	gin->keysym = getchar();
    }

    if (isprint(gin->keysym)) {
	gin->string[0] = gin->keysym;
	gin->string[1] = '\0';
    }
    else {
	gin->string[0] = '\0';
    }

    pldebug("LookupEvent",
	    "Keycode %x, string: %s\n", gin->keysym, gin->string);
}

/*--------------------------------------------------------------------------*\
 * LocateEH()
 *
 * Handles locate mode events.
 *
 * In locate mode: move cursor to desired location and select by pressing a
 * key or by clicking on the mouse (if available).  Typically the world
 * coordinates of the selected point are reported.
 *
 * There are two ways to enter Locate mode -- via the API, or via a driver
 * command.  The API entry point is the call plGetCursor(), which initiates
 * locate mode and does not return until input has been obtained.  The
 * driver entry point is by entering a 'L' while the driver is waiting for
 * events.  
 *
 * Locate mode input is reported in one of three ways:
 * 1. Through a returned PLGraphicsIn structure, when user has specified a
 *    locate handler via (*pls->LocateEH).
 * 2. Through a returned PLGraphicsIn structure, when locate mode is invoked
 *    by a plGetCursor() call.
 * 3. Through writes to stdout, when locate mode is invoked by a driver
 *    command and the user has not supplied a locate handler.
 *
 * Hitting <Escape> will at all times end locate mode.  Other keys will
 * typically be interpreted as locator input.  Selecting a point out of
 * bounds will end locate mode unless the user overrides with a supplied
 * Locate handler.
\*--------------------------------------------------------------------------*/

static void
LocateEH(PLStream *pls)
{
    TekDev *dev = (TekDev *) pls->dev;
    PLGraphicsIn *gin = &(dev->gin);

/* End locate mode on <Escape> */

    if (gin->keysym == PLK_Escape) {
	dev->locate_mode = 0;
	return;
    }

/* Call user locate mode handler if provided */

    if (pls->LocateEH != NULL)
	(*pls->LocateEH) (gin, pls->LocateEH_data, &dev->locate_mode);

/* Use default procedure */

    else {

    /* Try to locate cursor */

	if (plTranslateCursor(gin)) {

	/* Successful, so send report to stdout */

	    pltext();
	    if (isprint(gin->keysym)) 
		printf("%f %f %c\n", gin->wX, gin->wY, gin->keysym);
	    else
		printf("%f %f\n", gin->wX, gin->wY);

	    plgra();
	}
	else {

	/* Selected point is out of bounds, so end locate mode */

	    dev->locate_mode = 0;
	}
    }
}

/*--------------------------------------------------------------------------*\
 * InputEH()
 *
 * Event handler routine for xterm.  Just reacts to keyboard input.
 *
 * In locate mode: move cursor to desired location and select by pressing a
 * key or by clicking on the mouse (if available).  The world coordinates of
 * the selected point are output on the text screen.  Terminate by picking a
 * point out of bounds, hitting page advance, or the escape key.  If you
 * want to customize this, write an event handler to do it.
\*--------------------------------------------------------------------------*/

static void
InputEH(PLStream *pls)
{
    TekDev *dev = (TekDev *) pls->dev;
    PLGraphicsIn *gin = &(dev->gin);

/* Call user event handler.
 * Since this is called first, the user can disable all PLplot internal
 * event handling by setting gin->keysym to 0 and gin->string to '\0'.
*/
    if (pls->KeyEH != NULL)
	(*pls->KeyEH) (gin, pls->KeyEH_data, &dev->exit_eventloop);

/* Remaining internal event handling */

    switch (gin->keysym) {

    case PLK_Linefeed:
    /* Advance to next page (i.e. terminate event loop) on a <eol> */
	dev->exit_eventloop = TRUE;
	break;

    case 'Q':
    /* Terminate on a 'Q' (not 'q', since it's too easy to hit by mistake) */
	pls->nopause = TRUE;
	plexit("");
	break;

    case 'L':
    /* Begin locate mode */
	dev->locate_mode = 1;
	break;
    }
}

/*--------------------------------------------------------------------------*\
 * tty cbreak-mode handlers
 *
 * Taken from "Advanced Programming in the UNIX(R) Environment", 
 * by W. Richard Stevens.
\*--------------------------------------------------------------------------*/

#ifdef HAVE_TERMIOS_H

static void
tty_setup(void)				/* setup for terminal operations */
{
    if (tcgetattr(STDIN_FILENO, &termios_reset) < 0) {
	fprintf(stderr, "Unable to set up cbreak mode.\n");
	return;
    }

    termios_cbreak = termios_reset;		/* structure copy */

    termios_cbreak.c_lflag &= ~(ICANON);	/* canonical mode off */
    termios_cbreak.c_cc[VMIN] = 1;		/* 1 byte at a time */
    termios_cbreak.c_cc[VTIME] = 0;		/* no timer */

#ifdef STDC_HEADERS
    if (atexit(tty_atexit))
	fprintf(stderr, "Unable to set up atexit handler.\n");
#endif
    return;
}

static int
tty_cbreak(void)			/* put terminal into a cbreak mode */
{
    if (ttystate != CBREAK) {
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &termios_cbreak) < 0)
	    return -1;

	ttystate = CBREAK;
    }
    return 0;
}

static int
tty_reset(void)				/* restore terminal's mode */
{
    if (ttystate != RESET) {
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &termios_reset) < 0)
	    return -1;

	ttystate = RESET;
    }
    return 0;
}

static void
tty_atexit(void)			/* exit handler */
{
    tty_reset();
}

#endif			/* HAVE_TERMIOS_H */

#else
int pldummy_tek() {return 0;}

#endif	/*  defined(PLD_xterm) || ... */
