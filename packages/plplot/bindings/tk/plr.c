/* $Id: plr.c 3186 2006-02-15 18:17:33Z slbrow $

    Copyright 1993, 1994, 1995
    Maurice LeBrun
    IFS, University of Texas at Austin

    This software may be freely copied, modified and redistributed under the
    terms of the GNU Library General Public License.  

    There is no warranty or other guarantee of fitness of this software.
    It is provided solely "as is". The author(s) disclaim(s) all
    responsibility and liability with respect to this software's usage or
    its effect upon hardware or computer systems.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * Support routines to render a PLplot byte stream, interpreting the PLplot
 * metacode.  
 *
 * Although this code is duplicated to some extent by plrender and the
 * plot buffer redraw code (in plbuf.c), they are all different in some
 * significant ways, namely:
 *
 *    - plrender must always retain backward compatibility code to 
 *      handle old metafiles, as well as stuff to support seeking.
 *
 *    - plbuf by definition is redrawing on the same machine so no 
 *      special effort is necessary to make the byte stream portable,
 *      also, only low-level graphics calls are used (grline, etc)
 *
 * The rendering code here must (by contrast to plbuf) use the high level
 * plot routines (as does plrender), to support local zooms as well as the
 * ability to dump the associated window into plot space to a file, but is
 * otherwise pretty minimal.  A portable byte stream is used since network
 * communication over a socket may be used.
 *
\*--------------------------------------------------------------------------*/

/*
#define DEBUG
#define DEBUG_ENTER
*/

#include "plserver.h"
#include "plevent.h"
#include "metadefs.h"

/* Some wrapper macros to return (-1) on error */

#define plr_rd(code) \
if (code) { fprintf(stderr, \
	    "Unable to read from %s in %s at line %d, bytecount %ld\n", \
	    plr->iodev->typeName, __FILE__, __LINE__, plr->pdfs->bp); \
	    return -1; }

#define plr_cmd(code) \
if ((code) == -1) return -1;

/* Error termination */

#define barf(msg) \
{ fprintf(stderr, "%s\nCommand code: %d, byte count: %ld\n", \
	  msg, csave, plr->pdfs->bp); return -1; }

/* Static function prototypes. */

static int	plr_process1 	(PLRDev *plr, int c);
static int	plr_init	(PLRDev *plr);
static int	plr_line	(PLRDev *plr, int c);
static int	plr_eop		(PLRDev *plr);
static int	plr_bop 	(PLRDev *plr);
static int	plr_state	(PLRDev *plr);
static int	plr_esc		(PLRDev *plr);
static int	plr_get		(PLRDev *plr);
static int	plr_unget	(PLRDev *plr, U_CHAR c);
static int	get_ncoords	(PLRDev *plr, PLFLT *x, PLFLT *y, PLINT n);
static int	plresc_fill	(PLRDev *plr);

/* variables */

static int	csave = -1;
static U_CHAR	dum_uchar;
static U_SHORT	dum_ushort;
static PLFLT	x[PL_MAXPOLY], y[PL_MAXPOLY];

/*--------------------------------------------------------------------------*\
 * plr_start()
 *
 * Set default state parameters before anyone else has a chance to.
\*--------------------------------------------------------------------------*/

void
plr_start(PLRDev *plr)
{
    dbug_enter("plr_start");

    plr->xmin = 0;
    plr->xmax = PIXELS_X - 1;
    plr->ymin = 0;
    plr->ymax = PIXELS_Y - 1;

    plr->xold = PL_UNDEFINED;
    plr->yold = PL_UNDEFINED;
}

/*--------------------------------------------------------------------------*\
 * plr_process()
 *
 * Read & process commands until plr->nbytes bytes have been read.
\*--------------------------------------------------------------------------*/

int
plr_process(PLRDev *plr)
{
    int c;

    dbug_enter("plr_process");

    while (plr->pdfs->bp < plr->nbytes) {
	plr_cmd( c = plr_get(plr) );
	csave = c;
	plr_cmd( plr_process1(plr, c) );
    }
    return 0;
}

/*--------------------------------------------------------------------------*\
 * plr_process1()
 *
 * Process a command.  Note: because of line->polyline compression, this
 * may actually process an arbitrary number of LINE or LINETO commands.
 * Since the data buffer (fifo or socket) is only flushed after a complete
 * command, there should be no danger in rushing blindly ahead to execute
 * each plot command.
\*--------------------------------------------------------------------------*/

static int
plr_process1(PLRDev *plr, int c)
{
    switch (c) {

    case INITIALIZE:
	plr_cmd( plr_init(plr) );
	break;

    case LINE:
    case LINETO:
    case POLYLINE:
	plr_cmd( plr_line(plr, c) );
	break;

    case EOP:
	plr->at_eop = 1;
	plr_cmd( plr_eop(plr) );
	break;

    case BOP:
	plr->at_bop = 1;
	plr_cmd( plr_bop(plr) );
	break;

    case CHANGE_STATE:
	plr_cmd( plr_state(plr) );
	break;

    case ESCAPE:
	plr_cmd( plr_esc(plr) );
	break;

    default:
	fprintf(stderr, "plr_process1: Unrecognized command code %d\n", c);
    }

    return 0;
}

/*--------------------------------------------------------------------------*\
 * void plr_init()
 *
 * Handle initialization.
\*--------------------------------------------------------------------------*/

static int
plr_init(PLRDev *plr)
{
    char tk_magic[80], tk_version[80], tag[80];

    dbug_enter("plr_init");

/* Read header info */

    plr_cmd( pdf_rd_header(plr->pdfs, tk_magic) );
    if (strcmp(tk_magic, PLSERV_HEADER))
	barf("plr_init: Invalid header");

/* Read version field of header.  We need to check that we can read the */
/* byte stream, in case this is an old version of plserver. */

    plr_cmd( pdf_rd_header(plr->pdfs, tk_version) );
    if (strcmp(tk_version, PLSERV_VERSION) > 0) {
	fprintf(stderr,
	    "Error: incapable of reading output of version %s.\n", tk_version);
	barf("plr_init: Please obtain a newer copy of plserver.");
    }

/* Read tagged initialization info. */
/* Overkill, but a no-brainer since plrender already uses this */

    for (;;) {
	plr_cmd( pdf_rd_header(plr->pdfs, tag) );
	if (*tag == '\0')
	    break;

	if ( ! strcmp(tag, "xmin")) {
	    plr_rd( pdf_rd_2bytes(plr->pdfs, &dum_ushort) );
	    plr->xmin = dum_ushort;
	    continue;
	}

	if ( ! strcmp(tag, "xmax")) {
	    plr_rd( pdf_rd_2bytes(plr->pdfs, &dum_ushort) );
	    plr->xmax = dum_ushort;
	    continue;
	}

	if ( ! strcmp(tag, "ymin")) {
	    plr_rd( pdf_rd_2bytes(plr->pdfs, &dum_ushort) );
	    plr->ymin = dum_ushort;
	    continue;
	}

	if ( ! strcmp(tag, "ymax")) {
	    plr_rd( pdf_rd_2bytes(plr->pdfs, &dum_ushort) );
	    plr->ymax = dum_ushort;
	    continue;
	}

	if ( ! strcmp(tag, "width")) {
	    plr_rd( pdf_rd_1byte(plr->pdfs, &dum_uchar) );
	    plwid(dum_uchar);
	    continue;
	}

	barf("plr_init: Unrecognized initialization tag.");
    }

    return 0;
}

/*--------------------------------------------------------------------------*\
 * plr_line()
 *
 * Draw a line or polyline.
\*--------------------------------------------------------------------------*/

static int
plr_line(PLRDev *plr, int c)
{
    int c1;
    U_SHORT npts;

    npts = 1;
    x[0] = plr->xold;
    y[0] = plr->yold;

    switch ((int) c) {

    case LINE:
	plr_cmd( get_ncoords(plr, x, y, 1) );

    case LINETO:
	for (;;) {
	    plr_cmd( get_ncoords(plr, x + npts, y + npts, 1) );

	    npts++;
	    if (npts == PL_MAXPOLY || (plr->pdfs->bp == plr->nbytes))
		break;

	    plr_cmd( c1 = plr_get(plr) );
	    if (c1 != LINETO) {
		plr_cmd( plr_unget(plr, (U_CHAR) c1) );
		break;
	    }
	}
	break;

    case POLYLINE:
	plr_rd( pdf_rd_2bytes(plr->pdfs, &npts) );
	plr_cmd( get_ncoords(plr, x, y, npts) );
	break;
    }

    plline(npts, x, y);

    plr->xold = x[npts - 1];
    plr->yold = y[npts - 1];

    return 0;
}

/*--------------------------------------------------------------------------*\
 * get_ncoords()
 *
 * Read n coordinate vectors.
\*--------------------------------------------------------------------------*/

#define plr_rdn(code) \
if (code) { fprintf(stderr, \
"Unable to read from %s in %s at line %d, bytecount %d\n\
Bytes requested: %d\n", plr->iodev->typeName, __FILE__, __LINE__, \
(int) plr->pdfs->bp, (int) 2*n); return -1; }

static int
get_ncoords(PLRDev *plr, PLFLT *x, PLFLT *y, PLINT n)
{
    int i;
    short xs[PL_MAXPOLY], ys[PL_MAXPOLY];

    plr_rdn( pdf_rd_2nbytes(plr->pdfs, (U_SHORT *) xs, n) );
    plr_rdn( pdf_rd_2nbytes(plr->pdfs, (U_SHORT *) ys, n) );

    for (i = 0; i < n; i++) {
	x[i] = xs[i];
	y[i] = ys[i];
    }
    return 0;
}

/*--------------------------------------------------------------------------*\
 * plr_eop()
 *
 * Clear screen.
\*--------------------------------------------------------------------------*/

static int
plr_eop(PLRDev *plr)
{
    dbug_enter("plr_eop");

    pleop();
    return 0;
}

/*--------------------------------------------------------------------------*\
 * plr_bop()
 *
 * Page advancement.
\*--------------------------------------------------------------------------*/

static int
plr_bop(PLRDev *plr)
{
    dbug_enter("plr_bop");

/* Advance and setup the page */

    plbop();
    plvpor(0., 1., 0., 1.);
    plwind(plr->xmin, plr->xmax, plr->ymin, plr->ymax);

    return 0;
}

/*--------------------------------------------------------------------------*\
 * plr_state()
 *
 * Handle change in PLStream state (color, pen width, fill attribute,
 * etc).
\*--------------------------------------------------------------------------*/

static int
plr_state(PLRDev *plr)
{
    U_CHAR op;
    int i;

    plr_rd( pdf_rd_1byte(plr->pdfs, &op) );

    switch (op) {

    case PLSTATE_WIDTH:{
	U_SHORT width;

	plr_rd( pdf_rd_2bytes(plr->pdfs, &width) );

	plwid(width);
	break;
    }

    case PLSTATE_COLOR0:{
	short icol0;

	plr_rd( pdf_rd_2bytes(plr->pdfs, &icol0) );

	if (icol0 == PL_RGB_COLOR) {
            U_CHAR r, g, b;
	    plr_rd( pdf_rd_1byte(plr->pdfs, &r) );
	    plr_rd( pdf_rd_1byte(plr->pdfs, &g) );
	    plr_rd( pdf_rd_1byte(plr->pdfs, &b) );
	    plrgb1(r, g, b);
	}
	else {
	    plcol(icol0);
	}
	break;
    }

    case PLSTATE_COLOR1:{
	U_SHORT icol1;
	PLFLT col1;

	plr_rd( pdf_rd_2bytes(plr->pdfs, &icol1) );
	col1 = (double) icol1 / (double) plsc->ncol1;
	plcol1(col1);
	break;
    }

    case PLSTATE_FILL:{
	signed char patt;

	plr_rd( pdf_rd_1byte(plr->pdfs, (U_CHAR *) &patt) );
	plpsty(patt);
	break;
    }

    case PLSTATE_CMAP0:{
	U_SHORT ncol0;

	plr_rd( pdf_rd_2bytes(plr->pdfs, &ncol0) );
	plscmap0n((PLINT) ncol0);
	for (i = 0; i < plsc->ncol0; i++) {
	    plr_rd( pdf_rd_1byte(plr->pdfs, &plsc->cmap0[i].r) );
	    plr_rd( pdf_rd_1byte(plr->pdfs, &plsc->cmap0[i].g) );
	    plr_rd( pdf_rd_1byte(plr->pdfs, &plsc->cmap0[i].b) );
	}
	plP_state(PLSTATE_CMAP0);
	break;
    }

    case PLSTATE_CMAP1:{
	U_SHORT ncol1, ncp1;
	float h, l, s;
	U_CHAR rev;

	plr_rd( pdf_rd_2bytes(plr->pdfs, &ncol1) );
	plscmap1n((PLINT) ncol1);
	for (i = 0; i < plsc->ncol1; i++) {
	    plr_rd( pdf_rd_1byte(plr->pdfs, &plsc->cmap1[i].r) );
	    plr_rd( pdf_rd_1byte(plr->pdfs, &plsc->cmap1[i].g) );
	    plr_rd( pdf_rd_1byte(plr->pdfs, &plsc->cmap1[i].b) );
	}
    /* Get the control points */
	plr_rd( pdf_rd_2bytes(plr->pdfs, &ncp1) );
	plsc->ncp1 = ncp1;
	for (i = 0; i < plsc->ncp1; i++) {
	    plr_rd( pdf_rd_ieeef(plr->pdfs, &h) );
	    plr_rd( pdf_rd_ieeef(plr->pdfs, &l) );
	    plr_rd( pdf_rd_ieeef(plr->pdfs, &s) );
	    plr_rd( pdf_rd_1byte(plr->pdfs, &rev) );

	    plsc->cmap1cp[i].h = h;
	    plsc->cmap1cp[i].l = l;
	    plsc->cmap1cp[i].s = s;
	    plsc->cmap1cp[i].rev = rev;
	}
	plP_state(PLSTATE_CMAP1);
	break;
    }
    }

    return 0;
}

/*--------------------------------------------------------------------------*\
 * plr_esc()
 *
 * Handle all escape functions.
 * Only those that require additional data to be read need to be
 * explicitly handled; the others are merely passed on to the actual
 * driver. 
\*--------------------------------------------------------------------------*/

static int
plr_esc(PLRDev *plr)
{
    U_CHAR op;

    plr_rd( pdf_rd_1byte(plr->pdfs, &op) );

    switch (op) {

    case PLESC_FILL:
	plr_cmd( plresc_fill(plr) );
	break;

    default:
        pl_cmd((PLINT) op, NULL);
	break;
    }

    return 0;
}

/*--------------------------------------------------------------------------*\
 * plresc_fill()
 *
 * Fill polygon described in points plsc->dev_x[] and plsc->dev_y[].
\*--------------------------------------------------------------------------*/

static int
plresc_fill(PLRDev *plr)
{
    U_SHORT npts;

    dbug_enter("plresc_fill");

    plr_rd( pdf_rd_2bytes(plr->pdfs, &npts) );
    get_ncoords(plr, x, y, npts);
    plfill(npts, x, y);

    return 0;
}

/*--------------------------------------------------------------------------*\
 * plr_get()
 *
 * Read & return the next command
\*--------------------------------------------------------------------------*/

static int
plr_get(PLRDev *plr)
{
    int c;

    c = pdf_getc(plr->pdfs);
    if (c == EOF) {
	barf("plr_get: Unable to read character");
    }

    return c;
}

/*--------------------------------------------------------------------------*\
 * plr_unget()
 *
 * Push back the last command read.
\*--------------------------------------------------------------------------*/

static int
plr_unget(PLRDev *plr, U_CHAR c)
{
    if (pdf_ungetc(c, plr->pdfs) == EOF) {
	barf("plr_unget: Unable to push back character");
    }

    return 0;
}
