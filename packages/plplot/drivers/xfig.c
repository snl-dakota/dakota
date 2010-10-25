/* $Id: xfig.c 3186 2006-02-15 18:17:33Z slbrow $

	PLplot xfig device driver.
*/
#include "plDevs.h"

#ifdef PLD_xfig

#include "plplotP.h"
#include "drivers.h"

/* Device info */
char* plD_DEVICE_INFO_xfig = "xfig:Fig file:0:xfig:31:xfig";

/* Function prototypes */

void plD_dispatch_init_xfig	( PLDispatchTable *pdt );

void plD_init_xfig		(PLStream *);
void plD_line_xfig		(PLStream *, short, short, short, short);
void plD_polyline_xfig		(PLStream *, short *, short *, PLINT);
void plD_eop_xfig		(PLStream *);
void plD_bop_xfig		(PLStream *);
void plD_tidy_xfig		(PLStream *);
void plD_state_xfig		(PLStream *, PLINT);
void plD_esc_xfig		(PLStream *, PLINT, void *);

static void flushbuffer(PLStream *);

/* top level declarations */

#define FIGX	297 /* portrait A4 mm */
#define FIGY    210
#define DPI	1200

/* it looks like xfig-3.2.3c has a bug. A4 papersize is 297x210 mm,
   and at 1200 dpi this gives 14031x9921 dots. In a file saved from
   xfig, with a box of A4 size, the reported sizes are 13365x9450 */

#define BSIZE	25
#define XFIG_COLBASE 33 /* xfig first user color, plplot colormap0[0],
                           the background color */

static short *buffptr, bufflen;
static short count;
static int   curwid = 1;
static int   curcol = 1;
static int   firstline = 1;
static long  cmap0_pos, cmap1_pos;
static int   cmap0_ncol, cmap1_ncol;
static int   offset, offset_inc;

static void stcmap0(PLStream *);
static void stcmap1(PLStream *);
static void proc_str (PLStream *, EscText *);

static int text=0;

static DrvOpt xfig_options[] = {{"text", DRV_INT, &text, "Use Postscript text (text=1|0)"},
				  {NULL, DRV_INT, NULL, NULL}};

void plD_dispatch_init_xfig( PLDispatchTable *pdt )
{
#ifndef ENABLE_DYNDRIVERS
    pdt->pl_MenuStr  = "Xfig file";
    pdt->pl_DevName  = "xfig";
#endif
    pdt->pl_type     = plDevType_FileOriented;
    pdt->pl_seq      = 31;
    pdt->pl_init     = (plD_init_fp)     plD_init_xfig;
    pdt->pl_line     = (plD_line_fp)     plD_line_xfig;
    pdt->pl_polyline = (plD_polyline_fp) plD_polyline_xfig;
    pdt->pl_eop      = (plD_eop_fp)      plD_eop_xfig;
    pdt->pl_bop      = (plD_bop_fp)      plD_bop_xfig;
    pdt->pl_tidy     = (plD_tidy_fp)     plD_tidy_xfig;
    pdt->pl_state    = (plD_state_fp)    plD_state_xfig;
    pdt->pl_esc      = (plD_esc_fp)      plD_esc_xfig;
}

/*--------------------------------------------------------------------------*\
 * plD_init_xfig()
 *
 * Initialize device.
\*--------------------------------------------------------------------------*/

void
plD_init_xfig(PLStream *pls)
{
    PLDev *dev;

    plParseDrvOpts(xfig_options);
    if (text)
      pls->dev_text = 1; /* want to draw text */

/* Initialize family file info */

    plFamInit(pls);

/* Prompt for a file name if not already set */

    plOpenFile(pls);

/* Allocate and initialize device-specific data */

    dev = plAllocDev(pls);

    dev->xold = PL_UNDEFINED;
    dev->yold = PL_UNDEFINED;
    dev->xmin = 0;
    dev->xmax = FIGX;
    dev->ymin = 0;
    dev->ymax = FIGY;
    dev->xscale_dev = DPI/25.4;
    dev->yscale_dev = DPI/25.4;
    offset_inc = dev->ymax * (PLINT)dev->yscale_dev;
    offset = - offset_inc;
    pls->dev_fill0 = 1;	    /* Handle solid fills */
    if (!pls->colorset)
      pls->color = 1;         /* Is a color device */

    plP_setpxl(dev->xscale_dev, dev->xscale_dev); /* dpmm -- dots per mm */
    plP_setphy(0, FIGX * dev->xscale_dev, 0, FIGY * dev->yscale_dev); /* physical dimension in mm */ 

/* Write out header */

    fprintf(pls->OutFile, "#FIG 3.2\n");
    fprintf(pls->OutFile, "Landscape\n");
    fprintf(pls->OutFile, "Center\n");
    fprintf(pls->OutFile, "Metric\n");
    fprintf(pls->OutFile, "A4\n");
    fprintf(pls->OutFile, "100.0\n");
    fprintf(pls->OutFile, "Single\n");
    fprintf(pls->OutFile, "-2\n");
    fprintf(pls->OutFile, "%d 2\n", DPI);

    /* user defined colors, for colormap0 */
    cmap0_ncol = 2 * pls->ncol0; /* allow for a maximum of 2x the default cmap0 entries */
    cmap0_pos = ftell(pls->OutFile);
    stcmap0(pls);

    /* user defined colors, for colormap1 */
    cmap1_ncol = 2 * pls->ncol1; /* allow for a maximum of  2x the default cmap1 entries */
    cmap1_pos = ftell(pls->OutFile); 
    stcmap1(pls);

    bufflen = 2 * BSIZE;
    buffptr = (short *) malloc(sizeof(short) * bufflen);
    if (buffptr == NULL)
	plexit("Out of memory!");
}

void
stcmap0(PLStream *pls)
{
  long cur_pos;
  int i;

  if (pls->ncol0 > cmap0_ncol)
    plwarn("Too much colors for cmap0. Preallocate using command line '-ncol0 n.\n'");

      cur_pos = ftell(pls->OutFile);

      if (fseek(pls->OutFile, cmap0_pos, SEEK_SET))
	plexit("Sorry, only file based output, no pipes.\n");

      /* fill the colormap */
      for (i=0; i<pls->ncol0; i++)
	fprintf(pls->OutFile,"0 %d #%.2x%.2x%.2x\n", i+XFIG_COLBASE,
		pls->cmap0[i].r, pls->cmap0[i].g, pls->cmap0[i].b);

      /* fill the nonspecified entries colormap */
      for (i=pls->ncol0; i<cmap0_ncol; i++)
	fprintf(pls->OutFile,"0 %d #000000\n", i+XFIG_COLBASE);

      if (cur_pos != cmap0_pos)
	fseek(pls->OutFile, cur_pos, SEEK_SET);
}

void
stcmap1(PLStream *pls)
{
  long cur_pos;
  int i;

  if (pls->ncol1 > cmap1_ncol)
    plwarn("Too much colors for cmap1. Preallocate using command line '-ncol1 n.\n'");

      cur_pos = ftell(pls->OutFile);

      if (fseek(pls->OutFile, cmap1_pos, SEEK_SET))
	plexit("Sorry, only file based output, no pipes.\n");

      /* fill the colormap */
      for (i=0; i<pls->ncol1; i++)
	fprintf(pls->OutFile,"0 %d #%.2x%.2x%.2x\n", i+XFIG_COLBASE+cmap0_ncol,
		pls->cmap1[i].r, pls->cmap1[i].g, pls->cmap1[i].b);

      /* fill the nonspecified entries colormap */
      for (i=pls->ncol1; i<cmap1_ncol; i++)
	fprintf(pls->OutFile,"0 %d #000000\n", i+XFIG_COLBASE+cmap0_ncol);

      if (cur_pos != cmap1_pos)
	fseek(pls->OutFile, cur_pos, SEEK_SET);
}

/*--------------------------------------------------------------------------*\
 * plD_line_xfig()
 *
 * Draw a line in the current color from (x1,y1) to (x2,y2).
\*--------------------------------------------------------------------------*/

void
plD_line_xfig(PLStream *pls, short x1a, short y1a, short x2a, short y2a)
{
    PLDev *dev = (PLDev *) pls->dev;
    int x1 = x1a, y1 = y1a, x2 = x2a, y2 = y2a;
    short *tempptr;

/* If starting point of this line is the same as the ending point of */
/* the previous line then don't raise the pen. (This really speeds up */
/* plotting and reduces the size of the file. */

    if (firstline) {
	count = 0;
	*(buffptr + count++) = x1;
	*(buffptr + count++) = y1;
	*(buffptr + count++) = x2;
	*(buffptr + count++) = y2;
	firstline = 0;
    }
    else if (x1 == dev->xold && y1 == dev->yold) {
	if (count + 2 >= bufflen) {
	    bufflen += 2 * BSIZE;
	    tempptr = (short *)
		realloc((void *) buffptr, bufflen * sizeof(short));
	    if (tempptr == NULL) {
		free((void *) buffptr);
		plexit("Out of memory!");
	    }
	    buffptr = tempptr;
	}
	*(buffptr + count++) = x2;
	*(buffptr + count++) = y2;
    }
    else {
	flushbuffer(pls);
	*(buffptr + count++) = x1;
	*(buffptr + count++) = y1;
	*(buffptr + count++) = x2;
	*(buffptr + count++) = y2;
    }
    dev->xold = x2;
    dev->yold = y2;
}

/*--------------------------------------------------------------------------*\
 * plD_polyline_xfig()
 *
 * Draw a polyline in the current color.
\*--------------------------------------------------------------------------*/

void
plD_polyline_xfig(PLStream *pls, short *xa, short *ya, PLINT npts)
{
    PLINT i;

    for (i = 0; i < npts - 1; i++)
	plD_line_xfig(pls, xa[i], ya[i], xa[i + 1], ya[i + 1]);
}

/*--------------------------------------------------------------------------*\
 * plD_eop_xfig()
 *
 * End of page.
\*--------------------------------------------------------------------------*/

void
plD_eop_xfig(PLStream *pls)
{
    if (!firstline)
	flushbuffer(pls);
}

/*--------------------------------------------------------------------------*\
 * plD_bop_xfig()
 *
 * Set up for the next page.
 * Advance to next family file if necessary (file output).
\*--------------------------------------------------------------------------*/

void
plD_bop_xfig(PLStream *pls)
{
    PLDev *dev = (PLDev *) pls->dev;

    dev->xold = PL_UNDEFINED;
    dev->yold = PL_UNDEFINED;
    firstline = 1;

    if (!pls->termin)
      plGetFam(pls);

    pls->famadv = 1;
    pls->page++;

    offset += offset_inc;
    flushbuffer(pls);
    
    /* create background FIXME -- sync with orientation in header and pls->diorot */
    curcol = XFIG_COLBASE; /* colormap entry 0, background */
    fprintf(pls->OutFile, "2 1 0 1 %d %d 50 0 20 0.0 0 0 -1 0 0 5\n", curcol, curcol );
    fprintf(pls->OutFile, "%d %d %d %d %d %d %d %d %d %d\n",
	    0, offset,
	    0, (int) (FIGY * dev->yscale_dev) + offset,
	    (int) (FIGX * dev->xscale_dev), (int) (FIGY * dev->yscale_dev) + offset,
	    (int) (FIGX * dev->xscale_dev), offset,
	    0, offset);
    
}

/*--------------------------------------------------------------------------*\
 * plD_tidy_xfig()
 *
 * Close graphics file or otherwise clean up.
\*--------------------------------------------------------------------------*/

void
plD_tidy_xfig(PLStream *pls)
{
    flushbuffer(pls);
    free((void *) buffptr);
    fclose(pls->OutFile);
}

/*--------------------------------------------------------------------------*\
 * plD_state_xfig()
 *
 * Handle change in PLStream state (color, pen width, fill attribute, etc).
\*--------------------------------------------------------------------------*/

void 
plD_state_xfig(PLStream *pls, PLINT op)
{
  switch (op) {

  case PLSTATE_WIDTH:
    flushbuffer(pls);
    firstline = 1;
    curwid = pls->width < 1 ? 1: pls->width;
    break;

  case PLSTATE_COLOR0:
    flushbuffer(pls);
    curcol =  pls->icol0 + XFIG_COLBASE;
    break;

  case PLSTATE_COLOR1:
    flushbuffer(pls);
    curcol =  pls->icol1 + XFIG_COLBASE + pls->ncol0;
    break;

  case PLSTATE_CMAP0:
    stcmap0(pls);
    break;

  case PLSTATE_CMAP1:
    stcmap1(pls);
    break;
  }

}

/*--------------------------------------------------------------------------*\
 * plD_esc_xfig()
 *
 * Escape function.
 * Preliminary fill support for colormap0
\*--------------------------------------------------------------------------*/

void
plD_esc_xfig(PLStream *pls, PLINT op, void *ptr)
{
  PLDev *dev = pls->dev;
  int i, npts;
 
  switch (op) { 
  case PLESC_FILL:

    npts = pls->dev_npts;
    if (npts > PL_MAXPOLY)
      plexit("FillPolygonCmd: Too many points in polygon\n");
     
    flushbuffer(pls);
    fprintf(pls->OutFile, "2 1 0 1 %d %d 50 0 20 0.0 0 0 0 0 0 %d\n",
	    curcol, curcol, npts);

    for (i = 0; i < npts; i++)
      fprintf(pls->OutFile,"%d %d ",  pls->dev_x[i],
	      offset + dev->ymax * (int)dev->xscale_dev - pls->dev_y[i]);

    fprintf(pls->OutFile, "\n");
    break;

  case PLESC_HAS_TEXT:
    proc_str(pls, ptr);
    break;
  }  
}

/*--------------------------------------------------------------------------*\
 * Utility functions.
\*--------------------------------------------------------------------------*/

static void
flushbuffer(PLStream *pls)
{
  PLDev *dev = pls->dev;
  short i = 0;

  if (count == 0)
    return;

  fprintf(pls->OutFile, "2 1 0 %d %d 0 50 0 -1 0.0 0 0 0 0 0 %d\n",
	  curwid, curcol, count/2);
  while (i < count) {
    fprintf(pls->OutFile, "%d %d ", *(buffptr + i),
	     offset + dev->ymax * (int)dev->yscale_dev - *(buffptr + i + 1));
    i += 2;
  }
  fprintf(pls->OutFile, "\n");
  count = 0;
}

void
proc_str (PLStream *pls, EscText *args)
{
  PLFLT *t = args->xform;
  PLFLT a1, alpha, ft_ht, angle, ref;
  PLDev *dev = (PLDev *) pls->dev;
  PLINT clxmin, clxmax, clymin, clymax;
  int jst, font;

  /* font height */
  ft_ht = pls->chrht * 72.0/25.4; /* ft_ht in points. ht is in mm */

  /* calculate baseline text angle */
  angle = pls->diorot * 90.;
  a1 = acos(t[0]) * 180. / PI;
  if (t[2] > 0.)
    alpha = a1 - angle;
  else
    alpha = 360. - a1 - angle;

  alpha = alpha * PI / 180.;

  /* TODO: parse string for format (escape) characters */
  /* parse_str(args->string, return_string);*/

  /* apply transformations */
  difilt(&args->x, &args->y, 1, &clxmin, &clxmax, &clymin, &clymax);

  /* check clip limits. For now, only the reference point of the string is checked;
     but the the whole string should be checked -- using a postscript construct
     such as gsave/clip/grestore. This method can also be applied to the xfig and
     pstex drivers. Zoom side effect: the font size must be adjusted! */

  if ( args->x < clxmin || args->x > clxmax || args->y < clymin || args->y > clymax)
    return;

  /* 
   * Text justification.  Left, center and right justification, which
   *  are the more common options, are supported; variable justification is
   *  only approximate, based on plplot computation of it's string lenght
   */

  if (args->just == 0.5)
    jst = 1; /* center */
  else if (args->just == 1.)
    jst = 2; /* right */
  else {
    jst = 0; /* left */
    args->x = args->refx; /* use hints provided by plplot */
    args->y = args->refy;
  }

  /* 
   * Reference point (center baseline of string, not latex character reference point). 
   *  If base = 0, it is aligned with the center of the text box
   *  If base = 1, it is aligned with the baseline of the text box
   *  If base = 2, it is aligned with the top of the text box
   *  Currently plplot only uses base=0
   *  xfig use base=1
   */ 

  if (args->base == 2) /* not supported by plplot */
    ref = - DPI/72. * ft_ht / 2.; /* half font height in xfig unities (1/1200 inches) */
  else if (args->base == 1)
    ref = 0.;
  else
    ref = DPI/72. * ft_ht / 2.;

  /* rotate point in xfig is lower left corner, compensate */
  args->y = offset + dev->ymax * (int)dev->xscale_dev - (args->y - ref*cos(alpha));
  args->x = args->x + ref*sin(alpha);

  /*
   *  font family, serie and shape. Currently not supported by plplot
   *
   *  Use Postscript Times
   *  1: Normal font
   *  2: Roman font
   *  3: Italic font
   *  4: sans serif
   */

  switch (pls->cfont) {
  case (1): font = 0; break;
  case (2): font = 1; break;
  case (3): font = 3; break;
  case (4): font = 4; break;
  default:  font = 0;
  }

  fprintf(pls->OutFile,"4 %d %d 50 0 %d %f %f 4 1 1 %d %d %s\\001\n",
	  jst, curcol, font, 1.8 /*!*/ * ft_ht, alpha, args->x, args->y, args->string);

}

#else
int 
pldummy_xfig()
{
    return 0;
}

#endif				/* PLD_xfig */
