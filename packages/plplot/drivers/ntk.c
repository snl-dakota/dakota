/* $Id: ntk.c 3186 2006-02-15 18:17:33Z slbrow $

   Experimental tk driver using a plain "wish"

   Copyright (C) 2001  Joao Cardoso
   Copyright (C) 2004  Rafael Laboissiere

   This file is part of PLplot.

   PLplot is free software; you can redistribute it and/or modify
   it under the terms of the GNU Library General Public License as published
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

#include "plDevs.h"

#ifdef PLD_ntk

#include "plplotP.h"
#include "drivers.h"
#include "plevent.h"

#include <tk.h>

/* Device info */
char* plD_DEVICE_INFO_ntk = "ntk:New tk driver:1:ntk:43:ntk";


void plD_dispatch_init_ntk	( PLDispatchTable *pdt );

void plD_init_ntk		(PLStream *);
void plD_line_ntk		(PLStream *, short, short, short, short);
void plD_polyline_ntk		(PLStream *, short *, short *, PLINT);
void plD_eop_ntk		(PLStream *);
void plD_bop_ntk		(PLStream *);
void plD_tidy_ntk		(PLStream *);
void plD_state_ntk		(PLStream *, PLINT);
void plD_esc_ntk		(PLStream *, PLINT, void *);

void plD_dispatch_init_ntk( PLDispatchTable *pdt )
{
#ifndef ENABLE_DYNDRIVERS
    pdt->pl_MenuStr  = "New Tk device";
    pdt->pl_DevName  = "ntk";
#endif
    pdt->pl_type     = plDevType_Interactive;
    pdt->pl_seq      = 43;
    pdt->pl_init     = (plD_init_fp)     plD_init_ntk;
    pdt->pl_line     = (plD_line_fp)     plD_line_ntk;
    pdt->pl_polyline = (plD_polyline_fp) plD_polyline_ntk;
    pdt->pl_eop      = (plD_eop_fp)      plD_eop_ntk;
    pdt->pl_bop      = (plD_bop_fp)      plD_bop_ntk;
    pdt->pl_tidy     = (plD_tidy_fp)     plD_tidy_ntk;
    pdt->pl_state    = (plD_state_fp)    plD_state_ntk;
    pdt->pl_esc      = (plD_esc_fp)      plD_esc_ntk;
}

/* hardwired window size */
#define XPIXELS 600
#define YPIXELS 400

static PLFLT scale = 10.0; /* Tk canvas units are in pixels, giving corse curves, fool plplot, and scale down when sending to tk  */
static PLFLT ppm; /* device pixels per mm */

static Tcl_Interp *interp = NULL; /* tcl interpreter */
static Tk_Window mainw; /* tk main window */

static char curcolor[80]; /* current color in #rrggbb notation */
static char cmd[10000]; /* buffer to build command to interp */
static int ccanv = 0; /* current canvas number */
static char base[80]; /* name of frame that contains the canvas */
static char dash[80]; /* dash string, as <mark space>* */

/* line buffering */
#define NPTS 1000
static short xold=-1, yold=-1; /* last point of last 2 points line */
static short xb[NPTS], yb[NPTS]; /* buffer */
static int curpts = 0; /* current number of points buffered */

static int local = 1; /* "local" or "remote" interpreter */
static char rem_interp[80]; /* name of remote interp */

/* physical devices coordinates */
static PLINT xmin = 0;
static PLINT xmax = XPIXELS;
static PLINT ymin = 0;
static PLINT ymax = YPIXELS;

/* locator */
static PLGraphicsIn gin;

static void
tk_cmd(char *cmd)
{
  static char scmd[10000];

  if (local)
    Tcl_Eval(interp, cmd);
  else {
    /* the -async option makes it block, some times! but is *much* faster!
     * and was working OK till now :(
     * sprintf(scmd, "send -async %s {%s}", rem_interp, cmd);
     */
    sprintf(scmd, "send %s {%s}", rem_interp, cmd); /* mess! make it more efficient */
    if (Tcl_Eval(interp, scmd) != TCL_OK)
      fprintf(stderr,"%s\n", interp->result);
  }
}

static void
create_canvas(PLStream *pls)
{
  ccanv++;

  /* create new canvas */
  sprintf(cmd, "set ccanv %d; canvas $plf.f2.c$ccanv -width $xmax -height $ymax -background #%02x%02x%02x -xscrollcommand \"$hs set\" -yscrollcommand \"$vs set\" -scrollregion \"0 0 $xmax $ymax\"", ccanv, pls->cmap0[0].r, pls->cmap0[0].g , pls->cmap0[0].b);
  tk_cmd(cmd);

  /* add new canvas to option menu */
  sprintf(cmd, "$plf.f1.mb.menu add command -label \"Page $ccanv\" -command {\n"
"set w $plf.f2.c%d;\n"
"$hs configure -command \"$w xview\";\n"
"$vs configure -command \"$w yview\";\n"
"set dname \"Page %d\";\n"
"pack forget $ocanvas;\n"
"set ocanvas $plf.f2.c%d;\n"
"pack $ocanvas -fill both -expand 1;\n"
"scan [$w xview] \"%%f %%f\" i j;\n"
"$hs set $i $j;\n"
"scan [$w yview] \"%%f %%f\" i j;\n"
"$vs set $i $j;}",
	  ccanv, ccanv, ccanv);
  tk_cmd(cmd);

  sprintf(cmd, "set item(%d) 0", ccanv);
  tk_cmd(cmd);

  /* Shif-B1, zooms in */
  /* FIXME inform the core lib of the zoom, see plframe.c around line 2818 */

  sprintf(cmd, "bind $plf.f2.c$ccanv <Shift-Button-1> {\n"
"set cc %d;\n"
"incr item($cc); set tt $item($cc);\n"
"if {$tt == 1} {\n"
"incr scroll_use;\n"
"pack $hs -side bottom -fill x;\n"
"pack $vs -side right -fill y;\n"
"pack forget %%W; pack %%W -fill both -expand 1}\n"
"set zx($cc,$tt) %%x;\n"
"set zy($cc,$tt) %%y;\n"
"%%W scale all %%x %%y 1.6 1.6;\n"
"%%W configure -scrollregion [%%W bbox all];\n"
"}", ccanv);

  tk_cmd(cmd);

  /* Shif-B3, zooms out */
  sprintf(cmd, "bind $plf.f2.c$ccanv <Shift-Button-3> {\n"
"set cc %d; set tt $item($cc);\n"
"if {$tt != 0} {\n"
"%%W scale all $zx($cc,$tt) $zy($cc,$tt) 0.625 0.625\n"
"%%W configure -scrollregion [%%W bbox all];\n"
"set item($cc) [expr $tt - 1]}\n"
"if { $item($cc) == 0} {\n"
"set scroll_use [expr $scroll_use - 1];\n"
"if {$scroll_use == 0} {\n"
"pack forget $plf.f2.hscroll $plf.f2.vscroll}\n"
"%%W configure -scrollregion \"0 0 $xmax $ymax\"}}", ccanv);
  tk_cmd(cmd);

  /* Shift-B2, resets */
  sprintf(cmd, "bind $plf.f2.c$ccanv <Shift-Button-2> {\n"
"set cc %d; set tt $item($cc); \n"
"while {$tt != 0} {\n"
"%%W scale all $zx($cc,$tt) $zy($cc,$tt) 0.625 0.625\n"
"set tt [expr $tt - 1]};\n"
"set item($cc) 0;\n"
"%%W configure -scrollregion \"0 0 $xmax $ymax\";\n"
"set scroll_use [expr $scroll_use - 1];\n"
"if {$scroll_use == 0} {\n"
"pack forget $plf.f2.hscroll $plf.f2.vscroll}}", ccanv);
  tk_cmd(cmd);

  /* Control-B1-Motion, pan */
  sprintf(cmd, "bind $plf.f2.c$ccanv <Control-Button-1> \"$plf.f2.c%d scan mark %%x %%y\"", ccanv);
  tk_cmd(cmd);

  sprintf(cmd, "bind $plf.f2.c$ccanv <Control-Button1-Motion> \"$plf.f2.c%d scan dragto %%x %%y\"", ccanv);
  tk_cmd(cmd);

  /* Control-B2, identify and (in the far future) edit object */
  tk_cmd("bind $plf.f2.c$ccanv <Control-Button-2> {\n"
"set xx [ expr [winfo pointerx .] - [winfo rootx %W]];\n"
"set yy [ expr [winfo pointery .] - [winfo rooty %W]];\n"
"set near [%W find closest $xx $yy];\n"
"%W move $near 20 20;\n"
"after 500 \"%W move $near -20 -20\"}");

  /* change view to the new canvas by invoking the menu buttom */
  sprintf(cmd, "$plf.f1.mb.menu invoke %d", ccanv-1);
  tk_cmd(cmd);
}

/*--------------------------------------------------------------------------*\
 * plD_init_ntk()
 *
 * Initialize device (terminal).
\*--------------------------------------------------------------------------*/

void
plD_init_ntk(PLStream *pls)
{
    pls->dev_fill0 = 1;	    /* Handle solid fills */
    pls->dev_fill1 = 1;     /* Dont handle pattern fills */
    pls->color = 1;         /* Is a color device */
    pls->dev_dash = 1;	    /* Handle dashed lines */
    pls->plbuf_write = 1;   /* Use plot buffer */

    strcpy(curcolor, "black"); /* default color by name, not #rrggbb */

    if (pls->server_name != NULL) {
      local = 0;
      strcpy(rem_interp, pls->server_name);
    }

    if (pls->geometry != NULL)
      sscanf(pls->geometry, "%dx%d", &xmax, &ymax);

    if (pls->plwindow != NULL)
      strcpy(base, pls->plwindow);
    else
      strcpy(base,".plf"); /* default frame containing the canvas */

    interp = Tcl_CreateInterp();

    if (Tcl_Init(interp) != TCL_OK)
      plexit("Unable to initialize Tcl.");

    if (Tk_Init( interp ))
      plexit("Unable to initialize Tk.");

    mainw = Tk_MainWindow(interp);
    Tcl_Eval(interp, "rename exec {}");

    Tcl_Eval(interp, "tk appname PLplot_ntk"); /* give interpreter a name */

    if (!local) {
      Tcl_Eval(interp, "wm withdraw .");

      sprintf(cmd, "send %s \"set client [tk appname]; wm deiconify .\"", rem_interp);
      if (Tcl_Eval(interp, cmd) != TCL_OK) {
	fprintf(stderr,"%s\n", interp->result);
	plexit("No such tk server.");
      }
    }

    sprintf(cmd, "set scroll_use 0; set plf %s; set vs $plf.f2.vscroll; set hs $plf.f2.hscroll; set xmax %d; set ymax %d; set ocanvas .;", base, xmax, ymax);
    tk_cmd(cmd);

    tk_cmd("catch \"frame $plf\"; pack $plf -fill both -expand 1");

    sprintf(cmd, "frame $plf.f1;\n"
"frame $plf.f2 -width %d -height %d;\n"
"pack $plf.f1 -fill x;\n"
"pack $plf.f2 -fill both -expand 1", xmax, ymax);
    tk_cmd(cmd);

    tk_cmd("scrollbar $plf.f2.hscroll -orient horiz;\n"
"scrollbar $plf.f2.vscroll");

    tk_cmd("menubutton $plf.f1.mb -text \"Page 1\" -textvariable dname -relief raised -indicatoron 1 -menu $plf.f1.mb.menu;\n"
"menu $plf.f1.mb.menu -tearoff 0;\n"
"pack $plf.f1.mb -side left");

    if (local)
      tk_cmd("button $plf.f1.quit -text Quit -command exit;\n"
"pack $plf.f1.quit -side right");
    else
      tk_cmd("button $plf.f1.quit -text Quit -command {send -async $client exit;\n"
"destroy $plf;\n"
"wm withdraw .};\n"
"pack $plf.f1.quit -side right");

    /* FIXME: I just discovered that Tcl_Eval is slower than Tcl_EvalObj. Fix it global-wide, `man Tcl_Eval' */

    /* Set up device parameters */

    Tcl_Eval(interp, "tk scaling"); /* pixels per mm */
    ppm = (PLFLT) atof(interp->result)/(25.4/72.);
    plP_setpxl(ppm, ppm);
    plP_setphy(xmin, xmax*scale, ymin, ymax*scale);
}

static void
flushbuffer(PLStream *pls)
{
  if (curpts) {
    plD_polyline_ntk(pls, xb, yb, curpts);
/* if (curpts != 2) fprintf(stderr,"%d ", curpts); */
    xold = yold = -1; curpts = 0;
  }
}

void
plD_line_ntk(PLStream *pls, short x1a, short y1a, short x2a, short y2a)
{
  if (xold == x1a && yold == y1a) {
    xold = xb[curpts] = x2a; yold = yb[curpts] = y2a; curpts++;
  } else {
    flushbuffer(pls);
    xb[curpts] = x1a; yb[curpts] = y1a; curpts++;
    xold = xb[curpts] = x2a; yold = yb[curpts] = y2a; curpts++;
  }

  if (curpts == NPTS) {
    fprintf(stderr,"\nflush: %d ", curpts);
    flushbuffer(pls);
  }
}

void
plD_polyline_ntk(PLStream *pls, short *xa, short *ya, PLINT npts)
{
  PLINT i, j;

  /* there must exist a way to code this using the tk C API */
  j = sprintf(cmd,"$plf.f2.c%d create line ", ccanv);
  for (i = 0; i < npts; i++)
    j += sprintf(&cmd[j], "%.1f %.1f ", xa[i]/scale, ymax-ya[i]/scale);

  j += sprintf(&cmd[j]," -fill %s", curcolor);
  if (dash[0] == '-')
    j += sprintf(&cmd[j]," %s", dash);

  tk_cmd(cmd);
}

/* an event loop has to be designed, getcursor() and waitforpage() are just experimental */

static void
waitforpage(PLStream *pls)
{
  int key = 0, st = 0;
  /* why can't I bind to the canvas? or even any frame? */
/*tk_cmd("bind . <KeyPress> {set keypress %N; puts \"\n%k-%A-%K-%N\"}"); */
  tk_cmd("bind . <KeyPress> {set keypress %N}");

  while ((key & 0xff) != PLK_Return && (key & 0xff) != PLK_Linefeed && key != PLK_Next && key != 'Q') {
    while (st != 1) {
      tk_cmd("update");
      tk_cmd("info exists keypress");
      sscanf(interp->result,"%d", &st);
    }

    tk_cmd("set keypress");
    sscanf(interp->result,"%d", &key);
/*fprintf(stderr,"\n%d\n", key);fflush(stderr);*/
    tk_cmd("unset keypress");
    st = 0;
  }

  tk_cmd("bind . <Key> {};");
}

void
plD_eop_ntk(PLStream *pls)
{
  flushbuffer(pls);
  tk_cmd("update");
}

void
plD_bop_ntk(PLStream *pls)
{
  create_canvas(pls);
}

void
plD_tidy_ntk(PLStream *pls)
{
  if (! pls->nopause)
    waitforpage(pls);

  tk_cmd("destroy $plf; wm withdraw .");
}

void
plD_state_ntk(PLStream *pls, PLINT op)
{
    switch (op) {
    case PLSTATE_COLOR0:
    case PLSTATE_COLOR1:
      flushbuffer(pls);
      sprintf(curcolor, "#%02x%02x%02x",
	      pls->curcolor.r, pls->curcolor.g , pls->curcolor.b);
      break;
    }
}

static void
getcursor(PLStream *pls, PLGraphicsIn *ptr)
{
  int st = 0;

  plGinInit(&gin);

  if (0) {
    while (st != 1) {
      tk_cmd("update");
      tk_cmd("winfo exists $plf.f2.c$ccanv");
      sscanf(interp->result,"%d", &st);
    }
    st = 0;
    /* this give a "Segmentation fault", even after checking for the canvas! */
    tk_cmd("set ocursor [lindex [$plf.f2.c$ccanv configure -cursor] 4]");
  }

  tk_cmd("$plf.f2.c$ccanv configure -cursor cross;\n"
"bind $plf.f2.c$ccanv <Button> {set xloc %x; set yloc %y; set bloc %b; set sloc %s};\n"
"bind $plf.f2.c$ccanv <B1-Motion> {set xloc %x; set yloc %y; set bloc %b; set sloc %s};\n"
"bind $plf.f2.c$ccanv <B2-Motion> {set xloc %x; set yloc %y; set bloc %b; set sloc %s};\n"
"bind $plf.f2.c$ccanv <B3-Motion> {set xloc %x; set yloc %y; set bloc %b; set sloc %s};");

  while (st != 1) {
    tk_cmd("update");
    tk_cmd("info exists xloc");
    sscanf(interp->result,"%d", &st);
  }
  tk_cmd("set xloc");
  sscanf(interp->result,"%d", &gin.pX);
  tk_cmd("set yloc");
  sscanf(interp->result,"%d", &gin.pY);
  tk_cmd("set bloc");
  sscanf(interp->result,"%d", &gin.button);
  tk_cmd("set sloc");
  sscanf(interp->result,"%d", &gin.state);

  gin.dX = (PLFLT) gin.pX/xmax;
  gin.dY = 1. - (PLFLT) gin.pY/ymax;

  tk_cmd("bind $plf.f2.c$ccanv <ButtonPress> {};\n"
"bind $plf.f2.c$ccanv <ButtonMotion> {};\n"
"bind $plf.f2.c$ccanv <B2-Motion> {};\n"
"bind $plf.f2.c$ccanv <B3-Motion> {};\n"
"unset xloc");

  /* seg fault, see above. tk_cmd("$plf.f2.c$ccanv configure -cursor $ocursor"); */
  tk_cmd("$plf.f2.c$ccanv configure -cursor {}");

  *ptr = gin;
}

void
plD_esc_ntk(PLStream *pls, PLINT op, void *ptr)
{
  PLINT i,j;
  short *xa, *ya;
  Pixmap bitmap;
  static unsigned char bit_pat[] = {
    0x24, 0x01, 0x92, 0x00, 0x49, 0x00, 0x24, 0x00, 0x12, 0x00, 0x09, 0x00,
    0x04, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff};

  switch (op) {

  case PLESC_DASH:
    xa = (short *) malloc(sizeof(short) * pls->dev_npts);
    ya = (short *) malloc(sizeof(short) * pls->dev_npts);
    for (i = 0; i < pls->dev_npts; i++) {
      xa[i] = pls->dev_x[i];
      ya[i] = pls->dev_y[i];
    }

    j = sprintf(dash, "-dash {");
    for (i = 0; i < pls->nms; i++)
      j += sprintf(&dash[j]," %d %d",
		   (int) ceil(pls->mark[i]/1e3 * ppm),
		   (int) ceil(pls->space[i]/1e3 * ppm));
    sprintf(&dash[j], "}");
    plD_polyline_ntk(pls, xa, ya, pls->dev_npts);
    free(xa); free(ya);
    dash[0] = 0;
    break;

  case PLESC_FLUSH:
    tk_cmd("update");
    break;

  case PLESC_GETC:
    getcursor(pls, (PLGraphicsIn *) ptr);
    break;

  case PLESC_FILL:
    if (pls->patt != 0) {
      /* this is a hack! The real solution is in the if(0) bellow */
      pls->xpmm *= scale;
      pls->ypmm *= scale;
      plfill_soft( pls->dev_x, pls->dev_y, pls->dev_npts);
      pls->xpmm /= scale;
      pls->ypmm /= scale;
    } else {
      j = sprintf(cmd, "$plf.f2.c%d create polygon ", ccanv);
      for (i = 0; i < pls->dev_npts; i++)
	j += sprintf(&cmd[j], "%.1f %.1f ", pls->dev_x[i]/scale,
		     ymax-pls->dev_y[i]/scale);
      j += sprintf(&cmd[j]," -fill %s", curcolor);
      tk_cmd(cmd);
    }

    if (0) {
      if (pls->patt != 0) {
	Tk_DefineBitmap(interp, Tk_GetUid("foo"), bit_pat, 16, 16);
	bitmap = Tk_GetBitmap(interp, mainw, Tk_GetUid("patt"));
      }
      j = sprintf(cmd, "$plf.f2.c%d create polygon ", ccanv);
      for (i = 0; i < pls->dev_npts; i++)
	j += sprintf(&cmd[j], "%.1f %.1f ", pls->dev_x[i]/scale,
		     ymax-pls->dev_y[i]/scale);
      j += sprintf(&cmd[j]," -fill %s", curcolor);
      if (pls->patt != 0)
	sprintf(&cmd[j], " -stipple patt -outline black");

      tk_cmd(cmd);
  /*Tk_FreeBitmap(display, bitmap)*/
    }
    break;
  }
}

#else
int
pldummy_ntk()
{
    return 0;
}

#endif				/* PLD_ntkdev */
