/* $Id: os2pm.c 3186 2006-02-15 18:17:33Z slbrow $
 * $Log$
 * Revision 1.1  2006/02/15 18:14:44  slbrow
 * This is the commit of the DAKOTA 'autoconfiscation' I've been working on, which
 * enables builds using the GNU Autotools.  I have been merging in changes to the
 * mainline the entire time I've been working on these changes to my working copy
 * (in ~slbrow/projects/Dakota), so none of your changes to DAKOTA will be lost.
 * In addition, the final pre-autotools version of DAKOTA has been tagged with:
 *
 * Version-3-3-Plus-pre-autotools
 *
 * The GNU autotools will be needed by all developers who check DAKOTA out from
 * CVS with intentions to build; this is because the version of DAKOTA in the
 * repository after this commit does not have any configure or Makefile.in files
 * (since they are now machine-generated).  All the source tarballs rolled for
 * external consumption (both release and votd) will automatically include all the
 * configure and Makefile.in files our non-autotools savvy users will need, while
 * still omitting those proprietary sources we may not distribute (DOT, NPSOL).
 *
 * To re-generate the missing configure and Makefile.in files in a checkout of
 * DAKOTA, only one GNU Autotools script is necessary to run from the
 * command-line:
 *
 * $ autoreconf --install
 *
 * This script (and all the scripts it runs in sequence on a copy of DAKOTA
 * checked out of the repository) can be found on the SCICO LAN in the directory:
 *
 * /usr/netpub/autotools/bin
 *
 * This directory exists on all five of the DAKOTA nightly build platforms (AIX,
 * IRIX, Linux, Solaris and Tru64), and should be added to the beginning of your
 * $PATH if you are on the SCICO LAN.  If you are not, the versions of the tools
 * you need can be easily downloaded from the GNU website.  The versions we use:
 *
 * autoconf 2.59  (http://ftp.gnu.org/gnu/autoconf/autoconf-2.59.tar.gz)
 * automake 1.9.6 (http://ftp.gnu.org/gnu/automake/automake-1.9.6.tar.gz)
 * libtool 1.5.22 (http://ftp.gnu.org/gnu/libtool/libtool-1.5.22.tar.gz)
 * m4 1.4.3       (http://ftp.gnu.org/gnu/m4/m4-1.4.3.tar.gz)
 *
 * Here is a "short" summary of the major changes made in this )commit:
 *
 * 1. All configure.in and Makefile.in files are no longer in the CVS repository.
 *    They are replaced by configure.ac and Makefile.am files, respectively.  When
 *    you run autoreconf, the configure and Makefile.in files are regenerated
 *    automatically and recursively.  No $DAKOTA environment variable needs to be
 *    set in order to build DAKOTA.
 *
 * 2. Build directories as they were in DAKOTA using the Cygnus configure system
 *    no longer exist.  Executables built by the GNU Autotools (e.g., the dakota*
 *    binaries which were built in Dakota/src/i686-unknown-linux on Linux) are now
 *    built in Dakota/src; the autotools provide deployment targets for installing
 *    binaries and libraries in a deployment directory, as well as a simple means
 *    of maintaining more than one build of Dakota for multiple platforms (i.e.,
 *    using VPATH builds and source- and build-tree separation).
 *
 *    Suppose you have in your home directory a checked-out and 'autoreconf'd copy
 *    of Dakota, and you want to simultaneously build versions of DAKOTA for Linux
 *    and Solaris.  You would create two uniquely named directories in your home
 *    directory, called 'my_linux_build' and 'my_solaris_build', respectively.
 *    Assuming two terminals are open on both machines, you would 'cd' into the
 *    appropriate build directory on each platform and run this command on both:
 *
 *    $ ../Dakota/configure -C
 *
 *    This will create a directory tree which is identical to the source tree, but
 *    containing only Makefiles.  Then, when you run 'make', it will traverse the
 *    build tree, referencing the sources by relative paths to create objects and
 *    binaries.  At your option, you can then delete these build directories
 *    without touching your source tree (the 'autoreconf'd checkout of Dakota).  I
 *    should note here that the GNU Autotools do not _force_ this separation of
 *    trees; you can do a 'make distclean' after building for each platform in
 *    your 'autoreconf'd copy of Dakota with no problems.
 *
 * 3. All the configure flags which were supported in the old build system (i.e.,
 *    --without-acro, --enable-debugging, etc.), are supported in the new system
 *    with the following changes and additions:
 *
 *    --with-mpi/--without-mpi           -> --enable-mpi/--disable-mpi
 *    --with-graphics/--without-graphics -> --enable-graphics/--disable-graphics
 *    --enable-docs (default is off)
 *
 *    I should note that all the default settings have been preserved.  Further,
 *    since ACRO, OPT++ and DDACE haven't been integrated yet (see item 9), their
 *    flags are still commented out in 'Dakota/config/dak_vendoroptimizers.m4'.
 *
 * 4. The epetra and plplot directories under VendorPackages have been completely
 *    replaced with the versions available online (versions 3.3 and 5.5.2,
 *    respectively), which were originally flattened to provide support for our
 *    build system.  Epetra has been altered from the version distributed by the
 *    Trilinos team to provide full libtool support as that is necessary for
 *    building mixed-language archives under Solaris, while plplot has had all
 *    language-bindings except C++ turned off by default.  Additionally, GSL has
 *    been updated from version 1.7 available online, but preserving the flattened
 *    structure it had prior to autoconfiscation.
 *
 * 5. All other packages in the DAKOTA repository under VendorPackages and
 *    VendorOptimizers have been 'autoconfiscated' separately from DAKOTA.  To
 *    avoid duplicated configure-time probes, run configure from the top-level
 *    DAKOTA directory with the '-C' argument; this will cache the probe results
 *    from each subpackage, removing the need to do anything other than a lookup
 *    the next time the probe runs.
 *
 * 6. No 'mpi' symbolic link needs to be made from VendorPackages/mpi to the MPI
 *    implementation you wish to build against; MPI wrapper scripts for the C++
 *    compiler are used instead; if you wish to change the default choices for the
 *    MPI implementation to build against, you can specify where the wrapper
 *    scripts you wish to use are via environment variables, which may be set
 *    on separate lines (using setenv in CSH), or with env on the configure line:
 *
 *    $ env MPICC=mpicc MPICXX=mpiCC MPIF77=mpif77 ./configure -C
 *
 * 7. All the clean-up make targets ('clean', 'distclean' and a new one for those
 *    unfamiliar with the GNU Autotools, 'maintainer-clean') are now machine-
 *    generated.  The targets remove increasing numbers of files as you would
 *    expect; the last one removes all the autotools-generated files, returning
 *    you to a pristine CVS checkout.
 *
 * 8. Many changes have been made to the sources in Dakota/src:
 *
 *    A) Fortran calls are now done using FC_FUNC and F77_FUNC macro wrappers,
 *       removing the need for #ifdef RS6K preprocessor checks for underscores.
 *    B) Platform testing preprocessor conditionals have been replaced* with the
 *       feature that was assumed to be supported on that platform 'a priori'
 *       * mostly: MPI implementation choice is still done by platform.
 *         See ParallelLibrary.C for details.
 *    C) TFLOPS and COUGAR #ifdef's have been removed, leaving their #else
 *       branches to execute unconditionally (as those machines have gone away).
 *
 * 9. Since the GNU Autotools preferentially look for GCC compilers first; if you
 *    wish to build on Solaris with the Sun compilers you must set the compilers
 *    to use in environment variables in the same way the MPI wrappers must be set
 *    (see item 5 above):
 *
 *     $ env CC=cc CXX=CC F77=f77 FC=f90 ./configure ...
 *
 * Additionally, there are some minor problems to fix with the new build system:
 *
 * 10. This autoconfiscation covers Dakota, as integration with ACRO, OPT++ and
 *     DDACE has yet to be done.  I have integrated Surfpack and removed the
 *     separate ann and kriging packages from VendorPackages, based on positive
 *     results from some integration testing I've done on the Surfpack versions of
 *     ann and kriging against those in the separate versions in VendorPackages.
 *
 * 11. Purely static-linked executables have proved difficult with libtool.  The
 *     way libtool sets things up, all the DAKOTA package libraries are static and
 *     linked as such, but system libraries are linked against dynamically (the
 *     way it is currently done on Solaris).  Strangely though, the MPI libraries
 *     aren't linked against dynamically, so this is an area for investigation
 *     (especially as I planned on having it working for the ASC White/Purple
 *     deployment of Dakota 4.0).
 *
 * 12. Running 'make -j <n>' with positive integer n for parallel builds is not
 *     yet supported, as there are a couple packages with dependency requirements
 *     (LHS and IDR, namely) which preclude compiling in parallel until their
 *     requirements are met.  In LHS, those requirements arise from the need for
 *     all the *.mod module files to exist prior to anything which USE's them, and
 *     in IDR, they arise from the lex/yacc generated source dependencies.
 *
 * 13. Disabling F90 support (via the --disable-f90 configure flag) doesn't work
 *     yet.  GNU Automake isn't quite smart enough yet to accept arbitrary
 *     suffixes for F90 files, so it expects that the suffixes will be .f90 or
 *     .f95, _not_ .F!  The Automake developers are working on this, so in the
 *     meantime, I'm using a hack.  I put that hack into Dakota/src/Makefile.am to
 *     address this situation (the same one I use in LHS, by the way, which sets
 *     F77 and FFLAGS to their F90-equivalents) doesn't work when F90 is disabled.
 *
 * 14. The nightly build and test system (in Dakota/test/sqa) will need to be
 *     modified to use the new system, so the nightlies will be broken until I can
 *     get the scripts modified to use the new system.  That will be my next CVS
 *     checkin, and I don't expect it to take very long to make those mods.
 *
 * Revision 1.3  2000/12/18 21:01:50  airwin
 * Change to new style plplot/*.h header file locations.
 *
 * Revision 1.2  1996/10/22 18:21:57  furnish
 * Update the OS/2 driver with fixes supplied by Thorsten Feiweier.
 *
 * Revision 1.1  1994/05/25  09:27:01  mjl
 * Decided the os2/ subdir of sys/ was a better place for this.
 *
 * Revision 1.9  1994/03/23  06:34:33  mjl
 * All drivers: cleaned up by eliminating extraneous includes (stdio.h and
 * stdlib.h now included automatically by plplotP.h), extraneous clears
 * of pls->fileset, pls->page, and pls->OutFile = NULL (now handled in
 * driver interface or driver initialization as appropriate).  Special
 * handling for malloc includes eliminated (no longer needed) and malloc
 * prototypes fixed as necessary.
 *
 * Revision 1.8  1993/07/31  07:56:38  mjl
 * Several driver functions consolidated, for all drivers.  The width and color
 * commands are now part of a more general "state" command.  The text and
 * graph commands used for switching between modes is now handled by the
 * escape function (very few drivers require it).  The device-specific PLDev
 * structure is now malloc'ed for each driver that requires it, and freed when
 * the stream is terminated.
 *
 * Revision 1.7  1993/07/01  21:59:41  mjl
 * Changed all plplot source files to include plplotP.h (private) rather than
 * plplot.h.  Rationalized namespace -- all externally-visible plplot functions
 * now start with "pl"; device driver functions start with "plD_".
*/

/*
	os2pm.c

	Geoffrey Furnish
	9-22-91
	
	This driver sends plplot commands to the OS/2 PM PLPLOT Server.

	25 March 1992
	VERSION 1.0
*/

#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))

#define INCL_BASE
#include <os2.h>

#include "plplot/plplotP.h"
#include "plplot/drivers.h"
#include "plplot/pmdefs.h"

/* top level declarations */

static USHORT	rv;
static HFILE	hf;

static int i=0;
static PLINT buffer[ PIPE_BUFFER_SIZE ];
static PLINT cnt = 0;
static PLINT escape[2] = {0, 0};

static PLINT	xold = UNDEFINED;
static PLINT	yold = UNDEFINED;
	  
typedef	PLINT	COMMAND_ID;
typedef PLINT * CPARAMS;

static void	write_command( COMMAND_ID cid, CPARAMS p );

/*----------------------------------------------------------------------*\
*  os2_setup()
*
* Set up device.
\*----------------------------------------------------------------------*/

void os2_setup( PLStream *pls )
{
}

/*----------------------------------------------------------------------*\
*  os2_orient()
*
* Set up orientation.
\*----------------------------------------------------------------------*/

void os2_orient( PLStream *pls )
{
}

/*----------------------------------------------------------------------*\
*  plD_init_os2()
*
* Initialize device.
\*----------------------------------------------------------------------*/

void	plD_init_os2( PLStream *pls )
{
    ULONG	usAction;
    UCHAR	c = (UCHAR) INITIALIZE;

    pls->termin =- 0;		/* not an interactive terminal */
    pls->icol0 = 1;
    pls->width = 1;
    pls->bytecnt = 0;
    pls->page = 1;

    if (!pls->colorset)
        pls->color = 1;

    plP_setpxl( (PLFLT) PIXEL_RES_X, (PLFLT) PIXEL_RES_Y );
    plP_setphy( 0, PLMETA_X, 0, PLMETA_Y );

    rv = DosOpen( PIPE_NAME,		// name of the pipe.
		&hf,			// address of file handle.
		&usAction,		// action taken.
		0L,			// size of file.
		FILE_NORMAL,		// file attribute.
		FILE_OPEN,		// open the file.
		OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYNONE,
		0L );
    if (rv)
	plexit( "Unable to open connection to PM server.\n" );
	
    write_command( c, NULL );
}

/*----------------------------------------------------------------------*\
*  plD_line_os2()
*
* Draw a line in the current color from (x1,y1) to (x2,y2).
\*----------------------------------------------------------------------*/

void plD_line_os2( PLStream *pls, 
			short x1, short y1, short x2, short y2 )
{
	UCHAR c;
	PLINT	cp[4];

	if (	x1 < 0 || x1 > PLMETA_X ||
		x2 < 0 || x2 > PLMETA_X ||
		y1 < 0 || y1 > PLMETA_Y ||
		y2 < 0 || y2 > PLMETA_Y 	) {
		printf( "Something is out of bounds." );
	}

/* If continuation of previous line send the LINETO command, which uses 
   the previous (x,y) point as it's starting location.  This results in a
   storage reduction of not quite 50%, since the instruction length for
   a LINETO is 5/9 of that for the LINE command, and given that most 
   graphics applications use this command heavily.

   Still not quite as efficient as tektronix format since we also send the
   command each time (so shortest command is 25% larger), but a heck of
   a lot easier to implement than the tek method.
*/
	if(x1 == xold && y1 == yold) {

		c = (UCHAR) LINETO;
		cp[0]=x2;
		cp[1]=y2;
		write_command( c, cp );
	}
	else {
		c = (UCHAR) LINE;
		cp[0] = x1;
		cp[1] = y1;
		cp[2] = x2;
		cp[3] = y2;
		write_command( c, cp );
	}
	xold = x2;
	yold = y2;
}
  
/*----------------------------------------------------------------------*\
* plD_polyline_os2()
*
* Draw a polyline in the current color.
\*----------------------------------------------------------------------*/

void 
plD_polyline_os2 (PLStream *pls, short *xa, short *ya, PLINT npts)
{
    PLINT i;

    for (i=0; i<npts-1; i++) 
      plD_line_os2( pls, xa[i], ya[i], xa[i+1], ya[i+1] );
}

/*----------------------------------------------------------------------*\
*  plD_eop_os2()
*
*  Clear page.
\*----------------------------------------------------------------------*/

void	plD_eop_os2( PLStream *pls )
{
	UCHAR c = (UCHAR) CLEAR;

	write_command( c, NULL );
}

/*----------------------------------------------------------------------*\
*  plD_bop_os2()
*
*  Advance to next page.
\*----------------------------------------------------------------------*/

void	plD_bop_os2( PLStream *pls )
{
	UCHAR c = (UCHAR) PAGE;

	xold = UNDEFINED;
	yold = UNDEFINED;

	write_command( c, NULL );
}

/*----------------------------------------------------------------------*\
*  plD_tidy_os2()
*
*  Close graphics file
\*----------------------------------------------------------------------*/

void	plD_tidy_os2( PLStream *pls )
{
	UCHAR c = (UCHAR) CLOSE;
	
	write_command( c, NULL );
		 
	DosClose( hf );
}

/*----------------------------------------------------------------------*\
* plD_state_os2()
*
* Handle change in PLStream state (color, pen width, fill attribute, etc).
\*----------------------------------------------------------------------*/

void 
plD_state_os2(PLStream *pls, PLINT op)
{
  UCHAR c;
  /* write_command( CHANGE_STATE, NULL ); */



    switch (op) {

    case PLSTATE_WIDTH:
      c = (UCHAR) NEW_WIDTH;
      escape[0] = (PLINT) pls->width;
      // write_command( c, NULL);
      // write_command( (UCHAR) op, (CPARAMS) a );
      write_command ( c, NULL);
      break;

    case PLSTATE_COLOR0:
      c = (UCHAR) NEW_COLOR;
      escape[0] = (PLINT) pls->icol0;
      // write_command( c, NULL);
      // write_command( (UCHAR) op, (CPARAMS) a );
      write_command ( c, NULL);
      break;

    case PLSTATE_COLOR1:
      break;
    }
}

/*----------------------------------------------------------------------*\
*  plD_esc_os2()
*
*  Escape function.  Note that any data written must be in device
*  independent form to maintain the transportability of the metafile.
\*----------------------------------------------------------------------*/

void	plD_esc_os2( PLStream *pls, PLINT op, void *ptr )
{
	UCHAR c = (UCHAR) ESCAPE;
	float *color;
	unsigned long ired, igreen, iblue;
	unsigned long	pmrgb;

	// write_command( c, NULL );

	switch (op) {
	    case PLESC_SET_RGB: 
		// c = (UCHAR) ESC_RGB;
		color = (float *) ptr; //&ptr[0];
		ired =	min(256,max(0,(int)255.*color[0]));
		igreen= min(256,max(0,(int)255.*color[1]));
		iblue = min(256,max(0,(int)255.*color[2]));
		pmrgb	= (ired   & 0xff) << 16 |
			  (igreen & 0xff) <<  8 |
			  (iblue  & 0xff);
		escape[0] = (PLINT) ESC_RGB;
		escape[1] = (PLINT) pmrgb;
		write_command( c, NULL );
		//printf( "Changing to RGB value %lx \n", pmrgb );
		break;
	
	    default:
		// c = (UCHAR) ESC_NOOP;
		escape[0] = (PLINT) ESC_NOOP;
		write_command( c, NULL );
		break;
	}
}

/*----------------------------------------------------------------------*\
* Support routines
\*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*\
*	write_command()
*	
*	Function to write a command to the command pipe, and to flush
*	the pipe when full.
\*----------------------------------------------------------------------*/

void	write_command( COMMAND_ID cid,	CPARAMS p )
{
    i++;

    buffer[cnt++] = cid;
    switch( cid ) {
	case LINE:
	    buffer[cnt++] = *p++;
	    buffer[cnt++] = *p++;
	    buffer[cnt++] = *p++;
	    buffer[cnt++] = *p++;
	    break;

	case LINETO:
	    buffer[cnt++] = *p++;
	    buffer[cnt++] = *p++;
	    break;

	case NEW_COLOR:
	case NEW_WIDTH:
	     buffer[cnt++] = escape[0];
	     break;

	case INITIALIZE:
	case CLOSE:
	case SWITCH_TO_TEXT:
	case SWITCH_TO_GRAPH:
	case CLEAR:
	case PAGE:
	  break;
	
	case ESCAPE:
	  buffer[cnt++] = escape[0];
	  switch( escape[0]) {
	  case (PLINT) ESC_NOOP:
	    break;
	  case (PLINT) ESC_RGB:
	    buffer[cnt++] = escape[1];
	  default:
	    printf ("Unknown escape sequence\n");
	    break;
	  }
	  break;

	default:
	    printf( "Unknown command type--no params passed\n" );
	    break;
    }
    if (cnt >= (.9 * PIPE_BUFFER_SIZE) || cid == CLOSE) {
	short rv1, rv2;
	ULONG bytes1 = 0, bytes2 = 0;
	
	rv1 = DosWrite( hf, &cnt, sizeof( PLINT ), &bytes1 );
	rv2 = DosWrite( hf, buffer, (USHORT) (cnt * sizeof(PLINT)), &bytes2 );
	if (!rv1 && !rv2) 
	    /* printf( "%d, %d bytes were written.\n", bytes1, bytes2 ) */ ;
	else 
	    printf( "----> write to pipe failed <---- %i %i\n", rv1, rv2 );

	if (bytes1 != sizeof(PLINT) || bytes2 != cnt*sizeof(PLINT) )
	    printf( "*** Bogus # of bytes written ***\n" );

	cnt=0;
  }
}
