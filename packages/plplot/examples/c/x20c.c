/* $Id: x20c.c 3186 2006-02-15 18:17:33Z slbrow $

	plimage demo

*/

#include "plcdemos.h"
#include "plevent.h"
#include <math.h>

#define XDIM 260
#define YDIM 220

void save_plot(char *);
void gray_cmap(PLINT);
int read_img(char *, PLFLT ***, int *, int *, int *);
int get_clip(PLFLT *, PLFLT *, PLFLT *, PLFLT *);

int dbg = 0;
int nosombrero = 0;
int nointeractive = 0;
char *f_name = NULL;

static PLOptionTable options[] = {
{
    "dbg",			/* extra debugging plot */
    NULL,
    NULL,
    &dbg,
    PL_OPT_BOOL,
    "-dbg",
    "Extra debugging plot" },
{
    "nosombrero",			/* Turns on test of xor function */
    NULL,
    NULL,
    &nosombrero,
    PL_OPT_BOOL,
    "-nosombrero",
    "No sombrero plot" },
{
    "nointeractive",			/* Turns on test of xor function */
    NULL,
    NULL,
    &nointeractive,
    PL_OPT_BOOL,
    "-nointeractive",
    "No interactive selection" },
{
    "save",			/* For saving in postscript */
    NULL,
    NULL,
    &f_name,
    PL_OPT_STRING,
    "-save filename",
      "Save sombrero plot in color postscript `filename'" },
{
    NULL,			/* option */
    NULL,			/* handler */
    NULL,			/* client data */
    NULL,			/* address of variable to set */
    0,				/* mode flag */
    NULL,			/* short syntax */
    NULL }			/* long syntax */
};

int
main(int argc, char *argv[])
{
  PLFLT x[XDIM], y[YDIM], **z, **r;
  PLFLT xi, yi, xe, ye; 
  int i, j, width, height, num_col;
  PLFLT **img_f;

  /*
    Bugs in plimage():
     -at high magnifications, the left and right edge are ragged, try
        ./x20c -dev xwin -wplt 0.3,0.3,0.6,0.6 -ori 0.5
     
    Bugs in x20c.c:
     -if the window is resized after a selection is made on "lena", when
      making a new selection the old one will re-appear.
  */

  /* Parse and process command line arguments */

  plMergeOpts(options, "x20c options", NULL);
  plparseopts(&argc, argv, PL_PARSE_FULL);

  /* Initialize plplot */

  plinit();

  plAlloc2dGrid(&z, XDIM, YDIM);

  /* view image border pixels */
  if (dbg) { 
    plenv(1., (PLFLT) XDIM, 1., (PLFLT) YDIM, 1, 1); /* no plot box */
    
    /* build a one pixel square border, for diagnostics */
    for (i=0; i<XDIM; i++)
      z[i][YDIM-1] = 1.; /* right */
    for (i=0; i<XDIM; i++)
      z[i][0] = 1.; /* left */
  
    for (i=0; i<YDIM; i++)
      z[0][i] = 1.; /* top */
    for (i=0; i<YDIM; i++)
      z[XDIM-1][i] = 1.; /* botton */

    pllab("...around a blue square."," ","A red border should appear...");

    plimage(z, XDIM, YDIM,
	    1., (PLFLT) XDIM, 1., (PLFLT) YDIM, 0., 0.,
	    1., (PLFLT) XDIM, 1., (PLFLT) YDIM);

    pladv(0);
  }

  /* sombrero-like demo */
  if (!nosombrero) { 
    plAlloc2dGrid(&r, XDIM, YDIM);
    plcol0(2); /* draw a yellow plot box, useful for diagnostics! :( */
    plenv(0., 2.*PI, 0, 3.*PI, 1, -1);

    for (i=0; i<XDIM; i++)
      x[i] = i*2.*PI/(XDIM-1);
    for (i=0; i<YDIM; i++)
      y[i] = i*3.*PI/(YDIM-1);

    for (i=0; i<XDIM; i++)
      for (j=0; j<YDIM; j++) {
	r[i][j] = sqrt(x[i]*x[i]+y[j]*y[j])+1e-3;
	z[i][j] = sin(r[i][j]) / (r[i][j]);
      }

    pllab("No, an amplitude clipped \"sombrero\"", "", "Saturn?");
    plptex(2., 2., 3., 4., 0., "Transparent image");
    plimage(z, XDIM, YDIM, 0., 2.*PI, 0, 3.*PI, 0.05, 1.,
	    0., 2.*PI, 0, 3.*PI); 
    plFree2dGrid(r, XDIM, YDIM);

    /* save the plot */
    if (f_name)
      save_plot(f_name);

    pladv(0);
    }

  plFree2dGrid(z, XDIM, YDIM);

  /* read Lena image */
  if (read_img("lena.pgm", &img_f, &width, &height, &num_col)) {
    plabort("No such file");
    plend();
    exit(1);
  }

  /* set gray colormap */
  gray_cmap(num_col);

  /* display Lena */
  plenv(1., width, 1., height, 1, -1);

  if (!nointeractive)
    pllab("Set and drag Button 1 to (re)set selection, Button 2 to finish."," ","Lena...");
  else
    pllab(""," ","Lena...");

  plimage(img_f, width, height, 1., width, 1., height, 0., 0.,
	  1., width, 1., height);

  /* plend();exit(0); */

  /* selection/expansion demo */
  if (!nointeractive) { 
    xi = 200.; xe = 330.;
    yi = 280.; ye = 220.;

    if (get_clip(&xi, &xe, &yi, &ye)) { /* get selection rectangle */
      plend();
      exit(0);
    }
  
    /* 
       I'm unable to continue, clearing the plot and advancing to the next
       one, without hiting the enter key, or pressing the button... help!

       Forcing the xwin driver to leave locate mode and destroying the
       xhairs (in GetCursorCmd()) solves some problems, but I still have
       to press the enter key or press Button-2 to go to next plot, even
       if a pladv() is not present!  Using plbop() solves the problem, but
       it shouldn't be needed! 
    */

    /* plbop(); */

    /* 
       plspause(0), pladv(0), plspause(1), also works,
       but the above question remains.
       With this approach, the previous pause state is lost,
       as there is no API call to get its current state.
    */

    plspause(0);
    pladv(0);

    /* display selection only */
    plimage(img_f, width, height, 1., width, 1., height, 0., 0., xi, xe, ye, yi);

    plspause(1);
    pladv(0);

    /* zoom in selection */
    plenv(xi, xe, ye, yi, 1, -1);
    plimage(img_f, width, height, 1., width, 1., height, 0., 0., xi, xe, ye, yi);
    pladv(0);
  }

  plFree2dGrid(img_f, width, height);

  plend();
  exit(0);
}

/* read image from file in binary ppm format */
int read_img(char *fname, PLFLT ***img_f, int *width, int *height, int *num_col)
{
  FILE *fp;
  unsigned char *img;
  char ver[80];
  int i, j, w, h;
  PLFLT **imf;

  /* naive grayscale binary ppm reading. If you know how to, improve it */
  if ((fp = fopen(fname,"rb")) == NULL)
    return 1;

  fscanf(fp,"%s\n", ver); /* version */
  /* printf("version: %s\n", ver);*/

  if (strcmp(ver, "P5")) /* I only understand this! */
    return 1;

  while((i=fgetc(fp)) == '#') {
    fgets(ver, 80, fp); /* comments */
    /* printf("%s", ver); */
  }
  ungetc(i, fp);

  fscanf(fp,"%d%d%d", &w, &h, num_col); /* width, height num colors */
  /* printf("width=%d height=%d num_col=%d\n", w, h, *num_col); */

  img = (unsigned char *) malloc(w*h*sizeof(char));
  plAlloc2dGrid(&imf, w, h);

  fread(img, sizeof(char), w*h, fp);
  fclose(fp);

  for (i=0; i<w; i++)
    for (j=0; j<h; j++)
      imf[i][j] = img[(h-1-j)*w+i]; /* flip image up-down */
      
  free(img);

  *width = w;
  *height = h;
  *img_f = imf;
  return 0;
}

/* save plot */
void save_plot(char *fname)   
{
  PLINT cur_strm, new_strm; 

  plgstrm(&cur_strm); /* get current stream */
  plmkstrm(&new_strm); /* create a new one */ 
    
  plsdev("psc"); /* new device type. Use a known existing driver */
  plsfnam(fname); /* file name */

  plcpstrm(cur_strm, 0); /* copy old stream parameters to new stream */
  plreplot();	/* do the save */
  plend1(); /* close new device */

  plsstrm(cur_strm);	/* and return to previous one */
}

/*  get selection square interactively */
int get_clip(PLFLT *xi, PLFLT *xe, PLFLT *yi, PLFLT *ye)
{
  PLGraphicsIn gin;
  PLFLT xxi=*xi, yyi=*yi, xxe=*xe, yye=*ye, t;
  PLINT st, start = 0;

  plxormod(1, &st); /* enter xor mode to draw a selection rectangle */

  if (st) { /* driver has xormod capability, continue */
    while(1) {
      PLFLT sx[5], sy[5];

      plxormod(0, &st);
      plGetCursor(&gin);
      plxormod(1, &st);

      if (gin.button == 1) {
	xxi = gin.wX; yyi = gin.wY;
	if (start)
	  plline(5, sx, sy); /* clear previous rectangle */

	start = 0;

	sx[0] = xxi; sy[0] = yyi;
	sx[4] = xxi; sy[4] = yyi;
      }

      if (gin.state && 0x100) {
	xxe = gin.wX; yye = gin.wY;
	if (start)
	  plline(5, sx, sy); /* clear previous rectangle */

	start = 1;
  
	sx[2] = xxe; sy[2] = yye;
	sx[1] = xxe; sy[1] = yyi;
	sx[3] = xxi; sy[3] = yye;
	plline(5, sx, sy); /* draw new rectangle */
      }

      if (gin.button == 3 || gin.keysym == PLK_Return || gin.keysym == 'Q') {
	if (start)
	  plline(5, sx, sy); /* clear previous rectangle */
	break;
      }      
    }
    plxormod(0, &st); /* leave xor mod */
  }

  if (xxe < xxi) {
    t=xxi; xxi=xxe; xxe=t;}

  if (yyi < yye) {
    t=yyi; yyi=yye; yye=t;}

  *xe = xxe; *xi = xxi;
  *ye = yye; *yi = yyi;

  return (gin.keysym == 'Q');
}

/* set gray colormap */
void gray_cmap(PLINT num_col)
{
  PLFLT r[2], g[2], b[2], pos[2];
    
  r[0] = g[0] = b[0] = 0.0;
  r[1] = g[1] = b[1] = 1.0;
    
  pos[0] = 0.0;
  pos[1] = 1.0;
    
  plscmap1n(num_col);
  plscmap1l(1, 2, pos, r, g, b, NULL);
}
