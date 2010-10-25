/*
	os2pm.c

	Geoffrey Furnish
	9-22-91
	
	This driver sends plplot commands to the OS/2 PM PLPLOT Server.

	The Geoffrey Furnish Standard Disclaimer:
	"I hate any C compiler that isn't ANSI compliant, and I refuse to waste
	my time trying to support such garbage.  If you can't compile with an
	ANSI compiler, then don't expect this to work.  No appologies,
	now or ever."

	25 March 1992
	VERSION 1.0
*/

/* Some screwy thing with VOID goes wrong if this comes after the
   the os2.h stuff. */

#include "plplot/plplot.h"

#define INCL_BASE
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include "plplot/dispatch.h"

/* top level declarations */

static USHORT	rv;
static HFILE	hf;
static short	cnt;

static PLINT	xold = -100000;
static PLINT	yold = -100000;
	  
#include "plplot/pmdefs.h"

typedef	PLINT	COMMAND_ID;
typedef PLINT * CPARAMS;

static void	write_command( COMMAND_ID cid, CPARAMS p );

/*----------------------------------------------------------------------*\
*  os2setup()
*
* Set up device.
\*----------------------------------------------------------------------*/

void os2setup	PLARGS(( PLStream *pls ))
{
}

/*----------------------------------------------------------------------*\
*  os2orient()
*
* Set up orientation.
\*----------------------------------------------------------------------*/

void os2orient	PLARGS(( PLStream *pls ))
{
}

/*----------------------------------------------------------------------*\
*  os2init()
*
* Initialize device.
\*----------------------------------------------------------------------*/

void	os2init	PLARGS(( PLStream *pls ))
{
    USHORT	usAction;
    UCHAR	c = (UCHAR) INITIALIZE;

    pls->termin =- 0;		/* not an interactive terminal */
    pls->color = 1;
    pls->width = 1;
    pls->bytecnt = 0;
    pls->page = 1;

    setpxl( (PLFLT) PIXEL_RES_X, (PLFLT) PIXEL_RES_Y );
    setphy( 0, PLMETA_X, 0, PLMETA_Y );

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
*  os2line()
*
* Draw a line in the current color from (x1,y1) to (x2,y2).
\*----------------------------------------------------------------------*/

void os2line	PLARGS(( PLStream *pls, 
			PLINT x1, PLINT y1, PLINT x2, PLINT y2 ))
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
*  os2clear()
*
*  Clear page.
\*----------------------------------------------------------------------*/

void	os2clear	PLARGS(( PLStream *pls ))
{
	UCHAR c = (UCHAR) CLEAR;

	write_command( c, NULL );
}

/*----------------------------------------------------------------------*\
*  os2page()
*
*  Advance to next page.
\*----------------------------------------------------------------------*/

void	os2page	PLARGS(( PLStream *pls ))
{
	UCHAR c = (UCHAR) PAGE;

	xold = -100000;
	yold = -100000;

	write_command( c, NULL );
}

/*----------------------------------------------------------------------*\
* os2adv()
*
* Advance to the next page.
* Also write page information as in plmpage().
\*----------------------------------------------------------------------*/

void	os2adv	PLARGS(( PLStream *pls ))
{
    os2clear(pls);
    os2page(pls);
}

/*----------------------------------------------------------------------*\
*  os2tidy()
*
*  Close graphics file
\*----------------------------------------------------------------------*/

void	os2tidy	PLARGS(( PLStream *pls ))
{
	UCHAR c = (UCHAR) CLOSE;
	
	write_command( c, NULL );
		 
	DosClose( hf );
	pls->fileset = 0;
}

/*----------------------------------------------------------------------*\
*  os2color()
*
*  Set pen color.
\*----------------------------------------------------------------------*/

void	os2color	PLARGS(( PLStream *pls ))
{
	UCHAR c = (UCHAR) NEW_COLOR;
	
	write_command( c, &pls->color );
}

/*----------------------------------------------------------------------*\
*  os2text()
*
*  Switch to text mode.
\*----------------------------------------------------------------------*/

void	os2text	PLARGS(( PLStream *pls ))
{
	UCHAR c = (UCHAR) SWITCH_TO_TEXT;

	write_command( c, NULL );
}

/*----------------------------------------------------------------------*\
*  os2graph()
*
*  Switch to graphics mode.
\*----------------------------------------------------------------------*/

void	os2graph	PLARGS(( PLStream *pls ))
{
	UCHAR c = (UCHAR) SWITCH_TO_GRAPH;

	write_command( c, NULL );
}

/*----------------------------------------------------------------------*\
*  os2width()
*
*  Set pen width.
\*----------------------------------------------------------------------*/

void	os2width	PLARGS(( PLStream *pls ))
{
	UCHAR c = (UCHAR) NEW_WIDTH;
	
	write_command( c, &pls->width );
}

/*----------------------------------------------------------------------*\
*  os2esc()
*
*  Escape function.  Note that any data written must be in device
*  independent form to maintain the transportability of the metafile.
\*----------------------------------------------------------------------*/

void	os2esc	PLARGS(( PLStream *pls, PLINT op, char *ptr ))
{
	UCHAR c = (UCHAR) ESCAPE;
	float *color;
	unsigned long ired, igreen, iblue;
	unsigned long	pmrgb;

	write_command( c, NULL );

	switch (op) {
	    case PL_SET_RGB: 
		c = (UCHAR) ESC_RGB;
		color = (float *) &ptr[0];
		ired =	min(256,max(0,(int)255.*color[0]));
		igreen= min(256,max(0,(int)255.*color[1]));
		iblue = min(256,max(0,(int)255.*color[2]));
		pmrgb	= (ired   & 0xff) << 16 |
			  (igreen & 0xff) <<  8 |
			  (iblue  & 0xff);
		write_command( c, &pmrgb );
		//printf( "Changing to RGB value %lx \n", pmrgb );
		break;
	
	    default:
		c = (UCHAR) ESC_NOOP;
		write_command( c, NULL );
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
    static int i=0;
    static PLINT buffer[ PIPE_BUFFER_SIZE ];
    static PLINT cnt = 0;
    
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
	case ESC_RGB:
	    buffer[cnt++] = *p++;
	    break;

	case INITIALIZE:
	case CLOSE:
	case SWITCH_TO_TEXT:
	case SWITCH_TO_GRAPH:
	case CLEAR:
	case PAGE:
	case ESC_NOOP:
	    break;
	
	case ESCAPE:
	    break;

	default:
	    printf( "Unknown command type--no params passed\n" );
	    break;
    }
    if (cnt >= (.9 * PIPE_BUFFER_SIZE) || cid == CLOSE) {
	short rv1, rv2, bytes1, bytes2;
	
	rv1 = DosWrite( hf, &cnt, sizeof( PLINT ), &bytes1 );
	rv2 = DosWrite( hf, buffer, (USHORT) (cnt * sizeof(PLINT)), &bytes2 );
	if (!rv1 && !rv2) 
	    /* printf( "%d, %d bytes were written.\n", bytes1, bytes2 ) */ ;
	else 
	    printf( "----> write to pipe failed <----\n" );

	if (bytes1 != 4 || bytes2 != cnt*sizeof(PLINT) )
	    printf( "*** Bogus # of bytes written ***\n" );

	cnt=0;
    }
}
