/*
	pmserv.c
	Geoffrey Furnish
	22 September 1991

	This program is the OS/2 PM server for PLPLOT.	This program
	is run in a seperate process, before the plplot app is run.
	The app then communicates its requests to this server, which 
	renders the plot in a PM window.  Since the plplot app need not
	be a PM application, it can use normal printf output, and
	need not exhibit the message based architecture common in PM apps.

Theory of Operation:
	This program has two threads of execution.  The main thread handles
	the PM message queue, and basically just responds to messages to
	keep the image window up to date.  However, just before starting up
	the message queue, it spawns a second process which manages the
	pipe through which commands are accepted.  This second thread 
	accepts connection requests from one client at a time, and routes
	the drawing commands back to the first thread for display.  

	Although admittedly somewhat complex, this is actually a very efficient
	way to implement the desired functionality.  Threads with OS/2 are
	really great!

Revision History:
	1-24-92	Finally fixed the problem with the colors getting messed
		up on a repaint.  Also cleaning up the code a bit.
	2-1-92	Finally fixed the problem with the final command in the
		history buffer beeing fouled up in some cases.	More
		code cleaning.	Fixed numerous compiler warnings.
	2-1-92	Finally fixed the colors so they look about the same as
		with the X Windows driver.  However, I still haven't
		messed with the RGB escape function yet.  Later.
	2-1-92	Fixed all compiler warnings.
	2-2-92	Took out the majic number 200, and parameterized most
		pipe things with constants defined in pmdefs.h.  Also took
		this opportunity to choose a more sensible name for the
		pipe.  Some optimization of pipe performance.
	2-27-92	Finished the autoadvance code.	Seems to work about as well
		as could be hoped.  The only thing wrong is that pages with
		rgb never get drawn correctly, since they advance before they
		ever get refreshed with correct colors.  Sure would be nice
		if I could get them to draw right in the first place...
	3-13-92	Fixed the bug which prevented a page with RGB on it from ever
		being plotted right if it was the last page in the file.
		    
		Anyway, looks like this is good enough to be called 
			Version 1.0!
*/

#include "plplot/plplot.h"

#define INCL_WIN
#define INCL_DOS
#define INCL_GPI
#include <os2.h>
#include <process.h>
#include <stdlib.h>

#include <stdio.h>
#include <string.h>
#include <memory.h>

#include "plplot/pmdefs.h"

VOID CheckMenuItem( HWND hwnd, SHORT sMenuItem, BOOL fCheck );

void PipeManagerThread( void *stuff );
MRESULT EXPENTRY ClientWndProc( HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2 );
void InitiatePipeManagement( void );
long	pmcolor( long color );

static CHAR szClientClass[] = "PlDrawClass";
static ULONG flFrameFlags =	FCF_TITLEBAR | FCF_SYSMENU |
				FCF_SIZEBORDER | FCF_MINMAX |
				FCF_SHELLPOSITION | FCF_TASKLIST |
				FCF_MENU ;

HAB hab;
HMQ hmq;
HWND hwndFrame = NULLHANDLE, hwndClient = NULLHANDLE;
QMSG qmsg;

PLINT inbuflen = 0, inbuffer[ PIPE_BUFFER_SIZE ] = {};
int inbuffree = 1;

// I'm not really too clear on just how big this number should be.  When I
// had it set to only 16000, I found that examles 1, 12 and 13 all exhibited
// the property of overflowing the history buffer.  The code handles this
// correctly, in the sense that no error is generated, but clearly we want
// this number big enough for most graphs to work without a hitch.

#define MAX_LIST_LEN	60000
PLINT permbuflen = 0, permbufferfull=0;
PLINT permbuffer[ MAX_LIST_LEN ] = {};

#define WM_DRAW_MORE	(WM_USER + 0)
#define WM_NEW_SESSION	(WM_USER + 1)

/*----------------------------------------------------------------------------*\
* void main ( void )
*
* In this function we initialize the necessary data structures, spawn the 
* thread which manages the incoming command pipe, and finally start up the 
* PM message queue and process messages until all done.
*
* This is based on code from C. Petzold's book on OS/2 PM Programming.
\*----------------------------------------------------------------------------*/

void main ( void )
{
    hab = WinInitialize(0);
    hmq = WinCreateMsgQueue( hab, 0 );
    
    WinRegisterClass( hab, szClientClass, (PFNWP) ClientWndProc, CS_SIZEREDRAW, 
	0 );
    
    hwndFrame = WinCreateStdWindow( HWND_DESKTOP, WS_VISIBLE,
		&flFrameFlags, szClientClass,
		NULL,
		0L, (HMODULE) 0, ID_RESOURCE, &hwndClient );

    WinSendMsg( hwndFrame, WM_SETICON,
	(MPARAM) WinQuerySysPointer( HWND_DESKTOP, SPTR_APPICON, FALSE ), NULL );

    InitiatePipeManagement();	// spawns a new thread for this purpose.
    
    while( WinGetMsg( hab, &qmsg, NULLHANDLE, 0, 0 ) )
	WinDispatchMsg( hab, &qmsg );

    WinDestroyWindow( hwndFrame );
    WinDestroyMsgQueue( hmq );
    WinTerminate( hab );
}

/*----------------------------------------------------------------------------*\
* MRESULT EXPENTRY ClientWndProc(HWND hwnd,USHORT msg,MPARAM mp1,MPARAM mp2)
*
* In this function we handle all the messages to the main window.
\*----------------------------------------------------------------------------*/

MRESULT EXPENTRY ClientWndProc( HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2 )
{
    long i=0, j = 0;
    long startofset = 0, atendofimage = 0, commandlen = 0, range = 0, 
			pmrgb = 0, pmcolorindex = 0;
    HPS hps;
    POINTL ptl;
    static SHORT cx = 0, cy = 0;
    static float scalex = 0.0, scaley = 0.0;
    static int rgbused = 0;
    static int WaitingToAdvance = 0;
    static int justadvanced = 0;
    static int autoadvance = 0;
    SHORT  sMenuItem = 0;
    static long resumewith = 0;
    static PLINT oldx = 0, oldy = 0, oldcolor = CLR_WHITE, firstcolor = CLR_WHITE;

    switch (msg) {
	case WM_COMMAND:
	    switch ( SHORT1FROMMP (mp1) ) {                    // COMMANDMSG(&msg)->cmd ) {
		case IDM_ADVANCE:
		    WinAlarm( HWND_DESKTOP, WA_ERROR );
		    if (WaitingToAdvance) {
			WaitingToAdvance = 0;
			justadvanced = 1;
			WinPostMsg( hwndFrame, WM_DRAW_MORE, NULL, NULL );
		    }
		    return 0;

		case IDM_AUTO_ADVANCE:
		    autoadvance = !autoadvance;
		    sMenuItem = IDM_AUTO_ADVANCE;                               // COMMANDMSG(&msg)->cmd;
		    CheckMenuItem( hwnd, sMenuItem, autoadvance );
		    return 0;
	    }
	    break;
	
	case WM_SIZE:
	    cx = SHORT1FROMMP( mp2 );
	    cy = SHORT2FROMMP( mp2 );
	    scalex = cx / (float) PLMETA_X;
	    scaley = cy / (float) PLMETA_Y;
	    return 0;
	
	case WM_PAINT:
	    hps = WinBeginPaint( hwnd, NULLHANDLE, NULL );

	    ptl.x = oldx = 0; ptl.y = oldy = 0;
	    GpiMove( hps, &ptl );
	    ptl.x = cx; ptl.y = cy;

// Some ballogney to clear the window.	Gotta be a better way...

	    GpiSetPattern( hps, PATSYM_SOLID );
	    GpiSetColor( hps, CLR_BLACK );
	    GpiBox( hps, DRO_FILL, &ptl, 0, 0 );

// Now reset the color to the color which was the current color at the time
// of the last frame advance, i.e. the first color to use when drawing lines
// in this window.  This is not necessarily the same as oldcolor, or the
// first color in the color map.
// Also, reset the command index i, and other flags.

	    GpiSetColor( hps, pmcolor(firstcolor) );
	    i = 0; 
	    atendofimage = 0;

	    while ( i < (int) permbuflen && !atendofimage )
		switch ( permbuffer[i++] ) {
		    case INITIALIZE:
			break;
		
		    case SWITCH_TO_TEXT:
			break;
		    
		    case SWITCH_TO_GRAPH:
			break;
		    
		    case CLEAR:
			atendofimage = 1;
			break;
		    
		    case PAGE:
			break;
		    
		    case NEW_COLOR:
			oldcolor = permbuffer[i++];
			GpiSetColor( hps, pmcolor(oldcolor) );
			break;
		    
		    case NEW_WIDTH:
			i++;
			break;
		    
		    case LINETO:
			oldx = ptl.x = (long) (permbuffer[i++] * scalex);
			oldy = ptl.y = (long) (permbuffer[i++] * scaley);
			GpiLine( hps, &ptl );
			break;
		    
		    case LINE:
			ptl.x = (long) (permbuffer[i++] * scalex);
			ptl.y = (long) (permbuffer[i++] * scaley);
			GpiMove( hps, &ptl );
			oldx = ptl.x = (long) (permbuffer[i++] * scalex);
			oldy = ptl.y = (long) (permbuffer[i++] * scaley);
			GpiLine( hps, &ptl );
			break;

		    case CLOSE:
			atendofimage = 1;
			break;
		
		    case ESCAPE:
			switch(permbuffer[i++]) {
			    case ESC_NOOP:
				break;

			    case ESC_RGB:
				pmrgb = permbuffer[i++];
				// Do something to update the color here.
				pmcolorindex = GpiQueryColorIndex( hps,
				    LCOLOPT_REALIZED, pmrgb );
				if (pmcolorindex >= 0 && pmcolorindex <= 15) {
				    GpiSetColor( hps, pmcolorindex );
				    oldcolor = pmcolorindex;
				}
				break;
			
			    default:
				printf( "*** UNRECOGNIZED COMMAND ***\n" );
				WinAlarm( HWND_DESKTOP, WA_ERROR );
				exit(0);
				break;
			
			}
			break;
		
		    default:
			WinAlarm( HWND_DESKTOP, WA_ERROR );
			printf( "*** UNRECOGNIZED COMMAND ***\n" );
			exit(0);  // if bad here, then its bad bad news !!!
			break;
		}
	    
	    WinEndPaint( hps );
	    return 0;

	case WM_DRAW_MORE:
	    DosEnterCritSec();
	    inbuffree = 0;	// Mark the buffer "off-limits".

// Get a PS, and restore prior state.
	    
	    hps = WinGetPS( hwnd );
		GpiSetColor( hps, pmcolor(oldcolor) );
		GpiSetPattern( hps, PATSYM_SOLID );
		ptl.x = oldx; ptl.y = oldy;
		GpiMove( hps, &ptl );
				      
	    if (justadvanced) {
		i = resumewith;
		resumewith = 0;
		permbuflen = 0;
		permbufferfull = 0;
		ptl.x = oldx = 0; ptl.y = oldy = 0;
		GpiMove( hps, &ptl );
		ptl.x = cx; ptl.y = cy;

// Some ballogney to clear the window.	Gotta be a better way...

		GpiSetColor( hps, CLR_BLACK );
		GpiBox( hps, DRO_FILL, &ptl, 0, 0 );
		GpiSetColor( hps, pmcolor(oldcolor) );

		firstcolor = oldcolor;
		rgbused = 0;
		justadvanced = 0;
	    }

// Now process the new message stream.

	    startofset = i;
	    range = 0;

	    while ( i < (int) inbuflen && !WaitingToAdvance ) {
		commandlen = 1;
		switch ( inbuffer[i++] ) {
		    case INITIALIZE:
			break;
		
		    case SWITCH_TO_TEXT:
			break;
		    
		    case SWITCH_TO_GRAPH:
			break;
		    
		    case CLEAR:
			resumewith = i;
			WaitingToAdvance = 1;
			if (rgbused)
			    WinInvalidateRect( hwnd, NULL, FALSE );
			break;
		    
		    case PAGE:
			break;
		    
		    case NEW_COLOR:
			oldcolor = inbuffer[i++];
			GpiSetColor( hps, pmcolor(oldcolor) );
			commandlen++;
			break;
		    
		    case NEW_WIDTH:
			i++;
			commandlen++;
			break;
		    
		    case LINETO:
			oldx = ptl.x = (long) (inbuffer[i++] * scalex);
			oldy = ptl.y = (long) (inbuffer[i++] * scaley);
			GpiLine( hps, &ptl );
			commandlen += 2;
			break;
		    
		    case LINE:
			ptl.x = (long) (inbuffer[i++] * scalex);
			ptl.y = (long) (inbuffer[i++] * scaley);
			GpiMove( hps, &ptl );
			oldx = ptl.x = (long) (inbuffer[i++] * scalex);
			oldy = ptl.y = (long) (inbuffer[i++] * scaley);
			GpiLine( hps, &ptl );
			commandlen += 4;
			break;

		    case CLOSE:
			if (rgbused)
			    WinInvalidateRect( hwnd, NULL, FALSE );
			break;
		
		    case ESCAPE:
			commandlen++;
			switch(inbuffer[i++]) {
			    case ESC_NOOP:
				break;

			    case ESC_RGB:
				pmrgb = inbuffer[i++];
				commandlen++;
				// Do something to update the color here...
				// Unfortunately, this is very hard to do
				// here, since it keeps drawing in spurious
				// colors.  Only works right if done during
				// the WM_PAINT message handling.  So,
				// jury rig plots with RGB specified colors
				// to auto-repaint when done.
				rgbused = 1;
				break;
			
			    default:
				printf( "*** UNRECOGNIZED ESC COMMAND ***%i\n", (int) inbuffer[i-1] );
				WinAlarm( HWND_DESKTOP, WA_ERROR );
				exit(0);
				break;
			
			}
			break;
		
		default:
		  printf( "*** UNRECOGNIZED COMMAND ***%i\n", (int) inbuffer[i-1] );
		  WinAlarm( HWND_DESKTOP, WA_ERROR );
		  exit(0);
		  break;
		}
		if (!permbufferfull && permbuflen + range + commandlen 
				< MAX_LIST_LEN)
		    range += commandlen;
		else
		    permbufferfull = 1;
	    }

	    WinReleasePS( hps );

// Now add the commands processed this far to the permanent buffer list.
	    if ( range )
		for( j=startofset; range; range-- )
		    permbuffer[ permbuflen++ ] = inbuffer[ j++ ];

	    if (!WaitingToAdvance)
		inbuffree = 1;

// Should this next block be here, or just above the one above???

	    if (WaitingToAdvance && autoadvance) {
		WaitingToAdvance = 0;
		justadvanced = 1;
		WinPostMsg( hwndFrame, WM_DRAW_MORE, NULL, NULL );
	    }

            DosExitCritSec();

	    return 0;

	case WM_NEW_SESSION:
	    WinAlarm( HWND_DESKTOP, WA_ERROR );
	    resumewith = 0;
	    justadvanced = 1;
	    return 0;
    }

    return WinDefWindowProc( hwnd, msg, mp1, mp2 );
}

/*----------------------------------------------------------------------------*\
* long	  pmcolor( long color )
*
* In this function we convert the PLPLOT requested color into a an actual
* PM color index value.
\*----------------------------------------------------------------------------*/

long	pmcolor( long color )
{
    static long pmcolors[] = {
	CLR_BLACK, CLR_RED, CLR_YELLOW, CLR_GREEN,
	CLR_DARKCYAN, CLR_PINK, CLR_DARKGRAY, CLR_PALEGRAY,
	CLR_BROWN, CLR_BLUE, CLR_DARKBLUE, CLR_CYAN,
	CLR_RED, CLR_PINK, CLR_YELLOW, CLR_WHITE
    };

    if (color < 0 || color > 15)
	color = 15;

    return pmcolors[color];
}

/*----------------------------------------------------------------------------*\
* void InitiatePipeManagement( void )
*
* In this function we start the thread which reads the pipe, and passes 
* the graphics commands to the window procedure.
\*----------------------------------------------------------------------------*/

void InitiatePipeManagement( void )
{
    static VOID *pThreadStack = NULL;
    static TID	tidPipeManager;
    static PMSTUFF pmstuff;
    
    pmstuff.a = 0;	// Not currently used for anything...
    
    if ( NULL == (pThreadStack = malloc( STACKSIZE )) ) {
	printf( "Unable to allocate space for pipe manager thread.\n" );
	exit(0);
    }
    if ( -1 == (tidPipeManager = _beginthread( PipeManagerThread,
			pThreadStack, STACKSIZE, &pmstuff ))) {
	printf( "Unable to spawn pipe manager thread.\n" );
	exit(0);
    }
}

/*----------------------------------------------------------------------------*\
* VOID FAR PipeManagerThread( PMSTUFF *pmstuff )
*
* In this function we read commands from the pipe, and pass them off to
* the window procedure.
\*----------------------------------------------------------------------------*/

void PipeManagerThread( void *stuff )
{
    HPIPE hp;
    USHORT bytes2 = 0, rv1, rv2, retries;
    ULONG  bytes1 = 0L, bytes3 = 0L;
    PLINT buflen = 0, buffer[ PIPE_BUFFER_SIZE ] = {};
    char  *pbuffer = (char *) buffer;
    PMSTUFF *pmstuff = (PMSTUFF *) stuff;
    int isfree;
    
// Now open up the pipe and start handling it.
    
     DosCreateNPipe( /*"\\pipe\\bubba"*/ PIPE_NAME,	// pipe name.
		     &hp,			// pipe handle.
		     NP_ACCESS_INBOUND | NP_NOINHERIT | NP_NOWRITEBEHIND,
		     1 | NP_WAIT | NP_READMODE_BYTE | NP_TYPE_BYTE,
		     512,			// output-buffer size.
		     //512,			// input-buffer size.
		     2*PIPE_BUFFER_SIZE,	// input-buffer size.
		     500L );			// default timeout for DosWaitNmPipe
		       
// 2-1-92
// Comments:  In vol 1 of the OS/2 Programmer's Reference, the output
// and input buffer sizes are listed in the opposite order of what is
// shown in vol 3.  I am taking 3 to be correct, since it is the complete
// spec for the function.
//
// Secondly, in a prior life, I had to boost the output buffer (whic I thought
// at the time was the input buffer) size from 512 to 2048.  I've now (2-1-92)
// taken it back down to 512, and don't seem to have any problems with it.

// Really need to overhaul this next section.  Should replace all those
// crazy diagnostics with some useful, effective actions.

    do {
	if (DosConnectNPipe( hp )) {
	    DosClose( hp );
	    exit(0);
	}

	WinPostMsg( hwndFrame, WM_NEW_SESSION, NULL, NULL );

        do {
            rv1 = DosRead( hp, &buflen, sizeof( PLINT ), &bytes1 );
            if (bytes1 == 0) break;
            // if (bytes1 != sizeof (PLINT) )
		// printf( "Major screw up.  buflen not read correctly.\n" );

	    bytes2 = 0;
	    retries = -1;
	    do {
		retries++;
		rv2 = DosRead( hp, &pbuffer[bytes2], 
				(USHORT) (buflen*sizeof(PLINT) - (PLINT) bytes2),
				&bytes3 );
		bytes2 += (PLINT) bytes3;
		//if (!bytes3) printf( "No bytes returned!\n" );
		//if (rv2) printf( "Bogus pipe read.\n" );
		    
	    } while ( (PLINT) bytes2 < buflen*sizeof(PLINT) );
	
//	    printf( "buflen=%d, Read %d bytes, %d expected.\n", 
//		(int) buflen, (int) bytes2, (int) buflen*sizeof(PLINT) );
//	    if (retries) printf( "%d retries to get whole packet.\n", retries );
	    
	    if (rv1 || rv2)
		printf( "rv1=%d, rv2=%d \n", rv1, rv2 );

            DosEnterCritSec();
            isfree = inbuffree;
            DosExitCritSec();

            while (!isfree)
             {
              _sleep2(100);

              DosEnterCritSec();
              isfree = inbuffree;
              DosExitCritSec();
             }  
    
            DosEnterCritSec();
	    inbuflen = buflen;
	    memcpy( inbuffer, buffer, (int) buflen*sizeof(PLINT) );
	    inbuffree = 0;
            DosExitCritSec();
	    WinPostMsg( hwndFrame, WM_DRAW_MORE, NULL, NULL );

	} while( bytes2 );

	DosDisConnectNPipe( hp );
//	printf( "Connection closed.\n\n" );
    
    } while(1);

    _endthread();
}

/*----------------------------------------------------------------------------*\
* VOID CheckMenuItem( HWND hwnd, SHORT sMenuItem, BOOL fCheck )
*
* This function, obtained from Petzold's book, manages the checking and
* unchecking of a menu item.
\*----------------------------------------------------------------------------*/

VOID CheckMenuItem( HWND hwnd, SHORT sMenuItem, BOOL fCheck )
{
    HWND hwndParent	= WinQueryWindow( hwnd, QW_PARENT );
    HWND hwndMenu	= WinWindowFromID( hwndParent, FID_MENU );

    WinSendMsg( hwndMenu, MM_SETITEMATTR,
		MPFROM2SHORT( sMenuItem, TRUE ),
		MPFROM2SHORT( MIA_CHECKED, fCheck ? MIA_CHECKED : 0 ) );
}
