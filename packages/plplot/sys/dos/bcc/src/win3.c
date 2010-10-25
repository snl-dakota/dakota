/*
  win3.c

  Paul Casteels
  09-Sep-1994
  casteels@uia.ac.be

  A driver for Windows 3.x

*/
#include "plplot/plDevs.h"
#ifdef PLD_win3

#include <stdio.h>
#include <string.h>
#include "plplot/plplotP.h"
#include "plplot/drivers.h"
#include <windows.h>
#include <commdlg.h>
//#define NDEBUG
#include <assert.h>
#include "plplot/plplot.rc"

#define TEXT_MODE 0
#define GRAPHICS_MODE 1


#define CLEAN 0
#define DIRTY 1

const float xRes = 8192,yRes = 8192;
const int niceCnt = 20;
const char szPlPlotClass[] = "PlplotClass";
const char szPlPlotWName[] = "PlPlot Window";
const char aboutText[] = "        Plplot    "PLPLOT_VERSION
			 "\nComments/Questions to\n"
			 "plplot-list@dino.ph.utexas.edu\n";

LRESULT CALLBACK _export PlPlotWndProc (HWND hwnd,UINT message,
  UINT wParam,LONG lParam);

typedef struct {
  HWND hwnd;
  HMENU hMenu;
  HPEN hPen;
  HDC hdc;
  float xScale,yScale;
  int xPhMax,yPhMax;
  int nextPlot; // set to 1 by Nextplot menu
  int nice;                     // be nice for windows multitasking
  int rePaint;          // if the background is cleared we need a repaint
  int rePaintBsy;       // if we are repainting block the rest
			// plRemakePlot is not reentrant (in Windows)?
  int newCursor;
  float cursorX,cursorY;
} WinDev;

MSG msg;
long colors[16]={
  RGB(0,0,0),           // 0 = black
  RGB(0,0,255), // 1 = blue
  RGB(0,255,0), // 2 = green
  RGB(0,255,255),       // 3 = cyan
  RGB(255,0,0), // 4 = red
  RGB(255,0,255),       // 5 = magenta
  RGB(255,255,0),       // 6 = yellow
  RGB(255,255,255),     // 7 = white
  RGB(0,0,0),
  RGB(0,0,0),
  RGB(0,0,0),
  RGB(0,0,0),
  RGB(0,0,0),
  RGB(0,0,0),
  RGB(0,0,0),
  RGB(0,0,0)};

// Transfer control to windows
void checkMessage() {
  if (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
}

// Simplest way to connect to a printer
int GetPrinterDC (HWND hwnd,PRINTDLG *pd) {

  memset(pd, 0, sizeof(PRINTDLG));
  pd->lStructSize = sizeof(PRINTDLG);
  pd->hwndOwner = hwnd;
  pd->Flags = PD_RETURNDC;
  if (PrintDlg(pd) != 0)
    return 1;
  return 0;
}

/*----------------------------------------------------------------------*\
* Initialize device.
\*----------------------------------------------------------------------*/

void
plD_init_win3(PLStream *pls)
{
  HWND   hwndMain;
  WNDCLASS wndclass;
  HANDLE hInstance;

  WinDev *dev;
  int xPos,yPos;
  int nWidth,nHeight;

  pls->termin = 1;                       // is an interactive terminal
  pls->icol0 = 1;                        // current color
  pls->width = 1;                        // current pen width
  pls->bytecnt = 0;
  pls->page = 0;
  pls->plbuf_write = 1;  // buffer the output
  pls->dev_flush = 1;            // flush as we like

/* Set up device parameters */
  if (pls->dev != NULL)
    delete pls->dev;
  pls->dev = new WinDev;
  assert(pls->dev != NULL);
  dev = (WinDev *) pls->dev;
  dev->nextPlot = 0;
  dev->nice = 0;
  dev->hPen = CreatePen(PS_SOLID,0,colors[0]);

// Get/save this from plplot.ini ??
  xPos = 100;
  yPos = 100;
  nWidth = 600;
  nHeight = 400;

  hwndMain = GetActiveWindow();
  hInstance = GetWindowWord(hwndMain,GWW_HINSTANCE);
  dev->hMenu = LoadMenu(hInstance,"COMMANDS");

  wndclass.style = CS_HREDRAW | CS_VREDRAW;
  wndclass.lpfnWndProc = ::PlPlotWndProc;
  wndclass.cbClsExtra = 0;
  wndclass.cbWndExtra = sizeof(pls);
  wndclass.hInstance = hInstance;
  wndclass.hIcon = LoadIcon(hInstance,"PLICON");
  wndclass.hCursor = LoadCursor(NULL,IDC_ARROW);
  wndclass.hbrBackground = GetStockObject(WHITE_BRUSH);
  wndclass.lpszMenuName = NULL;
  wndclass.lpszClassName = szPlPlotClass;
  RegisterClass (&wndclass);

  dev->hwnd = CreateWindow(szPlPlotClass,szPlPlotWName,
		WS_OVERLAPPEDWINDOW,
		xPos,yPos,nWidth,nHeight,
		NULL,dev->hMenu,
		hInstance,NULL);

  SetWindowLong(dev->hwnd,0,(long)pls);

  ShowWindow(dev->hwnd,SW_SHOW);

  plP_setpxl(2.5,2.5);           /* Pixels/mm. */
  plP_setphy(0,xRes,0,yRes);
}

/*----------------------------------------------------------------------*\
* line()
*
* Draw a line in the current color from (x1,y1) to (x2,y2).
\*----------------------------------------------------------------------*/

void
plD_line_win3(PLStream *pls, short x1a, short y1a, short x2a, short y2a)
{
  WinDev *dev = (WinDev *)pls->dev;
  SelectObject(dev->hdc,dev->hPen);
  MoveTo(dev->hdc,x1a * dev->xScale,(yRes - y1a) * dev->yScale);
  LineTo(dev->hdc,x2a * dev->xScale,(yRes - y2a) * dev->yScale);
  if (!dev->rePaintBsy)
    if (dev->nice++ > niceCnt) {
      dev->nice = 0;
      checkMessage();
    }
}

/*----------------------------------------------------------------------*\
* bgi_polyline()
*
* Draw a polyline in the current color.
\*----------------------------------------------------------------------*/

void
plD_polyline_win3(PLStream *pls, short *xa, short *ya, PLINT npts)
{
  WinDev *dev = (WinDev *)pls->dev;
  SelectObject(dev->hdc,dev->hPen);
  MoveTo(dev->hdc,xa[0] * dev->xScale,(yRes - ya[0]) * dev->yScale);
  for (int i=1;i<npts;i++) {
    LineTo(dev->hdc,xa[i] * dev->xScale,(yRes - ya[i]) * dev->yScale);
  }
  if (!dev->rePaintBsy) {
    dev->nice = 0;
    checkMessage();
  }
}

/*----------------------------------------------------------------------*\
* bgi_eop()
*
* End of page.
\*----------------------------------------------------------------------*/

void
plD_eop_win3(PLStream *pls)
{
  WinDev *dev = (WinDev *)pls->dev;
  HCURSOR hCursor;

  ReleaseDC(dev->hwnd,dev->hdc);
  EnableMenuItem(dev->hMenu,CM_PRINTPLOT,MF_ENABLED);
  EnableMenuItem(dev->hMenu,CM_NEXTPLOT,MF_ENABLED);
  
  hCursor = LoadCursor(NULL,IDC_ARROW);
  SetClassWord(GetActiveWindow(),GCW_HCURSOR,hCursor);
  SetCursor(hCursor);

  while (!dev->nextPlot) {
    GetMessage(&msg,NULL,0,0);
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  InvalidateRect(dev->hwnd,NULL,TRUE);
  UpdateWindow(dev->hwnd);
  dev->nextPlot = 0;
}

/*----------------------------------------------------------------------*\
* bop()
*
* Set up for the next page.
* Advance to next family file if necessary (file output).
\*----------------------------------------------------------------------*/

void
plD_bop_win3(PLStream *pls)
{
  WinDev *dev = (WinDev *)pls->dev;
  RECT rect;
  HCURSOR hCursor;

  EnableMenuItem(dev->hMenu,CM_PRINTPLOT,MF_GRAYED);
  EnableMenuItem(dev->hMenu,CM_NEXTPLOT,MF_GRAYED);

  hCursor = LoadCursor(NULL,IDC_WAIT);
  SetClassWord(GetActiveWindow(),GCW_HCURSOR,hCursor);
  SetCursor(hCursor);

  dev->hdc = GetDC(dev->hwnd);
  GetClientRect(dev->hwnd,&rect);

  dev->xPhMax = rect.right;
  dev->yPhMax = rect.bottom;

  dev->xScale = rect.right / (xRes + 1);
  dev->yScale = rect.bottom / (yRes + 1);

  dev->rePaint = 0;
  dev->rePaintBsy = 0;
  dev->nice = 0;
  pls->page++;
}

/*----------------------------------------------------------------------*\
* bgi_tidy()
*
* Close graphics file or otherwise clean up.
\*----------------------------------------------------------------------*/

void
plD_tidy_win3(PLStream *pls)
{
  WinDev *dev = (WinDev *)pls->dev;

  pls->page = 0;
  pls->OutFile = NULL;
  DestroyWindow(dev->hwnd);
}

void plD_state_win3(PLStream *pls,PLINT op) {
  WinDev *dev = (WinDev *)pls->dev;

  switch(op) {
  case PLSTATE_WIDTH:
    break;
  case PLSTATE_COLOR0:
    DeleteObject(dev->hPen);
    dev->hPen = CreatePen(PS_SOLID,0,colors[(int)pls->icol0]);
    break;
  }
}


/*----------------------------------------------------------------------*\
* bgi_esc()
*
* Escape function.
\*----------------------------------------------------------------------*/

void
plD_esc_win3(PLStream *pls, PLINT op , void *ptr)
{
  WinDev *dev = (WinDev *)pls->dev;
  HCURSOR hCursor;

  if (op == PLESC_GETC) {
    hCursor = LoadCursor(NULL,IDC_CROSS);
    SetClassWord(GetActiveWindow(),GCW_HCURSOR,hCursor);
    SetCursor(hCursor);

    dev->newCursor = 0;
    while (!dev->newCursor)
      checkMessage();
    ((PLGraphicsIn *)ptr)->dX = dev->cursorX;
    ((PLGraphicsIn *)ptr)->dY = dev->cursorY;
    hCursor = LoadCursor(NULL,IDC_ARROW);
    SetClassWord(GetActiveWindow(),GCW_HCURSOR,hCursor);
    SetCursor(hCursor);
  }
}

LRESULT CALLBACK _export PlPlotWndProc (HWND hwnd,UINT message,
  UINT wParam,LONG lParam)
{
  RECT rect;
  PAINTSTRUCT ps;
  PLStream *pls = (PLStream *)GetWindowLong(hwnd,0);
  WinDev *dev = NULL;
  HCURSOR hcurSave;
  PRINTDLG pd;
  HMETAFILE hmf;
  GLOBALHANDLE hGMem;
  LPMETAFILEPICT lpMFP;
  DOCINFO docinf = {sizeof(DOCINFO),"PlPlot",NULL};

  if (pls)
	 dev = (WinDev *)pls->dev;

  switch (message) {
	 case WM_LBUTTONDOWN :
		if (dev)
	dev->nextPlot = 1;
		return 0;
	 case WM_RBUTTONDOWN :
		dev->newCursor = 1;
		dev->cursorX = float(LOWORD(lParam)) / float(dev->xPhMax);
		dev->cursorY = float(dev->yPhMax - HIWORD(lParam))
				/ float(dev->yPhMax);
		return 0;
	 case WM_ERASEBKGND :
		if (!dev->rePaintBsy)
		dev->rePaint = 1;
		break;
	 case WM_PAINT :
		if (dev) {
	if (dev->rePaint) {
	  dev->rePaint = 0;
	  dev->rePaintBsy = 1;
	  hcurSave = SetCursor(LoadCursor(NULL,IDC_WAIT));
	  dev->hdc = GetDC(dev->hwnd);
	  GetClientRect(dev->hwnd,&rect);
	  dev->xPhMax = rect.right;
	  dev->yPhMax = rect.bottom;
	  dev->xScale = rect.right / (xRes + 1);
	  dev->yScale = rect.bottom / (yRes + 1);
	  plRemakePlot(pls);
	  dev->rePaintBsy = 0;
	  SetCursor(hcurSave);
	  ReleaseDC(dev->hwnd,dev->hdc);
	}
	BeginPaint(hwnd,&ps);
	EndPaint(hwnd,&ps);
	return 0;
		}
		break;
	 case WM_DESTROY :
//    PostQuitMessage(0);
		return 0;
	 case WM_COMMAND :
		switch (wParam) {
	case  CM_NEXTPLOT :
	  if (dev)
		 dev->nextPlot = 1;
	  return 0;
	case CM_PRINTPLOT :
	  dev->rePaintBsy = 1;
	  if (GetPrinterDC(dev->hwnd,&pd)) {
		 dev->hdc = pd.hDC;
		 dev->xScale = GetDeviceCaps(dev->hdc,HORZRES) / (xRes + 1);
		 dev->yScale = GetDeviceCaps(dev->hdc,VERTRES) / (yRes + 1);
		 StartDoc(dev->hdc,&docinf);
		 hcurSave = SetCursor(LoadCursor(NULL,IDC_WAIT));
		 StartPage(dev->hdc);
		 plRemakePlot(pls);
		 EndPage(dev->hdc);
		 EndDoc(dev->hdc);
		 SetCursor(hcurSave);
		 DeleteDC(pd.hDC);
		 if (pd.hDevMode != NULL)
			GlobalFree(pd.hDevMode);
		 if (pd.hDevNames != NULL)
			GlobalFree(pd.hDevNames);
	  }
	  dev->rePaintBsy = 0;
	  dev->rePaint = 1;
	  return 0;
	case CM_EDITCOPY :
	  dev->rePaintBsy = 1;
	  dev->hdc = CreateMetaFile(NULL);
	  SetWindowExt(dev->hdc,xRes,yRes);
	  SetWindowOrg(dev->hdc,0,0);
/*
  dev->xScale = GetDeviceCaps(dev->hdc,HORZRES) / (xRes + 1);
  dev->yScale = GetDeviceCaps(dev->hdc,VERTRES) / (yRes + 1);
*/
	  dev->xScale = 1.0;
	  dev->yScale = 1.0;
	  hcurSave = SetCursor(LoadCursor(NULL,IDC_WAIT));
	  plRemakePlot(pls);
	  SetCursor(hcurSave);

	  hmf = CloseMetaFile(dev->hdc);
	  hGMem = GlobalAlloc(GHND,(DWORD)sizeof(METAFILEPICT));
	  lpMFP = (LPMETAFILEPICT) GlobalLock(hGMem);

	  lpMFP->mm = MM_ISOTROPIC;
	  lpMFP->xExt = xRes;
	  lpMFP->yExt = yRes;
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
	  MessageBox(hwnd,aboutText,"About",MB_OK);
	  return 0;
      } //switch(wParam)
  } //switch(message)
  return DefWindowProc(hwnd,message,wParam,lParam);
}
#else
pldummy_win3() {
  return 0;
}
#endif //WIN3
