/*
win3.cpp

  Driver for Win32

	Paul Casteels	11-Sep-1996	(casteels@uia.ua.ac.be)
	
	  Color Handling by
	
		Carla Carmelo Rosa  (l36658@alfa.ist.utl.pt)
		
		
  Modified 20/12/01 by Olof Svensson and Alexandre Gobbo
			
*/

#include "plDevs.h"
#ifdef PLD_win3

#include <stdio.h>
#include <string.h>
#include "plplotP.h"
#include "drivers.h"
#include "resource.h"
#include <windows.h>
#include <assert.h>

static int       color   = 1;
static unsigned int      hwnd = 0;
static unsigned int      buffered = 1;



static MSG       msg;
static DrvOpt    win3_options[] = {
	{"color", DRV_INT, &color, "Use color (color=0|1)"},
	{"hwnd", DRV_INT, &hwnd, "Windows HWND handle (not supposed to be given as a command line argument)"},
	{"buffered", DRV_INT, &buffered, "Sets buffered operation"},
	{NULL, DRV_INT, NULL, NULL}
};

LRESULT CALLBACK _export PlPlotWndProc (HWND hwnd,UINT message,UINT wParam,LONG lParam);

static const char szPlPlotClass[] = "PlplotClass";
static const char szPlPlotWName[] = "Plplot Window";

/* Function prototypes */

void plD_init_win3(PLStream *pls);
void plD_line_win3(PLStream *pls, short x1a, short y1a, short x2a, short y2a);
void plD_polyline_win3(PLStream *pls, short *xa, short *ya, PLINT npts);
void plD_eop_win3(PLStream *pls);
void plD_bop_win3(PLStream *pls);
void plD_tidy_win3(PLStream *pls);
void plD_state_win3(PLStream *pls, PLINT op);
void plD_esc_win3(PLStream *pls, PLINT op , void *ptr);
void FillPolygonCmd (PLStream *pls);
void plD_DrawImage_win3(PLStream *pls);
static void imageops(PLStream *pls, PLINT *ptr);
/* BOOL CALLBACK AbortProc( HDC hDC, int Error ); */

void plD_dispatch_init_win3	( PLDispatchTable *pdt )
{
	pdt->pl_MenuStr  = "PLplot Win32 Window";
	pdt->pl_DevName  = "win3";
	pdt->pl_type     = plDevType_Interactive;
	pdt->pl_seq      = 10;
	pdt->pl_init     = (plD_init_fp)     plD_init_win3;
	pdt->pl_line     = (plD_line_fp)     plD_line_win3;
	pdt->pl_polyline = (plD_polyline_fp) plD_polyline_win3;
	pdt->pl_eop      = (plD_eop_fp)      plD_eop_win3;
	pdt->pl_bop      = (plD_bop_fp)      plD_bop_win3;
	pdt->pl_tidy     = (plD_tidy_fp)     plD_tidy_win3;
	pdt->pl_state    = (plD_state_fp)    plD_state_win3;
	pdt->pl_esc      = (plD_esc_fp)      plD_esc_win3;
}

typedef struct {
	HWND	hwnd;
	HMENU 	hMenu;
	LOGPEN 	lp;
	LOGBRUSH lb;
	HPEN	hPen, hPenOld;
	HDC 	hdc;
	HPALETTE 	hpal;		
	HBRUSH	hbr, hbrOld;

	float 	xScale,yScale;
	int     xPhMax,yPhMax;
	int 	nextPlot; 	
	int 	rePaint;          /* if the background is cleared we need a repaint */
	int 	rePaintBsy;       /* if we are repainting block the rest */
	int     externalWindow;   /* if true the window is provided externally */
	
        int	write_to_window;	/* Set if plotting direct to window */
        int	write_to_pixmap;	/* Set if plotting to pixmap */

	int 	newCursor, button, state;
	float 	cursorX,cursorY;

	HBITMAP db_bmp;		
	HDC		db_hdc;		
	long	PenColor;
	int		PenWidth;
	long    backGroundColor;

        bool    isDead;	
} WinDev;

/*
static HDC GetPrinterDC(void)
{
PRINTDLG pdlg;
BOOLEAN ok;

  // Initialize the PRINTDLG structure
  memset( &pdlg, 0, sizeof( PRINTDLG ) );
  pdlg.lStructSize = sizeof( PRINTDLG );
  // Set the flag to return printer DC
  pdlg.Flags = PD_RETURNDC;

    // Invoke the printer dialog box
    ok = PrintDlg( &pdlg );
    // hDC member of the PRINTDLG structure contains
    // the printer DC
    return pdlg.hDC;
	}
*/

/*--------------------------------------------------------------------------*\
* Provide a solution to the problem of temporary files (AM)
\*--------------------------------------------------------------------------*/

extern "C" {

FILE *tmpfile( void )
{
   FILE *outfile ;
   char *string  ;
   char *ptemp   ;
   char  buffer[100] ;
   char  buffer2[100] ;

   ptemp = getenv( "TEMP" ) ;
   if ( ptemp == NULL )
   {
      ptemp = getenv( "TMP" ) ;
   }
   if ( ptemp == NULL )
   {
      ptemp = "C:" ;
   }
   string  = tmpnam( buffer ) ;
   strcpy( buffer2, ptemp ) ;
   strcat( buffer2, string ) ;
   outfile = fopen( buffer2, "wb+" ) ;

   return outfile ;
}

}

/*--------------------------------------------------------------------------*\
* Initialize device.
\*--------------------------------------------------------------------------*/


void plD_init_win3(PLStream *pls)
{
	HWND      hwndMain;
	WNDCLASS  wndclass;
	HINSTANCE hInstance;
	WinDev    *dev;
	int       greyvalue;
	char      *ptitle;
	//long      backGroundColor;
	
	/* Initial window position */
	int xPos    = 100;
	int yPos    = 100;
	
	/* Initial window size */
	int nWidth  = 720;
	int nHeight = 540;
	
	int xmin = 0;
	int xmax = PIXELS_X-1;
	int ymin = 0;
	int ymax = PIXELS_Y-1;
	
	color = 1;
	hwnd = 0;
	pls->color = 1;		/* Is a color device */
	plParseDrvOpts(win3_options);
	if (!color) pls->color = 0; /* But user does not want color */
	
	/* Set up device parameters */
	pls->termin      = 1; /* is an interactive terminal */
	pls->icol0       = 1; /* current color */
	pls->width       = 1; /* current pen width */
	pls->bytecnt     = 0;
	pls->page        = 0;
	if (buffered)
		pls->plbuf_write = 1; /* buffer the output */
	else
		pls->plbuf_write = 0;
	pls->dev_flush   = 1; /* flush as we like */
	pls->dev_fill0   = 1;	
	pls->dev_fastimg = 1; /* is a fast image device */
	pls->dev_xor     = 1; /* device support xor mode */
	if (pls->dev != NULL) delete pls->dev;
	pls->dev = new WinDev;
	assert(pls->dev != NULL);
	
	dev = (WinDev *) pls->dev;
	dev->nextPlot = 0;
	dev->write_to_window = 1;
	dev->write_to_pixmap = 0;
	dev->PenColor=RGB(pls->cmap0[0].r,pls->cmap0[0].g,pls->cmap0[0].b);
	dev->PenWidth=0;
	
	dev->hPen     = CreatePen(PS_SOLID,dev->PenWidth,dev->PenColor);
	dev->hPenOld = (HPEN)SelectObject(dev->hdc,dev->hPen);
	dev->hbr      = CreateSolidBrush(RGB(pls->cmap0[0].r,pls->cmap0[0].g,pls->cmap0[0].b));
	dev->hbrOld   = (HBRUSH)SelectObject(dev->hdc,dev->hbr);
	dev->hMenu    = NULL;

        dev->isDead   = FALSE;
	
	if (pls->color) {
		dev->backGroundColor = RGB(pls->cmap0[0].r,pls->cmap0[0].g,pls->cmap0[0].b);
	} else {
		greyvalue = (pls->cmap0[0].r+pls->cmap0[0].g+pls->cmap0[0].b)/3;
		dev->backGroundColor = RGB(greyvalue,greyvalue,greyvalue);
	}

	if (!hwnd) {
		/* Window created by the driver */
		dev->externalWindow = 0;
		hInstance = GetModuleHandle(NULL);
		
		wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_SAVEBITS;
		wndclass.lpfnWndProc = ::PlPlotWndProc;
		wndclass.cbClsExtra = 0;
		wndclass.cbWndExtra = sizeof(pls);
		wndclass.hInstance = hInstance;
		wndclass.hIcon = LoadIcon(hInstance,"PLICON");
		wndclass.hCursor = LoadCursor(NULL,IDC_ARROW);
        wndclass.hbrBackground = (struct HBRUSH__ *)CreateSolidBrush(dev->backGroundColor);
		wndclass.lpszMenuName = NULL;
		wndclass.lpszClassName = szPlPlotClass;
		RegisterClass (&wndclass);
		ptitle = (char *) &szPlPlotWName[0] ;
		if ( pls->plwindow ) ptitle = pls->plwindow ;
		
		dev->hwnd = CreateWindow(szPlPlotClass,ptitle,
			WS_OVERLAPPEDWINDOW,
			xPos,yPos,nWidth,nHeight,
			NULL,dev->hMenu,
			hInstance,NULL);
		
		SetWindowLong(dev->hwnd,GWL_USERDATA,(long)pls);
		
		ShowWindow(dev->hwnd,SW_SHOWDEFAULT);
		
		SetForegroundWindow(dev->hwnd);
		
	} else {
		/* Window provided externally */		
		dev->hwnd = (HWND)hwnd;
		dev->externalWindow = 1;
	}


	dev->hdc = GetDC(dev->hwnd);

	SetPolyFillMode(dev->hdc,WINDING);
	
	plP_setpxl(xmax/150.0/nWidth*nHeight,ymax/150.0);
	plP_setphy(xmin,xmax,ymin,ymax);


	if (pls->db)
	{
     	// create a compatible device context
     	dev->db_hdc = CreateCompatibleDC(dev->hdc);
     	dev->db_bmp = CreateCompatibleBitmap(dev->hdc, nWidth,nHeight);
		SelectObject(dev->db_hdc, dev->db_bmp);
		dev->hdc=dev->db_hdc;
	}
}


void setPen(PLStream *pls)
{
	WinDev *dev = (WinDev *) pls->dev;
	SelectObject(dev->hdc, dev->hPenOld);
	SelectObject(dev->hdc, dev->hbrOld);
	DeleteObject(dev->hPen);
	DeleteObject(dev->hbr);
	dev->lp.lopnColor=dev->PenColor;
	dev->lb.lbColor=dev->PenColor;
	dev->hPen = CreatePen(PS_SOLID,dev->PenWidth,dev->PenColor);
	dev->hbr= CreateSolidBrush(dev->PenColor);
	dev->hPenOld = (HPEN)SelectObject(dev->hdc,dev->hPen);
	dev->hbrOld   = (HBRUSH)SelectObject(dev->hdc,dev->hbr);

}

/*--------------------------------------------------------------------------*\
* setColor()
*
* Change the color of the pen and the brush
\*--------------------------------------------------------------------------*/


void setColor(PLStream *pls, int r, int g, int b)
{
	WinDev *dev = (WinDev *) pls->dev;
	dev->PenColor=RGB(r,g,b);
	setPen(pls);
}

/*--------------------------------------------------------------------------*\
* plD_state_win3()
*
* Handle change in PLStream state (color, pen width, fill attribute, etc).
\*--------------------------------------------------------------------------*/


void plD_state_win3(PLStream *pls, PLINT op)
{
	int cores, greyvalue;
	HPEN oldPen;
	
	switch(op) {

	case PLSTATE_WIDTH:	
		{
			WinDev *dev = (WinDev *) pls->dev;
			dev->PenWidth=pls->width;
			setPen(pls);
		}
		break;

	case PLSTATE_COLOR0:
		if ( ! pls->color ) {
			if ((pls->cmap0[0].r+pls->cmap0[0].g+pls->cmap0[0].b)/3 > 128) {
				if (pls->icol0) setColor(pls,0,0,0);
				else            setColor(pls,255,255,255);
			} else {
				if (pls->icol0) setColor(pls,255,255,255);
				else            setColor(pls,0,0,0);
			}
			break;
		}
		/* else fallthrough */
	case PLSTATE_COLOR1:
	        if (pls->color) {
			setColor(pls,pls->curcolor.r,pls->curcolor.g,pls->curcolor.b);
	        } else {
			greyvalue = (pls->curcolor.r+pls->curcolor.g+pls->curcolor.b)/3;
			setColor(pls,greyvalue,greyvalue,greyvalue);
		}
		break;

    case PLSTATE_CMAP0:
		{
			WinDev *dev = (WinDev *) pls->dev;
			if (pls->color) {
				dev->backGroundColor = RGB(pls->cmap0[0].r,pls->cmap0[0].g,pls->cmap0[0].b);
			} else {
				int greyvalue = (pls->cmap0[0].r+pls->cmap0[0].g+pls->cmap0[0].b)/3;
				dev->backGroundColor = RGB(greyvalue,greyvalue,greyvalue);
			}
		}
		break;

	}
}
/*--------------------------------------------------------------------------*\
* plD_line_win3()
*
* Draw a line in the current color from (x1,y1) to (x2,y2).
\*--------------------------------------------------------------------------*/
void plD_line_win3(PLStream *pls, short x1a, short y1a, short x2a, short y2a)
{
	WinDev *dev = (WinDev *)pls->dev;
	int    xpixb ;
	int    ypixb ;
	int    xpixe ;
	int    ypixe ;
	xpixb = x1a * dev->xScale ;
	ypixb = (PIXELS_Y - y1a) * dev->yScale ;
	xpixe = x2a * dev->xScale ;
	ypixe = (PIXELS_Y - y2a) * dev->yScale ;
	if ( xpixb == xpixe && ypixb == ypixe ) {
		SetPixel(dev->hdc,xpixb,ypixb,dev->PenColor);
	} else {
		MoveToEx(dev->hdc,xpixb,ypixb,NULL);
		LineTo(dev->hdc,xpixe,ypixe);
	}
	
/* Was:
		MoveToEx(dev->hdc,x1a * dev->xScale,(PIXELS_Y - y1a) * dev->yScale,NULL);
		LineTo(dev->hdc,x2a * dev->xScale,(PIXELS_Y - y2a) * dev->yScale);
*/
}

/*--------------------------------------------------------------------------*\
* plD_polyline_win3()
*
* Draw a polyline in the current color.
\*--------------------------------------------------------------------------*/
void plD_polyline_win3(PLStream *pls, short *xa, short *ya, PLINT npts)
{
	WinDev *dev = (WinDev *)pls->dev;
	POINT pt[PL_MAXPOLY];
	int i;
	
	if (npts > PL_MAXPOLY)
		plexit("FillPolygonCmd : Too many points in polygon\n");
	for (i=0; i < npts;i++) {
		pt[i].x = xa[i] * dev->xScale;
		pt[i].y = (PIXELS_Y - ya[i]) * dev->yScale;
	}
	Polyline(dev->hdc,pt,npts);
}
/*--------------------------------------------------------------------------*\
* plD_eop_win3()
*
* End of page.
\*--------------------------------------------------------------------------*/
void plD_eop_win3(PLStream *pls)
{
	WinDev *dev = (WinDev *)pls->dev;
	HCURSOR hCursor;
	if (!pls->db)
		ReleaseDC(dev->hwnd,dev->hdc);
	if (!dev->externalWindow) {
	       /* EnableMenuItem(dev->hMenu,CM_PRINTPLOT,MF_ENABLED); */
	       /* EnableMenuItem(dev->hMenu,CM_NEXTPLOT,MF_ENABLED);  */
	       hCursor = LoadCursor(NULL,IDC_ARROW);
		   SetClassLong(GetActiveWindow(),GCL_HCURSOR,(long)hCursor);
		   SetCursor(hCursor);
		   while (!dev->nextPlot && !dev->isDead) {
			   GetMessage(&msg,NULL,0,0);
			   TranslateMessage(&msg);
			   DispatchMessage(&msg);
		   }
	}
	if (!pls->db)
	{
		InvalidateRect(dev->hwnd,NULL,TRUE);
		//UpdateWindow(dev->hwnd);
		PAINTSTRUCT ps;
		HDC winDC;
		winDC = BeginPaint(dev->hwnd, &ps);
		EndPaint(dev->hwnd,  &ps);

	}
	else
	{
		RECT rect;
		GetClientRect(dev->hwnd,&rect);
		HBRUSH hbr = CreateSolidBrush(dev->backGroundColor);
		FillRect(dev->hdc, &rect,hbr);
	}


	dev->nextPlot = 0;
}




/*--------------------------------------------------------------------------*\
* plD_bop_win3()
*
* Set up for the next page.
* Advance to next family file if necessary (file output).
\*--------------------------------------------------------------------------*/
void plD_bop_win3(PLStream *pls)
{
	WinDev *dev = (WinDev *)pls->dev;
	RECT rect;
	HCURSOR hCursor;
	
	/*	EnableMenuItem(dev->hMenu,CM_PRINTPLOT,MF_GRAYED); */
	/*	EnableMenuItem(dev->hMenu,CM_NEXTPLOT,MF_GRAYED);  */
	if (!dev->externalWindow) {
		hCursor = LoadCursor(NULL,IDC_WAIT);
		SetClassLong(GetActiveWindow(),GCL_HCURSOR,(long)hCursor);
		SetCursor(hCursor);
	}

	if (pls->db)
		dev->hdc = dev->db_hdc;
	else
		dev->hdc = GetDC(dev->hwnd);

	
	GetClientRect(dev->hwnd,&rect);
	dev->xPhMax = rect.right;
	dev->yPhMax = rect.bottom;
	dev->xScale = rect.right / ((float)PIXELS_X);
	dev->yScale = rect.bottom / ((float)PIXELS_Y);
	
	dev->rePaint = 0;
	dev->rePaintBsy = 0;
	pls->page++;
	
	plD_state_win3(pls, PLSTATE_COLOR0); /* Set drawing color */
}

/*--------------------------------------------------------------------------*\
* plD_tidy_win3()
*
* Close graphics file or otherwise clean up.
\*--------------------------------------------------------------------------*/
void plD_tidy_win3(PLStream *pls)
{
	WinDev *dev = (WinDev *)pls->dev;
	
	pls->page = 0;
	pls->OutFile = NULL;
	if (!dev->externalWindow) DestroyWindow(dev->hwnd);
}



/*--------------------------------------------------------------------------*\
* plD_esc_win3()
*
* Escape function.
\*--------------------------------------------------------------------------*/

#define DPMM 4.0

void plD_esc_win3(PLStream *pls, PLINT op , void *ptr)
{
	WinDev *dev = (WinDev *)pls->dev;
	HCURSOR holdcursor,hnewcursor;
	
	switch (op) {

	case PLESC_GETC:
		hnewcursor = LoadCursor(NULL,IDC_CROSS);
		holdcursor = (HCURSOR)GetClassLong(GetActiveWindow(),GCL_HCURSOR);
		SetClassLong(GetActiveWindow(),GCL_HCURSOR,(long)hnewcursor);
		SetCursor(hnewcursor);
		dev->newCursor = 0;
		dev->button = 0;
		dev->state = 0;
		while (!dev->newCursor) {
			GetMessage(&msg,NULL,0,0);
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		((PLGraphicsIn *)ptr)->dX = float(dev->cursorX)/float(PIXELS_X*dev->xScale);
		((PLGraphicsIn *)ptr)->dY = float(PIXELS_Y*dev->yScale - dev->cursorY) / float(PIXELS_Y*dev->yScale);
		((PLGraphicsIn *)ptr)->button = dev->button;
		((PLGraphicsIn *)ptr)->state = dev->state;
		SetClassLong(GetActiveWindow(),GCL_HCURSOR,(long)holdcursor);
		break;

	case PLESC_FILL:
		FillPolygonCmd(pls);
		break;

	case PLESC_XORMOD:
		if (*(PLINT*)(ptr) == 0)
			SetROP2(dev->hdc,R2_COPYPEN);
		else
			SetROP2(dev->hdc,R2_XORPEN);
		break;
		
	case PLESC_REDRAW:
		break;
		
	case PLESC_RESIZE:
		{
			RECT rect;
			dev->hdc = GetDC(dev->hwnd);
			GetClientRect(dev->hwnd,&rect);
			dev->xPhMax = rect.right;
			dev->yPhMax = rect.bottom;
			dev->xScale = rect.right / ((float)PIXELS_X);
			dev->yScale = rect.bottom / ((float)PIXELS_Y);
			

			{
			PLFLT pxlx = DPMM/dev->xScale;
			PLFLT pxly = DPMM/dev->yScale;
			plP_setpxl(pxlx, pxly);
			}


			if (pls->db)
			{
				//SetBitmapDimensionEx(dev->db_bmp,rect.right,rect.bottom,NULL);
				DeleteObject(dev->db_bmp);
		     	dev->db_bmp = CreateCompatibleBitmap(dev->hdc, rect.right,rect.bottom);
				SelectObject(dev->db_hdc, dev->db_bmp);
				dev->hdc = dev->db_hdc;
				HBRUSH hbr = CreateSolidBrush( dev->backGroundColor);
				FillRect(dev->hdc, &rect,hbr);
				plreplot ();
			}
			else
				InvalidateRect(dev->hwnd,NULL,TRUE);
			
		}
		break;
		
	case PLESC_EXPOSE:
		if (pls->db)
		{
			PAINTSTRUCT ps;
			HDC winDC;
			RECT r;

			// Set up a display context to begin painting
			winDC = BeginPaint(dev->hwnd, &ps);
			GetClientRect(dev->hwnd, &r);

     		// copy the buffer to the screen
     		BitBlt(GetDC(dev->hwnd), 0, 0, r.right, r.bottom, dev->db_hdc, 0, 0, SRCCOPY);

			// Tell Windows you are done painting
			EndPaint(dev->hwnd,  &ps);
		}
		else
			plreplot ();
		break;
		
        case PLESC_IMAGE:
                plD_DrawImage_win3(pls);
                break;

        case PLESC_IMAGEOPS:
                imageops(pls, (PLINT *) ptr);
                break;
	}
}

/*-------------------------------------------------------------*\
* FillPolygonCmd()
\*-------------------------------------------------------------*/
static void FillPolygonCmd(PLStream *pls) {
	POINT pt[PL_MAXPOLY];
	int i;
	WinDev *dev = (WinDev *)pls->dev;
	if (pls->dev_npts > PL_MAXPOLY)
		plexit("FillPolygonCmd : Too many points in polygon\n");
	for (i=0; i < pls->dev_npts;i++) {
		pt[i].x = pls->dev_x[i] * dev->xScale;
		pt[i].y = (PIXELS_Y - pls->dev_y[i]) * dev->yScale;
	}
	Polygon(dev->hdc,pt,pls->dev_npts);
}

/*--------------------------------------------------------------------------*\
 * plD_DrawImage_win3()
 *
 * Experimental! Currently only works on 32-bit displays
\*--------------------------------------------------------------------------*/


void plD_DrawImage_win3(PLStream *pls)
{
  WinDev *dev = (WinDev *) pls->dev;
  HDC hdcMemory;

  HBITMAP bitmap, bitmapOld;
  BYTE    *byteArray;
	
  int     byteArrayXsize, byteArrayYsize;
  int     imageX0, imageY0, imageWX, imageWY;
  int     image_kx_start, image_ky_start, image_kx_end, image_ky_end, image_wkx, image_wky;
  int     imageXmin, imageXmax, imageYmin, imageYmax;

  int i, npts, nx, ny, ix, iy, corners[5], Ppts_x[5], Ppts_y[5];
  int clpxmi, clpxma, clpymi, clpyma, icol1;

  int level;
  float wcxmi, wcxma, wcymi, wcyma;
  long ptr1, ptr2;

  clpxmi = plsc->imclxmin;
  clpxma = plsc->imclxmax;
  clpymi = plsc->imclymin;
  clpyma = plsc->imclymax;

  //  printf("clpxmi %d %d %d %d\n", clpxmi, clpxma, clpymi, clpyma);

  wcxmi = (clpxmi - pls->wpxoff)/pls->wpxscl;
  wcxma = (clpxma - pls->wpxoff)/pls->wpxscl;
  wcymi = (clpymi - pls->wpyoff)/pls->wpyscl;
  wcyma = (clpyma - pls->wpyoff)/pls->wpyscl;

  npts = plsc->dev_nptsX*plsc->dev_nptsY;

  nx = pls->dev_nptsX;
  ny = pls->dev_nptsY;

  imageXmin = pls->dev_ix[0];
  imageYmin = pls->dev_iy[0];
  imageXmax = pls->dev_ix[nx*ny-1];
  imageYmax = pls->dev_iy[nx*ny-1];

  //  printf("imageXmin %d %d %d %d\n", imageXmin, imageXmax, imageYmin, imageYmax);

  if (clpxmi > imageXmin) {
    imageX0 = dev->xScale * clpxmi+1;
    image_kx_start = (int)wcxmi;
  } else {
    imageX0 = dev->xScale * imageXmin+1;
    image_kx_start = 0;
  }

  if (clpxma < imageXmax) {
    imageWX = dev->xScale*clpxma-imageX0;
    image_kx_end = (int)wcxma;
    image_wkx =  image_kx_end-image_kx_start+1;
  } else {
    imageWX = dev->xScale * imageXmax - imageX0;
    image_kx_end = image_kx_start+nx;
    image_wkx = nx;
  }

  if (clpymi > imageYmin) {
    imageY0 = dev->yScale*(PIXELS_Y-clpyma)+1;
    image_ky_start = ny - 1 - (int)wcyma;
  } else {
    imageY0 = dev->yScale * (PIXELS_Y - imageYmax)+1;
    image_ky_start = 0;
  }

  if (clpyma < imageYmax) {
    imageWY = dev->yScale*(PIXELS_Y-clpymi)-imageY0;
    image_ky_end = ny -1 - (int)(wcymi);
    image_wky = image_ky_end-image_ky_start+1;
  } else {
    imageWY = dev->yScale * (PIXELS_Y - imageYmin) - imageY0;
    image_ky_end = ny - 1 - image_ky_start;
    image_wky = ny;
  }

  //  printf("imageX0 %d %d %d %d\n", imageX0, imageY0, imageWX, imageWY);
  //  printf("kx %d %d %d %d\n", image_kx_start, image_kx_end, image_ky_start, image_ky_end);
  //  printf("Before malloc... %d %d \n", nx, ny);
  byteArray = (BYTE*)malloc(nx*ny*sizeof(BYTE)*4);
  //  printf("After malloc...\n");

  for (ix=0; ix<nx; ix++)
    if ((ix >= image_kx_start) || (ix <= image_kx_end))
      {
	ptr1 = 4*ix;
	for (iy=0; iy<ny; iy++)
	  if ((iy >= image_ky_start) || (iy <= image_ky_end))
	  {
	    icol1 = pls->dev_z[ix*(ny-1)+iy]/65535.*pls->ncol1;
	    icol1 = MIN(icol1, pls->ncol1-1);
	    ptr2 = ptr1+4*(ny-iy-1)*nx;
	    //	    printf("%d  ", ptr2);
	    *(BYTE*)(byteArray+sizeof(BYTE)*ptr2++) = pls->cmap1[icol1].b;
	    *(BYTE*)(byteArray+sizeof(BYTE)*ptr2++) = pls->cmap1[icol1].g;
	    *(BYTE*)(byteArray+sizeof(BYTE)*ptr2++) = pls->cmap1[icol1].r;
	    *(BYTE*)(byteArray+sizeof(BYTE)*ptr2) = 255;
	  }
      }
  //  printf("Before CreateCompatibleBitmap...\n");
  bitmap = CreateCompatibleBitmap(dev->hdc, nx, ny);
  SetBitmapBits(bitmap, 4*npts, (const void*)byteArray);

  //  printf("Before CreateCompatibleDC...\n");
  hdcMemory = CreateCompatibleDC(dev->hdc);
  bitmapOld = (HBITMAP)SelectObject(hdcMemory, bitmap);
  SetStretchBltMode(dev->hdc, HALFTONE);
  //  printf("%d %d %d %d %d %d %d %d\n",imageX0, imageY0, imageWX, imageWY, image_kx_start, image_ky_start, image_wkx, image_wky);
  StretchBlt(dev->hdc, imageX0, imageY0, imageWX, imageWY, hdcMemory, image_kx_start, image_ky_start, image_wkx, image_wky, SRCCOPY);
  SelectObject(hdcMemory, bitmapOld);
  DeleteObject(bitmap);
  ReleaseDC(dev->hwnd,hdcMemory);				
  free(byteArray);
}

static void imageops(PLStream *pls, PLINT *ptr)
{

  WinDev *dev = (WinDev *) pls->dev;

  /* TODO: store/revert to/from previous state */

  switch (*ptr) {
  case ZEROW2D:
    dev->write_to_window = 0;
    break;

  case ONEW2D:
    dev->write_to_window = 1;
    break;

  case ZEROW2B:
    dev->write_to_pixmap = 0;
    break;

  case ONEW2B:
    //    XFlush(xwd->display);
    dev->write_to_pixmap = 1;
    break;
  }
}

LRESULT CALLBACK __declspec(dllexport) PlPlotWndProc (HWND hwnd,UINT message,
	UINT wParam,LONG lParam)
{
	RECT rect;
	PAINTSTRUCT ps;
	PLStream *pls = (PLStream *)GetWindowLong(hwnd,GWL_USERDATA);
	WinDev *dev = NULL;
	HCURSOR hcurSave;
	HMETAFILE hmf;
	GLOBALHANDLE hGMem;
	LPMETAFILEPICT lpMFP;
	
	if (pls)
		dev = (WinDev *)pls->dev;
	
	switch (message) {
	case WM_CHAR :
		if ( wParam == VK_RETURN ) {
			dev->nextPlot = 1 ;
		}
		return 0;
	case WM_LBUTTONDOWN :
		dev->newCursor = 1;
		dev->cursorX = LOWORD(lParam);
		dev->cursorY = HIWORD(lParam);
		dev->button = 1;
		dev->state = 0x000;
		return 0;
	case WM_RBUTTONDOWN :
		dev->newCursor = 1;
		dev->cursorX = LOWORD(lParam);
		dev->cursorY = HIWORD(lParam);
		dev->button = 3;
		if (dev) dev->nextPlot = 1;
		dev->state = 0x000;
		return 0;
	case WM_MOUSEMOVE :
		if ((wParam & MK_LBUTTON) || (wParam & MK_RBUTTON)) {
			dev->newCursor = 1;
			dev->cursorX = LOWORD(lParam);
			dev->cursorY = HIWORD(lParam);
			dev->state = 0x100;
			return 0;
		}
		break;
	case WM_ERASEBKGND :
		if (!dev->rePaintBsy)
			dev->rePaint = 1;
		break;
	case WM_PAINT :
		if (dev) {
			/* if (dev->rePaint) { */
			if (1) {
			        HDC hdc_old = dev->hdc;
				dev->rePaint = 0;
				dev->rePaintBsy = 1;
				hcurSave = SetCursor(LoadCursor(NULL,IDC_WAIT));
				dev->hdc = GetDC(dev->hwnd);
				GetClientRect(dev->hwnd,&rect);
				dev->xPhMax = rect.right;
				dev->yPhMax = rect.bottom;
				dev->xScale = rect.right / ((float)PIXELS_X);
				dev->yScale = rect.bottom / ((float)PIXELS_Y);
				plRemakePlot(pls);
				dev->rePaintBsy = 0;
				SetCursor(hcurSave);
				ReleaseDC(dev->hwnd,dev->hdc);
 	                        dev->hdc = hdc_old;
				plD_state_win3(pls, PLSTATE_COLOR0); /* Set drawing color */
			}
			BeginPaint(hwnd,&ps);
			EndPaint(hwnd,&ps);
			return 0;
		}
		break;
	case WM_DESTROY :
		//              PostQuitMessage(0);
	        dev->isDead = TRUE;
		return 0;
		/*
		case WM_COMMAND :
		switch (wParam) {
		case  CM_NEXTPLOT :
		if (dev)
		dev->nextPlot = 1;
		return 0;
		case CM_PRINTPLOT :
		dev->rePaintBsy = 1;
		if (dev->hdc = GetPrinterDC()) {
		dev->xScale = GetDeviceCaps(dev->hdc,HORZRES) / ((float)PIXELS_X);
		dev->yScale = GetDeviceCaps(dev->hdc,VERTRES) / ((float)PIXELS_Y);
		#ifdef WIN32
		DOCINFO di;
		di.cbSize = sizeof(DOCINFO);
		di.lpszDocName = "Plplot - Print";
		di.lpszOutput = NULL;
		di.lpszDatatype = NULL;
		di.fwType = NULL;
		if( SetAbortProc( dev->hdc, (int(__stdcall *)(struct HDC__ *,int))AbortProc ) == SP_ERROR ) {
		MessageBox( NULL, "Error setting up AbortProc",
		"Error", MB_APPLMODAL | MB_OK);
		break;
		}
		StartDoc(dev->hdc,&di);
		StartPage(dev->hdc);
		hcurSave = SetCursor(LoadCursor(NULL,IDC_WAIT));
		plRemakePlot(pls);
		EndPage(dev->hdc);
		EndDoc(dev->hdc);
		#else
		Escape(dev->hdc,STARTDOC,0,NULL,NULL);
		hcurSave = SetCursor(LoadCursor(NULL,IDC_WAIT));
		plRemakePlot(pls);
		Escape(dev->hdc,NEWFRAME,0,NULL,NULL);
		Escape(dev->hdc,ENDDOC,0,NULL,NULL);
		#endif
		SetCursor(hcurSave);
		DeleteDC(dev->hdc);
		}
		dev->rePaintBsy = 0;
		dev->rePaint = 1;
		return 0;
		case CM_EDITCOPY :
		dev->rePaintBsy = 1;
		dev->hdc = CreateMetaFile(NULL);
		SetWindowExtEx(dev->hdc,PIXELS_X,PIXELS_Y,NULL);
		SetWindowOrgEx(dev->hdc,0,0,NULL);
		dev->xScale = 1.0;
		dev->yScale = 1.0;
		hcurSave = SetCursor(LoadCursor(NULL,IDC_WAIT));
		plRemakePlot(pls);
		SetCursor(hcurSave);
		
		  hmf = CloseMetaFile(dev->hdc);
		  hGMem = GlobalAlloc(GHND,(DWORD)sizeof(METAFILEPICT));
		  lpMFP = (LPMETAFILEPICT) GlobalLock(hGMem);
		
			lpMFP->mm = MM_ISOTROPIC;
			lpMFP->xExt = PIXELS_X;
			lpMFP->yExt = PIXELS_Y;
			lpMFP->hMF = hmf;
			
			  GlobalUnlock(hGMem);
			
				OpenClipboard(dev->hwnd);
				EmptyClipboard();
				SetClipboardData(CF_METAFILEPICT,hGMem);
				CloseClipboard();
				
				  dev->rePaintBsy = 0;
				  dev->rePaint = 1;
				  return 0;
				  case CM_ABOUT :
				  //			 MessageBox(hwnd,aboutText,"About",MB_OK);
				  return 0;
				
					}
		*/
  }
  return DefWindowProc(hwnd,message,wParam,lParam);
}

/*
BOOL CALLBACK AbortProc( HDC hDC, int Error )
{
MSG   msg;
while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
{
TranslateMessage( &msg );
DispatchMessage( &msg );
}
return TRUE;
}
*/
#else
pldummy_win3() {
	return 0;
}
#endif   //WIN3
