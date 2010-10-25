/* $Id: next.c 3186 2006-02-15 18:17:33Z slbrow $

	PLplot NeXT display driver.

	Attention: this driver needs a supporter.  It may not even work
	any more.  Also, there have been many improvements to ps.c that
	were not incorporated here.  If you use this please consider
	becoming the official maintainer.  Email mjl@dino.ph.utexas.edu
	for more info.
*/
#include "plDevs.h"

#ifdef PLD_next

#include "plplotP.h"
#include "drivers.h"

/* top level declarations */

#define LINELENGTH      70
#define COPIES          1
#define XSIZE           540	/* 7.5" x 7.5"  (72 points equal 1 inch) */
#define YSIZE           540
#define ENLARGE         5
#define XPSSIZE         ENLARGE*XSIZE
#define YPSSIZE         ENLARGE*YSIZE
#define XOFFSET         18
#define YOFFSET         18
#define XSCALE          100
#define YSCALE          100
#define LINESCALE       100
#define ANGLE           90
#define PSX             XPSSIZE-1
#define PSY             YPSSIZE-1
#define OF		pls->OutFile
#define MIN_WIDTH	1		/* Minimum pen width */
#define MAX_WIDTH	10		/* Maximum pen width */
#define DEF_WIDTH	3		/* Default pen width */

static char outbuf[128];
static int llx = XPSSIZE, lly = YPSSIZE, urx = 0, ury = 0, ptcnt;

/*------------------------------------------------------------------------*\
 * plD_init_nx()
 *
 * Initialize device.
\*------------------------------------------------------------------------*/

void
plD_init_nx(PLStream *pls)
{
    PLDev *dev;

/* Allocate and initialize device-specific data */

    dev = plAllocDev(pls);

    dev->xold = PL_UNDEFINED;
    dev->yold = PL_UNDEFINED;
    dev->xmin = 0;
    dev->xmax = PSX;
    dev->ymin = 0;
    dev->ymax = PSY;

    plP_setpxl((PLFLT) 11.81, (PLFLT) 11.81);	/* 300 dpi */

    plP_setphy(0, PSX, 0, PSY);
}

/*------------------------------------------------------------------------*\
 * plD_line_nx()
 *
 * Draw a line in the current color from (x1,y1) to (x2,y2).
\*------------------------------------------------------------------------*/

void
plD_line_nx(PLStream *pls, short x1a, short y1a, short x2a, short y2a)
{
    PLDev *dev = (PLDev *) pls->dev;
    int x1 = x1a, y1 = y1a, x2 = x2a, y2 = y2a;
    int ori;

    if (pls->linepos + 21 > LINELENGTH) {
	putc('\n', OF);
	pls->linepos = 0;
    }
    else
	putc(' ', OF);

    pls->bytecnt++;

    if (x1 == dev->xold && y1 == dev->yold && ptcnt < 40) {
	sprintf(outbuf, "%d %d D", x2, y2);
	ptcnt++;
    }
    else {
	sprintf(outbuf, "Z %d %d M %d %d D", x1, y1, x2, y2);
	llx = MIN(llx, x1);
	lly = MIN(lly, y1);
	urx = MAX(urx, x1);
	ury = MAX(ury, y1);
	ptcnt = 1;
    }
    llx = MIN(llx, x2);
    lly = MIN(lly, y2);
    urx = MAX(urx, x2);
    ury = MAX(ury, y2);

    fprintf(OF, "%s", outbuf);
    pls->bytecnt += strlen(outbuf);
    dev->xold = x2;
    dev->yold = y2;
    pls->linepos += 21;
}

/*------------------------------------------------------------------------*\
 * plD_polyline_nx()
 *
 * Draw a polyline in the current color.
\*------------------------------------------------------------------------*/

void
plD_polyline_nx(PLStream *pls, short *xa, short *ya, PLINT npts)
{
    PLINT i;

    for (i = 0; i < npts - 1; i++)
	plD_line_nx(pls, xa[i], ya[i], xa[i + 1], ya[i + 1]);
}

/*------------------------------------------------------------------------*\
 * plD_eop_nx()
 *
 * End of page.
\*------------------------------------------------------------------------*/

void
plD_eop_nx(PLStream *pls)
{
    fprintf(OF, " S\neop\n");

    pclose(OF);
}

/*------------------------------------------------------------------------*\
 * plD_bop_nx()
 *
 * Set up for the next page.
\*------------------------------------------------------------------------*/

void
plD_bop_nx(PLStream *pls)
{
    PLDev *dev = (PLDev *) pls->dev;

    dev->xold = PL_UNDEFINED;
    dev->yold = PL_UNDEFINED;

/* Pipe output to Preview */

    OF = popen("open", "w");

/* Header comments */

    fprintf(OF, "%%!PS-Adobe-2.0 EPSF-2.0\n");
    fprintf(OF, "%%%%Title: PLplot Graph\n");
    fprintf(OF, "%%%%Creator: PLplot Version %s\n", PLPLOT_VERSION);
    fprintf(OF, "%%%%BoundingBox: 0 0 576 576\n");
    fprintf(OF, "%%%%EndComments\n\n");

/* Definitions */

/* - eop -  -- end a page */

    fprintf(OF, "/eop\n");
    fprintf(OF, "   {\n");
    fprintf(OF, "    showpage\n");
    fprintf(OF, "   } def\n");

/* Set line parameters */

    fprintf(OF, "/@line\n");
    fprintf(OF, "   {0 setlinecap\n");
    fprintf(OF, "    0 setlinejoin\n");
    fprintf(OF, "    1 setmiterlimit\n");
    fprintf(OF, "   } def\n");

/* d @hsize -  horizontal clipping dimension */

    fprintf(OF, "/@hsize   {/hs exch def} def\n");
    fprintf(OF, "/@vsize   {/vs exch def} def\n");

/* d @hoffset - shift for the plots */

    fprintf(OF, "/@hoffset {/ho exch def} def\n");
    fprintf(OF, "/@voffset {/vo exch def} def\n");

/* s @hscale - scale factors */

    fprintf(OF, "/@hscale  {100 div /hsc exch def} def\n");
    fprintf(OF, "/@vscale  {100 div /vsc exch def} def\n");

/* Set line width */

    fprintf(OF, "/lw %d def\n", (int) (
	(pls->width < MIN_WIDTH) ? DEF_WIDTH :
	(pls->width > MAX_WIDTH) ? MAX_WIDTH : pls->width));

/* Setup user specified offsets, scales, sizes for clipping */

    fprintf(OF, "/@SetPlot\n");
    fprintf(OF, "   {\n");
    fprintf(OF, "    ho vo translate\n");
    fprintf(OF, "    XScale YScale scale\n");
    fprintf(OF, "    lw setlinewidth\n");
    fprintf(OF, "   } def\n");

/* Setup x & y scales */

    fprintf(OF, "/XScale\n");
    fprintf(OF, "   {hsc hs mul %d div} def\n", YPSSIZE);
    fprintf(OF, "/YScale\n");
    fprintf(OF, "   {vsc vs mul %d div} def\n", XPSSIZE);

/* Macro definitions of common instructions, to keep output small */

    fprintf(OF, "/M {moveto} def\n");
    fprintf(OF, "/D {lineto} def\n");
    fprintf(OF, "/S {stroke} def\n");
    fprintf(OF, "/Z {stroke newpath} def\n");
    fprintf(OF, "/F {fill} def\n");
    fprintf(OF, "/C {setrgbcolor} def\n");
    fprintf(OF, "/G {setgray} def\n");
    fprintf(OF, "/W {setlinewidth} def\n");

/* Set up the plots */

    fprintf(OF, "@line\n");
    fprintf(OF, "%d @hsize\n", YSIZE);
    fprintf(OF, "%d @vsize\n", XSIZE);
    fprintf(OF, "%d @hoffset\n", YOFFSET);
    fprintf(OF, "%d @voffset\n", XOFFSET);
    fprintf(OF, "%d @hscale\n", YSCALE);
    fprintf(OF, "%d @vscale\n", XSCALE);
    fprintf(OF, "@SetPlot\n\n");
    pls->page++;
    pls->linepos = 0;
}

/*------------------------------------------------------------------------*\
 * plD_tidy_nx()
 *
 * Close graphics file or otherwise clean up.
\*------------------------------------------------------------------------*/

void
plD_tidy_nx(PLStream *pls)
{
}

/*------------------------------------------------------------------------*\
 * plD_state_nx()
 *
 * Handle change in PLStream state (color, pen width, fill attribute, etc).
\*------------------------------------------------------------------------*/

void 
plD_state_nx(PLStream *pls, PLINT op)
{
    switch (op) {

    case PLSTATE_WIDTH:{
	int width = 
	    (pls->width < MIN_WIDTH) ? DEF_WIDTH :
	    (pls->width > MAX_WIDTH) ? MAX_WIDTH : pls->width;

	fprintf(OF, " S\n%d W", width);

	dev->xold = PL_UNDEFINED;
	dev->yold = PL_UNDEFINED;
	break;
    }
    case PLSTATE_COLOR0:
	break;

    case PLSTATE_COLOR1:
	break;
    }
}

/*------------------------------------------------------------------------*\
 * plD_esc_nx()
 *
 * Escape function.
\*------------------------------------------------------------------------*/

void
plD_esc_nx(PLStream *pls, PLINT op, void *ptr)
{
}

#else
int 
pldummy_next()
{
    return 0;
}

#endif			/* PLD_next */
