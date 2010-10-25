/* $Id: plrender.c 3186 2006-02-15 18:17:33Z slbrow $

    Copyright 1991, 1992, 1993, 1994, 1995
    Geoffrey Furnish			furnish@dino.ph.utexas.edu
    Maurice LeBrun			mjl@dino.ph.utexas.edu
    Institute for Fusion Studies	University of Texas at Austin

    Copyright (C) 2004  Alan W. Irwin
    Copyright (C) 2004  Maurice LeBrun
    Copyright (C) 2004  Andrew Ross

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


* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    This file contains the code to render a PLplot metafile, written by
    the metafile driver, plmeta.c.
*/

char ident[] = "@(#) $Id: plrender.c 3186 2006-02-15 18:17:33Z slbrow $";

#define DEBUG
#define DEBUG_ENTER

#define NEED_PLDEBUG
#include "plplotP.h"
#include "plevent.h"
#include "metadefs.h"
#include <ctype.h>

/* Static function prototypes. */
/* These handle the command loop */

static void	process_next	(U_CHAR c);
static void	plr_init	(U_CHAR c);
static void	plr_line	(U_CHAR c);
static void	plr_eop 	(U_CHAR c);
static void	plr_eop1 	(U_CHAR c);
static void	plr_bop 	(U_CHAR c);
static void	plr_state	(U_CHAR c);
static void	plr_esc		(U_CHAR c);
static void	plresc_fill	(void);
static void	plresc_rgb	(void);
static void	plresc_ancol	(void);

/* Support functions */

static U_CHAR	getcommand	(void);
static void	ungetcommand	(U_CHAR);
static void	get_ncoords	(PLFLT *x, PLFLT *y, PLINT n);
static void	plr_exit	(char *errormsg);
static void	NextFamilyFile	(U_CHAR *);
static void	PrevFamilyFile	(void);
static void	ReadPageHeader	(void);
static void	plr_KeyEH	(PLGraphicsIn *, void *, int *);
static void	SeekToDisp	(long);
static void	SeekOnePage	(void);
static void	SeekToNextPage	(void);
static void	SeekToCurPage	(void);
static void	SeekToPrevPage	(void);
static void	SeekTo		(FPOS_T);
static void	doseek		(FPOS_T);
static void	PageIncr	(void);
static void	PageDecr	(void);

/* Initialization functions */

static void	Init		(int, char **);
static int	ProcessFile	(int, char **);
static int	OpenMetaFile	(char **);
static int	ReadFileHeader	(void);

/* Option handlers */

static int Opt_v	(char *, char *, void *);
static int Opt_i	(char *, char *, void *);
static int Opt_b	(char *, char *, void *);
static int Opt_e	(char *, char *, void *);
static int Opt_p	(char *, char *, void *);

/* Global variables */

/* Copies of argc, argv, for use with multiple files */

static int myargc;
static char **myargv;

/* Page info */

static PLINT	disp_beg=1;	/* Where to start plotting */
static PLINT	disp_end=-1;	/* Where to stop (0 to disable) */
static PLINT	curdisp;	/* Current page number */
static PLINT	cursub;		/* Current subpage */
static PLINT	curpage;	/* Current plot number */
static PLINT	nsubx;		/* subpages in x */
static PLINT	nsuby;		/* subpages in y */
static PLINT	target_disp;	/* Page we are seeking to */
static PLINT	target_page;	/* Plot we are seeking to */
static PLINT	delta;		/* Number of pages to go forward/back */
static PLINT	pages;		/* Number of pages in file */

static int	no_pagelinks;	/* Set if metafile doesn't have page links */
static int	end_of_page;	/* Set when we're at the end of a page */
static int	seek_mode;	/* Set after a seek, before a BOP */
static int	addeof_beg;	/* Set when we are counting back from eof */
static int	addeof_end;	/* Set when we are counting back from eof */
static int	first_page;	/* Set when we're on the first page in the file */

static FPOS_T	prevpage_loc;	/* Byte position of previous page header */
static FPOS_T	curpage_loc;	/* Byte position of current page header */
static FPOS_T	nextpage_loc;	/* Byte position of next page header */

/* File info */

static int	input_type;	/* 0 for file, 1 for stream */
static int	isfile;		/* shorthand -- set if file */
static int	do_file_loop=1;	/* loop over multiple files if set */
static PDFstrm	*pdfs;		/* PDF stream handle */
static FILE	*MetaFile;	/* Actual metafile handle, for seeks etc */

static char	BaseName[80] = "", FileName[90] = "";
static PLINT	is_family, member=1;
static char	mf_magic[40], mf_version[40];

/* Dummy vars for reading stuff that is to be thrown away */

static U_CHAR	dum_uchar;
static U_SHORT	dum_ushort;
static float	dum_float;

/* Plot dimensions */

static short	xmin = 0;
static short	xmax = PLMETA_X_OLD;
static short	ymin = 0;
static short	ymax = PLMETA_Y_OLD;
static PLINT	xlen, ylen;

static float	pxlx = PIXEL_RES_X_OLD;
static float	pxly = PIXEL_RES_Y_OLD;

static PLFLT	dev_xpmm, dev_ypmm;
static PLINT	dev_xmin, dev_xmax, dev_ymin, dev_ymax, dev_xlen, dev_ylen;
static PLFLT	vpxmin, vpxmax, vpxlen, vpymin, vpymax, vpylen;

/* Geometry info */

static float    xdpi, ydpi;
static PLINT    xlength, ylength, xoffset, yoffset;

/* Miscellaneous */

static U_CHAR 	c_old, c1;
static U_SHORT	npts;
static int	direction_flag, isanum, at_eop;
static char	num_buffer[20];
static PLFLT	x[PL_MAXPOLY], y[PL_MAXPOLY];
static char	buffer[256];
static char *	cmdstring[256];

/* Exit codes */

#define	EX_SUCCESS	0		/* success! */
#define	EX_ARGSBAD	1		/* invalid args */
#define	EX_BADFILE	2		/* invalid filename or contents */

/* A little function to help with debugging */

#ifdef DEBUG
#define DEBUG_PRINT_LOCATION(a) PrintLocation(a)

static void PrintLocation(char *tag)
{
    if (isfile) {
	FPOS_T current_offset;

	if (pl_fgetpos(MetaFile, &current_offset))
	    plexit("PrintLocation (plrender.c): fgetpos call failed");

	pldebug(tag, "at offset %d in file %s\n",
		(int) current_offset, FileName);
    }
}
#else
#define DEBUG_PRINT_LOCATION(a)
#endif

/*--------------------------------------------------------------------------*\
 * Options data structure definition.
\*--------------------------------------------------------------------------*/

static PLOptionTable options[] = {
{
    "v",			/* Version */
    Opt_v,
    NULL,
    NULL,
    PL_OPT_FUNC | PL_OPT_NODELETE,
    "-v",
    "Print out the plrender version number" },
{
    "i",			/* Input file */
    Opt_i,
    NULL,
    NULL,
    PL_OPT_FUNC | PL_OPT_ARG,
    "-i name",
    "Input filename" },
{
    "b",			/* Beginning page number */
    Opt_b,
    NULL,
    NULL,
    PL_OPT_FUNC | PL_OPT_ARG,
    "-b number",
    "Beginning page number" },
{
    "e",			/* End page number */
    Opt_e,
    NULL,
    NULL,
    PL_OPT_FUNC | PL_OPT_ARG,
    "-e number",
    "End page number" },
{
    "p",			/* Specified page only */
    Opt_p,
    NULL,
    NULL,
    PL_OPT_FUNC | PL_OPT_ARG,
    "-p page",
    "Plot given page only" },
{
    NULL,			/* option */
    NULL,			/* handler */
    NULL,			/* client data */
    NULL,			/* address of variable to set */
    0,				/* mode flag */
    NULL,			/* short syntax */
    NULL }			/* long syntax */
};

static char *notes[] = {
"If the \"-i\" flag is omitted, unrecognized input will assumed to be filename",
"parameters.  Specifying \"-\" for the input or output filename means use stdin",
"or stdout, respectively.  See the manual for more detail.",
NULL};

/*--------------------------------------------------------------------------*\
 * main()
 *
 * plrender -- render a series of PLplot metafiles.
\*--------------------------------------------------------------------------*/

int
main(int argc, char *argv[])
{
    Init(argc, argv);

/* Process first file.  There must be at least one. */

    if (ProcessFile(argc, argv)) {
	fprintf(stderr, "\nNo filename specified.\n");
	plOptUsage();
	exit(EX_ARGSBAD);
    }

/* Process any additional files */

    if (do_file_loop) {
	pltext();
	while ( ! ProcessFile(argc, argv))
	    ;
    }
    plend();
    if (myargv) 
      free(myargv);
    exit(EX_SUCCESS);
}

/*--------------------------------------------------------------------------*\
 * Init()
 *
 * Do initialization for main().
\*--------------------------------------------------------------------------*/

static void
Init(int argc, char **argv)
{
    int i;

    dbug_enter("Init");

/* Set up for argv processing */

    plSetUsage("plrender", "\nUsage:\n        plrender [options] [files]\n");

    plMergeOpts(options, "plrender options", notes);

/* Save argv list for future reuse */

    myargv = (char **) malloc(argc * sizeof(char *));
    myargc = argc;
    for (i = 0; i < argc; i++) {
	myargv[i] = argv[i];
    }

/* Set up names for commands for debugging */

    for (i = 0; i < 256; i++)
	cmdstring[i]	= "UNDEFINED";

    cmdstring[INITIALIZE]	= "INITIALIZE";
    cmdstring[CLOSE]		= "CLOSE";
    cmdstring[SWITCH_TO_TEXT]	= "SWITCH_TO_TEXT";
    cmdstring[SWITCH_TO_GRAPH]	= "SWITCH_TO_GRAPH";
    cmdstring[EOP]		= "EOP";
    cmdstring[BOP]		= "BOP";
    cmdstring[NEW_COLOR]	= "NEW_COLOR";
    cmdstring[NEW_WIDTH]	= "NEW_WIDTH";
    cmdstring[LINE]		= "LINE";
    cmdstring[LINETO]		= "LINETO";
    cmdstring[ESCAPE]		= "ESCAPE";
    cmdstring[ADVANCE]		= "ADVANCE";
    cmdstring[POLYLINE]		= "POLYLINE";
    cmdstring[NEW_COLOR0]	= "NEW_COLOR0";
    cmdstring[NEW_COLOR1]	= "NEW_COLOR1";
    cmdstring[CHANGE_STATE]	= "CHANGE_STATE";
    cmdstring[BOP0]		= "BOP0";
    cmdstring[END_OF_FIELD]	= "END_OF_FIELD";
}

/*--------------------------------------------------------------------------*\
 * ProcessFile()
 *
 * Renders a file, using given command flags.
\*--------------------------------------------------------------------------*/

static int
ProcessFile(int argc, char **argv)
{
    int i;
    U_CHAR c=0;
    char devname[80];

    dbug_enter("ProcessFile");

/* Do all rendering in a new plot stream to make cleanup easier. */

    plsstrm(1);

/* Process plrender and PLplot (internal) command line options */
/* Since we aren't using full parsing, plparseopts() will stop when it hits */
/* a non-flag item */

    if (plparseopts(&argc, argv, 0))
	exit(1);

/* Any remaining flags are illegal. */

    if (argv[1] != NULL && (argv)[1][0] == '-') {
	fprintf(stderr, "\nBad command line option \"%s\"\n", argv[1]);
	plOptUsage();
	exit(1);
    }

/* Try to open metafile. */

    if (OpenMetaFile(argv))
	return 1;

/* Initialize file and read header */

    pdfs = pdf_finit(MetaFile);

    if (ReadFileHeader())
	exit(EX_BADFILE);

/* Read & process any state info before the INITIALIZE */

    for (;;) {
	c_old = c;
	c = getcommand();

	if (c == INITIALIZE) {
	    ungetcommand(c);
	    break;
	}
	process_next(c);
    }

/*
 * Reprocess the command line options so that they supercede their possible
 * counterparts in the metafile header.
 */

    argc = myargc;
    for (i = 0; i < argc; i++) {
	argv[i] = myargv[i];
    }
    (void) plparseopts(&argc, argv, 0);

/* Miscellaneous housekeeping */

    if (addeof_beg) disp_beg += pages;
    if (addeof_end) disp_end += pages;

    plgdev(devname);
    if (strncmp(devname,"tk",2)==0) {
      plSetInternalOpt("-drvopt", "tcl_cmd=set plw_create_proc plr_create");
    }

/*
 * Read & process metafile commands.
 * If familying is turned on, the end of one member file is just treated as
 * a page break assuming the next member file exists.
 */

    for (;;) {
	c_old = c;
	c = getcommand();

	if (c == CLOSE) {
	    if (is_family)
		NextFamilyFile(&c);
	    if ( ! is_family)
		break;
	}

	if ((c == BOP || c == BOP0 || c == ADVANCE) && curdisp == disp_end)
	    break;

	process_next(c);
    }

/* Finish up */

    pdf_close(pdfs);
    *FileName = '\0';

/* A hack for old metafiles */

    if (strcmp(mf_version, "1993a") >= 0)
	plspause(0);

    plend1();

/* Restore the old argc/argv */

    argc = myargc;
    for (i = 0; i < argc; i++) {
	argv[i] = myargv[i];
    }

    return 0;
}

/*--------------------------------------------------------------------------*\
 * OpenMetaFile()
 *
 * Attempts to open a metafile.  If the output file isn't already determined
 * via the -i or -f flags, we assume it's the second argument in argv[] (the
 * first should still hold the program name).  Null out the string after
 * copying it so that it doesn't appear on subsequent passes of the command
 * line.
\*--------------------------------------------------------------------------*/

static int
OpenMetaFile(char **argv)
{
    char name[70];

    dbug_enter("OpenMetaFile");

    if (!strcmp(FileName, "-"))
	input_type = 1;

    isfile = (input_type == 0);

    if (!isfile) {
	MetaFile = stdin;
	do_file_loop = 0;

    } else {
	if (*FileName == '\0') {
	    if (argv[1] != NULL && *argv[1] != '\0') {
		strncpy(FileName, argv[1], sizeof(FileName) - 1);
		FileName[sizeof(FileName) - 1] = '\0';
		argv[1][0] = '\0';
	    }
	    else {
		return 1;
	    }
	}

    /* Try to read named Metafile.  The following cases are checked in order:
     *	<FileName>
     *	<FileName>.1
     *	<FileName>.plm
     *	<FileName>.plm.1
     */
	pldebug("OpenMetaFile", "Trying to open metafile %s.\n", FileName);
	strncpy(name, FileName, sizeof(name) - 1);
	name[sizeof(name) - 1] = '\0';

	if ((MetaFile = fopen(FileName, "rb")) != NULL) {
	    return 0;
	}

	(void) sprintf(FileName, "%s.%i", name, (int) member);
	if ((MetaFile = fopen(FileName, "rb")) != NULL) {
	    (void) sprintf(BaseName, "%s", name);
	    is_family = 1;
	    return 0;
	}

	(void) sprintf(FileName, "%s.plm", name);
	if ((MetaFile = fopen(FileName, "rb")) != NULL) {
	    return 0;
	}

	(void) sprintf(FileName, "%s.plm.%i", name, (int) member);
	if ((MetaFile = fopen(FileName, "rb")) != NULL) {
	    (void) sprintf(BaseName, "%s.plm", name);
	    is_family = 1;
	    return 0;
	}

	fprintf(stderr, "\nUnable to open: %s.\n", name);
	plOptUsage();
	exit(EX_BADFILE);
    }

    return 0;
}

/*--------------------------------------------------------------------------*\
 * 			Process the command loop
\*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*\
 * process_next()
 *
 * Process a command.
 * Typically plrender issues commands to PLplot much like an application
 * program would.  This results in a more robust and flexible API.  On the
 * other hand, it is sometimes necessary to directly call low-level PLplot
 * routines to achieve certain special effects, increase performance, or
 * simplify the code.
\*--------------------------------------------------------------------------*/

static void
process_next(U_CHAR c)
{
/* Specially handle line draws to contain output when debugging */

    switch ((int) c) {

    case LINE:
    case LINETO:
    case POLYLINE:
	plr_line(c);
	if (c != c_old)
	    pldebug("process_next", "processing command %s\n", cmdstring[c]);
	return;
    }

/* Everything else */

    pldebug("process_next", "processing command %s\n", cmdstring[c]);

    switch ((int) c) {

    case INITIALIZE:
	plr_init(c);
	return;

    case EOP:
	plr_eop(c);
	return;

    case BOP:
    case BOP0:
	plr_bop(c);
	return;

    case CHANGE_STATE:
	plr_state(getcommand());
	return;

    case ESCAPE:
	plr_esc(c);
	return;

/* These are all commands that should be absent from current metafiles but */
/* are recognized here for backward compatibility with old metafiles */

    case ADVANCE:
    /* Note here we use plr_eop1() to avoid multiple ungetc's */
	plr_eop1(c);
	plr_bop(c);
	return;

    case NEW_WIDTH:
	plr_state(PLSTATE_WIDTH);
	return;

    case NEW_COLOR0:
	plr_state(PLSTATE_COLOR0);
	return;

    case NEW_COLOR1:
	plr_state(PLSTATE_COLOR1);
	return;

    case SWITCH_TO_TEXT:
    case SWITCH_TO_GRAPH:
	return;

    default:
	sprintf(buffer, "process_next: Unrecognized command code %d", (int) c);
	plr_exit(buffer);
    }
}

/*--------------------------------------------------------------------------*\
 * void plr_init()
 *
 * Handle initialization.
\*--------------------------------------------------------------------------*/

static void
plr_init(U_CHAR c)
{
    int debug=0;
    PLFLT aspect, dev_aspect, ratio;

    dbug_enter("plr_init");

/* Register event handler */

    plsKeyEH(plr_KeyEH, NULL);

/* Note: many drivers ignore these, needed to preserve the aspect ratio */

    plspage( xdpi, ydpi, xlength, ylength, xoffset, yoffset );

/* Start up PLplot */

    plinit();
    plP_gsub(&nsubx, &nsuby, &cursub);

/* Set aspect ratio to the natural ratio of the metafile coordinate system. */

    xlen = xmax - xmin;
    ylen = ymax - ymin;

    aspect = (ylen / pxly) / (xlen / pxlx);

    if (debug)
        printf( "xlen %d pxlx %f ylen %d pxly %f\n",
                xlen, pxlx, ylen, pxly );

/* Aspect ratio of output device */

    plP_gphy(&dev_xmin, &dev_xmax, &dev_ymin, &dev_ymax);
    plP_gpixmm(&dev_xpmm, &dev_ypmm);

    dev_xlen = dev_xmax - dev_xmin;
    dev_ylen = dev_ymax - dev_ymin;

    if (debug)
        printf( "dev_xlen %d dev_xpmm %f dev_ylen %d dev_ypmm %f\n",
                dev_xlen, dev_xpmm, dev_ylen, dev_ypmm );

    dev_aspect = (dev_ylen / dev_ypmm) / (dev_xlen / dev_xpmm);

    if (dev_aspect <= 0.)
	fprintf(stderr, "Aspect ratio error: dev_aspect = %f\n", dev_aspect);

    ratio = aspect / dev_aspect;

    if (debug)
        printf( "ratio %f aspect %f dev_aspect %f\n",
                ratio, aspect, dev_aspect );

/* Default relative coordinate space */

    vpxlen = 1.0;
    vpylen = 1.0;
    vpxmin = 0.5 - vpxlen / 2.;
    vpymin = 0.5 - vpylen / 2.;
    vpxmax = vpxmin + vpxlen;
    vpymax = vpymin + vpylen;

/*
 * Construct viewport that preserves the aspect ratio of the original device
 * (plmeta output file).  Thus you automatically get all physical coordinate
 * plots to come out correctly.  Note: this could also be done using the
 * driver interface function plsdimap.
 */

    if (ratio <= 0)
	fprintf(stderr, "Aspect ratio error: ratio = %f\n", ratio);
    else if (ratio < 1)
	vpylen = ratio;
    else
	vpxlen = 1. / ratio;

    vpxmin = (1. - vpxlen) / 2.;
    vpxmax = vpxmin + vpxlen;

    vpymin = (1. - vpylen) / 2.;
    vpymax = vpymin + vpylen;

    if (debug)
        printf( "vpxmin %f vpxmax %f vpymin %f vpymax %f\n",
                vpxmin, vpxmax, vpymin, vpymax );

/* Seek to first page */

    cursub = nsubx*nsuby;
    if (disp_beg > 1) {
	if (no_pagelinks)
	    plwarn("plrender: Metafile does not support page seeks");
	else {
	    ReadPageHeader();
	    SeekToDisp(disp_beg);
	}
    }
}

/*--------------------------------------------------------------------------*\
 * plr_line()
 *
 * Draw a line or polyline.
 * Multiple connected lines (i.e. LINETO's) are collapsed into a polyline.
\*--------------------------------------------------------------------------*/

static void
plr_line(U_CHAR c)
{
    npts = 1;

    switch ((int) c) {

    case LINE:
	get_ncoords(x, y, 1);

    case LINETO:
	for (;;) {
	    get_ncoords(x + npts, y + npts, 1);

	    npts++;
	    if (npts == PL_MAXPOLY)
		break;

	    c1 = getcommand();
	    if (c1 != LINETO) {
		ungetcommand(c1);
		break;
	    }
	}
	break;

    case POLYLINE:
	plm_rd( pdf_rd_2bytes(pdfs, &npts) );
	get_ncoords(x, y, npts);
	break;
    }

    plline(npts, x, y);

    x[0] = x[npts - 1];
    y[0] = y[npts - 1];
}

/*--------------------------------------------------------------------------*\
 * get_ncoords()
 *
 * Read n coordinate vectors.
\*--------------------------------------------------------------------------*/

static void
get_ncoords(PLFLT *x, PLFLT *y, PLINT n)
{
    short xs[PL_MAXPOLY], ys[PL_MAXPOLY];
    PLINT i;

    plm_rd( pdf_rd_2nbytes(pdfs, (U_SHORT *) xs, n) );
    plm_rd( pdf_rd_2nbytes(pdfs, (U_SHORT *) ys, n) );

    for (i = 0; i < n; i++) {
	x[i] = xs[i];
	y[i] = ys[i];
    }
}

/*--------------------------------------------------------------------------*\
 * plr_eop()
 *
 * Handle end of page.
 *
 * Here we run into a bit of difficulty with packed pages -- at the end
 * there is no EOP operation done if the page is only partially full.
 * So I peek ahead to see if the next operation is a CLOSE, and if so,
 * push back the CLOSE and issue an EOP regardless.
\*--------------------------------------------------------------------------*/

static void
plr_eop(U_CHAR c)
{
    dbug_enter("plr_eop");

    c1 = getcommand();
    ungetcommand(c1);
    if (c1 == CLOSE)
	end_of_page = 1;

    plr_eop1(c);
}

/*--------------------------------------------------------------------------*\
 * plr_eop1()
 *
 * Handle end of page.
 *
 * This is for use with page advances where plr_eop's packed-page logic isn't
 * needed and in fact results in back-to-back ungetc's which are not
 * guaranteed to work.
\*--------------------------------------------------------------------------*/

static void
plr_eop1(U_CHAR c)
{
    dbug_enter("plr_eop1");

    if (cursub == nsubx * nsuby)
	end_of_page = 1;

    if (end_of_page == 1) {
	at_eop = 1;
	plP_eop();
	at_eop = 0;
    }
}

/*--------------------------------------------------------------------------*\
 * plr_bop()
 *
 * Page/subpage advancement.
\*--------------------------------------------------------------------------*/

static void
plr_bop(U_CHAR c)
{
    dbug_enter("plr_bop");

    ungetcommand(c);
    ReadPageHeader();

/* Advance and setup the page or subpage */

    if (end_of_page) {
	plP_bop();
	end_of_page = 0;
	seek_mode = 0;
    }

    plP_ssub(nsubx, nsuby, cursub);
    plP_setsub();

    plvpor(vpxmin, vpxmax, vpymin, vpymax);
    plwind((PLFLT) xmin, (PLFLT) xmax, (PLFLT) ymin, (PLFLT) ymax);
}

/*--------------------------------------------------------------------------*\
 * plr_state()
 *
 * Handle change in PLStream state (color, pen width, fill attribute, etc).
\*--------------------------------------------------------------------------*/

static void
plr_state(U_CHAR op)
{
    int i;

    dbug_enter("plr_state");

    switch (op) {

    case PLSTATE_WIDTH:{
	U_SHORT width;

	plm_rd( pdf_rd_2bytes(pdfs, &width) );

	plwid(width);
	break;
    }

    case PLSTATE_COLOR0:{
	if (strcmp(mf_version, "2005a") >= 0)
        {
            short icol0;
	    plm_rd( pdf_rd_2bytes(pdfs, &icol0) );

	    if (icol0 == PL_RGB_COLOR)
            {
                U_CHAR r, g, b;
		plm_rd( pdf_rd_1byte(pdfs, &r) );
		plm_rd( pdf_rd_1byte(pdfs, &g) );
		plm_rd( pdf_rd_1byte(pdfs, &b) );
		plrgb1(r, g, b);
	    }
	    else {
		plcol(icol0);
	    }
        }
	else if (strcmp(mf_version, "1993a") >= 0)
        {
            U_CHAR icol0;
	    plm_rd( pdf_rd_1byte(pdfs, &icol0) );

	    if (icol0 == 1<<7)
            {
                U_CHAR r, g, b;
		plm_rd( pdf_rd_1byte(pdfs, &r) );
		plm_rd( pdf_rd_1byte(pdfs, &g) );
		plm_rd( pdf_rd_1byte(pdfs, &b) );
		plrgb1(r, g, b);
	    }
	    else {
		plcol(icol0);
	    }
	}
	else
        {
            U_SHORT icol;
	    plm_rd( pdf_rd_2bytes(pdfs, &icol) );
	    plcol(icol);
	}
	break;
    }

    case PLSTATE_COLOR1:{
	U_SHORT icol1;
	PLFLT col1;

	plm_rd( pdf_rd_2bytes(pdfs, &icol1) );
	col1 = (PLFLT) icol1 / (PLFLT) plsc->ncol1;
	plcol1(col1);
	break;
    }

    case PLSTATE_FILL:{
	signed char patt;

	plm_rd( pdf_rd_1byte(pdfs, (U_CHAR *) &patt) );
	plpsty(patt);
	break;
    }

    case PLSTATE_CMAP0:{
	if (strcmp(mf_version, "2005a") >= 0)
        {
            U_SHORT ncol0;
            plm_rd(pdf_rd_2bytes(pdfs, &ncol0));
            plscmap0n(ncol0);
        }
        else
        {
            U_CHAR ncol0;
            plm_rd(pdf_rd_1byte(pdfs, &ncol0));
            plscmap0n(ncol0);
        }
	for (i = 0; i < plsc->ncol0; i++) {
	    plm_rd(pdf_rd_1byte(pdfs, &plsc->cmap0[i].r));
	    plm_rd(pdf_rd_1byte(pdfs, &plsc->cmap0[i].g));
	    plm_rd(pdf_rd_1byte(pdfs, &plsc->cmap0[i].b));
	}
	if (plsc->level > 0)
	    plP_state(PLSTATE_CMAP0);

	break;
    }

    case PLSTATE_CMAP1:{
	U_SHORT ncol1;

	plm_rd(pdf_rd_2bytes(pdfs, &ncol1));
	plscmap1n(ncol1);
	for (i = 0; i < plsc->ncol1; i++) {
	    plm_rd(pdf_rd_1byte(pdfs, &plsc->cmap1[i].r));
	    plm_rd(pdf_rd_1byte(pdfs, &plsc->cmap1[i].g));
	    plm_rd(pdf_rd_1byte(pdfs, &plsc->cmap1[i].b));
	}
	if (plsc->level > 0)
	    plP_state(PLSTATE_CMAP1);

	break;
    }
    }

    DEBUG_PRINT_LOCATION("end of plr_state");
}

/*--------------------------------------------------------------------------*\
 * plr_esc()
 *
 * Handle all escape functions.
\*--------------------------------------------------------------------------*/

static void
plr_esc(U_CHAR c)
{
    U_CHAR op;

    dbug_enter("plr_esc");

    plm_rd( pdf_rd_1byte(pdfs, &op) );

    switch (op) {

    case PLESC_FILL:
	plresc_fill();
	break;

/* These are all commands that should be absent from current metafiles but */
/* are recognized here for backward compatibility with old metafiles */

    case PLESC_SET_RGB:
	plresc_rgb();
	return;

    case PLESC_ALLOC_NCOL:
	plresc_ancol();
	return;

    case PLESC_SET_LPB:
	plm_rd( pdf_rd_2bytes(pdfs, &dum_ushort) );
	plm_rd( pdf_rd_2bytes(pdfs, &dum_ushort) );
	plm_rd( pdf_rd_2bytes(pdfs, &dum_ushort) );
	plm_rd( pdf_rd_2bytes(pdfs, &dum_ushort) );
	return;
    }
}

/*--------------------------------------------------------------------------*\
 * plresc_fill()
 *
 * Fill polygon described in points pls->dev_x[] and pls->dev_y[].
\*--------------------------------------------------------------------------*/

static void
plresc_fill(void)
{
    dbug_enter("plresc_fill");

    plm_rd( pdf_rd_2bytes(pdfs, &npts) );
    get_ncoords(x, y, npts);
    plfill(npts, x, y);
}

/*--------------------------------------------------------------------------*\
 * plresc_rgb()
 *
 * Process escape function for RGB color selection.
 * Note that RGB color selection is no longer handled this way by
 * PLplot but we must handle it here for old metafiles.
\*--------------------------------------------------------------------------*/

static void
plresc_rgb(void)
{
    PLFLT red, green, blue;
    U_SHORT ired, igreen, iblue;

    dbug_enter("plresc_rgb");

    plm_rd( pdf_rd_2bytes(pdfs, &ired) );
    plm_rd( pdf_rd_2bytes(pdfs, &igreen) );
    plm_rd( pdf_rd_2bytes(pdfs, &iblue) );

    red = (double) ired / 65535.;
    green = (double) igreen / 65535.;
    blue = (double) iblue / 65535.;

    plrgb(red, green, blue);
}

/*--------------------------------------------------------------------------*\
 * plresc_ancol()
 *
 * Process escape function for named color table allocation.
 * OBSOLETE -- just read the info and move on.
\*--------------------------------------------------------------------------*/

static void
plresc_ancol(void)
{
    U_CHAR icolor;
    char name[80];

    dbug_enter("plresc_ancol");

    plm_rd( pdf_rd_1byte(pdfs, &icolor) );
    plm_rd( pdf_rd_header(pdfs, name) );
}

/*--------------------------------------------------------------------------*\
 * 			Support routines
\*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*\
 * NextFamilyFile()
 *
 * Start the next family if it exists.
\*--------------------------------------------------------------------------*/

static void
NextFamilyFile(U_CHAR *c)
{
    dbug_enter("NextFamilyFile");

    (void) fclose(MetaFile);
    member++;
    (void) sprintf(FileName, "%s.%i", BaseName, (int) member);

    if ((MetaFile = fopen(FileName, "rb")) == NULL) {
	is_family = 0;
	return;
    }
    if (ReadFileHeader()) {
	is_family = 0;
	return;
    }
    pdfs->file = MetaFile;

/*
 * If the family file was created correctly, the first instruction in the
 * file (after state information) MUST be an INITIALIZE.  We throw this
 * away; a BOP0 will follow immediately thereafter.
 */

    *c = getcommand();
    while (*c == CHANGE_STATE) {
	plr_state(getcommand());
	*c = getcommand();
    }

    if (*c != INITIALIZE)
	fprintf(stderr,
		"First instruction in member file not an INITIALIZE!\n");
    else {

    /* Update position offset */

	if (pl_fgetpos(MetaFile, &curpage_loc))
	    plr_exit("plrender: fgetpos call failed");

	*c = getcommand();
	if (!(*c == BOP0 || *c == BOP))
	    fprintf(stderr,
		    "First instruction after INITIALIZE not a BOP!\n");
    }

    pldebug("NextFamilyFile", "successfully opened %s\n", FileName);
    DEBUG_PRINT_LOCATION("end of NextFamilyFile");
}

/*--------------------------------------------------------------------------*\
 * PrevFamilyFile()
 *
 * Go back to the previous family file.
\*--------------------------------------------------------------------------*/

static void
PrevFamilyFile(void)
{
    dbug_enter("PrevFamilyFile");

    if (member <= 0)
	return;

    (void) fclose(MetaFile);
    member--;
    (void) sprintf(FileName, "%s.%i", BaseName, (int) member);

    if ((MetaFile = fopen(FileName, "rb")) == NULL) {
	is_family = 0;
	return;
    }
    if (ReadFileHeader()) {
	is_family = 0;
	return;
    }
    pdfs->file = MetaFile;

    pldebug("PrevFamilyFile", "successfully opened %s\n", FileName);
    DEBUG_PRINT_LOCATION("end of PrevFamilyFile");
}

/*--------------------------------------------------------------------------*\
 * getcommand()
 *
 * Read & return the next command
\*--------------------------------------------------------------------------*/

static U_CHAR
getcommand(void)
{
    int c;

    c = getc(MetaFile);
    if (c == EOF)
	plr_exit("getcommand: Unable to read from MetaFile");

    return (U_CHAR) c;
}

/*--------------------------------------------------------------------------*\
 * ungetcommand()
 *
 * Push back the last command read.
\*--------------------------------------------------------------------------*/

static void
ungetcommand(U_CHAR c)
{
    if (ungetc(c, MetaFile) == EOF)
	plr_exit("ungetcommand: Unable to push back character");
}

/*--------------------------------------------------------------------------*\
 * void plr_exit()
 *
 * In case of an abort this routine is called.  Unlike plexit(), it does
 * NOT turn off pause, so that if a problem occurs you can still see what
 * output remains on the screen before the program terminates.
\*--------------------------------------------------------------------------*/

static void
plr_exit(char *errormsg)
{
    int status = 1;

    plend();
    if (*errormsg != '\0') {
	fprintf(stderr, "\n*** PLRENDER ERROR ***\n");
	fprintf(stderr, "%s\n", errormsg);
    }

    fprintf(stderr, "Program aborted\n");

/* For really bad debugging cases, you may want to see what the next bit of
 * metafile looked like.
 */

#ifdef DEBUG
    {
	int i, imax=256, c_end;
	fprintf(stderr, "next %d chars in metafile are:\n", imax);
	for (i=1; i<imax; i++) {
	    c_end = getc(MetaFile);
	    if (c_end == EOF)
		break;
	    fprintf(stderr, " %d", c_end);
	}
	fprintf(stderr, "\n");
    }
#endif

    exit(status);
}

/*--------------------------------------------------------------------------*\
 * plr_KeyEH()
 *
 * Keyboard event handler.  For mapping keyboard sequences to commands
 * not usually supported by PLplot, such as seeking around in the
 * metafile.  Recognized commands:
 *
 * <Backspace>	|
 * <Delete>	| Back page
 * <Page up>	|
 *
 * +<num><CR>	Seek forward <num> pages.
 * -<num><CR>	Seek backward <num> pages.
 *
 * <num><CR>	Seek to page <num>.
 * --<num><CR>	Seek to <num> pages before EOF.
 *
 * Both <BS> and <DEL> are recognized for a back-page since the target
 * system may use either as its erase key.  <Page Up> is present on some
 * keyboards (different from keypad key).
 *
 * No user data is passed in this case, although a test case is
 * illustrated.
 *
 * Illegal input is ignored.
\*--------------------------------------------------------------------------*/

static void
plr_KeyEH(PLGraphicsIn *gin, void *user_data, int *p_exit_eventloop)
{
    char *tst = (char *) user_data;
    int input_num, dun_seek = 0, terminator_seen = 0;

    pldebug("plr_KeyEH", "gin->keysym = %x\n", gin->keysym);

/* TEST */

    if (tst != NULL) {
	pltext();
	fprintf(stderr, "tst string: %s\n", tst);
	plgra();
    }

/* The rest deals with seeking only; so we return if it is disabled */

    if (no_pagelinks || !isfile)
	return;

/* Forward (+) or backward (-) */

    if (gin->string[0] == '+')
	direction_flag++;

    else if (gin->string[0] == '-')
	direction_flag--;

/* If a number, store into num_buffer */

    if (isdigit(gin->string[0])) {
	isanum = TRUE;
	(void) strncat(num_buffer, gin->string, (20-strlen(num_buffer)));
    }

/*
 * Seek to specified page, or page advance.
 * Not done until user hits <return>.
 * Need to check for both <LF> and <CR> for portability.
 */
    if (gin->keysym == PLK_Return ||
	gin->keysym == PLK_Linefeed ||
	gin->keysym == PLK_Next)
    {
	terminator_seen = 1;
	if (isanum) {
	    input_num = atoi(num_buffer);
	    if (input_num == 0) {
		if (strcmp(num_buffer, "0"))
		    input_num = -1;
	    }
	    if (input_num >= 0) {
		if (direction_flag == 0)
		    target_disp = input_num;
		else if (direction_flag == 1)
		    target_disp = curdisp + input_num;
		else if (direction_flag == -1)
		    target_disp = curdisp - input_num;
		else if (direction_flag == -2)
		    target_disp = pages - input_num;

		SeekToDisp(target_disp);
		dun_seek = 1;
	    }
	}
	else {
	    target_disp = curdisp + 1;
	    SeekToDisp(target_disp);
	    dun_seek = 1;
	}
    }

/* Page backward */

    if (gin->keysym == PLK_BackSpace ||
	gin->keysym == PLK_Delete ||
	gin->keysym == PLK_Prior)
    {
	terminator_seen = 1;
	target_disp = curdisp - 1;
	SeekToDisp(target_disp);
	dun_seek = 1;
    }

/* Cleanup */

    if (terminator_seen) {
	num_buffer[0] = '\0';
	direction_flag = 0;
	isanum = 0;
	gin->keysym = 0;
    }
    if (dun_seek && at_eop)
	*p_exit_eventloop = TRUE;
}

/*--------------------------------------------------------------------------*\
 * SeekToDisp()
 *
 * Seek to 'target_disp' displayed page.
 *
 * Several things combine to make this much harder than one would think.
 * These include: taking packed pages into account, seeking from mid-plot,
 * and multiple contiguous seek events.  All in all a disgusting mess.
 *
 * When this routine is called we are probably at the end of a page, but
 * maybe not (e.g. if the user hit <Backspace> or <Return> in the middle
 * of the screen update, and the driver immediately processes the event).
 * Also we might be at the end of the file.  So only if we are right
 * before the desired displayed page AND the end of page condition is met,
 * we are done.
 *
 * When reference is made to a displayed (i.e. possibly packed) page, I
 * will use variables with the "disp" tag.  For metafile pages, I will
 * use "page".  I could have used the word "plot" somewhere but I am
 * reserving that for future uses.
 *
 * To deal with multiple contiguous seek events (e.g. the user hits
 * <Backspace> twice before the renderer has a chance to respond) the
 * "seek_mode" variable was found to be necessary.
\*--------------------------------------------------------------------------*/

static void
SeekToDisp(long target_disp)
{
    dbug_enter("SeekToDisp");

/* Determine target_page */
/* Needs to be the last subpage on the preceding displayed page,  */
/* so we subtract 1 unless the last seek is still active */

    if ( ! seek_mode)
	--target_disp;

    target_page = target_disp * nsubx * nsuby;
    delta = target_page - curpage;
    seek_mode = 1;

/* Handle special cases first -- none of these require seeks. */
/* An example of how each condition might arise is given. */

/* <Return> at the end of a page */

    if ((delta == 0) && at_eop)
	return;

/* <Return> while drawing the last page */

    if ((delta >= 0) && (nextpage_loc == 0))
	return;

/* Anything else requires at least one seek */

    pldebug("SeekToDisp",
	    "Before seek: target_page = %d, curpage = %d, curdisp = %d\n",
	    target_page, curpage, curdisp);

/* <Return> while drawing any page but the last */

    if (delta == 0) {
	SeekToNextPage();
	goto done;
    }

/* Prepare for backward seeks by seeking to the start of the current page */
/* If on the first page, we're done */

    if (delta < 0) {
	SeekToCurPage();
	if (prevpage_loc == 0)
	    goto done;
    }

/* Now seek by pages until we arrive at the target page */

    while (delta != 0)
	SeekOnePage();

 done:
    pldebug("SeekToDisp",
	    "After seek: curpage = %d, curdisp = %d, cursub = %d\n",
	    curpage, curdisp, cursub);

    end_of_page = 1;
    return;
}

/*--------------------------------------------------------------------------*\
 * SeekOnePage()
 *
 * Seeks one page in appropriate direction, and updates delta.
 * For out of bounds seeks, just stay on the boundary page (first or last).
\*--------------------------------------------------------------------------*/

static void
SeekOnePage(void)
{
    if (delta > 0) {
	if (nextpage_loc == 0) {
	    SeekToCurPage();
	    delta = 0;
	    return;
	}
	SeekToNextPage();
    }
    else {
	if (prevpage_loc == 0) {
	    SeekToCurPage();
	    delta = 0;
	    return;
	}
	SeekToPrevPage();
    }

    delta = target_page - curpage;
    pldebug("SeekOnePage", "Now at page %d, disp %d, delta %d\n",
	    curpage, curdisp, delta);

    return;
}

/*--------------------------------------------------------------------------*\
 * SeekToCurPage()
 *
 * Seeks to beginning of current page, changing the page counters if
 * we pass a page boundary in the process.  Are you sufficiently sick
 * yet?  I know I am.
\*--------------------------------------------------------------------------*/

static void
SeekToCurPage(void)
{
    FPOS_T loc;

    if (pl_fgetpos(MetaFile, &loc))
	plr_exit("plrender: fgetpos call failed");

    if (loc != curpage_loc)
	PageDecr();

    SeekTo(curpage_loc);
}

/*--------------------------------------------------------------------------*\
 * SeekToNextPage()
 *
 * Seeks to beginning of next page, changing the page counters if
 * we pass a page boundary in the process.
\*--------------------------------------------------------------------------*/

static void
SeekToNextPage(void)
{
    FPOS_T loc;

    if (pl_fgetpos(MetaFile, &loc))
	plr_exit("plrender: fgetpos call failed");

    if (loc == curpage_loc)
	PageIncr();

    SeekTo(nextpage_loc);
}

/*--------------------------------------------------------------------------*\
 * SeekToPrevPage()
 *
 * Seeks to beginning of previous page, changing page counters to
 * take into account each page header we skip over.  Getting sicker
 * all the time..
\*--------------------------------------------------------------------------*/

static void
SeekToPrevPage(void)
{
    FPOS_T loc;

    dbug_enter("SeekToPrevPage");

    if (pl_fgetpos(MetaFile, &loc))
	plr_exit("plrender: fgetpos call failed");

    if (loc != curpage_loc)
	PageDecr();

    if (is_family && first_page)
	PrevFamilyFile();

    SeekTo(prevpage_loc);
    PageDecr();
}

/*--------------------------------------------------------------------------*\
 * SeekTo()
 *
 * Seeks to specified location, updating page links.
\*--------------------------------------------------------------------------*/

static void
SeekTo(FPOS_T loc)
{
    pldebug("SeekTo", "Seeking to: %d\n", loc);
    doseek(loc);

/* Update page links */

    ReadPageHeader();
    doseek(curpage_loc);
    PageDecr();
}

/*--------------------------------------------------------------------------*\
 * Utility functions:
 *
 * doseek()	Seeks to the specified location in the file.
 * PageIncr()	Increments page counters
 * PageDecr()	Decrements page counters
\*--------------------------------------------------------------------------*/

static void
doseek(FPOS_T loc)
{
    if (pl_fsetpos(MetaFile, &loc))
	plr_exit("plrender: fsetpos call failed");
}

static void
PageDecr(void)
{
    curpage--;
    cursub--;
    if (cursub < 1) {
	cursub = nsubx * nsuby;
	curdisp--;
    }
}

static void
PageIncr(void)
{
    cursub++;
    curpage++;
    if (cursub > nsubx * nsuby) {
	cursub = 1;
	curdisp++;
    }
}

/* Yes, finally done with the seek routines.
	"BELIEVE IT!" - John Walker, from "Buckaroo Bonzai" */

/*--------------------------------------------------------------------------*\
 * ReadPageHeader()
 *
 * Reads the metafile, processing page header info.
 * Assumes the file pointer is positioned immediately before a BOP.
\*--------------------------------------------------------------------------*/

static void
ReadPageHeader(void)
{
    U_CHAR c;
    U_SHORT page;
    U_LONG prevpage, nextpage;

    dbug_enter("ReadPageHeader");

    DEBUG_PRINT_LOCATION("beginning of ReadPageHeader");

/* Read page header */

    if (isfile) {
	if (pl_fgetpos(MetaFile, &curpage_loc))
	    plr_exit("plrender: fgetpos call failed");
    }

    c = getcommand();
    if (c == CLOSE && is_family)
	NextFamilyFile(&c);

    if (!(c == BOP0 || c == BOP || c == ADVANCE)) {
	sprintf(buffer, "plrender: page advance expected; found command code %d\n \
file: %s, position: %d", c, FileName, (int) curpage_loc);
	plr_exit(buffer);
    }

    first_page = (c == BOP0);

/* Update page/subpage counters and update page links */

    PageIncr();

    if (strcmp(mf_version, "1992a") >= 0) {
	if (strcmp(mf_version, "1993a") >= 0) {
	    plm_rd( pdf_rd_2bytes(pdfs, &page) );
	    plm_rd( pdf_rd_4bytes(pdfs, &prevpage) );
	    plm_rd( pdf_rd_4bytes(pdfs, &nextpage) );
	    prevpage_loc = prevpage;
	    nextpage_loc = nextpage;
	    pldebug("ReadPageHeader",
		    "page: %d, prev page offset: %d, next page offset: %d\n",
		    (int) page, (int) prevpage, (int) nextpage);
	}
	else {
	    plm_rd( pdf_rd_2bytes(pdfs, &dum_ushort) );
	    plm_rd( pdf_rd_2bytes(pdfs, &dum_ushort) );
	}
    }
    pldebug("ReadPageHeader", "Now at page %d, disp %d\n", curpage, curdisp);
    DEBUG_PRINT_LOCATION("end of ReadPageHeader");
}

/*--------------------------------------------------------------------------*\
 * ReadFileHeader()
 *
 * Checks file header.  Returns 1 if an error occured.
\*--------------------------------------------------------------------------*/

static int
ReadFileHeader(void)
{
    char tag[80];

    dbug_enter("ReadFileHeader");

/* Read label field of header to make sure file is a PLplot metafile */

    plm_rd( pdf_rd_header(pdfs, mf_magic) );
    if (strcmp(mf_magic, PLMETA_HEADER)) {
	fprintf(stderr, "Not a PLplot metafile!\n");
	return 1;
    }

/* Read version field of header.  We need to check that we can read the
   metafile, in case this is an old version of plrender. */

    plm_rd( pdf_rd_header(pdfs, mf_version) );
    if (strcmp(mf_version, PLMETA_VERSION) > 0) {
	fprintf(stderr,
	    "Error: incapable of reading metafile version %s.\n", mf_version);
	fprintf(stderr, "Please obtain a newer copy of plrender.\n");
	return 1;
    }
    pldebug( "ReadFileHeader", "Metafile version %s\n", mf_version );

/* Disable page seeking on versions without page links */

    if (strcmp(mf_version, "1993a") < 0)
	no_pagelinks=1;

/* Return if metafile older than version 1992a (no tagged info). */

    if (strcmp(mf_version, "1992a") < 0) {
	return 0;
    }

/* Read tagged initialization info. */
/* This is an easy way to guarantee backward compatibility. */

    for (;;) {
	plm_rd( pdf_rd_header(pdfs, tag) );
	if (*tag == '\0')
	    break;

	pldebug("ReadFileHeader",
		"Read tag: %s\n", tag);

	if ( ! strcmp(tag, "pages")) {
	    plm_rd( pdf_rd_2bytes(pdfs, &dum_ushort) );
	    pages = dum_ushort;
	    continue;
	}

	if ( ! strcmp(tag, "xmin")) {
	    plm_rd( pdf_rd_2bytes(pdfs, &dum_ushort) );
	    xmin = dum_ushort;
	    continue;
	}

	if ( ! strcmp(tag, "xmax")) {
	    plm_rd( pdf_rd_2bytes(pdfs, &dum_ushort) );
	    xmax = dum_ushort;
	    continue;
	}

	if ( ! strcmp(tag, "ymin")) {
	    plm_rd( pdf_rd_2bytes(pdfs, &dum_ushort) );
	    ymin = dum_ushort;
	    continue;
	}

	if ( ! strcmp(tag, "ymax")) {
	    plm_rd( pdf_rd_2bytes(pdfs, &dum_ushort) );
	    ymax = dum_ushort;
	    continue;
	}

	if ( ! strcmp(tag, "pxlx")) {
	    plm_rd( pdf_rd_ieeef(pdfs, &pxlx) );
	    continue;
	}

	if ( ! strcmp(tag, "pxly")) {
	    plm_rd( pdf_rd_ieeef(pdfs, &pxly) );
	    continue;
	}

	if ( ! strcmp(tag, "width")) {
	    plm_rd( pdf_rd_1byte(pdfs, &dum_uchar) );
	    plwid(dum_uchar);
	    continue;
	}

    /* Geometry info */

	if ( ! strcmp(tag, "xdpi")) {
	    plm_rd( pdf_rd_ieeef(pdfs, &xdpi) );
	    continue;
	}

	if ( ! strcmp(tag, "ydpi")) {
	    plm_rd( pdf_rd_ieeef(pdfs, &ydpi) );
	    continue;
	}

	if ( ! strcmp(tag, "xlength")) {
	    plm_rd( pdf_rd_2bytes(pdfs, &dum_ushort) );
	    xlength = dum_ushort;
	    continue;
	}

	if ( ! strcmp(tag, "ylength")) {
	    plm_rd( pdf_rd_2bytes(pdfs, &dum_ushort) );
	    ylength = dum_ushort;
	    continue;
	}

	if ( ! strcmp(tag, "xoffset")) {
	    plm_rd( pdf_rd_2bytes(pdfs, &dum_ushort) );
	    xoffset = dum_ushort;
	    continue;
	}

	if ( ! strcmp(tag, "yoffset")) {
	    plm_rd( pdf_rd_2bytes(pdfs, &dum_ushort) );
	    yoffset = dum_ushort;
	    continue;
	}

    /* Obsolete tags */

	if ( ! strcmp(tag, "orient")) {
	    plm_rd( pdf_rd_1byte(pdfs, &dum_uchar) );
	    continue;
	}

	if ( ! strcmp(tag, "aspect")) {
	    plm_rd( pdf_rd_ieeef(pdfs, &dum_float) );
	    continue;
	}

	fprintf(stderr, "Unrecognized PLplot metafile header tag.\n");
	exit(EX_BADFILE);
    }

    DEBUG_PRINT_LOCATION("end of ReadFileHeader");
    return 0;
}

/*--------------------------------------------------------------------------*\
 * Input handlers
\*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*\
 * Opt_v()
 *
 * Performs appropriate action for option "v".
 *
 * Note: the return code of 1 and the PL_OPT_NODELETE option tag ensures
 * that processing continues after Opt_v() returns, to pick up the internal
 * -v handling.
\*--------------------------------------------------------------------------*/

static int
Opt_v(char *opt, char *optarg, void *client_data)
{
/* Version */

    fprintf(stderr, "PLplot metafile version: %s\n", PLMETA_VERSION);
    return 1;
}

/*--------------------------------------------------------------------------*\
 * Opt_i()
 *
 * Performs appropriate action for option "i".
\*--------------------------------------------------------------------------*/

static int
Opt_i(char *opt, char *optarg, void *client_data)
{
/* Input file */

    strncpy(FileName, optarg, sizeof(FileName) - 1);
    FileName[sizeof(FileName) - 1] = '\0';
    do_file_loop = 0;

    return 0;
}

/*--------------------------------------------------------------------------*\
 * Opt_b()
 *
 * Performs appropriate action for option "b".
\*--------------------------------------------------------------------------*/

static int
Opt_b(char *opt, char *optarg, void *client_data)
{
/* Beginning page */

    if (*optarg == '-') {
	optarg++;
	addeof_beg = 1;
    }
    disp_beg = atoi(optarg);

    return 0;
}

/*--------------------------------------------------------------------------*\
 * Opt_e()
 *
 * Performs appropriate action for option "e".
\*--------------------------------------------------------------------------*/

static int
Opt_e(char *opt, char *optarg, void *client_data)
{
/* Ending page */

    if (*optarg == '-') {
	optarg++;
	addeof_end = 1;
    }
    disp_end = atoi(optarg);

    return 0;
}

/*--------------------------------------------------------------------------*\
 * Opt_p()
 *
 * Performs appropriate action for option "p".
\*--------------------------------------------------------------------------*/

static int
Opt_p(char *opt, char *optarg, void *client_data)
{
/* Specified page only */

    if (*optarg == '-') {
	optarg++;
	addeof_beg = 1;
	addeof_end = 1;
    }
    disp_beg = atoi(optarg);
    disp_end = disp_beg;

    return 0;
}
