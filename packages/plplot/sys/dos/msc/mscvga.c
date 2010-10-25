/* ///////////////////////////////////////////////////////////////////
//  name change from <dosvga.c> since the driver is microsoft specific
//
//  File:       mscvga.c                                         [msc]
//
//  Descript:   the VGA display driver for DOS or OS/2 DOS mode
//
//  Library:    plplot
//
//  Requires:   Miscrosoft C 7.0 (6.0 ?)
//
//  Public:     plD_init_vga()
//              plD_line_vga()
//              plD_polyline_vga()
//              plD_eop_vga()
//              plD_bop_vga()
//              plD_tidy_vga()
//              plD_state_vga()
//              plD_esc_vga()
//
//              pldummy_mscvga()
//
//  Private:    pause()
//              vga_graphics()
//              vga_text()
//
//  Notes:      ---
//
//  Revisions:
//  13 Apr 94   mjo     modified to use graphics calls for C7
//                      "cleaned up" polyline routine
//  -- --- --   ---     ---
//  below is the original header
//
// Revision 1.11  1993/07/31  07:56:28  mjl
// Several driver functions consolidated, for all drivers.  The width and color
// commands are now part of a more general "state" command.  The text and
// graph commands used for switching between modes is now handled by the
// escape function (very few drivers require it).  The device-specific PLDev
// structure is now malloc'ed for each driver that requires it, and freed when
// the stream is terminated.
//
// Revision 1.10  1993/07/16  22:11:14  mjl
// Eliminated low-level coordinate scaling; now done by driver interface.
//
// Revision 1.9  1993/07/01  21:59:32  mjl
// Changed all plplot source files to include plplotP.h (private) rather than
// plplot.h.  Rationalized namespace -- all externally-visible plplot functions
// now start with "pl"; device driver functions start with "plD_".
//
/////////
//      dosvga.c
//      Geoffrey Furnish
//      5-1-91
//
//      This file constitutes the driver for a VGA display under DOS or
//      OS/2 DOS mode.  It is based on the xterm driver.
//
//      NOTE:
//      This file is intended to be compiled with Microsoft C or QuickC.
//      I use graphics functions from their library
//
//      The Geoffrey Furnish Standard Disclaimer:
//      "I hate any C compiler that isn't ANSI compliant, and I refuse to waste
//      my time trying to support such garbage.  If you can't compile with an
//      ANSI compiler, then don't expect this to work.  No appologies,
//      now or ever."
//
/////////////////////////////////////////////////////////////////// */

#if defined (_MSC_VER) && defined(MSDOS)  /* Only compile for DOS and Microsoft */

#include "plplot/plplotP.h"
#include <stdio.h>
#include <stdlib.h>
#include "plplot/drivers.h"
#include <graph.h>
#include <conio.h>

static void	pause		(PLStream *);
static void     vga_text        (PLStream *);
static void     vga_graphics    (PLStream *);

#ifndef TRUE
  #define TRUE 1                /* define truth */
  #define FALSE 0
#endif

/* the initialization values are _only_ for reference - not used */
static short
    isPageDirty = FALSE,
    DetectedVideoMode = _VRES16COLOR,
    vgaXres = 639,
    vgaYres = 479;

/*----------------------------------------------------------------------*\
* plD_init_vga()
*
* Initialize device.
\*----------------------------------------------------------------------*/

void
plD_init_vga(PLStream *pls)
{                               /* ~~~~ */
#if !(_MSC_VER < 700)
  struct _videoconfig vcfg;
#endif

  pls->termin = 1;              /* is an interactive terminal */
  pls->icol0 = 1;
  pls->width = 1;
  pls->bytecnt = 0;
  pls->page = 0;
  pls->graphx = FALSE;  /* assume not in graphics mode (ie. text mode) */
  if (!pls->colorset)
    pls->color = 1;

#if (_MSC_VER >= 700)
  _setvideomode( _MAXRESMODE );       /* try for max resolution */
  _getvideoconfig( &vcfg );           /* get the new setup */
  if ( vcfg.numcolors < 16 ) {          /* 16 colours ? */
    if ( !_setvideomode(_VRES16COLOR) ) {       /* try for 16 colours ? */
      fputs("Unable to set graphics mode.", stderr);
      exit(0);
    }
    _getvideoconfig( &vcfg );         /* got something */
  }

  DetectedVideoMode = vcfg.mode;
  vgaXres = vcfg.numxpixels -1;
  vgaYres = vcfg.numypixels -1;
#else   /* (_MSC_VER >= 700) */
  if ( !_setvideomode(_VRES16COLOR) ) {         /* try for 16 colours ? */
    fputs("Unable to set graphics mode.", stderr);
    exit(0);
  }
  DetectedVideoMode = _VRES16COLOR;
  vgaXres = 640 -1;
  vgaYres = 480 -1;
#endif  /* (_MSC_VER >= 700) */
  pls->graphx = TRUE;
  isPageDirty = FALSE;

/* Set up device parameters */
  plP_setpxl(2.5, 2.5); /* My best guess.  Seems to work okay. */

  plP_setphy((PLINT) 0, (PLINT) vgaXres, (PLINT) 0, (PLINT) vgaYres );
}

/*----------------------------------------------------------------------*\
* plD_line_vga()
*
* Draw a line in the current color from (x1,y1) to (x2,y2).
\*----------------------------------------------------------------------*/

void
plD_line_vga(PLStream *pls, short x1a, short y1a, short x2a, short y2a)
{                               /* ~~~~ */
  isPageDirty = TRUE;
  _moveto( (short) x1a, (short) (vgaYres - y1a) );
  _lineto( (short) x2a, (short) (vgaYres - y2a) );
}

/*----------------------------------------------------------------------*\
* plD_polyline_vga()
*
* Draw a polyline in the current color.
\*----------------------------------------------------------------------*/

void
plD_polyline_vga(PLStream *pls, short *xa, short *ya, PLINT npts)
{                               /* ~~~~ */
  register PLINT i;

  isPageDirty = TRUE;
  _moveto( (short) xa[0], (short) (vgaYres - ya[0]) );
  for ( i = 1; i < npts; i++ )
    _lineto( (short) xa[i], (short) (vgaYres - ya[i]) );
}

/*----------------------------------------------------------------------*\
* plD_eop_vga()
*
* End of page.
\*----------------------------------------------------------------------*/

void
plD_eop_vga(PLStream *pls)
{                               /* ~~~~ */
  if ( isPageDirty == TRUE )
    pause(pls);
#if ( _MSC_VER < 700 )
  _eopscreen(_GCLEARSCREEN);    /* did this _ever_ work? */
#else
  _clearscreen( _GCLEARSCREEN );
#endif
  isPageDirty = FALSE;
}

/*----------------------------------------------------------------------*\
* plD_bop_vga()
*
* Set up for the next page.
* Advance to next family file if necessary (file output).
\*----------------------------------------------------------------------*/

void
plD_bop_vga(PLStream *pls)
{                               /* ~~~~ */
  pls->page++;
  plD_eop_vga(pls);
}

/*----------------------------------------------------------------------*\
* plD_tidy_vga()
*
* Close graphics file or otherwise clean up.
\*----------------------------------------------------------------------*/

void
plD_tidy_vga(PLStream *pls)
{                               /* ~~~~ */
  vga_text(pls);
  pls->page = 0;
  pls->OutFile = NULL;
}

/*----------------------------------------------------------------------*\
* plD_state_vga()
*
* Handle change in PLStream state (color, pen width, fill attribute, etc).
\*----------------------------------------------------------------------*/

void
plD_state_vga(PLStream *pls, PLINT op)
{                               /* ~~~~ */
  switch (op) {

  case PLSTATE_WIDTH:
    break;

  case PLSTATE_COLOR0:{
    static long cmap[16] = {
        _WHITE, _RED, _LIGHTYELLOW, _GREEN,
        _CYAN, _WHITE, _WHITE, _GRAY,
        _WHITE, _BLUE, _GREEN, _CYAN,
        _RED, _MAGENTA, _LIGHTYELLOW, _WHITE
        };

    if (pls->icol0 < 0 || pls->icol0 > 15)
        pls->icol0 = 15;

    _remappalette((short) pls->icol0, cmap[pls->icol0]);
    _setcolor((short) pls->icol0);
    break;
  }

  case PLSTATE_COLOR1:
    break;
  }
}

/*----------------------------------------------------------------------*\
* plD_esc_vga()
*
* Escape function.
\*----------------------------------------------------------------------*/

void
plD_esc_vga(PLStream *pls, PLINT op, void *ptr)
{                               /* ~~~~ */
  switch (op) {
  case PLESC_TEXT:
    vga_text(pls);
    break;

  case PLESC_GRAPH:
    vga_graphics(pls);
    break;
  }
}

/*----------------------------------------------------------------------*\
* vga_text()
*
* Switch to text mode.
\*----------------------------------------------------------------------*/

static void
vga_text(PLStream *pls)
{                               /* ~~~~ */
  if ( pls->graphx ) {
    if ( isPageDirty == TRUE )
      pause(pls);
    _setvideomode(_DEFAULTMODE);
    pls->graphx = FALSE;
  }
}

/*----------------------------------------------------------------------*\
* vga_graphics()
*
* Switch to graphics mode.
\*----------------------------------------------------------------------*/

static void
vga_graphics(PLStream *pls)
{                               /* ~~~~ */
  if ( !pls->graphx ) {
    if (!_setvideomode( DetectedVideoMode )) {
      fputs("Unable to set graphics mode.", stderr);    /* shouldn't happen */
      exit(0);
    }
    pls->graphx = TRUE;
    isPageDirty = FALSE;
  }
}

/*----------------------------------------------------------------------*\
* pause()
*
* Wait for a keystroke.
\*----------------------------------------------------------------------*/

static void
pause(PLStream *pls)
{
    if (pls->nopause)
	return;

    _settextposition(0, 0);
    _outtext("pause->");
    while ( !_getch() );
}

#else
int
pldummy_mscvga()
{                               /* ~~~~ */
  return 0;
}

#endif                          /* (_MSC_VER) && (MSDOS) */
/* ///////////////////// end of file (c source) /////////////////// */
