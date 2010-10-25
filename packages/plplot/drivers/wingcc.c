/* $Id: wingcc.c 3186 2006-02-15 18:17:33Z slbrow $

	PLplot WIN32 under GCC device driver.

   Copyright (C) 2004  Andrew Roach

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
#include "plDevs.h"

#ifdef PLD_wingcc

#include <string.h>
#include <windows.h>

#include "plplotP.h"
#include "drivers.h"
#include "plevent.h"

#ifdef HAVE_FREETYPE

/*
 *  Freetype support has been added to the wingcc driver using the
 *  plfreetype.c module, and implemented as a driver-specific optional extra
 *  invoked via the -drvopt command line toggle. It uses the
 *  "PLESC_HAS_TEXT" command for rendering within the driver.
 *
 *  Freetype support is turned on/off at compile time by defining
 *  "HAVE_FREETYPE".
 *
 *  To give the user some level of control over the fonts that are used,
 *  environmental variables can be set to over-ride the definitions used by
 *  the five default plplot fonts.
 *
 *  Freetype rendering is used with the command line "-drvopt text".
 *  Anti-aliased fonts can be used by issuing "-drvopt text,smooth"
 */

#include "plfreetype.h"

#ifndef max_number_of_grey_levels_used_in_text_smoothing
#define max_number_of_grey_levels_used_in_text_smoothing 64
#endif

#endif


/* Device info */

char* plD_DEVICE_INFO_wingcc = "wingcc:Win32 (GCC):1:wingcc:9:wingcc";

/* Struct to hold device-specific info. */

typedef struct {
    PLFLT       scale;               /* scaling factor to "blow up" to the "virtual" page in removing hidden lines*/
    PLINT       width;               /* Window width (which can change) */
    PLINT       height;              /* Window Height */

/*
 * WIN32 API variables
 */

    COLORREF      colour;              /* Current Colour               */
    MSG		      msg;		            /* A Win32 message structure. */
    WNDCLASSEX	   wndclass;	         /* An extended window class structure. */
    HWND          hwnd;                /* Handle for the main window. */
    HPEN          pen;                 /* Windows pen used for drawing */
    HDC           hdc;                 /* Driver Context */
    PAINTSTRUCT   ps;                  /* used to paint the client area of a window owned by that application */
    RECT		      rect;                /* defines the coordinates of the upper-left and lower-right corners of a rectangle */
    HBRUSH        bgbrush;             /* brush used for filling the background */
    HCURSOR       cursor;              /* Current windows cursor for this window */

    PLINT         draw_mode;
    char          truecolour;          /* Flag to indicate 24 bit mode */
    char          waiting;             /* Flag to indicate drawing is done, and it is waiting; */
                                       /* we only do a windows redraw if plplot is plotting */
    char          enterresize;         /* Used to keep track of reszing messages from windows */
    char          resize;              /* When "entersize" is set and "reszie" isnt, user maximised the window */

  } wingcc_Dev;


void plD_dispatch_init_wingcc	( PLDispatchTable *pdt );

void plD_init_wingcc       (PLStream *);
void plD_line_wingcc       (PLStream *, short, short, short, short);
void plD_polyline_wingcc   (PLStream *, short *, short *, PLINT);
void plD_eop_wingcc        (PLStream *);
void plD_bop_wingcc        (PLStream *);
void plD_tidy_wingcc       (PLStream *);
void plD_state_wingcc      (PLStream *, PLINT);
void plD_esc_wingcc        (PLStream *, PLINT, void *);

#ifdef HAVE_FREETYPE

static void plD_pixel_wingcc (PLStream *pls, short x, short y);
static void plD_pixelV_wingcc (PLStream *pls, short x, short y);
static void init_freetype_lv1 (PLStream *pls);
static void init_freetype_lv2 (PLStream *pls);

extern void plD_FreeType_init(PLStream *pls);
extern void plD_render_freetype_text (PLStream *pls, EscText *args);
extern void plD_FreeType_Destroy(PLStream *pls);
extern void pl_set_extended_cmap0(PLStream *pls, int ncol0_width, int ncol0_org);
extern void pl_RemakeFreeType_text_from_buffer (PLStream *pls);
#endif


/*--------------------------------------------------------------------------*\
 *  Local Function definitions and function-like defines
\*--------------------------------------------------------------------------*/

static int GetRegValue(char *key_name, char *key_word, char *buffer, int size);
static int SetRegValue(char *key_name, char *key_word, char *buffer,int dwType, int size);
static void Resize ( PLStream *pls );
static void plD_fill_polygon_wingcc(PLStream *pls);

#define SetRegStringValue(a,b,c) SetRegValue(a, b, c, REG_SZ, strlen(c)+1 )
#define SetRegBinaryValue(a,b,c,d) SetRegValue(a, b, (char *)c, REG_BINARY, d )
#define SetRegIntValue(a,b,c) SetRegValue(a, b, (char *)c, REG_DWORD, 4 )
#define GetRegStringValue(a,b,c,d) GetRegValue(a, b, c, d)
#define GetRegIntValue(a,b,c) GetRegValue(a, b, (char *)c, 4)
#define GetRegBinaryValue(a,b,c,d) GetRegValue(a, b, (char *)c, d )

/*--------------------------------------------------------------------------*\
 *  Some debugging macros
\*--------------------------------------------------------------------------*/

#define Verbose(...) do {if (pls->verbose){fprintf(stderr,__VA_ARGS__);}}while(0)
#define Debug(...) do {if (pls->debug){fprintf(stderr,__VA_ARGS__);}}while(0)

#define ReportWinError() do { \
LPVOID lpMsgBuf; \
FormatMessage( \
FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, \
NULL, GetLastError(), \
MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf, 0, NULL ); \
MessageBox( NULL, lpMsgBuf, "GetLastError", MB_OK|MB_ICONINFORMATION ); \
LocalFree( lpMsgBuf );}while(0)

#define NormalCursor() do { \
dev->cursor = LoadCursor(NULL,IDC_ARROW); \
SetClassLong(dev->hwnd,GCL_HCURSOR,(long)dev->cursor); \
SetCursor(dev->cursor);}while(0)

#define BusyCursor() do { \
dev->cursor = LoadCursor(NULL,IDC_WAIT); \
SetClassLong(dev->hwnd,GCL_HCURSOR,(long)dev->cursor); \
SetCursor(dev->cursor);}while(0)


void plD_dispatch_init_wingcc( PLDispatchTable *pdt )
{
#ifndef ENABLE_DYNDRIVERS
    pdt->pl_MenuStr  = "Win32 GCC device";
    pdt->pl_DevName  = "wingcc";
#endif
    pdt->pl_type     = plDevType_Interactive;
    pdt->pl_seq      = 9;
    pdt->pl_init     = (plD_init_fp)     plD_init_wingcc;
    pdt->pl_line     = (plD_line_fp)     plD_line_wingcc;
    pdt->pl_polyline = (plD_polyline_fp) plD_polyline_wingcc;
    pdt->pl_eop      = (plD_eop_fp)      plD_eop_wingcc;
    pdt->pl_bop      = (plD_bop_fp)      plD_bop_wingcc;
    pdt->pl_tidy     = (plD_tidy_fp)     plD_tidy_wingcc;
    pdt->pl_state    = (plD_state_fp)    plD_state_wingcc;
    pdt->pl_esc      = (plD_esc_fp)      plD_esc_wingcc;
}

static char*		szWndClass = "PlplotWin";


/*--------------------------------------------------------------------------*\
 * This is the window function for the plot window. Whenever a message is
 * dispatched using DispatchMessage (or sent with SendMessage) this function
 * gets called with the contents of the message.
\*--------------------------------------------------------------------------*/

LRESULT CALLBACK PlplotWndProc (HWND hwnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
  PLStream *pls = NULL;
  wingcc_Dev *dev = NULL;

/*
 * The window carries a 32bit user defined pointer which points to the 
 * plplot stream (pls). This is used for tracking the window.
 * Unfortunately, this is "attached" to the window AFTER it is created
 * so we can not initialise PLStream or wingcc_Dev "blindly" because
 * they may not yet have been initialised.
 * WM_CREATE is called before we get to initialise those variables, so
 * we wont try to set them.
 */

  if (nMsg == WM_CREATE)
    {
      return(0);
    }
  else
    {
      pls = (PLStream *)GetWindowLong(hwnd,GWL_USERDATA); /* Try to get the address to pls for this window */
      if (pls) /* If we got it, then we will initialise this windows plplot private data area */
        {
          dev = (wingcc_Dev *)pls->dev;
        }
    }

/*
 * Process the windows messages
 *
 * Everything except WM_CREATE is done here and it is generally hoped that 
 * pls and dev are defined already by this stage.
 * That will be true MOST of the time. Some times WM_PAINT will be called
 * before we get to initialise the user data area of the window with the
 * pointer to the windows plplot stream
 */

	switch (nMsg)
	{
		case WM_DESTROY:
        if (dev)
           Debug("WM_DESTROY\t");
			PostQuitMessage (0);
			return(0);
			break;

		case WM_PAINT:
        if (dev)
          {
          	Debug("WM_PAINT\t");
            if (GetUpdateRect(dev->hwnd,&dev->rect,TRUE))
              {

              	 BusyCursor();
                BeginPaint (dev->hwnd, &dev->ps);
                if (dev->waiting==1)
                {
                    plRemakePlot(pls);
                    #ifdef HAVE_FREETYPE
                    pl_RemakeFreeType_text_from_buffer(pls);
                    #endif
                 }
                EndPaint (dev->hwnd, &dev->ps);
                NormalCursor();
         		 return(0);
              }
          }
			return(1);
			break;

      case WM_SIZE:
        if (dev)
          {
            Debug("WM_SIZE\t");
            if (dev->enterresize==0)
              Resize(pls);
            else
              dev->resize=1;
          }
        return(0);
      break;

      case WM_ENTERSIZEMOVE:
        if (dev)
          {
            Debug("WM_ENTERSIZEMOVE\t");
            dev->enterresize=1;
          }
        return(0);
      break;


		case WM_EXITSIZEMOVE:
        if (dev)
          {
          	Debug("WM_EXITSIZEMOVE\t");
            Resize(pls);
          	dev->enterresize=0;   /* Reset the variables that track sizing ops */
          	dev->resize=0;
          }

          return(0);
			break;

      case WM_ERASEBKGND:
      if (dev)
        {
          Debug("WM_ERASEBKGND\t");
          dev->bgbrush = CreateSolidBrush(RGB(pls->cmap0[0].r,pls->cmap0[0].g,pls->cmap0[0].b));
          GetClientRect(hwnd,&dev->rect);
          SelectObject (dev->hdc, dev->bgbrush);
          FillRect(dev->hdc, &dev->rect,dev->bgbrush);
          DeleteObject (dev->bgbrush);
          return(1);
        }
        return(0);
        break;

		case WM_COMMAND:
        if (dev)
		     Debug("WM_COMMAND\t");
			return(0);
			break;

	}

	/* If we don't handle a message completely we hand it to the system
	 * provided default window function. */
	return DefWindowProc (hwnd, nMsg, wParam, lParam);
}


/*--------------------------------------------------------------------------*\
 * plD_init_wingcc()
 *
 * Initialize device (terminal).
\*--------------------------------------------------------------------------*/

void
plD_init_wingcc(PLStream *pls)
{
    wingcc_Dev *dev;

#ifdef HAVE_FREETYPE
    static int freetype=0;
    static int smooth_text=0;
    static int save_reg=0;
    FT_Data *FT;

/*
 *  Variables used for reading the registary keys
 *  might eventually add a user defined pallette here, but for now it just does freetype
 */
    char key_name[]="Software\\PLplot\\wingcc";
    char Keyword_text[]="freetype";
    char Keyword_smooth[]="smooth";
#endif

    DrvOpt wingcc_options[] = {
#ifdef HAVE_FREETYPE
                              	{"text", DRV_INT, &freetype, "Use driver text (FreeType)"},
                              {"smooth", DRV_INT, &smooth_text, "Turn text smoothing on (1) or off (0)"},
                              {"save", DRV_INT, &save_reg, "Save defaults to registary"},

#endif
			      {NULL, DRV_INT, NULL, NULL}};

/* Allocate and initialize device-specific data */

    if (pls->dev != NULL)
    	free((void *) pls->dev);

    pls->dev = calloc(1, (size_t) sizeof(wingcc_Dev));
    if (pls->dev == NULL)
    	plexit("plD_init_wingcc_Dev: Out of memory.");

    dev = (wingcc_Dev *) pls->dev;

    pls->icol0 = 1;              /* Set a fall back pen colour in case user doesn't */

    pls->termin = 1;             /* interactive device */
    pls->graphx = GRAPHICS_MODE; /*  No text mode for this driver (at least for now, might add a console window if I ever figure it out and have the inclination) */
    pls->dev_fill0 = 1;          /* driver can do solid area fills */
    pls->dev_xor = 1;            /* driver supports xor mode */
    pls->dev_clear = 1;          /* driver supports clear */
    pls->dev_dash = 0;           /* driver can not do dashed lines (yet) */
    pls->plbuf_write = 1;        /* driver uses the buffer for redraws */

    if (!pls->colorset)
	pls->color = 1;


#ifdef HAVE_FREETYPE

/*
 *  Read registry to see if the user has set up default values
 *  for text and smoothing. These will be overriden by anything that
 *  might be given on the command line, so we will load the
 *  values right into the same memory slots we pass to plParseDrvOpts
 */

    GetRegIntValue(key_name, Keyword_text, &freetype);
    GetRegIntValue(key_name, Keyword_smooth, &smooth_text);

#endif

/* Check for and set up driver options */

    plParseDrvOpts(wingcc_options);

#ifdef HAVE_FREETYPE

/*
 *  We will now save the settings to the registary if the user wants
 */

    if (save_reg==1)
      {
        SetRegIntValue(key_name, Keyword_text, &freetype);
        SetRegIntValue(key_name, Keyword_smooth, &smooth_text);
      }

#endif

/* Set up device parameters */

  if (pls->xlength <= 0 || pls->ylength <=0)
      {
        /* use default width, height of 800x600 if not specifed by -geometry option
         * or plspage */
        plspage(0., 0., 800, 600, 0, 0);
      }

     dev->width = pls->xlength - 1;	/* should I use -1 or not??? */
     dev->height = pls->ylength - 1;

/*
 * Begin initialising the window
 */

	/* Initialize the entire structure to zero. */
	memset (&dev->wndclass, 0, sizeof(WNDCLASSEX));

	/* This class is called WinTestWin */
	dev->wndclass.lpszClassName = szWndClass;

	/* cbSize gives the size of the structure for extensibility. */
	dev->wndclass.cbSize = sizeof(WNDCLASSEX);

	/* All windows of this class redraw when resized. */
	dev->wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_SAVEBITS | CS_DBLCLKS;

	/* All windows of this class use the WndProc window function. */
	dev->wndclass.lpfnWndProc = PlplotWndProc;

	/* This class is used with the current program instance. */

	dev->wndclass.hInstance = GetModuleHandle(NULL);

	/* Use standard application icon and arrow cursor provided by the OS */
	dev->wndclass.hIcon = LoadIcon (NULL, IDI_APPLICATION);
	dev->wndclass.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
	dev->wndclass.hCursor = LoadCursor (NULL, IDC_ARROW);
	/* Color the background white */
	dev->wndclass.hbrBackground = NULL;

    dev->wndclass.cbWndExtra = sizeof(pls);


	/*
	 * Now register the window class for use.
	 */

     RegisterClassEx (&dev->wndclass);


	/*
	 * Create our main window using that window class.
	 */
    dev->hwnd = CreateWindowEx (  WS_EX_WINDOWEDGE+WS_EX_LEFT,
                                  szWndClass,               /* Class name */
                                  pls->program,             /* Caption */
                                  WS_OVERLAPPEDWINDOW,      /* Style */
                                  pls->xoffset,            /* Initial x (use default) */
                                  pls->yoffset,            /* Initial y (use default) */
                                  pls->xlength,             /* Initial x size (use default) */
                                  pls->ylength,             /* Initial y size (use default) */
                                  NULL,                     /* No parent window */
                                  NULL,                     /* No menu */
                                  dev->wndclass.hInstance,  /* This program instance */
                                  NULL                      /* Creation parameters */
                                  );


/*
 * Attach a pointer to the stream to the window's user area
 * this pointer will be used by the windows call back for
 * process this window
 */

SetWindowLong(dev->hwnd,GWL_USERDATA,(long)pls);
dev->hdc = GetDC (dev->hwnd);

#ifdef HAVE_FREETYPE

if (freetype)
   {
    pls->dev_text = 1; /* want to draw text */
    pls->dev_unicode = 1; /* want unicode */
    init_freetype_lv1(pls);
    FT=(FT_Data *)pls->FT;
    FT->want_smooth_text=smooth_text;
   }

#endif



plD_state_wingcc(pls, PLSTATE_COLOR0);
	/*
	 * Display the window which we just created (using the nShow
	 * passed by the OS, which allows for start minimized and that
	 * sort of thing).
	 */
	ShowWindow (dev->hwnd, SW_SHOWDEFAULT);
   SetForegroundWindow(dev->hwnd);

   /*
    * Set up the DPI etc...
    */


     if (pls->xdpi<=0) /* Get DPI from windows */
     {
         plspage(GetDeviceCaps(dev->hdc,HORZRES)/GetDeviceCaps(dev->hdc,HORZSIZE)*25.4,
         GetDeviceCaps(dev->hdc,VERTRES)/GetDeviceCaps(dev->hdc,VERTSIZE)*25.4, 0, 0, 0, 0);
     }
     else
     {
        pls->ydpi=pls->xdpi;        /* Set X and Y dpi's to the same value */
     }


/*
 *  Now we have to find out, from windows, just how big our drawing area is
 *  when we specified the page size earlier on, that includes the borders,
 *  title bar etc... so now that windows has done all its initialisations,
 *  we will ask how big the drawing area is, and tell plplot
 */

  	 GetClientRect(dev->hwnd,&dev->rect);
    dev->width=dev->rect.right;
    dev->height=dev->rect.bottom;

    if (dev->width>dev->height)           /* Work out the scaling factor for the  */
      {                                   /* "virtual" (oversized) page           */
        dev->scale=(PLFLT)PIXELS_X/dev->width;
      }
    else
      {
        dev->scale=(PLFLT)PIXELS_Y/dev->height;
      }

     Debug("Scale = %f (FLT)\n",dev->scale);

     plP_setpxl(dev->scale*pls->xdpi/25.4,dev->scale*pls->ydpi/25.4);

     plP_setphy(0, dev->scale*dev->width, 0, dev->scale*dev->height);

#ifdef HAVE_FREETYPE
if (pls->dev_text)
   {
    init_freetype_lv2(pls);
   }
#endif

}

/*----------------------------------------------------------------------*\
 * plD_line_wingcc()
 *
 * Draw a line in the current color from (x1,y1) to (x2,y2).
\*----------------------------------------------------------------------*/

void
plD_line_wingcc(PLStream *pls, short x1a, short y1a, short x2a, short y2a)
{
  wingcc_Dev *dev=(wingcc_Dev *)pls->dev;
  POINT points[2];

  points[0].x=x1a/dev->scale;
  points[1].x=x2a/dev->scale;
  points[0].y=dev->height - (y1a/dev->scale);
  points[1].y=dev->height - (y2a/dev->scale);

  Polyline(dev->hdc, points,2);

}


/*----------------------------------------------------------------------*\
 * plD_polyline_wingcc()
 *
 * Draw a polyline in the current color.
\*----------------------------------------------------------------------*/

void
plD_polyline_wingcc(PLStream *pls, short *xa, short *ya, PLINT npts)
{
  wingcc_Dev *dev=(wingcc_Dev *)pls->dev;
  int i;
  POINT *points=NULL;

  if (npts > 0)
    {
      points = GlobalAlloc(GMEM_ZEROINIT|GMEM_FIXED,(size_t)npts * sizeof(POINT));
      if (points!=NULL)
        {
          for (i = 0; i < npts; i++)
            {
              points[i].x =  (unsigned long) xa[i]/dev->scale;
              points[i].y =  (unsigned long) dev->height - (ya[i]/dev->scale);
            }
          Polyline(dev->hdc,points,npts);
          GlobalFree(points);
        }
      else
      {
        plexit("Could not allocate memory to \"plD_polyline_wingcc\"\n");
      }
    }

}

/*----------------------------------------------------------------------*\
 * plD_fill_polygon_wingcc()
 *
 * Fill polygon described in points pls->dev_x[] and pls->dev_y[].
\*----------------------------------------------------------------------*/

static void
plD_fill_polygon_wingcc(PLStream *pls)
{
  wingcc_Dev *dev=(wingcc_Dev *)pls->dev;
  int i;
  POINT *points=NULL;

  if (pls->dev_npts > 0)
    {
      points = GlobalAlloc(GMEM_ZEROINIT,(size_t)pls->dev_npts * sizeof(POINT));

      if (points==NULL)
        plexit("Could not allocate memory to \"plD_fill_polygon_wingcc\"\n");

      for (i = 0; i < pls->dev_npts; i++)
        {
          points[i].x = pls->dev_x[i]/dev->scale;
          points[i].y = dev->height -(pls->dev_y[i]/dev->scale);
        }

      dev->bgbrush = CreateSolidBrush(dev->colour);
      SelectObject (dev->hdc, dev->bgbrush);
      Polygon(dev->hdc,points,pls->dev_npts);
      DeleteObject (dev->bgbrush);
      GlobalFree(points);
    }
}

void
plD_eop_wingcc(PLStream *pls)
{
  wingcc_Dev *dev=(wingcc_Dev *)pls->dev;

  Debug("End of the page\n");

  NormalCursor();

  if ( ! pls->nopause )
    {
      dev->waiting=1;
          while (GetMessage (&dev->msg, NULL, 0, 0))
              {
                TranslateMessage (&dev->msg);
                switch((int)dev->msg.message)
                  {

                    case WM_CHAR:
                    if (((TCHAR)(dev->msg.wParam)== 32)||
                        ((TCHAR)(dev->msg.wParam)== 13))
                        {
                            dev->waiting=0;
                        }
                    else if (((TCHAR)(dev->msg.wParam)== 27)||
                             ((TCHAR)(dev->msg.wParam)== 'q')||
                             ((TCHAR)(dev->msg.wParam)== 'Q'))
                        {
                          dev->waiting=0;
                          PostQuitMessage(0);
                        }
                    break;

                    case WM_LBUTTONDBLCLK:
                    Debug("WM_LBUTTONDBLCLK\t");
                      dev->waiting=0;
                    break;

                    default:
                      DispatchMessage (&dev->msg);
                    break;
                  }
                 if (dev->waiting==0) break;
              }
    }
}

/*--------------------------------------------------------------------------*\
 *  Beginning of the new page
\*--------------------------------------------------------------------------*/
void
plD_bop_wingcc(PLStream *pls)
{
  wingcc_Dev *dev=(wingcc_Dev *)pls->dev;
#ifdef HAVE_FREETYPE
  FT_Data *FT=(FT_Data *)pls->FT;
#endif
  Debug("Start of Page\t");

/*
 *  Turn the cursor to a busy sign, clear the page by "invalidating" it
 *  reset freetype, if we have to, then reset the colours and pen width
 */

#ifdef HAVE_FREETYPE
 pl_FreeTypeBOP();
#endif

  BusyCursor();
  RedrawWindow(dev->hwnd,NULL,NULL,RDW_ERASE|RDW_INVALIDATE);


  plD_state_wingcc(pls, PLSTATE_COLOR0);
}

void
plD_tidy_wingcc(PLStream *pls)
{

#ifdef HAVE_FREETYPE
  if (pls->dev_text)
    {
      FT_Data *FT=(FT_Data *)pls->FT;
      plscmap0n(FT->ncol0_org);
      plD_FreeType_Destroy(pls);
    }
#endif

  if (pls->dev!=NULL)
    free(pls->dev);
}




/*----------------------------------------------------------------------*\
 * plD_state_png()
 *
 * Handle change in PLStream state (color, pen width, fill attribute, etc).
\*----------------------------------------------------------------------*/

void
plD_state_wingcc(PLStream *pls, PLINT op)
{
  wingcc_Dev *dev=(wingcc_Dev *)pls->dev;

      switch (op) {

      case PLSTATE_COLOR0:
      case PLSTATE_COLOR1:
      dev->colour=RGB(pls->curcolor.r,pls->curcolor.g, pls->curcolor.b);
      break;

      case PLSTATE_CMAP0:
      case PLSTATE_CMAP1:
      dev->colour=RGB(pls->curcolor.r,pls->curcolor.g, pls->curcolor.b);


  	break;
      }

  if (dev->pen!=NULL) DeleteObject(dev->pen);
  dev->pen=CreatePen( PS_SOLID, pls->width,dev->colour);
  SelectObject (dev->hdc, dev->pen);
}

void
plD_esc_wingcc(PLStream *pls, PLINT op, void *ptr)
{
  wingcc_Dev *dev=(wingcc_Dev *)pls->dev;

  switch (op) {

    case PLESC_GETC:
    break;

    case PLESC_FILL:
   	plD_fill_polygon_wingcc(pls);
    break;

    case PLESC_DOUBLEBUFFERING:
    break;

    case PLESC_XORMOD:
		if (*(PLINT*)(ptr) == 0)
			SetROP2(dev->hdc,R2_COPYPEN);
		else
			SetROP2(dev->hdc,R2_XORPEN);
    break;

#ifdef HAVE_FREETYPE
     case PLESC_HAS_TEXT:
        plD_render_freetype_text(pls, (EscText *)ptr);
        break;

/*     case PLESC_LIKES_UNICODE:
        plD_render_freetype_sym(pls, (EscText *)ptr);
        break;*/

#endif

      }

}

/*--------------------------------------------------------------------------*\
 * static void Resize( PLStream *pls )
 *
 * This function calculates how to resize a window after a message has been
 * received from windows telling us the window has been changed.
 * It tries to recalculate the scale of the window so everything works out
 * just right.
 * The window is only resized if plplot has finished all of its plotting.
 * That means that if you resize while a picture is being plotted,
 * unpredictable results may result. The reason I do this is because the
 * resize function calls redraw window, which replays the whole plot.
\*--------------------------------------------------------------------------*/

static void Resize( PLStream *pls )
{
  wingcc_Dev *dev=(wingcc_Dev *)pls->dev;
#ifdef HAVE_FREETYPE
  FT_Data *FT=(FT_Data *)pls->FT;
#endif

 	if (dev->waiting==1)     /* Only resize the window IF plplot has finished with it */
  	{
   	GetClientRect(dev->hwnd,&dev->rect);
   	if ((dev->rect.right>0)&&(dev->rect.bottom>0))
        {
          dev->width=dev->rect.right;
          dev->height=dev->rect.bottom;
          if (dev->width>dev->height)           /* Work out the scaling factor for the  */
            {                                   /* "virtual" (oversized) page           */
              dev->scale=(PLFLT)PIXELS_X/dev->width;
            }
          else
            {
              dev->scale=(PLFLT)PIXELS_Y/dev->height;
            }

#ifdef HAVE_FREETYPE
          if (FT)
            {
              FT->scale=dev->scale;
              FT->ymax=dev->height;
            }
#endif

          RedrawWindow(dev->hwnd,NULL,NULL,RDW_INVALIDATE|RDW_ERASE);
        }
     }
}


/*--------------------------------------------------------------------------*\
 * int SetRegValue(char *key_name, char *key_word, char *buffer,int dwType, int size)
 *
 *  Function set the registry; if registary entry does not exist, it is
 *  created. Actually, the key is created before it is set just to make sure
 *  that is is there !
\*--------------------------------------------------------------------------*/

static int SetRegValue(char *key_name, char *key_word, char *buffer,int dwType, int size)
{
  int j=0;
  
  DWORD lpdwDisposition;
  HKEY hKey;
  
  j=RegCreateKeyEx(
                    HKEY_CURRENT_USER,
                    key_name,
                    0,                          /* reserved */
                    NULL,                       /* address of class string */
                    REG_OPTION_NON_VOLATILE,	   /* special options flag */
                    KEY_WRITE,                  /* desired security access */
                    NULL,                       /* address of key security structure */
                    &hKey,                      /* address of buffer for opened handle */
                    &lpdwDisposition            /* address of disposition value buffer */
                  );

  if (j==ERROR_SUCCESS)
    {
      RegSetValueEx(hKey, key_word, 0, dwType, buffer, size);
      RegCloseKey(hKey);
    }
  return(j);
}

/*--------------------------------------------------------------------------*\
 * int GetRegValue(char *key_name, char *key_word, char *buffer, int size)
 *
 *  Function reads the registry and gets a string value from it
 *  buffer must be allocated by the caller, and the size is given in the size
 *  paramater.
 *  Return code is 1 for success, and 0 for failure.
\*--------------------------------------------------------------------------*/

static int GetRegValue(char *key_name, char *key_word, char *buffer, int size)
{
  int ret=0;
  HKEY hKey;
  int dwType;
  int dwSize=size;

    if (RegOpenKeyEx(HKEY_CURRENT_USER, key_name, 0, KEY_READ, &hKey)==ERROR_SUCCESS)
      {
      if (RegQueryValueEx(hKey, key_word, 0, (LPDWORD)&dwType,buffer,(LPDWORD)&dwSize)==ERROR_SUCCESS)
        {
          ret=1;
        }
      RegCloseKey(hKey);
      }
  return(ret);
}

#ifdef HAVE_FREETYPE

/*----------------------------------------------------------------------*\
 *  void plD_pixel_wingcc (PLStream *pls, short x, short y)
 *
 *  callback function, of type "plD_pixel_fp", which specifies how a single
 *  pixel is set in the current colour.
\*----------------------------------------------------------------------*/

void plD_pixel_wingcc (PLStream *pls, short x, short y)
{
  wingcc_Dev *dev=(wingcc_Dev *)pls->dev;

   SetPixel(dev->hdc, x, y,dev->colour);

}

void plD_pixelV_wingcc (PLStream *pls, short x, short y)
{
  wingcc_Dev *dev=(wingcc_Dev *)pls->dev;

   SetPixelV(dev->hdc, x, y,dev->colour);

}


/*----------------------------------------------------------------------*\
 *  void init_freetype_lv1 (PLStream *pls)
 *
 *  "level 1" initialisation of the freetype library.
 *  "Level 1" initialisation calls plD_FreeType_init(pls) which allocates
 *  memory to the pls->FT structure, then sets up the pixel callback
 *  function.
\*----------------------------------------------------------------------*/

static void init_freetype_lv1 (PLStream *pls)
{
FT_Data *FT;
int x;
wingcc_Dev *dev=(wingcc_Dev *)pls->dev;

plD_FreeType_init(pls);

FT=(FT_Data *)pls->FT;


/*
 *  Work out if our device support "fast" pixel setting
 *  and if so, use that instead of "slow" pixel setting
 */

x=GetDeviceCaps(dev->hdc,RASTERCAPS);

if (x&&RC_BITBLT)
  FT->pixel= (plD_pixel_fp)plD_pixelV_wingcc;
else
  FT->pixel= (plD_pixel_fp)plD_pixel_wingcc;


}

/*----------------------------------------------------------------------*\
 *  void init_freetype_lv2 (PLStream *pls)
 *
 *  "Level 2" initialisation of the freetype library.
 *  "Level 2" fills in a few setting that aren't public until after the
 *  graphics sub-syetm has been initialised.
 *  The "level 2" initialisation fills in a few things that are defined
 *  later in the initialisation process for the GD driver.
 *
 *  FT->scale is a scaling factor to convert co-ordinates. This is used by
 *  the GD and other drivers to scale back a larger virtual page and this
 *  eliminate the "hidden line removal bug". Set it to 1 if your device
 *  doesn't have scaling.
 *
 *  Some coordinate systems have zero on the bottom, others have zero on
 *  the top. Freetype does it one way, and most everything else does it the
 *  other. To make sure everything is working ok, we have to "flip" the
 *  coordinates, and to do this we need to know how big in the Y dimension
 *  the page is, and whether we have to invert the page or leave it alone.
 *
 *  FT->ymax specifies the size of the page FT->invert_y=1 tells us to
 *  invert the y-coordinates, FT->invert_y=0 will not invert the
 *  coordinates.
\*----------------------------------------------------------------------*/

static void init_freetype_lv2 (PLStream *pls)
{
wingcc_Dev *dev=(wingcc_Dev *)pls->dev;
FT_Data *FT=(FT_Data *)pls->FT;

FT->scale=dev->scale;
FT->ymax=dev->height;
FT->invert_y=1;

if (FT->want_smooth_text==1)    /* do we want to at least *try* for smoothing ? */
   {
    FT->ncol0_org=pls->ncol0;                                   /* save a copy of the original size of ncol0 */
    FT->ncol0_xtra=16777216-(pls->ncol1+pls->ncol0);            /* work out how many free slots we have */
    FT->ncol0_width=max_number_of_grey_levels_used_in_text_smoothing;              /* find out how many different shades of anti-aliasing we can do */
    FT->ncol0_width=max_number_of_grey_levels_used_in_text_smoothing;                 /* set a maximum number of shades */
    plscmap0n(FT->ncol0_org+(FT->ncol0_width*pls->ncol0));      /* redefine the size of cmap0 */
/* the level manipulations are to turn off the plP_state(PLSTATE_CMAP0)
 * call in plscmap0 which (a) leads to segfaults since the GD image is
 * not defined at this point and (b) would be inefficient in any case since
 * setcmap is always called later (see plD_bop_png) to update the driver
 * color palette to be consistent with cmap0. */
         {
          PLINT level_save;
          level_save = pls->level;
          pls->level = 0;
          pl_set_extended_cmap0(pls, FT->ncol0_width, FT->ncol0_org); /* call the function to add the extra cmap0 entries and calculate stuff */
          pls->level = level_save;
         }
        FT->smooth_text=1;      /* Yippee ! We had success setting up the extended cmap0 */
      }
}

#endif

#else
int
pldummy_wingcc()
{
    return(0);
}

#endif				/* PLD_wingccdev */
