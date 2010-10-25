#define __PLSTUBS_H__
#include "plplot.h"
#include <math.h>
#include <windows.h>

void *h_pldll;

extern "C" int APIENTRY
  DllMain(HINSTANCE hInstance,DWORD dwReason,LPVOID lpReserved) {

  if (dwReason == DLL_PROCESS_ATTACH) {
//          MessageBox(NULL,"dll init","TDLL",MB_OK);
	  h_pldll = hInstance;
  } else if (dwReason == DLL_PROCESS_DETACH) {
//          MessageBox(NULL,"dll done","TDLL",MB_OK);
  }
  return 1;
}
/*--------------------------------------------------------------------------*\
 *              Function Prototypes
\*--------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

	/* All void types */

	/* C routines callable from stub routines come first */

/* Advance to subpage "page", or to the next one if "page" = 0. */

void WINAPI
pladv(PLINT page) {
	c_pladv(page);
}

/* Allocate a 2D grid */

void WINAPI
win_plAlloc2dGrid(PLFLT ***f, PLINT nx, PLINT ny) {
	plAlloc2dGrid(f, nx, ny);
}

/* Frees a block of memory allocated with plAlloc2dGrid(). */

void WINAPI
win_plFree2dGrid(PLFLT **f, PLINT nx, PLINT ny) {
	plFree2dGrid(f, nx, ny);
}

/* This functions similarly to plbox() except that the origin of the axes */
/* is placed at the user-specified point (x0, y0). */

void WINAPI
plaxes(PLFLT x0, PLFLT y0, const char *xopt, PLFLT xtick,
	   PLINT nxsub, const char *yopt, PLFLT ytick,
	   PLINT nysub) {
	c_plaxes(x0,y0,xopt,xtick,nxsub,yopt,ytick,nysub);
}

/* Plot a histogram using x to store data values and y to store frequencies */

void WINAPI
plbin(PLINT nbin, PLFLT *x, PLFLT *y, PLINT center) {
	c_plbin(nbin,x,y,center);
}

/* Start new page.  Should only be used with pleop(). */

void WINAPI
plbop(void) {
	c_plbop();
}

/* This draws a box around the current viewport. */

void WINAPI
plbox(const char *xopt, PLFLT xtick, PLINT nxsub,
	const char *yopt, PLFLT ytick, PLINT nysub) {
	c_plbox(xopt,xtick,nxsub,yopt,ytick,nysub);
}

/* This is the 3-d analogue of plbox(). */

void WINAPI
plbox3(const char *xopt, const char *xlabel, PLFLT xtick, PLINT nsubx,
	 const char *yopt, const char *ylabel, PLFLT ytick, PLINT nsuby,
	 const char *zopt, const char *zlabel, PLFLT ztick, PLINT nsubz) {
	c_plbox3(xopt,xlabel,xtick,nsubx,
		yopt,ylabel,ytick,nsuby,
		zopt,zlabel,ztick,nsubz);
}

/* Set color, map 0.  Argument is integer between 0 and 15. */

void WINAPI
plcol0(PLINT icol0) {
	c_plcol0(icol0);
}

/* Set color, map 1.  Argument is a float between 0. and 1. */

void WINAPI
plcol1(PLFLT col1) {
	c_plcol1(col1);
}

/* Draws a contour plot from data in f(nx,ny).  Is just a front-end to
 * plfcont, with a particular choice for f2eval and f2eval_data.
 */

void WINAPI
plcont(PLFLT **f, PLINT nx, PLINT ny, PLINT kx, PLINT lx,
	 PLINT ky, PLINT ly, PLFLT *clevel, PLINT nlevel,
	 void (*pltr) (PLFLT, PLFLT, PLFLT *, PLFLT *, PLPointer),
	 PLPointer pltr_data) {
	c_plcont(f,nx,ny,kx,lx,
		ky,ly,clevel,nlevel,
		pltr,pltr_data);
}

/* Copies state parameters from the reference stream to the current stream. */

void WINAPI
plcpstrm(PLINT iplsr, PLINT flags) {
	c_plcpstrm(iplsr,flags);
}

/* End a plotting session for all open streams. */

void WINAPI
plend(void) {
	c_plend();
}

/* End a plotting session for the current stream only. */

void WINAPI
plend1(void) {
	plend1();
}

/* Simple interface for defining viewport and window. */

void WINAPI
plenv(PLFLT xmin, PLFLT xmax, PLFLT ymin, PLFLT ymax,
	PLINT just, PLINT axis) {
	c_plenv(xmin,xmax,ymin,ymax,
		just,axis);
}

/* End current page.  Should only be used with plbop(). */

void WINAPI
pleop(void) {
	c_pleop();
}

/* Plot horizontal error bars (xmin(i),y(i)) to (xmax(i),y(i)) */

void WINAPI
plerrx(PLINT n, PLFLT *xmin, PLFLT *xmax, PLFLT *y) {
	c_plerrx(n,xmin,xmax,y);
}

/* Plot vertical error bars (x,ymin(i)) to (x(i),ymax(i)) */

void WINAPI
plerry(PLINT n, PLFLT *x, PLFLT *ymin, PLFLT *ymax) {
	c_plerry(n,x,ymin,ymax);
}

/* Advance to the next family file on the next new page */

void WINAPI
plfamadv(void) {
	c_plfamadv();
}

/* Pattern fills the polygon bounded by the input points. */

void WINAPI
plfill(PLINT n, PLFLT *x, PLFLT *y) {
	c_plfill(n,x,y);
}

/* Flushes the output stream.  Use sparingly, if at all. */

void WINAPI
plflush(void) {
	c_plflush();
}

/* Sets the global font flag to 'ifont'. */

void WINAPI
plfont(PLINT ifont) {
	c_plfont(ifont);
}

/* Load specified font set. */

void WINAPI
plfontld(PLINT fnt) {
	c_plfontld(fnt);
}

/* Get character default height and current (scaled) height */

void WINAPI
plgchr(PLFLT *p_def, PLFLT *p_ht) {
	c_plgchr(p_def,p_ht);
}

/* Returns 8 bit RGB values for given color from color map 0 */

void WINAPI
plgcol0(PLINT icol0, PLINT *r, PLINT *g, PLINT *b) {
	c_plgcol0(icol0,r,g,b);
}

/* Returns the background color by 8 bit RGB value */

void WINAPI
plgcolbg(PLINT *r, PLINT *g, PLINT *b) {
	c_plgcolbg(r,g,b);
}

/* Retrieve current window into device space */

void WINAPI
plgdidev(PLFLT *p_mar, PLFLT *p_aspect, PLFLT *p_jx, PLFLT *p_jy) {
	c_plgdidev(p_mar,p_aspect,p_jx,p_jy);
}

/* Get plot orientation */

void WINAPI
plgdiori(PLFLT *p_rot) {
	c_plgdiori(p_rot);
}

/* Wait for graphics input event and translate to world coordinates */

void WINAPI
win_plGetCursor(PLGraphicsIn *gin ) {
	plGetCursor(gin);
}

/* Retrieve current window into plot space */

void WINAPI
plgdiplt(PLFLT *p_xmin, PLFLT *p_ymin, PLFLT *p_xmax, PLFLT *p_ymax) {
	c_plgdiplt(p_xmin,p_ymin,p_xmax,p_ymax);
}

/* Get family file parameters */

void WINAPI
plgfam(PLINT *p_fam, PLINT *p_num, PLINT *p_bmax) {
	c_plgfam(p_fam,p_num,p_bmax);
}

/* Get the (current) output file name.  Must be preallocated to >80 bytes */

void WINAPI
plgfnam(char *fnam) {
	c_plgfnam(fnam);
}

/* Get output device parameters. */

void WINAPI
plgpage(PLFLT *p_xp, PLFLT *p_yp,
	  PLINT *p_xleng, PLINT *p_yleng, PLINT *p_xoff, PLINT *p_yoff) {
	c_plgpage(p_xp,p_yp,
		p_xleng,p_yleng,p_xoff,p_yoff);
}

/* Switches to graphics screen. */

void WINAPI
plgra(void) {
	c_plgra();
}

/* Get subpage boundaries in absolute coordinates */

void WINAPI
plgspa(PLFLT *xmin, PLFLT *xmax, PLFLT *ymin, PLFLT *ymax) {
	plgspa(xmin,xmax,ymin,ymax);
}

/* Get current stream number. */

void WINAPI
plgstrm(PLINT *p_strm) {
	c_plgstrm(p_strm);
}

/* Get the current library version number */

void WINAPI
plgver(char *p_ver) {
	c_plgver(p_ver);
}

/* Get x axis labeling parameters */

void WINAPI
plgxax(PLINT *p_digmax, PLINT *p_digits) {
	c_plgxax(p_digmax,p_digits);
}

/* Get y axis labeling parameters */

void WINAPI
plgyax(PLINT *p_digmax, PLINT *p_digits) {
	c_plgyax(p_digmax,p_digits);
}

/* Get z axis labeling parameters */

void WINAPI
plgzax(PLINT *p_digmax, PLINT *p_digits) {
	c_plgzax(p_digmax,p_digits);
}

/* Draws a histogram of n values of a variable in array data[0..n-1] */

void WINAPI
plhist(PLINT n, PLFLT *data, PLFLT datmin, PLFLT datmax,
	 PLINT nbin, PLINT oldwin) {
	c_plhist(n,data,datmin,datmax,
		nbin,oldwin);
}

/* Set current color (map 0) by hue, lightness, and saturation. */

void WINAPI
plhls(PLFLT h, PLFLT l, PLFLT s) {
	c_plhls(h,l,s);
}

/* Initializes PLplot, using preset or default options */

void WINAPI
plinit(void) {
	c_plinit();
}

/* Draws a line segment from (x1, y1) to (x2, y2). */

void WINAPI
pljoin(PLFLT x1, PLFLT y1, PLFLT x2, PLFLT y2) {
	c_pljoin(x1,y1,x2,y2);
}

/* Simple routine for labelling graphs. */

void WINAPI
pllab(const char *xlabel, const char *ylabel, const char *tlabel) {
	c_pllab(xlabel,ylabel,tlabel);
}

/* Draws line segments connecting a series of points. */

void WINAPI
plline(PLINT n, PLFLT *x, PLFLT *y) {
	c_plline(n,x,y);
}

/* Draws a line in 3 space.  */

void WINAPI
plline3(PLINT n, PLFLT *x, PLFLT *y, PLFLT *z) {
	c_plline3(n,x,y,z);
}

/* Set line style. */

void WINAPI
pllsty(PLINT lin) {
	c_pllsty(lin);
}

/* Merge user option table into internal info structure. */

int WINAPI
win_plMergeOpts(PLOptionTable *options, char *name, char **notes) {
    return plMergeOpts( options, name, notes) ;
}


void WINAPI
plmesh(PLFLT *x, PLFLT *y, PLFLT **z, PLINT nx, PLINT ny, PLINT opt) {
	c_plmesh(x,y,z,nx,ny,opt);
}

/* Creates a new stream and makes it the default.  */

void WINAPI
plmkstrm(PLINT *p_strm) {
	c_plmkstrm(p_strm);
}

/* Prints out "text" at specified position relative to viewport */

void WINAPI
plmtex(const char *side, PLFLT disp, PLFLT pos, PLFLT just,
	 const char *text) {
	c_plmtex(side,disp,pos,just,
		text);
}

/* Plots a 3-d representation of the function z[x][y]. */

void WINAPI
plot3d(PLFLT *x, PLFLT *y, PLFLT **z,
	 PLINT nx, PLINT ny, PLINT opt, PLINT side) {
	c_plot3d(x,y,z,nx,ny,opt,side);
}

/* Process options list using current options info. */

int WINAPI
win_plParseOpts(int *p_argc, char **argv, PLINT mode) {
    return plParseOpts(p_argc, argv, mode) ;
}

/* Set fill pattern directly. */

void WINAPI
plpat(PLINT nlin, PLINT *inc, PLINT *del) {
	c_plpat(nlin,inc,del);
}

/* Plots array y against x for n points using ASCII code "code".*/

void WINAPI
plpoin(PLINT n, PLFLT *x, PLFLT *y, PLINT code) {
	c_plpoin(n,x,y,code);
}

/* Draws a series of points in 3 space. */

void WINAPI
plpoin3(PLINT n, PLFLT *x, PLFLT *y, PLFLT *z, PLINT code) {
	c_plpoin3(n,x,y,z,code);
}

/* Draws a polygon in 3 space.  */

void WINAPI
plpoly3(PLINT n, PLFLT *x, PLFLT *y, PLFLT *z, PLINT *draw, PLINT ifcc) {
	c_plpoly3(n,x,y,z,draw,ifcc);
}

/* Set the floating point precision (in number of places) in numeric labels. */

void WINAPI
plprec(PLINT setp, PLINT prec) {
	c_plprec(setp,prec);
}

/* Set fill pattern, using one of the predefined patterns.*/

void WINAPI
plpsty(PLINT patt) {
	c_plpsty(patt);
}

/* Prints out "text" at world cooordinate (x,y). */

void WINAPI
plptex(PLFLT x, PLFLT y, PLFLT dx, PLFLT dy, PLFLT just, const char *text) {
	c_plptex(x,y,dx,dy,just,text);
}

/* Replays contents of plot buffer to current device/file. */

void WINAPI
plreplot(void) {
	c_plreplot();
}

/* Set line color by red, green, blue from  0. to 1. */

void WINAPI
plrgb(PLFLT r, PLFLT g, PLFLT b) {
	c_plrgb(r,g,b);
}

/* Set line color by 8 bit RGB values. */

void WINAPI
plrgb1(PLINT r, PLINT g, PLINT b) {
	c_plrgb1(r,g,b);
}

/* Set character height. */

void WINAPI
plschr(PLFLT def, PLFLT scale) {
	c_plschr(def,scale);
}

/* Set number of colors in cmap 0 */

void WINAPI
plscmap0n(PLINT ncol0) {
	c_plscmap0n(ncol0);
}

/* Set number of colors in cmap 1 */

void WINAPI
plscmap1n(PLINT ncol1) {
	c_plscmap1n(ncol1);
}

/* Set color map 0 colors by 8 bit RGB values */

void WINAPI
plscmap0(PLINT *r, PLINT *g, PLINT *b, PLINT ncol0) {
	c_plscmap0(r,g,b,ncol0);
}

/* Set color map 1 colors by 8 bit RGB values */

void WINAPI
plscmap1(PLINT *r, PLINT *g, PLINT *b, PLINT ncol1) {
	c_plscmap1(r,g,b,ncol1);
}

/* Set color map 1 colors using a piece-wise linear relationship between */
/* intensity [0,1] (cmap 1 index) and position in HLS or RGB color space. */

void WINAPI
plscmap1l(PLINT itype, PLINT npts, PLFLT *intensity,
	    PLFLT *coord1, PLFLT *coord2, PLFLT *coord3, PLINT *rev) {
	c_plscmap1l(itype,npts,intensity,coord1,coord2,coord3,rev);
}

/* Set a given color from color map 0 by 8 bit RGB value */

void WINAPI
plscol0(PLINT icol0, PLINT r, PLINT g, PLINT b) {
	c_plscol0(icol0,r,g,b);
}

/* Set the background color by 8 bit RGB value */

void WINAPI
plscolbg(PLINT r, PLINT g, PLINT b) {
	c_plscolbg(r,g,b);
}

/* Used to globally turn color output on/off */

void WINAPI
plscolor(PLINT color) {
	c_plscolor(color);
}

/* Set the device (keyword) name */

void WINAPI
plsdev(const char *devname) {
	c_plsdev(devname);
}

/* Set window into device space using margin, aspect ratio, and */
/* justification */

void WINAPI
plsdidev(PLFLT mar, PLFLT aspect, PLFLT jx, PLFLT jy) {
	c_plsdidev(mar,aspect,jx,jy);
}

/* Set up transformation from metafile coordinates. */

void WINAPI
plsdimap(PLINT dimxmin, PLINT dimxmax, PLINT dimymin, PLINT dimymax,
	   PLFLT dimxpmm, PLFLT dimypmm) {
	c_plsdimap(dimxmin,dimxmax,dimymin,dimymax,
		dimxpmm,dimypmm);
}

/* Set plot orientation, specifying rotation in units of pi/2. */

void WINAPI
plsdiori(PLFLT rot) {
	c_plsdiori(rot);
}

/* Set window into plot space */

void WINAPI
plsdiplt(PLFLT xmin, PLFLT ymin, PLFLT xmax, PLFLT ymax) {
	c_plsdiplt(xmin,ymin,xmax,ymax);
}

/* Set window into plot space incrementally (zoom) */

void WINAPI
plsdiplz(PLFLT xmin, PLFLT ymin, PLFLT xmax, PLFLT ymax) {
	c_plsdiplz(xmin,ymin,xmax,ymax);
}

/* Set the escape character for text strings. */

void WINAPI
plsesc(char esc) {
	c_plsesc(esc);
}

/* Set family file parameters */

void WINAPI
plsfam(PLINT fam, PLINT num, PLINT bmax) {
	c_plsfam(fam,num,bmax);
}

/* Set the output file name. */

void WINAPI
plsfnam(const char *fnam) {
	c_plsfnam(fnam);
}

/* Shade region. */
typedef int  DefineFuncPtr( PLFLT, PLFLT ) ;
typedef void *FillFuncPtr(PLINT, PLFLT *, PLFLT *) ;
typedef void *PlotFuncPtr(PLFLT, PLFLT, PLFLT *, PLFLT *, PLPointer) ;

void  WINAPI
plshade(PLFLT **a, PLINT nx, PLINT ny, DefineFuncPtr defined,
	  PLFLT left, PLFLT right, PLFLT bottom, PLFLT top,
	  PLFLT shade_min, PLFLT shade_max,
	  PLINT sh_cmap, PLFLT sh_color, PLINT sh_width,
	  PLINT min_color, PLINT min_width,
	  PLINT max_color, PLINT max_width,
	  FillFuncPtr fill, PLINT rectangular, PlotFuncPtr pltr,
	  PLPointer pltr_data) {
	c_plshade(a,nx,ny,defined,
		left,right,bottom,top,
		shade_min,shade_max,
		sh_cmap,sh_color,sh_width,
		min_color,min_width,
		max_color,max_width,
		(void (*) (PLINT, PLFLT *, PLFLT *))fill,rectangular,
		(void (*) (PLFLT, PLFLT, PLFLT *, PLFLT *, PLPointer))pltr,
		pltr_data);
}

void WINAPI
plshade1(PLFLT *a, PLINT nx, PLINT ny, DefineFuncPtr defined,
	 PLFLT left, PLFLT right, PLFLT bottom, PLFLT top,
	 PLFLT shade_min, PLFLT shade_max,
	 PLINT sh_cmap, PLFLT sh_color, PLINT sh_width,
	 PLINT min_color, PLINT min_width,
	 PLINT max_color, PLINT max_width,
	 FillFuncPtr fill, PLINT rectangular,
	 PlotFuncPtr pltr,
	 PLPointer pltr_data) {
	c_plshade1(a,nx,ny,defined,
	 left,right,bottom,top,
	 shade_min,shade_max,
	 sh_cmap,sh_color,sh_width,
	 min_color,min_width,
	 max_color,max_width,
	 (void (*) (PLINT, PLFLT *, PLFLT *))fill,rectangular,
	 (void (*) (PLFLT, PLFLT, PLFLT *, PLFLT *, PLPointer))pltr,
	 pltr_data);
}


/* Set up lengths of major tick marks. */

void WINAPI
plsmaj(PLFLT def, PLFLT scale) {
	 c_plsmaj(def,scale);
}

/* Set up lengths of minor tick marks. */

void WINAPI
plsmin(PLFLT def, PLFLT scale) {
	c_plsmin(def,scale);
}

/* Set orientation.  Must be done before calling plinit. */

void WINAPI
plsori(PLINT ori) {
	c_plsori(ori);
}

/* Set output device parameters.  Usually ignored by the driver. */

void WINAPI
plspage(PLFLT xp, PLFLT yp, PLINT xleng, PLINT yleng,
	  PLINT xoff, PLINT yoff) {
	c_plspage(xp,yp,xleng,yleng,
		xoff,yoff);
}

/* Set the pause (on end-of-page) status */

void WINAPI
plspause(PLINT pause) {
	c_plspause(pause);
}

/* Set stream number.  */

void WINAPI
plsstrm(PLINT strm) {
	c_plsstrm(strm);
}

/* Set the number of subwindows in x and y */

void WINAPI
plssub(PLINT nx, PLINT ny) {
	c_plssub(nx,ny);
}

/* Set symbol height. */

void WINAPI
plssym(PLFLT def, PLFLT scale) {
	c_plssym(def,scale);
}

/* Initialize PLplot, passing in the windows/page settings. */

void WINAPI
plstar(PLINT nx, PLINT ny) {
	c_plstar(nx,ny);
}

/* Initialize PLplot, passing the device name and windows/page settings. */

void WINAPI
plstart(const char *devname, PLINT nx, PLINT ny) {
	c_plstart(devname,nx,ny);
}

/* Set up a new line style */

void WINAPI
plstyl(PLINT nms, PLINT *mark, PLINT *space) {
	c_plstyl(nms,mark,space);
}

void WINAPI
plsvect(PLFLT *arrowx, PLFLT *arrowy, PLINT npts, PLINT fill) {
	c_plsvect(arrowx, arrowy, npts, fill);
}

/* Sets the edges of the viewport to the specified absolute coordinates */

void WINAPI
plsvpa(PLFLT xmin, PLFLT xmax, PLFLT ymin, PLFLT ymax) {
	c_plsvpa(xmin,xmax,ymin,ymax);
}

/* Set x axis labeling parameters */

void WINAPI
plsxax(PLINT digmax, PLINT digits) {
	c_plsxax(digmax,digits);
}

/* Set y axis labeling parameters */

void WINAPI
plsyax(PLINT digmax, PLINT digits) {
	c_plsyax(digmax,digits);
}

/* Plots array y against x for n points using Hershey symbol "code" */

void WINAPI
plsym(PLINT n, PLFLT *x, PLFLT *y, PLINT code) {
	c_plsym(n,x,y,code);
}

/* Set z axis labeling parameters */

void WINAPI
plszax(PLINT digmax, PLINT digits) {
	c_plszax(digmax,digits);
}

/* Switches to text screen. */

void WINAPI
pltext(void) {
	c_pltext();
}

void WINAPI plvect(PLFLT **u, PLFLT **v, PLINT nx, PLINT ny, PLFLT scale,
              void (*pltr) (PLFLT, PLFLT, PLFLT *, PLFLT *, PLPointer),
              PLPointer pltr_data) {
	c_plvect(u, v, nx, ny, scale, pltr, pltr_data);
}

/* Sets the edges of the viewport with the given aspect ratio, leaving */
/* room for labels. */

void WINAPI
plvasp(PLFLT aspect) {
	c_plvasp(aspect);
}

/* Creates the largest viewport of the specified aspect ratio that fits */
/* within the specified normalized subpage coordinates. */

void WINAPI
plvpas(PLFLT xmin, PLFLT xmax, PLFLT ymin, PLFLT ymax, PLFLT aspect) {
	c_plvpas(xmin,xmax,ymin,ymax,aspect);
}

/* Creates a viewport with the specified normalized subpage coordinates. */

void WINAPI
plvpor(PLFLT xmin, PLFLT xmax, PLFLT ymin, PLFLT ymax) {
	c_plvpor(xmin,xmax,ymin,ymax);
}

/* Defines a "standard" viewport with seven character heights for */
/* the left margin and four character heights everywhere else. */

void WINAPI
plvsta(void) {
	c_plvsta();
}

/* Set up a window for three-dimensional plotting. */

void WINAPI
plw3d(PLFLT basex, PLFLT basey, PLFLT height, PLFLT xmin0,
	PLFLT xmax0, PLFLT ymin0, PLFLT ymax0, PLFLT zmin0,
	PLFLT zmax0, PLFLT alt, PLFLT az) {
	c_plw3d(basex,basey,height,xmin0,
		xmax0,ymin0,ymax0,zmin0,
		zmax0,alt,az);
}

/* Set pen width. */

void WINAPI
plwid(PLINT width) {
	c_plwid(width);
}

/* Set up world coordinates of the viewport boundaries (2d plots). */

void WINAPI
plwind(PLFLT xmin, PLFLT xmax, PLFLT ymin, PLFLT ymax) {
	c_plwind(xmin,xmax,ymin,ymax);
}

#ifdef __cplusplus
}
#endif
