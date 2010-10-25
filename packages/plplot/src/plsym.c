/* $Id: plsym.c 3186 2006-02-15 18:17:33Z slbrow $

	Point, symbol, and string plotting routines.
	Also font management code.  See the description of plLibOpen() for
	the search path used in finding the font files.

   Copyright (C) 1992  Geoffrey Furnish 
   Copyright (C) 1993, 1994, 1995, 2000, 2001, 2002  Maurice LeBrun
   Copyright (C) 2000, 2002, 2004, 2005  Alan W. Irwin
   Copyright (C) 2001, 2003, 2004  Rafael Laboissiere
   Copyright (C) 2002  Vincent Darley
   Copyright (C) 2004  Andrew Ross

   This file is part of PLplot.

   PLplot is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Library Public License as published
   by the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   PLplot is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with PLplot; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#ifndef __PLSYM_H__
#define __PLSYM_H__

#include "plplotP.h"
#include <float.h>
#include <ctype.h>
#include "plhershey-unicode.h"

/* Declarations */

static short int *fntlkup;
static short int *fntindx;
static signed char *fntbffr;
static short int numberfonts, numberchars;
static short int indxleng;

static short fontloaded = 0;
/* moved to plstr.h, plsc->cfont  static PLINT font = 1;  current font */

#define PLMAXSTR	300
#define STLEN		250

static const char font_types[] = "nris";

static short symbol_buffer[PLMAXSTR];
static signed char xygrid[STLEN];

int hershey2unicode ( int in );

/* Static function prototypes */

static void
pldeco(short int **sym, PLINT *length, const char *text);

static void
plchar(signed char *xygrid, PLFLT *xform, PLINT base, PLINT oline, PLINT uline,
       PLINT refx, PLINT refy, PLFLT scale, PLFLT xpmm, PLFLT ypmm,
       PLFLT *p_xorg, PLFLT *p_yorg, PLFLT *p_width);

static PLINT
plcvec(PLINT ch, signed char **xygr);

static void
plhrsh(PLINT ch, PLINT x, PLINT y);

static void
plhrsh2(PLINT ch, PLINT x, PLINT y);

/*--------------------------------------------------------------------------*\
 * void plsym()
 *
 * Plots array y against x for n points using Hershey symbol "code".
\*--------------------------------------------------------------------------*/

void
c_plsym(PLINT n, PLFLT *x, PLFLT *y, PLINT code)
{
    PLINT i;

    if (plsc->level < 3) {
	plabort("plsym: Please set up window first");
	return;
    }
    if (code < 0) {
	plabort("plsym: Invalid code");
	return;
    }

    for (i = 0; i < n; i++)
      {
        plhrsh(code, plP_wcpcx(x[i]), plP_wcpcy(y[i]));
      }
}

/*--------------------------------------------------------------------------*\
 * void plpoin()
 *
 * Plots array y against x for n points using ASCII code "code".
 *
 * code=-1 means try to just draw a point.  Right now it's just a move and
 * a draw at the same place.  Not ideal, since a sufficiently intelligent
 * output device may optimize it away, or there may be faster ways of
 * doing it.  This is OK for now, though, and offers a 4X speedup over
 * drawing a Hershey font "point" (which is actually diamond shaped and
 * therefore takes 4 strokes to draw).
\*--------------------------------------------------------------------------*/

void
c_plpoin(PLINT n, PLFLT *x, PLFLT *y, PLINT code)
{
    PLINT i, sym, ifont = plsc->cfont;

    if (plsc->level < 3) {
	plabort("plpoin: Please set up window first");
	return;
    }
    if (code < -1 || code > 127) {
	plabort("plpoin: Invalid code");
	return;
    }

    if (code == -1) {
	for (i = 0; i < n; i++)
	    pljoin(x[i], y[i], x[i], y[i]);
    }
    else {
        if (ifont > numberfonts)
	    ifont = 1;
	sym = *(fntlkup + (ifont - 1) * numberchars + code);

	for (i = 0; i < n; i++)
	    plhrsh(sym, plP_wcpcx(x[i]), plP_wcpcy(y[i]));
    }
}

/*--------------------------------------------------------------------------*\
 * void plpoin3(n, x, y, z, code)
 *
 * Draws a series of points in 3 space.  Setup similar to plline3().
\*--------------------------------------------------------------------------*/

void
c_plpoin3(PLINT n, PLFLT *x, PLFLT *y, PLFLT *z, PLINT code)
{
    PLINT i, sym, ifont = plsc->cfont;
    PLFLT u, v;
    PLFLT xmin, xmax, ymin, ymax, zmin, zmax, zscale;

    if (plsc->level < 3) {
	plabort("plpoin3: Please set up window first");
	return;
    }
    if (code < -1 || code > 127) {
	plabort("plpoin3: Invalid code");
	return;
    }

    plP_gdom(&xmin, &xmax, &ymin, &ymax);
    plP_grange(&zscale, &zmin, &zmax);

    if (code == -1) {
	for (i = 0; i < n; i++) {
 	  if(x[i] >= xmin && x[i] <= xmax &&
 	     y[i] >= ymin && y[i] <= ymax &&
	     z[i] >= zmin && z[i] <= zmax) {
	    u = plP_wcpcx(plP_w3wcx( x[i], y[i], z[i] ));
	    v = plP_wcpcy(plP_w3wcy( x[i], y[i], z[i] ));
	    plP_movphy(u,v);
	    plP_draphy(u,v);
	  }
	}
    }
    else {
        if (ifont > numberfonts)
	    ifont = 1;
	sym = *(fntlkup + (ifont - 1) * numberchars + code);

	for( i=0; i < n; i++ ) {
 	  if(x[i] >= xmin && x[i] <= xmax &&
 	     y[i] >= ymin && y[i] <= ymax &&
	     z[i] >= zmin && z[i] <= zmax) {
	    u = plP_wcpcx(plP_w3wcx( x[i], y[i], z[i] ));
	    v = plP_wcpcy(plP_w3wcy( x[i], y[i], z[i] ));
	    plhrsh(sym, u, v);
	  }
	}
    }
    return;
}

/*--------------------------------------------------------------------------*\
 * static void plhrsh(PLINT ch, PLINT x, PLINT y)
 *    PLINT ch - hershey code to plot
 *    PLINT x - device-world x coordinate of hershey character
 *    PLINT y - device-world y coordinate of hershey character
 *
 *  Writes the Hershey symbol "ch" centred at the physical coordinate (x,y).
 *  This function is now just a "spoof" front end to the old plhersh,
 *  which has now been renamed to plhrsh2(). All this function does is
 *  decide whether or not we should render natively as unicode, and then
 *  convert between hershey and unicode.
 *
 *  If the function KNOWS there isn't a unicode equivalent, then it will
 *  try to render it as a hershey font. Understandably, this might make
 *  testing out the unicode functions a little tricky, so if you want
 *  to disable this behaviour, recompile with TEST_FOR_MISSING_GLYPHS
 *  defined.
\*--------------------------------------------------------------------------*/

static void
plhrsh(PLINT ch, PLINT x, PLINT y)
{
EscText args;
int idx;
PLUNICODE unicode_char;

  /* Check to see if the device understands unicode and wants to draw
   * symbols.
   */
  if ((plsc->dev_text)&&(plsc->dev_unicode)&&(!plsc->dev_hrshsym))       
    {
      idx=plhershey2unicode(ch); /* Get the index in the lookup table */
      unicode_char=hershey_to_unicode_lookup_table[idx].Unicode;

    /*
     *  Test to see if there is a defined unicode glyph for this hershey code;
     *  if there isn't, then we pass the glyph to plhersh, and have it
     *  rendered the old fashioned way.
     *  Otherwise, we let the driver render it as unicode
     */

      if ((unicode_char==0)||(idx==-1))
        {
#ifndef TEST_FOR_MISSING_GLYPHS
          plhrsh2(ch, x, y);
#endif
        }
      else
        {
	  PLUNICODE  plhrsh_unicode_buffer[3], fci;
	  PLFLT xform[] = {1.0, 0.0, 0.0, 1.0};
	  char esc;
          args.unicode_char=unicode_char;
          args.font_face=hershey_to_unicode_lookup_table[idx].Font;
          args.base = 1;
          args.just = .5;
          args.xform = 0;
          args.x = x;
          args.y = y;
	  args.string=NULL;  /* Since we are using unicode, we want this to be NULL */
	  /* "array method" */
	  plgesc(&esc);
	  args.xform = xform;
	  args.unicode_array_len=2;
	  /* Temporary Symbol font for every character. */
	  plgfci(&fci);
	  plP_hex2fci(PL_FCI_SYMBOL, PL_FCI_FAMILY, &fci);
	  plhrsh_unicode_buffer[0] = fci;
	  plhrsh_unicode_buffer[1] = unicode_char;
	  /* watch out for escape character and unescape it by appending
	   * one extra. */
	  if (unicode_char == esc) {
	     args.unicode_array_len=3;
	     plhrsh_unicode_buffer[2] = unicode_char;
	  }
	   
	  /* No need to change font back since only one character. */
	  args.unicode_array=&plhrsh_unicode_buffer[0];   /* Get address of the unicode buffer (even though it is currently static) */

          plP_esc(PLESC_HAS_TEXT, &args);
        }

    }
  else
    {
       plhrsh2(ch, x, y);
    }
}

/*--------------------------------------------------------------------------*\
 * void plhrsh2()
 *
 * Writes the Hershey symbol "ch" centred at the physical coordinate (x,y).
\*--------------------------------------------------------------------------*/

static void
plhrsh2(PLINT ch, PLINT x, PLINT y)
{
    PLINT cx, cy, k, penup, style;
    signed char *vxygrid = 0;
    PLFLT scale, xscale, yscale;
    PLINT llx[STLEN], lly[STLEN], l = 0;

    penup = 1;
    scale = 0.05 * plsc->symht;

    if ( ! plcvec(ch, &vxygrid)) {
	plP_movphy(x, y);
	return;
    }

/* Line style must be continuous */

    style = plsc->nms;
    plsc->nms = 0;

/* Compute how many physical pixels correspond to a character pixel */

    xscale = scale * plsc->xpmm;
    yscale = scale * plsc->ypmm;

    k = 4;
    for (;;) {
	cx = vxygrid[k++];
	cy = vxygrid[k++];
	if (cx == 64 && cy == 64) {
	  if (l) {
	    plP_draphy_poly(llx, lly, l);
	    l = 0;
	  }
	  plP_movphy(x, y);
	  plsc->nms = style;
	  return;
	}
	else if (cx == 64 && cy == 0)
	    penup = 1;
	else {
	    if (penup == 1) {
             if (l) {
	       plP_draphy_poly(llx, lly, l);
	       l = 0;
	     }
             llx[l] = ROUND(x+ xscale * cx);
	     lly[l++] = ROUND(y + yscale * cy);
             plP_movphy(llx[l-1], lly[l-1]);
	     penup = 0;
	    }
	    else {
	      llx[l] = ROUND(x+ xscale * cx);
	      lly[l++] = ROUND(y + yscale * cy);
	    }
	}
    }
}

/*--------------------------------------------------------------------------*\
 * void pllab()
 *
 * Simple routine for labelling graphs.
\*--------------------------------------------------------------------------*/

void
c_pllab(const char *xlabel, const char *ylabel, const char *tlabel)
{
    if (plsc->level < 2) {
	plabort("pllab: Please set up viewport first");
	return;
    }

    plmtex("t", (PLFLT) 2.0, (PLFLT) 0.5, (PLFLT) 0.5, tlabel);
    plmtex("b", (PLFLT) 3.2, (PLFLT) 0.5, (PLFLT) 0.5, xlabel);
    plmtex("l", (PLFLT) 5.0, (PLFLT) 0.5, (PLFLT) 0.5, ylabel);
}

/*--------------------------------------------------------------------------*\
 * void plmtex()
 *
 * Prints out "text" at specified position relative to viewport
 * (may be inside or outside)
 *
 * side	String which is one of the following:
 *	B or b  :  Bottom of viewport
 *	T or t  :  Top of viewport
 *	BV or bv : Bottom of viewport, vertical text
 *	TV or tv : Top of viewport, vertical text
 *	L or l  :  Left of viewport
 *	R or r  :  Right of viewport
 *	LV or lv : Left of viewport, vertical text
 *	RV or rv : Right of viewport, vertical text
 *
 * disp Displacement from specified edge of viewport, measured outwards from
 *	the viewport in units of the current character height. The
 *	centerlines of the characters are aligned with the specified
 *	position.
 *
 * pos	Position of the reference point of the string relative to the
 *	viewport edge, ranging from 0.0 (left-hand edge) to 1.0 (right-hand
 *	edge)
 *
 * just	Justification of string relative to reference point
 *	just = 0.0 => left hand edge of string is at reference
 *	just = 1.0 => right hand edge of string is at reference
 *	just = 0.5 => center of string is at reference
\*--------------------------------------------------------------------------*/

void
c_plmtex(const char *side, PLFLT disp, PLFLT pos, PLFLT just,
	 const char *text)
{
    PLINT clpxmi, clpxma, clpymi, clpyma;
    PLINT vert, refx, refy, x, y;
    PLFLT xdv, ydv, xmm, ymm, refxmm, refymm, shift, xform[4];
    PLFLT chrdef, chrht;
    PLFLT dispx, dispy;

    if (plsc->level < 2) {
	plabort("plmtex: Please set up viewport first");
	return;
    }

/* Open clip limits to subpage limits */

    plP_gclp(&clpxmi, &clpxma, &clpymi, &clpyma); /* get and store current clip limits */
    plP_sclp(plsc->sppxmi, plsc->sppxma, plsc->sppymi, plsc->sppyma);

    if (plP_stindex(side, "BV") != -1 || plP_stindex(side, "bv") != -1) {
	vert = 1;
	xdv  = plsc->vpdxmi + (plsc->vpdxma - plsc->vpdxmi) * pos;
	ydv  = plsc->vpdymi;
	dispx = 0;
	dispy = -disp;
    }
    else if (plP_stindex(side, "TV") != -1 || plP_stindex(side, "tv") != -1) {
	vert = 1;
	xdv  = plsc->vpdxmi + (plsc->vpdxma - plsc->vpdxmi) * pos;
	ydv  = plsc->vpdyma;
	dispx = 0;
	dispy = disp;
    }
    else if (plP_stsearch(side, 'b')) {
	vert = 0;
	xdv = plsc->vpdxmi + (plsc->vpdxma - plsc->vpdxmi) * pos;
	ydv = plsc->vpdymi;
	dispx = 0;
	dispy = -disp;

    } else if (plP_stsearch(side, 't')) {
	vert = 0;
	xdv = plsc->vpdxmi + (plsc->vpdxma - plsc->vpdxmi) * pos;
	ydv = plsc->vpdyma;
	dispx = 0;
	dispy = disp;

    } else if (plP_stindex(side, "LV") != -1 || plP_stindex(side, "lv") != -1) {
	vert = 0;
	xdv = plsc->vpdxmi;
	ydv = plsc->vpdymi + (plsc->vpdyma - plsc->vpdymi) * pos;
	dispx = -disp;
	dispy = 0;

    } else if (plP_stindex(side, "RV") != -1 || plP_stindex(side, "rv") != -1) {
	vert = 0;
	xdv = plsc->vpdxma;
	ydv = plsc->vpdymi + (plsc->vpdyma - plsc->vpdymi) * pos;
	dispx = disp;
	dispy = 0;

    } else if (plP_stsearch(side, 'l')) {
	vert = 1;
	xdv = plsc->vpdxmi;
	ydv = plsc->vpdymi + (plsc->vpdyma - plsc->vpdymi) * pos;
	dispx = -disp;
	dispy = 0;

    } else if (plP_stsearch(side, 'r')) {
	vert = 1;
	xdv = plsc->vpdxma;
	ydv = plsc->vpdymi + (plsc->vpdyma - plsc->vpdymi) * pos;
	dispx = disp;
	dispy = 0;

    } else {
	plP_sclp(clpxmi, clpxma, clpymi, clpyma); /* restore initial clip limits */
	return;
    }

/* Transformation matrix */

    if (vert != 0) {
	xform[0] = 0.0;
	xform[1] = -1.0;
	xform[2] = 1.0;
	xform[3] = 0.0;
    } else {
	xform[0] = 1.0;
	xform[1] = 0.0;
	xform[2] = 0.0;
	xform[3] = 1.0;
    }

/* Convert to physical units (mm) and compute shifts */

    plgchr(&chrdef, &chrht);
    shift = (just == 0.0) ? 0.0 : plstrl(text) * just;

    xmm = plP_dcmmx(xdv) + dispx * chrht;
    ymm = plP_dcmmy(ydv) + dispy * chrht;
    refxmm = xmm - shift * xform[0];
    refymm = ymm - shift * xform[2];

/* Convert to device units (pixels) and call text plotter */

    x = plP_mmpcx(xmm);
    y = plP_mmpcy(ymm);
    refx = plP_mmpcx(refxmm);
    refy = plP_mmpcy(refymm);

    plP_text(0, just, xform, x, y, refx, refy, text);
    plP_sclp(clpxmi, clpxma, clpymi, clpyma); /* restore clip limits */
}

/*--------------------------------------------------------------------------*\
 * void plptex()
 *
 * Prints out "text" at world cooordinate (wx,wy). The text may be
 * at any angle "angle" relative to the horizontal. The parameter
 * "just" adjusts the horizontal justification of the string:
 *	just = 0.0 => left hand edge of string is at (wx,wy)
 *	just = 1.0 => right hand edge of string is at (wx,wy)
 *	just = 0.5 => center of string is at (wx,wy) etc.
\*--------------------------------------------------------------------------*/

void
c_plptex(PLFLT wx, PLFLT wy, PLFLT dx, PLFLT dy, PLFLT just, const char *text)
{
    PLINT x, y, refx, refy;
    PLFLT xdv, ydv, xmm, ymm, refxmm, refymm, shift, cc, ss;
    PLFLT xform[4], diag;
    PLFLT chrdef, chrht;
    PLFLT dispx, dispy;

    if (plsc->level < 3) {
	plabort("plptex: Please set up window first");
	return;
    }

    if (dx == 0.0 && dy == 0.0) {
	dx = 1.0;
	dy = 0.0;
    }
    cc = plsc->wmxscl * dx;
    ss = plsc->wmyscl * dy;
    diag = sqrt(cc * cc + ss * ss);
    cc /= diag;
    ss /= diag;

    xform[0] = cc;
    xform[1] = -ss;
    xform[2] = ss;
    xform[3] = cc;

    xdv = plP_wcdcx(wx);
    ydv = plP_wcdcy(wy);

    dispx = 0.;
    dispy = 0.;

/* Convert to physical units (mm) and compute shifts */

    plgchr(&chrdef, &chrht);
    shift = (just == 0.0) ? 0.0 : plstrl(text) * just;

    xmm = plP_dcmmx(xdv) + dispx * chrht;
    ymm = plP_dcmmy(ydv) + dispy * chrht;
    refxmm = xmm - shift * xform[0];
    refymm = ymm - shift * xform[2];

    x = plP_mmpcx(xmm);
    y = plP_mmpcy(ymm);
    refx = plP_mmpcx(refxmm);
    refy = plP_mmpcy(refymm);

    plP_text(0, just, xform, x, y, refx, refy, text);
}

/*--------------------------------------------------------------------------*\
 * void plstr()
 *
 * Prints out a "string" at reference position with physical coordinates
 * (refx,refy). The coordinates of the vectors defining the string are
 * passed through the linear mapping defined by the 2 x 2 matrix xform()
 * before being plotted.  The reference position is at the left-hand edge of
 * the string. If base = 1, it is aligned with the baseline of the string.
 * If base = 0, it is aligned with the center of the character box.
 *
 * Note, all calculations are done in terms of millimetres. These are scaled
 * as necessary before plotting the string on the page.
\*--------------------------------------------------------------------------*/

void
plstr(PLINT base, PLFLT *xform, PLINT refx, PLINT refy, const char *string)
{
    short int *symbol;
    signed char *vxygrid = 0;

    PLINT ch, i, length, level = 0, style, oline = 0, uline = 0;
    PLFLT width = 0., xorg = 0., yorg = 0., def, ht, dscale, scale;

    plgchr(&def, &ht);
    dscale = 0.05 * ht;
    scale = dscale;

/* Line style must be continuous */

    style = plsc->nms;
    plsc->nms = 0;

    pldeco(&symbol, &length, string);

    for (i = 0; i < length; i++) {
	ch = symbol[i];
	if (ch == -1) { /* super-script */
	    level++;
	    yorg += 16.0 * scale;
	    scale = dscale * pow(0.75, (double) ABS(level));
	}
	else if (ch == -2) { /* sub-script */
	    level--;
	    scale = dscale * pow(0.75, (double) ABS(level));
	    yorg -= 16.0 * scale;
	}
	else if (ch == -3) /* back-char */
	    xorg -= width * scale;
	else if (ch == -4) /* toogle overline */
	    oline = !oline;
	else if (ch == -5)  /* toogle underline */
	    uline = !uline;
	else {
	    if (plcvec(ch, &vxygrid))
		plchar(vxygrid, xform, base, oline, uline, refx, refy, scale,
		       plsc->xpmm, plsc->ypmm, &xorg, &yorg, &width);
	}
    }
    plsc->nms = style;
}

/*--------------------------------------------------------------------------*\
 * plchar()
 *
 * Plots out a given stroke font character.
\*--------------------------------------------------------------------------*/

static void
plchar(signed char *vxygrid, PLFLT *xform, PLINT base, PLINT oline, PLINT uline,
       PLINT refx, PLINT refy, PLFLT scale, PLFLT xpmm, PLFLT ypmm,
       PLFLT *p_xorg, PLFLT *p_yorg, PLFLT *p_width)
{
    PLINT xbase, ybase, ydisp, lx, ly, cx, cy;
    PLINT k, penup;
    PLFLT x, y;
    PLINT llx[STLEN], lly[STLEN], l = 0;

    xbase = vxygrid[2];
    *p_width = vxygrid[3] - xbase;
    if (base == 0) {
	ybase = 0;
	ydisp = vxygrid[0];
    }
    else {
	ybase = vxygrid[0];
	ydisp = 0;
    }
    k = 4;
    penup = 1;

    for (;;) {
	cx = vxygrid[k++];
	cy = vxygrid[k++];
	if (cx == 64 && cy == 64) {
	  if (l) {
	    plP_draphy_poly(llx, lly, l);
	    l = 0;
	  }
	  break;
	}
	if (cx == 64 && cy == 0) {
	  if (l) {
	    plP_draphy_poly(llx, lly, l);
	    l = 0;
	  }
	  penup = 1;
	}
	else {
	    x = *p_xorg + (cx - xbase) * scale;
	    y = *p_yorg + (cy - ybase) * scale;
	    lx = refx + ROUND(xpmm * (xform[0] * x + xform[1] * y));
	    ly = refy + ROUND(ypmm * (xform[2] * x + xform[3] * y));
	    if (penup == 1) {
	      if (l) {
		plP_draphy_poly(llx, lly, l);
		l = 0;
	      }
	      llx[l] = lx;
	      lly[l++] = ly; /* store 1st point ! */
	      plP_movphy(lx, ly);
	      penup = 0;
	    }
	    else {
	      llx[l] = lx;
	      lly[l++] = ly;
	    }
	}
    }

    if (oline) {
	x = *p_xorg;
	y = *p_yorg + (30 + ydisp) * scale;
	lx = refx + ROUND(xpmm * (xform[0] * x + xform[1] * y));
	ly = refy + ROUND(ypmm * (xform[2] * x + xform[3] * y));
	plP_movphy(lx, ly);
	x = *p_xorg + *p_width * scale;
	lx = refx + ROUND(xpmm * (xform[0] * x + xform[1] * y));
	ly = refy + ROUND(ypmm * (xform[2] * x + xform[3] * y));
	plP_draphy(lx, ly);
    }
    if (uline) {
	x = *p_xorg;
	y = *p_yorg + (-5 + ydisp) * scale;
	lx = refx + ROUND(xpmm * (xform[0] * x + xform[1] * y));
	ly = refy + ROUND(ypmm * (xform[2] * x + xform[3] * y));
	plP_movphy(lx, ly);
	x = *p_xorg + *p_width * scale;
	lx = refx + ROUND(xpmm * (xform[0] * x + xform[1] * y));
	ly = refy + ROUND(ypmm * (xform[2] * x + xform[3] * y));
	plP_draphy(lx, ly);
    }
    *p_xorg = *p_xorg + *p_width * scale;
}

/*--------------------------------------------------------------------------*\
 * PLFLT plstrl()
 *
 * Computes the length of a string in mm, including escape sequences.
\*--------------------------------------------------------------------------*/

PLFLT
plstrl(const char *string)
{
    short int *symbol;
    signed char *vxygrid = 0;
    PLINT ch, i, length, level = 0;
    PLFLT width = 0., xorg = 0., dscale, scale, def, ht;

    plgchr(&def, &ht);
    dscale = 0.05 * ht;
    scale = dscale;
    pldeco(&symbol, &length, string);

    for (i = 0; i < length; i++) {
	ch = symbol[i];
	if (ch == -1) {
	    level++;
	    scale = dscale * pow(0.75, (double) ABS(level));
	}
	else if (ch == -2) {
	    level--;
	    scale = dscale * pow(0.75, (double) ABS(level));
	}
	else if (ch == -3)
	    xorg -= width * scale;
	else if (ch == -4 || ch == -5);
	else {
	    if (plcvec(ch, &vxygrid)) {
		width = vxygrid[3] - vxygrid[2];
		xorg += width * scale;
	    }
	}
    }
    return (PLFLT) xorg;
}

/*--------------------------------------------------------------------------*\
 * PLINT plcvec()
 *
 * Gets the character digitisation of Hershey table entry "char".
 * Returns 1 if there is a valid entry.
\*--------------------------------------------------------------------------*/

static PLINT
plcvec(PLINT ch, signed char **xygr)
{
    PLINT k = 0, ib;
    signed char x, y;

    ch--;
    if (ch < 0 || ch >= indxleng)
	return (PLINT) 0;
    ib = fntindx[ch] - 2;
    if (ib == -2)
	return (PLINT) 0;

    do {
	ib++;
	x = fntbffr[2 * ib];
	y = fntbffr[2 * ib + 1];
	xygrid[k++] = x;
	xygrid[k++] = y;
    } while ((x != 64 || y != 64) && k <= (STLEN - 2));

    if (k == (STLEN-1)) {
	/* This is bad if we get here */
	xygrid[k] = 64;
	xygrid[k] = 64;
    }

    *xygr = xygrid;
    return (PLINT) 1;
}

/*--------------------------------------------------------------------------*\
 * void pldeco()
 *
 * Decode a character string, and return an array of float integer symbol
 * numbers. This routine is responsible for interpreting all escape sequences.
 * At present the following escape sequences are defined (the letter following
 * the <esc> may be either upper or lower case):
 *
 * <esc>u	: up one level (returns -1)
 * <esc>d	: down one level (returns -2)
 * <esc>b	: backspace (returns -3)
 * <esc>+	: toggles overline mode (returns -4)
 * <esc>-	: toggles underline mode (returns -5)
 * <esc><esc>	: <esc>
 * <esc>gx	: greek letter corresponding to roman letter x
 * <esc>fn	: switch to Normal font
 * <esc>fr	: switch to Roman font
 * <esc>fi	: switch to Italic font
 * <esc>fs	: switch to Script font
 * <esc>(nnn)	: Hershey symbol number nnn (any number of digits)
 *
 * The escape character defaults to '#', but can be changed to any of
 * [!#$%&*@^~] via a call to plsesc.
\*--------------------------------------------------------------------------*/

static void
pldeco(short int **symbol, PLINT *length, const char *text)
{
    PLINT ch, ifont = plsc->cfont, ig, j = 0, lentxt = strlen(text);
    char test, esc;
    short int *sym = symbol_buffer;

/* Initialize parameters. */

    *length = 0;
    *symbol = symbol_buffer;
    plgesc(&esc);
    if (ifont > numberfonts)
	ifont = 1;

/* Get next character; treat non-printing characters as spaces. */

    while (j < lentxt) {
	if (*length >= PLMAXSTR)
	    return;
	test = text[j++];
	ch = test;
	if (ch < 0 || ch > 175)
	    ch = 32;

    /* Test for escape sequence (#) */

	if (ch == esc && (lentxt - j) >= 1) {
	    test = text[j++];
	    if (test == esc)
		sym[(*length)++] = *(fntlkup + (ifont - 1) * numberchars + ch);

	    else if (test == 'u' || test == 'U')
		sym[(*length)++] = -1;

	    else if (test == 'd' || test == 'D')
		sym[(*length)++] = -2;

	    else if (test == 'b' || test == 'B')
		sym[(*length)++] = -3;

	    else if (test == '+')
		sym[(*length)++] = -4;

	    else if (test == '-')
		sym[(*length)++] = -5;

	    else if (test == '(') {
		sym[*length] = 0;
		while ('0' <= text[j] && text[j] <= '9') {
		    sym[*length] = sym[*length] * 10 + text[j] - '0';
		    j++;
		}
		(*length)++;
		if (text[j] == ')')
		    j++;
	    }
	    else if (test == 'f' || test == 'F') {
		test = text[j++];
		ifont = 1 + plP_strpos(font_types,
				       isupper(test) ? tolower(test) : test);
		if (ifont == 0 || ifont > numberfonts)
		    ifont = 1;
	    }
	    else if (test == 'g' || test == 'G') {
		test = text[j++];
		ig = plP_strpos(plP_greek_mnemonic, test) + 1;
		sym[(*length)++] =
		    *(fntlkup + (ifont - 1) * numberchars + 127 + ig);
	    }
	    else {
		;
	    }
	}
	else {

	/* Decode character. */
	/* >>PC<< removed increment from following expression to fix */
	/* compiler bug */

	    sym[(*length)] = *(fntlkup + (ifont - 1) * numberchars + ch);
	    (*length)++;
	}
    }
}

/*--------------------------------------------------------------------------*\
 * PLINT plP_strpos()
 *
 * Searches string str for first occurence of character chr.  If found
 * the position of the character in the string is returned (the first
 * character has position 0).  If the character is not found a -1 is
 * returned.
\*--------------------------------------------------------------------------*/

PLINT
plP_strpos(const char *str, int chr)
{
    char *temp;

    if ((temp = strchr(str, chr)))
	return (PLINT) (temp - str);
    else
	return (PLINT) -1;
}

/*--------------------------------------------------------------------------*\
 * PLINT plP_stindex()
 *
 * Similar to strpos, but searches for occurence of string str2.
\*--------------------------------------------------------------------------*/

PLINT
plP_stindex(const char *str1, const char *str2)
{
    PLINT base, str1ind, str2ind;

    for (base = 0; *(str1 + base) != '\0'; base++) {
	for (str1ind = base, str2ind = 0; *(str2 + str2ind) != '\0' &&
	     *(str2 + str2ind) == *(str1 + str1ind); str1ind++, str2ind++)
	    ;

	if (*(str2 + str2ind) == '\0')
	    return (PLINT) base;
    }
    return (PLINT) -1;		/* search failed */
}

/*--------------------------------------------------------------------------*\
 * PLINT plP_stsearch()
 *
 * Searches string str for character chr (case insensitive).
\*--------------------------------------------------------------------------*/

PLINT
plP_stsearch(const char *str, int chr)
{
    if (strchr(str, chr))
	return (PLINT) 1;
    else if (strchr(str, toupper(chr)))
	return (PLINT) 1;
    else
	return (PLINT) 0;
}

/*--------------------------------------------------------------------------*\
 * void c_plfont(ifont)
 *
 * Sets the global font flag to 'ifont'.
\*--------------------------------------------------------------------------*/

void
c_plfont(PLINT ifont)
{
    PLUNICODE fci = PL_FCI_MARK;
    if (plsc->level < 1) {
	plabort("plfont: Please call plinit first");
	return;
    }
    if (ifont < 1 || ifont > 4) {
	plabort("plfont: Invalid font");
	return;
    }

    plsc->cfont = ifont;

   /* Provide some degree of forward compatibility if dealing with 
    * unicode font. But better procedure is to call plsfci directly rather
    * than using this lame Hershey font interface.
    */
    switch(ifont)
     {
      case 1:
	/* normal = (medium, upright, sans serif) */
	plP_hex2fci(PL_FCI_SANS, PL_FCI_FAMILY, &fci);
	plsfci(fci);
	break;
	/* roman = (medium, upright, serif) */
      case 2:
	plP_hex2fci(PL_FCI_SERIF, PL_FCI_FAMILY, &fci);
	plsfci(fci);
	break;
	/* italic = (medium, italic, serif) */
      case 3:
	plP_hex2fci(PL_FCI_ITALIC, PL_FCI_STYLE, &fci);
	plP_hex2fci(PL_FCI_SERIF, PL_FCI_FAMILY, &fci);
	plsfci(fci);
	break;
	/* script = (medium, upright, script) */
      case 4:
	plP_hex2fci(PL_FCI_SCRIPT, PL_FCI_FAMILY, &fci);
	plsfci(fci);
	break;
     }
}

/*--------------------------------------------------------------------------*\
 * void plfntld(fnt)
 *
 * Loads either the standard or extended font.
\*--------------------------------------------------------------------------*/

void
plfntld(PLINT fnt)
{
    static PLINT charset;
    short bffrleng;
    PDFstrm *pdfs;

    if (fontloaded && (charset == fnt))
	return;

    plfontrel();
    fontloaded = 1;
    charset = fnt;

    if (fnt)
	pdfs = plLibOpenPdfstrm(PL_XFONT);
    else
	pdfs = plLibOpenPdfstrm(PL_SFONT);

    if (pdfs == NULL)
	plexit("Unable to open or allocate memory for font file");

/* Read fntlkup[] */

    pdf_rd_2bytes(pdfs, (U_SHORT *) &bffrleng);
    numberfonts = bffrleng / 256;
    numberchars = bffrleng & 0xff;
    bffrleng = numberfonts * numberchars;
    fntlkup = (short int *) malloc(bffrleng * sizeof(short int));
    if ( ! fntlkup)
	plexit("plfntld: Out of memory while allocating font buffer.");

    pdf_rd_2nbytes(pdfs, (U_SHORT *) fntlkup, bffrleng);

/* Read fntindx[] */

    pdf_rd_2bytes(pdfs, (U_SHORT *) &indxleng);
    fntindx = (short int *) malloc(indxleng * sizeof(short int));
    if ( ! fntindx)
	plexit("plfntld: Out of memory while allocating font buffer.");

    pdf_rd_2nbytes(pdfs, (U_SHORT *) fntindx, indxleng);

/* Read fntbffr[] */
/* Since this is an array of char, there are no endian problems */

    pdf_rd_2bytes(pdfs, (U_SHORT *) &bffrleng);
    fntbffr = (signed char *) malloc(2 * bffrleng * sizeof(signed char));
    if ( ! fntbffr)
	plexit("plfntld: Out of memory while allocating font buffer.");

#if PLPLOT_USE_TCL_CHANNELS
    pdf_rdx(fntbffr, sizeof(signed char)*(2 * bffrleng), pdfs);
#else
    fread((void *) fntbffr, (size_t) sizeof(signed char),
	  (size_t) (2 * bffrleng), pdfs->file);
#endif

/* Done */

    pdf_close(pdfs);
}

/*--------------------------------------------------------------------------*\
 * void plfontrel()
 *
 * Release memory for fonts.
\*--------------------------------------------------------------------------*/

void
plfontrel(void)
{
    if (fontloaded) {
	free_mem(fntindx)
	free_mem(fntbffr)
	free_mem(fntlkup)
	fontloaded = 0;
    }
}

/*--------------------------------------------------------------------------*\
 *  int plhershey2unicode ( int in )
 *
 *  Function searches for in, the input hershey code, in a lookup table and
 *  returns the corresponding index in that table.
 *  Using this index you can work out the unicode equivalent as well as
 *  the closest approximate to the font-face. If the returned index is
 *  -1 then no match was possible.
 *
 *  Two versions of the function exist, a simple linear search version,
 *  and a more complex, but significantly faster, binary search version.
 *  If there seem to be problems with the binary search method, the brain-dead
 *  linear search can be enabled by defining SIMPLE_BUT_SAFE_HERSHEY_LOOKUP
 *  at compile time.
\*--------------------------------------------------------------------------*/

int plhershey2unicode ( int in )
{
#ifdef SIMPLE_BUT_SAFE_HERSHEY_LOOKUP
  int ret=-1;
  int i;

  for (i=0;(i<number_of_entries_in_hershey_to_unicode_table)&&(ret==-1);i++)
    {
      if (hershey_to_unicode_lookup_table[i].Hershey==in) ret=i;
    }

  return(ret);

#else

   int jlo = -1, jmid, jhi = number_of_entries_in_hershey_to_unicode_table;
   while (jhi - jlo > 1) 
     {
	/* Note that although jlo or jhi can be just outside valid
	 * range (see initialization above) because of while condition
	 * jlo < jmid < jhi and jmid must be in valid range.
	 */
	jmid = (jlo+jhi)/2;
	if (in > hershey_to_unicode_lookup_table[jmid].Hershey)
	  jlo = jmid;
	else if (in < hershey_to_unicode_lookup_table[jmid].Hershey)
	  jhi = jmid;
	else
	  /* We have found it!
	   * in == hershey_to_unicode_lookup_table[jmid].Hershey 
	   */
	  return (jmid);
     }
   /* jlo is invalid or it is valid and in > hershey_to_unicode_lookup_table[jlo].Hershey.
    * jhi is invalid or it is valid and in < hershey_to_unicode_lookup_table[jhi].Hershey.
    * All these conditions together imply in cannot be found in
    * hershey_to_unicode_lookup_table[j].Hershey, for all j.
    */
   return(-1);
#endif
}

/*--------------------------------------------------------------------------*\
 *  char *
 *  plP_FCI2FontName ( PLUNICODE fci, 
 *                     const FCI_to_FontName_Table lookup[], const int nlookup)
 *
 *  Function takes an input FCI (font characterization integer) index, 
 *  looks through the lookup table (which must be sorted by PLUNICODE fci),
 *  then returns the corresponding pointer to a valid font name.  If the FCI
 *  index is not present the returned value is NULL.
 \*--------------------------------------------------------------------------*/

char *
plP_FCI2FontName ( PLUNICODE fci, 
		     const FCI_to_FontName_Table lookup[], const int nlookup)
{
   int jlo = -1, jmid, jhi = nlookup;
   while (jhi - jlo > 1) 
     {
	/* Note that although jlo or jhi can be just outside valid
	 * range (see initialization above) because of while condition
	 * jlo < jmid < jhi and jmid must be in valid range.
	 */
	jmid = (jlo+jhi)/2;
	if (fci > lookup[jmid].fci)
	  jlo = jmid;
	else if (fci < lookup[jmid].fci)
	  jhi = jmid;
	else
	  /* We have found it!
	   * fci == lookup[jmid].fci 
	   */
	  return (lookup[jmid].pfont);
     }
   /* jlo is invalid or it is valid and fci > lookup[jlo].Unicode.
    * jhi is invalid or it is valid and fci < lookup[jhi].Unicode.
    * All these conditions together imply fci index cannot be found in lookup.
    * Mark lookup failure with NULL pointer.
    */
   return(NULL);
}


#undef PLSYM_H
#endif
