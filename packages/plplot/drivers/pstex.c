/* $Id: pstex.c 3186 2006-02-15 18:17:33Z slbrow $

   PLplot pstex (Postscript/LaTeX) device driver.
*/

#include "plDevs.h"

#ifdef PLD_pstex

#include "plplotP.h"
#include "drivers.h"
#include "ps.h"

/* Device info */
char* plD_DEVICE_INFO_pstex =
  "pstex:Combined Postscript/LaTeX files:0:pstex:41:pstex";

/*--------------------------------------------------------------------------*\
 * plD_init_pstex()
 *
 * Initialize device.
\*--------------------------------------------------------------------------*/


static void parse_str(const char *str, char *dest);
static void proc_str (PLStream *pls, EscText *args);
static long cur_pos;
static FILE *fp;
static int  color = 1;

static DrvOpt pstex_options[] = {{"color", DRV_INT, &color,
				  "Color Postscript/LaTeX (color=1|0)"},
				 {NULL, DRV_INT, NULL, NULL}};

void plD_dispatch_init_pstex( PLDispatchTable *pdt )
{
#ifndef ENABLE_DYNDRIVERS
    pdt->pl_MenuStr  ="Combined Postscript/LaTeX files" ;
    pdt->pl_DevName  = "pstex";
#endif
    pdt->pl_type     = plDevType_FileOriented;
    pdt->pl_seq      = 41;
    pdt->pl_init     = (plD_init_fp)     plD_init_pstex;
    pdt->pl_line     = (plD_line_fp)     plD_line_ps;
    pdt->pl_polyline = (plD_polyline_fp) plD_polyline_ps;
    pdt->pl_eop      = (plD_eop_fp)      plD_eop_ps;
    pdt->pl_bop      = (plD_bop_fp)      plD_bop_pstex;
    pdt->pl_tidy     = (plD_tidy_fp)     plD_tidy_pstex;
    pdt->pl_state    = (plD_state_fp)    plD_state_ps;
    pdt->pl_esc      = (plD_esc_fp)      plD_esc_pstex;
}

void
plD_init_pstex(PLStream *pls)
{
  char ofile[80];

  plParseDrvOpts(pstex_options);
  if (color)
    plD_init_psc(pls); /* init color postscript driver */
  else
    plD_init_psm(pls); /* init monochrome postscript driver */

  pls->dev_text = 1; /* want to draw text */

  /* open latex output file */
  strncpy(ofile, pls->FileName, 80);
  strcat(ofile, "_t");
  fp = fopen(ofile, "w");

  fprintf(fp,"\\begin{picture}(0,0)(0,0)%%\n");
  fprintf(fp,"\\includegraphics[scale=1.,clip]{%s}%%\n",pls->FileName); 
  fprintf(fp,"\\end{picture}%%\n");
  fprintf(fp,"\\setlength{\\unitlength}{%fbp}%%\n", 72./25.4/pls->xpmm);
  fprintf(fp,"\\begingroup\\makeatletter\\ifx\\SetFigFont\\undefined%%\n");
  fprintf(fp,"\\gdef\\SetFigFont#1#2#3#4#5{%%\n");
  fprintf(fp,"\\reset@font\\fontsize{#1}{#2pt}%%\n");
  fprintf(fp,"\\fontfamily{#3}\\fontseries{#4}\\fontshape{#5}%%\n");
  fprintf(fp,"\\selectfont}%%\n");
  fprintf(fp,"\\fi\\endgroup%%\n"); 

  cur_pos = ftell(fp);
  fprintf(fp,"\\begin{picture}(xxxxxx,xxxxxx)(xxxxxx,xxxxxx)%%\n");
}

void
plD_esc_pstex(PLStream *pls, PLINT op, void *ptr)
{
  switch (op) {
  case PLESC_HAS_TEXT:
    proc_str(pls, ptr);
    break;
  default:
    plD_esc_ps(pls, op, ptr);
  }
}

void
plD_bop_pstex(PLStream *pls)
{
  plD_bop_ps(pls);
  plGetFam(pls);
}

void
plD_tidy_pstex(PLStream *pls)
{
  PSDev *dev = (PSDev *) pls->dev;
  PLFLT scale;

  plD_tidy_ps(pls);
  
  scale = pls->xpmm * 25.4 / 72.;
  
  fprintf(fp,"\\end{picture}\n");

  fseek(fp, cur_pos, SEEK_SET);
  fprintf(fp,"\\begin{picture}(%d,%d)(%d,%d)%%\n%%",
	  ROUND((dev->urx - dev->llx) * scale),
	  ROUND((dev->ury - dev->lly) * scale),
	  ROUND((dev->llx - XOFFSET) * scale),
	  ROUND((dev->lly - YOFFSET) * scale));

  fclose(fp);
}

void
proc_str (PLStream *pls, EscText *args)
{
  PLFLT *t = args->xform;
  PLFLT a1, alpha, ft_ht, angle;
  char cptr[256], jst, ref;
  PSDev *dev = (PSDev *) pls->dev;
  PLINT clxmin, clxmax, clymin, clymax;

  /* font height */
  ft_ht = 1.6 /*!*/ * pls->chrht * 72.0/25.4; /* ft_ht in points. ht is in mm */

  /* calculate baseline text angle */
  angle = ((PLFLT)(ORIENTATION-1) + pls->diorot) * 90.;
  a1 = acos(t[0]) * 180. / PI;
  if (t[2] > 0.)
    alpha = a1 - angle - 90.;
  else
    alpha = 360. - a1 - angle - 90.;

  /* parse string for format (escape) characters */
  parse_str(args->string, cptr);

  /* 
   * Reference point (center baseline of string, not latex character reference point). 
   *  If base = 0, it is aligned with the center of the text box
   *  If base = 1, it is aligned with the baseline of the text box
   *  If base = 2, it is aligned with the top of the text box
   *  Currently plplot only uses base=0
   */

  if (args->base == 2) /* not supported by plplot */
    ref = 't';
  else if (args->base == 1)
    ref = 'b';
  else
    ref = 'c';

  /* 
   * Text justification.  Left, center and right justification, which
   *  are the more common options, are supported; variable justification is
   *  only approximate, based on plplot computation of it's string lenght
   */

  if (args->just == 0.5)
    jst = 'c';
  else if (args->just == 1.)
    jst = 'r';
  else {
    jst = 'l';
    args->x = args->refx; /* use hints provided by plplot */
    args->y = args->refy;
  }

  /* apply transformations */
  difilt(&args->x, &args->y, 1, &clxmin, &clxmax, &clymin, &clymax);

  /* check clip limits. For now, only the reference point of the string is checked;
     but the the whole string should be checked -- using a postscript construct
     such as gsave/clip/grestore. This method can also be applied to the xfig and
     pstex drivers. Zoom side effect: the font size must be adjusted! */

  if ( args->x < clxmin || args->x > clxmax || args->y < clymin || args->y > clymax)
    return;

  plRotPhy(ORIENTATION, dev->xmin, dev->ymin, dev->xmax, dev->ymax,
	   &(args->x), &(args->y));

#ifdef DEBUG
  fprintf(fp,"\\put(%d,%d){\\circle{10}}\n",
	  args->x, args->y);
#endif

  fprintf(fp,"\\put(%d,%d){\\rotatebox{%.1f}{\\makebox(0,0)[%c%c]{\\SetFigFont{%.1f}{12}",
	  args->x, args->y, alpha, jst, ref, ft_ht);

  /*
   *  font family, serie and shape. Currently not supported by plplot
   *
   *  Use current font instead:
   *  1: Normal font (latex document default font)
   *  2: Roman font
   *  3: Italic font (most probably latex slanted)
   *  4: Script font (latex sans serif)
   */

  switch (pls->cfont) {
  case (1): fprintf(fp, "{\\familydefault}"); break;
  case (2): fprintf(fp, "{\\rmdefault}"); break;
  case (3): fprintf(fp, "{\\itdefault}"); break;
  case (4): fprintf(fp, "{\\sfdefault}"); break;
  default:  fprintf(fp, "{\\familydefault}");
  }

  fprintf(fp,"{\\mddefault}{\\updefault}\n");

  /* font color. */

  if (color)
    fprintf(fp,"\\special{ps: %.3f %.3f %.3f setrgbcolor}{",
	    pls->curcolor.r /255., pls->curcolor.g /255., pls->curcolor.b /255.);
  else
    fprintf(fp,"\\special{ps: 0 0 0 setrgbcolor}{");

  fprintf(fp,"%% Your text follows:\n");
  fprintf(fp,"%s\n", cptr);
  fprintf(fp,"}}}}");
  
  /*
   * keep ps driver happy -- needed for background and orientation.
   * arghhh! can't calculate it, as I only have the string reference
   * point, not its extent!
   * Quick (and final?) *hack*, ASSUME that no more than a char height
   * extents after/before the string reference point.
   */

  dev->llx = MIN(dev->llx, args->x - ft_ht * 25.4 / 72. * pls->xpmm);
  dev->lly = MIN(dev->lly, args->y - ft_ht * 25.4 / 72. * pls->ypmm);
  dev->urx = MAX(dev->urx, args->x + ft_ht * 25.4 / 72. * pls->xpmm);
  dev->ury = MAX(dev->ury, args->y + ft_ht * 25.4 / 72. * pls->ypmm);
}

void
parse_str(const char *str, char *dest)
{
  int  n, opened = 0, raised = 0, overline = 0, underline = 0, fontset = 0, math = 0;
  char *tp = dest, c, esc;
  char greek[] = "abgGdDezyhHiklLmncCopPrsStuUfFxqQwW";
  char *mathgreek[] = {"alpha", "beta", "gamma", "Gamma", "delta", "Delta",
		       "epsilon", "zeta", "eta", "theta", "Theta", "iota",
		       "kappa", "lambda", "Lambda", "mu", "nu", "xi", "Xi",
		       "o", "pi", "Pi", "rho", "sigma", "Sigma", "tau",
		       "upsilon", "Upsilon", "phi", "Phi", "chi",
		       "psi", "Psi", "omega", "Omega"};

  plgesc(&esc);

  while (*str) {

    if (*str != esc) {
      *tp++ = *str++;
      continue;
    }
    str++;

    switch (*str++) {

    case 'u': /* up one level */
      if (raised < 0) {
	*tp++ = '}';
	opened--;
      } else {
	n = sprintf(tp, "\\raisebox{%.2fex}{", 0.6);
	tp += n; opened++;
      }
      raised++;
      break;

    case 'd': /* down one level */
      if (raised > 0) {
	*tp++ = '}';
	opened--;
      } else {
	n = sprintf(tp, "\\raisebox{%.2fex}{", -0.6);
	tp += n; opened++;
      }
      raised--;
      break;

    case 'b': /* backspace */
      n = sprintf(tp,"\\hspace{-1em}");
      tp += n;
      break;
          
    case '+': /* toggles overline mode. Side effect, enter math mode. */
      if (overline) {
	if (--math)
	  *tp++ = '}';
	else {
	  n = sprintf(tp, "}$");
	  tp += n;
	}
	overline--; opened--;
      } else {
	if (!math)
	  *tp++ = '$';

	n = sprintf(tp, "\\overline{");
	tp += n; overline++; opened++; math++;
      }
      break;

    case '-': /* toggles underline mode. Side effect, enter math mode. */
      if (underline) {
        if (--math)
	  *tp++ = '}';
	else {
          n = sprintf(tp, "}$");
	  tp += n;
        }
	underline--; opened--;
      } else {
        if (!math)
	  *tp++ = '$';

	n = sprintf(tp, "\\underline{");      
	tp += n; underline++; opened++; math++;
      }
      break;

    case 'g': /* greek letter corresponding to roman letter x */
      c = *str++;
      n = plP_strpos(greek, c);
      if (n != -1) {
        if (!math) 
	  *tp++ = '$';

	*tp++ = '\\';
	strcpy(tp, mathgreek[n]);
	if (isupper(c))
	  *tp = toupper(*tp);
	tp += strlen(mathgreek[n]);
	if (!math) 
	  *tp++ = '$';

      } else
	*tp++ = c;

      break;

    case '(': /* Hershey symbol number (nnn) (any number of digits) FIXME ???*/
      plwarn("'g(...)' text escape sequence not processed.");
      while (*str++ != ')');
      break;

    case 'f': /* switch font */
     
      switch (*str++) {
      case 'n': /* Normal */
	while (fontset--) {
	  *tp++ = '}';
	  opened--;
	}

	if (math) {
	  *tp++ = '$';
	  math = 0;
	}

	n = sprintf(tp, "\\normalfont ");
	tp += n;
	break;

      case 'r': /* Roman */
	if (math)
	  n = sprintf(tp, "\\mathrm{");
	else
	  n = sprintf(tp, "\\textrm{");

	tp += n; opened++; fontset++;
	break;

      case 'i': /* Italic */
	if (math)
	  n = sprintf(tp, "\\mathit{");
	else
	  n = sprintf(tp, "\\textit{");

	tp += n; opened++; fontset++;
	break;

      case 's': /* Script. Don't, use sans serif */
	if (math)
	  n = sprintf(tp, "\\mathsf{");
	else
	  n = sprintf(tp, "\\textsf{");

	tp += n; opened++; fontset++;
	break;

      }

    default:
      if (*str == esc)
	*tp++ = esc;
    }
  }

  while(opened--)
    *tp++ = '}';
  *tp = '\0';

}

#else
int 
pldummy_pstex()
{
  return 0;
}

#endif            /* PLD_pstexdev */
