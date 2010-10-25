/* $Id: tektest.c 3186 2006-02-15 18:17:33Z slbrow $

	Maurice LeBrun
	IFS, University of Texas
	Feb 15, 1995

	Test program for illustrating possible Tek GIN anomalies.  In
	particular, for problems with GIN reports encountered in VLT (Tek
	4107 emulator for the Amiga).  May not work right with other Tek
	emulators.

	To compile: "$CC foo.c -o foo", where $CC is an ANSI-C compiler.

	Usage:

	% foo [x0 [y0]]

	To reproduce problem:
	Run program, then turn on graphic crosshair with XY and position over
	the intersection of the drawn lines.  Experiment with different
	crosshair locations, entered via the command line.  Invariably,
	the reported position is systematically <= than the drawn one,
	sometimes by as much as 2 pixels.

	Note: this anomaly has important consequences when doing graphic
	input, because the reported world coordinates are then
	systematically off.
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

/* Static function prototypes */

void plD_init_vlt		(void);
void plD_line_tek		(short, short, short, short);
void plD_eop_tek		(void);
void plD_bop_tek		(void);
void plD_tidy_tek		(void);

static void	WaitForPage	(void);
static void	tek_text	(void);
static void	tek_graph	(void);
static void	encode_int	(char *c, int i);
static void	encode_vector	(char *c, int x, int y);
static void	tek_vector	(int x, int y);

/* Graphics control characters. */

#define RING_BELL	"\007"		/* ^G = 7 */
#define CLEAR_VIEW	"\033\f"	/* clear the view = ESC FF */

#define ALPHA_MODE	"\037"		/* Enter Alpha  mode:  US */
#define VECTOR_MODE	"\035"		/* Enter Vector mode:  GS */
#define GIN_MODE	"\033\032"	/* Enter GIN    mode:  ESC SUB */
#define BYPASS_MODE	"\033\030"	/* Enter Bypass mode:  ESC CAN */
#define XTERM_VTMODE	"\033\003"	/* End xterm-Tek mode: ESC ETX */
#define CANCEL		"\033KC"	/* Cancel */	

/*----------------------------------------------------------------------*\
 * main
 *
 * Generates simple test case.
\*----------------------------------------------------------------------*/

int
main(int argc, char *argv[])
{
    short x0 = 300, y0 = 300, l = 100;
    short x1, x2, y1, y2;

/* Optional x0, y0 specification by the command line */

    if (argc > 1)
	x0 = atoi(argv[1]);

    if (argc > 2)
	y0 = atoi(argv[2]);

    plD_init_vlt();

/* Draw boundary */

    plD_line_tek(0, 0, 1023, 0);
    plD_line_tek(1023, 0, 1023, 779);

    plD_line_tek(1023, 779, 0, 779);
    plD_line_tek(0, 779, 0, 0);

/* Draw crosshairs centered around (x0, y0) of length 2l */

    x1 = x0 - l, x2 = x0 + l;
    y1 = y0 - l, y2 = y0 + l;

    plD_line_tek(x1, y0, x2, y0);
    plD_line_tek(x0, y1, x0, y2);

    plD_eop_tek();
    plD_tidy_tek();
    exit(0);
}

/*--------------------------------------------------------------------------*\
 * plD_init_vlt()	VLT emulator (Amiga)
\*--------------------------------------------------------------------------*/

void
plD_init_vlt(void)
{
    tek_graph();
    fprintf(stdout, VECTOR_MODE);	/* Enter vector mode */
    fprintf(stdout, CLEAR_VIEW);	/* erase and home */
}

/*--------------------------------------------------------------------------*\
 * plD_line_tek()
 *
 * Draw a line from (x1,y1) to (x2,y2).
\*--------------------------------------------------------------------------*/

void
plD_line_tek(short x1, short y1, short x2, short y2)
{
    fprintf(stdout, VECTOR_MODE);
    tek_vector(x1, y1);
    tek_vector(x2, y2);
}

/*--------------------------------------------------------------------------*\
 * plD_eop_tek()
 *
 * End of page.  User must hit a <CR> to continue (terminal output).
\*--------------------------------------------------------------------------*/

void
plD_eop_tek(void)
{
    WaitForPage();
    fprintf(stdout, CLEAR_VIEW);		/* erase and home */
}

/*--------------------------------------------------------------------------*\
 * plD_tidy_tek()
 *
 * Close graphics file or otherwise clean up.
\*--------------------------------------------------------------------------*/

void
plD_tidy_tek(void)
{
    tek_text();
}

/*--------------------------------------------------------------------------*\
 * tek_text()
\*--------------------------------------------------------------------------*/

static void 
tek_text(void)
{
    printf("\033[?38l");	/* vt100 screen */
}

/*--------------------------------------------------------------------------*\
 * tek_graph()
\*--------------------------------------------------------------------------*/

static void 
tek_graph(void)
{
    printf("\033[?38h");	/* switch to tek screen */
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
tek_vector(int x, int y)
{
    char c[5];
    c[0] = (y >> 5)   + 0x20;		/* hy */
    c[1] = (y & 0x1f) + 0x60;		/* ly */
    c[2] = (x >> 5)   + 0x20;		/* hx */
    c[3] = (x & 0x1f) + 0x40;		/* lx */
    c[4] = '\0';			/* NULL */

    fprintf( stdout, "%s", c );
}

/*--------------------------------------------------------------------------*\
 * WaitForPage()
 *
 * This routine waits for the user to advance the plot, while handling
 * all other events.
\*--------------------------------------------------------------------------*/

static void
WaitForPage(void)
{
    printf(ALPHA_MODE);		/* Switch to alpha mode (necessary) */
    printf(RING_BELL);		/* and ring bell */
    printf(VECTOR_MODE);	/* Switch out of alpha mode */
    fflush(stdout);

    while ( ! getchar() )
	;
}
