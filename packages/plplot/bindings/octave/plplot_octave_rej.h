/* 
 * jc: this segment of plplot.h was rejected as errors by matwrap,
 * but the defined functions are needed.
 *
 * The rejection occurs because of function pointer arguments
 * in the function prototypes.
 */


/* Draws a contour plot from data in f(nx,ny).  Is just a front-end to
 * plfcont, with a particular choice for f2eval and f2eval_data. 
 */

void c_plcont(PLFLT **f, PLINT nx, PLINT ny, PLINT kx, PLINT lx,
	 PLINT ky, PLINT ly, PLFLT *clevel, PLINT nlevel,
	 void (*pltr) (PLFLT, PLFLT, PLFLT *, PLFLT *, PLPointer),
	 PLPointer pltr_data);  //%nowrap

/* Draws a contour plot using the function evaluator f2eval and data stored
 * by way of the f2eval_data pointer.  This allows arbitrary organizations
 * of 2d array data to be used. 
 */

void plfcont(PLFLT (*f2eval) (PLINT, PLINT, PLPointer),
	PLPointer f2eval_data,
	PLINT nx, PLINT ny, PLINT kx, PLINT lx,
	PLINT ky, PLINT ly, PLFLT *clevel, PLINT nlevel,
	void (*pltr) (PLFLT, PLFLT, PLFLT *, PLFLT *, PLPointer),
	PLPointer pltr_data);  //%nowrap

/* plot continental outline in world coordinates */

void plmap(void (*mapform)(PLINT, PLFLT *, PLFLT *), char *type,
      PLFLT minlong, PLFLT maxlong, PLFLT minlat, PLFLT maxlat); //%nowrap

/* Plot the latitudes and longitudes on the background. */

void plmeridians(void (*mapform)(PLINT, PLFLT *, PLFLT *), 
	    PLFLT dlong, PLFLT dlat,
	    PLFLT minlong, PLFLT maxlong, PLFLT minlat, PLFLT maxlat);  //%nowrap
 
/* Shade region. */

void c_plshade(PLFLT **a, PLINT nx, PLINT ny, const char **defined,
	  PLFLT left, PLFLT right, PLFLT bottom, PLFLT top,
	  PLFLT shade_min, PLFLT shade_max,
	  PLINT sh_cmap, PLFLT sh_color, PLINT sh_width,
	  PLINT min_color, PLINT min_width,
	  PLINT max_color, PLINT max_width,
	  void (*fill) (PLINT, PLFLT *, PLFLT *), PLINT rectangular,
	  void (*pltr) (PLFLT, PLFLT, PLFLT *, PLFLT *, PLPointer),
	  PLPointer pltr_data); //%nowrap

/* multiple plshade, specify number of countours to shade */

void c_plshades(PLFLT **a, PLINT nx, PLINT ny, const char **defined,
	  PLFLT left, PLFLT right, PLFLT bottom, PLFLT top,
	PLFLT *clevel, PLINT nlevel, PLINT fill_width,
	PLINT cont_color, PLINT cont_width,
	  void (*fill) (PLINT, PLFLT *, PLFLT *), PLINT rectangular,
	  void (*pltr) (PLFLT, PLFLT, PLFLT *, PLFLT *, PLPointer),
	  PLPointer pltr_data); //%nowrap
		
/* the next one seems to use the C calling convention,
 *	but did not work at first, so I will use the proven F calling
 */
 
void plshade1(PLFLT *a, PLINT nx, PLINT ny, const char *defined,
	 PLFLT left, PLFLT right, PLFLT bottom, PLFLT top,
	 PLFLT shade_min, PLFLT shade_max,
	 PLINT sh_cmap, PLFLT sh_color, PLINT sh_width,
	 PLINT min_color, PLINT min_width,
	 PLINT max_color, PLINT max_width,
	 void (*fill) (PLINT, PLFLT *, PLFLT *), PLINT rectangular,
	 void (*pltr) (PLFLT, PLFLT, PLFLT *, PLFLT *, PLPointer),
	 PLPointer pltr_data);  //%nowrap 
	 
void plfshade(PLFLT (*f2eval) (PLINT, PLINT, PLPointer),
	 PLPointer f2eval_data,
	 PLFLT (*c2eval) (PLINT, PLINT, PLPointer),
	 PLPointer c2eval_data,
	 PLINT nx, PLINT ny, 
	 PLFLT left, PLFLT right, PLFLT bottom, PLFLT top,
	 PLFLT shade_min, PLFLT shade_max,
	 PLINT sh_cmap, PLFLT sh_color, PLINT sh_width,
	 PLINT min_color, PLINT min_width,
	 PLINT max_color, PLINT max_width,
	 void (*fill) (PLINT, PLFLT *, PLFLT *), PLINT rectangular,
	 void (*pltr) (PLFLT, PLFLT, PLFLT *, PLFLT *, PLPointer),
	 PLPointer pltr_data);  //%nowrap

/* Create 1d stripchart */

void c_plstripc(PLINT *id, char *xspec, char *yspec,
	PLFLT xmin, PLFLT xmax, PLFLT xjump, PLFLT ymin, PLFLT ymax,
	PLFLT xlpos, PLFLT ylpos,
	PLINT y_ascl, PLINT acc,
	PLINT colbox, PLINT collab,
	PLINT *colline, PLINT *styline, char *legline[],
	char *labx, char *laby, char *labtop); //%nowrap

/* Set the function pointer for the keyboard event handler */

void plsKeyEH(void (*KeyEH) (PLGraphicsIn *, void *, int *), void *KeyEH_data); //%nowrap

/* Set the function pointer for the (mouse) button event handler */

void plsButtonEH(void (*ButtonEH) (PLGraphicsIn *, void *, int *),
	    void *ButtonEH_data); //%nowrap
	    
/* Sets an optional user exit handler. */

void plsexit(int (*handler) (char *));   
