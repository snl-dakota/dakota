/* $Id: dosbgi.c 3186 2006-02-15 18:17:33Z slbrow $
   $Log$
   Revision 1.1  2006/02/15 18:14:49  slbrow
   This is the commit of the DAKOTA 'autoconfiscation' I've been working on, which
   enables builds using the GNU Autotools.  I have been merging in changes to the
   mainline the entire time I've been working on these changes to my working copy
   (in ~slbrow/projects/Dakota), so none of your changes to DAKOTA will be lost.
   In addition, the final pre-autotools version of DAKOTA has been tagged with:

   Version-3-3-Plus-pre-autotools

   The GNU autotools will be needed by all developers who check DAKOTA out from
   CVS with intentions to build; this is because the version of DAKOTA in the
   repository after this commit does not have any configure or Makefile.in files
   (since they are now machine-generated).  All the source tarballs rolled for
   external consumption (both release and votd) will automatically include all the
   configure and Makefile.in files our non-autotools savvy users will need, while
   still omitting those proprietary sources we may not distribute (DOT, NPSOL).

   To re-generate the missing configure and Makefile.in files in a checkout of
   DAKOTA, only one GNU Autotools script is necessary to run from the
   command-line:

   $ autoreconf --install

   This script (and all the scripts it runs in sequence on a copy of DAKOTA
   checked out of the repository) can be found on the SCICO LAN in the directory:

   /usr/netpub/autotools/bin

   This directory exists on all five of the DAKOTA nightly build platforms (AIX,
   IRIX, Linux, Solaris and Tru64), and should be added to the beginning of your
   $PATH if you are on the SCICO LAN.  If you are not, the versions of the tools
   you need can be easily downloaded from the GNU website.  The versions we use:

   autoconf 2.59  (http://ftp.gnu.org/gnu/autoconf/autoconf-2.59.tar.gz)
   automake 1.9.6 (http://ftp.gnu.org/gnu/automake/automake-1.9.6.tar.gz)
   libtool 1.5.22 (http://ftp.gnu.org/gnu/libtool/libtool-1.5.22.tar.gz)
   m4 1.4.3       (http://ftp.gnu.org/gnu/m4/m4-1.4.3.tar.gz)

   Here is a "short" summary of the major changes made in this )commit:

   1. All configure.in and Makefile.in files are no longer in the CVS repository.
      They are replaced by configure.ac and Makefile.am files, respectively.  When
      you run autoreconf, the configure and Makefile.in files are regenerated
      automatically and recursively.  No $DAKOTA environment variable needs to be
      set in order to build DAKOTA.

   2. Build directories as they were in DAKOTA using the Cygnus configure system
      no longer exist.  Executables built by the GNU Autotools (e.g., the dakota*
      binaries which were built in Dakota/src/i686-unknown-linux on Linux) are now
      built in Dakota/src; the autotools provide deployment targets for installing
      binaries and libraries in a deployment directory, as well as a simple means
      of maintaining more than one build of Dakota for multiple platforms (i.e.,
      using VPATH builds and source- and build-tree separation).

      Suppose you have in your home directory a checked-out and 'autoreconf'd copy
      of Dakota, and you want to simultaneously build versions of DAKOTA for Linux
      and Solaris.  You would create two uniquely named directories in your home
      directory, called 'my_linux_build' and 'my_solaris_build', respectively.
      Assuming two terminals are open on both machines, you would 'cd' into the
      appropriate build directory on each platform and run this command on both:

      $ ../Dakota/configure -C

      This will create a directory tree which is identical to the source tree, but
      containing only Makefiles.  Then, when you run 'make', it will traverse the
      build tree, referencing the sources by relative paths to create objects and
      binaries.  At your option, you can then delete these build directories
      without touching your source tree (the 'autoreconf'd checkout of Dakota).  I
      should note here that the GNU Autotools do not _force_ this separation of
      trees; you can do a 'make distclean' after building for each platform in
      your 'autoreconf'd copy of Dakota with no problems.

   3. All the configure flags which were supported in the old build system (i.e.,
      --without-acro, --enable-debugging, etc.), are supported in the new system
      with the following changes and additions:

      --with-mpi/--without-mpi           -> --enable-mpi/--disable-mpi
      --with-graphics/--without-graphics -> --enable-graphics/--disable-graphics
      --enable-docs (default is off)

      I should note that all the default settings have been preserved.  Further,
      since ACRO, OPT++ and DDACE haven't been integrated yet (see item 9), their
      flags are still commented out in 'Dakota/config/dak_vendoroptimizers.m4'.

   4. The epetra and plplot directories under VendorPackages have been completely
      replaced with the versions available online (versions 3.3 and 5.5.2,
      respectively), which were originally flattened to provide support for our
      build system.  Epetra has been altered from the version distributed by the
      Trilinos team to provide full libtool support as that is necessary for
      building mixed-language archives under Solaris, while plplot has had all
      language-bindings except C++ turned off by default.  Additionally, GSL has
      been updated from version 1.7 available online, but preserving the flattened
      structure it had prior to autoconfiscation.

   5. All other packages in the DAKOTA repository under VendorPackages and
      VendorOptimizers have been 'autoconfiscated' separately from DAKOTA.  To
      avoid duplicated configure-time probes, run configure from the top-level
      DAKOTA directory with the '-C' argument; this will cache the probe results
      from each subpackage, removing the need to do anything other than a lookup
      the next time the probe runs.

   6. No 'mpi' symbolic link needs to be made from VendorPackages/mpi to the MPI
      implementation you wish to build against; MPI wrapper scripts for the C++
      compiler are used instead; if you wish to change the default choices for the
      MPI implementation to build against, you can specify where the wrapper
      scripts you wish to use are via environment variables, which may be set
      on separate lines (using setenv in CSH), or with env on the configure line:

      $ env MPICC=mpicc MPICXX=mpiCC MPIF77=mpif77 ./configure -C

   7. All the clean-up make targets ('clean', 'distclean' and a new one for those
      unfamiliar with the GNU Autotools, 'maintainer-clean') are now machine-
      generated.  The targets remove increasing numbers of files as you would
      expect; the last one removes all the autotools-generated files, returning
      you to a pristine CVS checkout.

   8. Many changes have been made to the sources in Dakota/src:

      A) Fortran calls are now done using FC_FUNC and F77_FUNC macro wrappers,
         removing the need for #ifdef RS6K preprocessor checks for underscores.
      B) Platform testing preprocessor conditionals have been replaced* with the
         feature that was assumed to be supported on that platform 'a priori'
         * mostly: MPI implementation choice is still done by platform.
           See ParallelLibrary.C for details.
      C) TFLOPS and COUGAR #ifdef's have been removed, leaving their #else
         branches to execute unconditionally (as those machines have gone away).

   9. Since the GNU Autotools preferentially look for GCC compilers first; if you
      wish to build on Solaris with the Sun compilers you must set the compilers
      to use in environment variables in the same way the MPI wrappers must be set
      (see item 5 above):

       $ env CC=cc CXX=CC F77=f77 FC=f90 ./configure ...

   Additionally, there are some minor problems to fix with the new build system:

   10. This autoconfiscation covers Dakota, as integration with ACRO, OPT++ and
       DDACE has yet to be done.  I have integrated Surfpack and removed the
       separate ann and kriging packages from VendorPackages, based on positive
       results from some integration testing I've done on the Surfpack versions of
       ann and kriging against those in the separate versions in VendorPackages.

   11. Purely static-linked executables have proved difficult with libtool.  The
       way libtool sets things up, all the DAKOTA package libraries are static and
       linked as such, but system libraries are linked against dynamically (the
       way it is currently done on Solaris).  Strangely though, the MPI libraries
       aren't linked against dynamically, so this is an area for investigation
       (especially as I planned on having it working for the ASC White/Purple
       deployment of Dakota 4.0).

   12. Running 'make -j <n>' with positive integer n for parallel builds is not
       yet supported, as there are a couple packages with dependency requirements
       (LHS and IDR, namely) which preclude compiling in parallel until their
       requirements are met.  In LHS, those requirements arise from the need for
       all the *.mod module files to exist prior to anything which USE's them, and
       in IDR, they arise from the lex/yacc generated source dependencies.

   13. Disabling F90 support (via the --disable-f90 configure flag) doesn't work
       yet.  GNU Automake isn't quite smart enough yet to accept arbitrary
       suffixes for F90 files, so it expects that the suffixes will be .f90 or
       .f95, _not_ .F!  The Automake developers are working on this, so in the
       meantime, I'm using a hack.  I put that hack into Dakota/src/Makefile.am to
       address this situation (the same one I use in LHS, by the way, which sets
       F77 and FFLAGS to their F90-equivalents) doesn't work when F90 is disabled.

   14. The nightly build and test system (in Dakota/test/sqa) will need to be
       modified to use the new system, so the nightlies will be broken until I can
       get the scripts modified to use the new system.  That will be my next CVS
       checkin, and I don't expect it to take very long to make those mods.

   Revision 1.3  2000/12/18 21:01:50  airwin
   Change to new style plplot/*.h header file locations.

   Revision 1.2  1995/06/24 06:58:58  mjl
   Latest bcc distribution from Paul Casteels.

 * Revision 1.3  1994/08/10  01:16:36  mjl
 * Updates to the DOS BGI (BCC) driver, submitted by Paul Casteels.
 *
 * Revision 1.2  1994/05/25  22:08:19  mjl
 * Win3 driver added, other changes to bring DOS/BGI (bcc) support
 * up to date, contributed by Paul Casteels.
 *
 * Revision 1.8  1993/03/15  21:39:04  mjl
 * Changed all _clear/_page driver functions to the names _eop/_bop, to be
 * more representative of what's actually going on.
 *
 * Revision 1.7  1993/03/03  19:41:54  mjl
 * Changed PLSHORT -> short everywhere; now all device coordinates are expected
 * to fit into a 16 bit address space (reasonable, and good for performance).
 *
 * Revision 1.6  1993/02/27  04:46:31  mjl
 * Fixed errors in ordering of header file inclusion.  "plplot.h" should
 * always be included first.
 *
 * Revision 1.5  1993/02/22  23:10:50  mjl
 * Eliminated the gradv() driver calls, as these were made obsolete by
 * recent changes to plmeta and plrender.  Also eliminated page clear commands
 * from grtidy() -- plend now calls grclr() and grtidy() explicitly.
 *
 * Revision 1.4  1993/01/23  05:41:39  mjl
 * Changes to support new color model, polylines, and event handler support
 * (interactive devices only).
 *
 * Revision 1.3  1992/11/07  07:48:37  mjl
 * Fixed orientation operation in several files and standardized certain startup
 * operations. Fixed bugs in various drivers.
 *
 * Revision 1.2  1992/09/29  04:44:39  furnish
 * Massive clean up effort to remove support for garbage compilers (K&R).
 *
 * Revision 1.1  1992/05/20  21:32:32  furnish
 * Initial checkin of the whole PLPLOT project.
 *
*/

/*
  dosbgi.c

	Adapted from dosvga.c
  Paul Casteels
  25-Sep-1993

  A driver for Borland's BGI drivers.
  Compile this with Borland C 3.1

*/
#include "plplot/plDevs.h"
#ifdef PLD_bgi

#include "plplot/plplotP.h"
#include <stdio.h>
#include <stdlib.h>
#include "plplot/drivers.h"
#include <graphics.h>
#include <process.h>
#include <conio.h>

static void pause(PLStream *pls);

/* A flag to tell us whether we are in text or graphics mode */

#define TEXT_MODE 0
#define GRAPHICS_MODE 1

static int mode = TEXT_MODE;

#define CLEAN 0
#define DIRTY 1

static page_state;

static PLDev device;
static PLDev *dev = &device;

int gdriver,gmode,errorcode;


#ifdef TSENG4
#define Mode640x400     0
#define Mode640x480     1
#define Mode800x600     2
#define Mode1024x768    3

int Tseng4Mode(void)
{
//  return(Mode640x400);
//  return(Mode640x480);
	return(Mode800x600);
//  return(Mode1024x768);
}
#endif

/*----------------------------------------------------------------------*\
* bgi_graph()
*
* Switch to graphics mode.
\*----------------------------------------------------------------------*/

void
bgi_graph(PLStream *pls)
{
	char *bgidir;

	 if (pls->graphx == TEXT_MODE) {
	gdriver = DETECT;
	if ((bgidir = getenv("BGIDIR")) != NULL)
		initgraph(&gdriver,&gmode,bgidir);
	else
		initgraph(&gdriver,&gmode,"\\bc4\\bgi");
	errorcode = graphresult();
	if (errorcode != grOk) {
		 printf("Unable to set graphics mode.");
		 exit(0);
	}
	pls->graphx = GRAPHICS_MODE;
	page_state = CLEAN;
	 }
}

/*----------------------------------------------------------------------*\
* bgi_init()
*
* Initialize device.
\*----------------------------------------------------------------------*/

void
plD_init_vga(PLStream *pls)
{
	int driver;

	 pls->termin = 1;            /* is an interactive terminal */
	 pls->icol0 = 1;
	 pls->width = 1;
	 pls->bytecnt = 0;
	 pls->page = 0;
	 pls->graphx = TEXT_MODE;
	 pls->plbuf_write = 1;

	 if (!pls->colorset)
	pls->color = 1;

/* Set up device parameters */

#ifdef TSENG4
  driver =installuserdriver ("TSENG4", Tseng4Mode);
  errorcode = graphresult();

  errorcode = graphresult();
  if (errorcode != grOk) {
	 printf("Graphics error: %s\n", grapherrormsg(errorcode));
	 printf("Press any key to halt:");
	 getch();
	 exit(1); /* terminate with an error code */
  }
#endif

	 bgi_graph(pls);

	 dev->xold = UNDEFINED;
	 dev->yold = UNDEFINED;
	 dev->xmin = 0;
	 dev->xmax = getmaxx();
	 dev->ymin = 0;
	 dev->ymax = getmaxy();

	 plP_setpxl(2.5, 2.5);           /* Pixels/mm. */

	 plP_setphy((PLINT) 0, (PLINT) dev->xmax, (PLINT) 0, (PLINT) dev->ymax);

}

/*----------------------------------------------------------------------*\
* bgi_line()
*
* Draw a line in the current color from (x1,y1) to (x2,y2).
\*----------------------------------------------------------------------*/

void
plD_line_vga(PLStream *pls, short x1a, short y1a, short x2a, short y2a)
{
	 int x1 = x1a, y1 = y1a, x2 = x2a, y2 = y2a;
/*
	 if (pls->pscale)
	plSclPhy(pls, dev, &x1, &y1, &x2, &y2);
*/
	 y1 = dev->ymax - y1;
	 y2 = dev->ymax - y2;

	 moveto(x1, y1);
	 lineto(x2, y2);

	 page_state = DIRTY;
}

/*----------------------------------------------------------------------*\
* bgi_polyline()
*
* Draw a polyline in the current color.
\*----------------------------------------------------------------------*/

void
plD_polyline_vga(PLStream *pls, short *xa, short *ya, PLINT npts)
{
	 PLINT i;

	 for (i = 0; i < npts - 1; i++)
	plD_line_vga(pls, xa[i], ya[i], xa[i + 1], ya[i + 1]);
}

/*----------------------------------------------------------------------*\
* bgi_eop()
*
* End of page.
\*----------------------------------------------------------------------*/

void
plD_eop_vga(PLStream *pls)
{
	 if (page_state == DIRTY)
	pause(pls);
	 if (pls->graphx == GRAPHICS_MODE) {
		clearviewport();
	 }
//    _eopscreen(_GCLEARSCREEN);
	 page_state = CLEAN;
}

/*----------------------------------------------------------------------*\
* bgi_bop()
*
* Set up for the next page.
* Advance to next family file if necessary (file output).
\*----------------------------------------------------------------------*/

void
plD_bop_vga(PLStream *pls)
{
	 pls->page++;
	 plD_eop_vga(pls);
}

/*----------------------------------------------------------------------*\
* bgi_tidy()
*
* Close graphics file or otherwise clean up.
\*----------------------------------------------------------------------*/

void
plD_tidy_vga(PLStream *pls)
{
/*
	 bgi_text(pls);
*/
	 closegraph();
	 pls->page = 0;
	 pls->OutFile = NULL;
}

/*----------------------------------------------------------------------*\
* bgi_color()
*
* Set pen color.
\*----------------------------------------------------------------------*/

/*
enum COLORS {
		BLACK,
    YELLOW,
		MAGENTA,
    CYAN,
    RED,
    GREEN,
    BLUE,
		LIGHTGRAY,
    DARKGRAY,
    LIGHTBLUE,
    LIGHTGREEN,
		LIGHTCYAN,
		LIGHTRED,
    LIGHTMAGENTA,
    BROWN,
    WHITE
};
//#endif
*/

void
bgi_color(PLStream *pls)
{
  int i;
/*
	 static int cmap[16] = {
	WHITE, RED, YELLOW, GREEN,
	CYAN, WHITE, WHITE, LIGHTGRAY,
	WHITE, BLUE, GREEN, CYAN,
	RED, MAGENTA, YELLOW, WHITE
	 };
*/
	 if (pls->icol0 < 0 || pls->icol0 > 15)
	pls->icol0 = 15;

/*
  for (i=0;i<=15;i++)
	 setpalette(i,cmap[i]);
*/
//  setpalette(0,WHITE);
  setcolor(pls->icol0);

//    _remappalette((short) pls->icol0, cmap[pls->icol0]);
//    _setcolor((short) pls->icol0);

}

void plD_state_vga(PLStream *pls,PLINT op) {
//  PSDev *dev = (PSDev *) pls->dev;
/*
	static int cmap[16] = {
	WHITE, RED, YELLOW, GREEN,
	CYAN, WHITE, WHITE, LIGHTGRAY,
	WHITE, BLUE, GREEN, CYAN,
	RED, MAGENTA, YELLOW, WHITE
	 };
*/

	switch(op) {
	case PLSTATE_WIDTH:
	 break;
	case PLSTATE_COLOR0:
	 if (pls->color) {
		setcolor(pls->icol0);
	 }
	 break;
	}
}


/*----------------------------------------------------------------------*\
* bgi_text()
*
* Switch to text mode.
\*----------------------------------------------------------------------*/

void
bgi_text(PLStream *pls)
{
	 if (pls->graphx == GRAPHICS_MODE) {
	if (page_state == DIRTY)
		 pause(pls);
	restorecrtmode();
//      _setvideomode(_DEFAULTMODE);
	pls->graphx = TEXT_MODE;
	 }
}

void bgi_text2graph(PLStream *pls)
{
	if (pls->graphx == TEXT_MODE)
	{
	setgraphmode(getgraphmode());   
		pls->graphx = GRAPHICS_MODE;
		page_state = CLEAN;
	}
}


/*----------------------------------------------------------------------*\
* bgi_width()
*
* Set pen width.
\*----------------------------------------------------------------------*/

void
bgi_width(PLStream *pls)
{
}

/*----------------------------------------------------------------------*\
* bgi_esc()
*
* Escape function.
\*----------------------------------------------------------------------*/

void
plD_esc_vga(PLStream *pls, PLINT op, void *ptr)
{
	switch (op) {
	case PLESC_TEXT:
		bgi_text(pls);
		break;

	case PLESC_GRAPH:
		bgi_text2graph(pls);
		break;
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
	char *pstr = "pause->";

  if (pls->graphx == TEXT_MODE) {
    gotoxy(0,0);
    cprintf(pstr);
  } else
    outtextxy(0,0,pstr);   // are we in graphics mode ??
    while (!kbhit());
    getch();
}

#else
int
pldummy_dosbgi()
{
    return 0;
}

#endif
