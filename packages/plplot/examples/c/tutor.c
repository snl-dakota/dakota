/* $Id: tutor.c 3186 2006-02-15 18:17:33Z slbrow $
 *
 * Tony Richardson
 *
 * This program is intended to be used as a template for creating simple
 * two-dimensional plotting programs which use the PLplot plotting
 * library.  The program was written with an emphasis on trying to clearly
 * illustrate how to use the PLplot library functions.  
 *
 * This program reads data for M lines with N points each from an input
 * data file and plots them on the same graph using different symbols.  It
 * draws axes with labels and places a title at the top of the figure.  A
 * legend is drawn to the right of the figure.  The input data file must
 * have the following format:
 *
 *    M        N
 *    x[1]     y[1][1]     y[1][2]     .     .     .     y[1][M]
 *    x[2]     y[2][1]     y[2][2]     .     .     .     y[2][M]
 *    x[3]     y[3][1]     y[3][2]     .     .     .     y[3][M]
 *     .          .           .        .     .     .        .
 *     .          .           .        .     .     .        .
 *     .          .           .        .     .     .        .
 *    x[N]     y[N][1]     y[N][2]     .     .     .     y[N][M]
 *
 * (The first line contains the integer values M and N.  The succeeding
 * N lines contain the x-coordinate and the corresponding y-coordinates
 * of each of the M lines.) 
 */

#include "plcdemos.h"

static int
error(char *str);

/*
 * You can select a different set of symbols to use when plotting the
 * lines by changing the value of OFFSET. 
 */

#define OFFSET  2

int
main(int argc, char *argv[])
{
/* ==============  Begin variable definition section. ============= */

/*
 * i, j, and k are counting variables used in loops and such. M is the
 * number of lines to be plotted and N is the number of sample points
 * for each line.
 */

    int i, j, k, M, N, leglen;

/*
 * x is a pointer to an array containing the N x-coordinate values.  y
 * points to an array of M pointers each of which points to an array
 * containing the N y-coordinate values for that line.
 */

    PLFLT *x, **y;

/* Define storage for the min and max values of the data. */

    PLFLT xmin, xmax, ymin, ymax, xdiff, ydiff;

/* Define storage for the filename and define the input file pointer. */

    char filename[80], string[80], tmpstr[80];
    FILE *datafile;

/* Here are the character strings that appear in the plot legend. */

    static char *legend[] =
    {
	"Aardvarks",
	"Gnus",
	"Llamas",
	NULL};			/* Make sure last element is NULL */

/* ==============  Read in data from input file. ============= */

/* Parse and process command line arguments */

    (void) plparseopts(&argc, argv, PL_PARSE_FULL);

/* First prompt the user for the input data file name */

    printf("Enter input data file name. ");
    scanf("%s", filename);

/* and open the file. */

    datafile = fopen(filename, "r");
    if (datafile == NULL)	/* error opening input file */
	error("Error opening input file.");

/* Read in values of M and N */

    k = fscanf(datafile, "%d %d", &M, &N);
    if (k != 2)			/* something's wrong */
	error("Error while reading data file.");

/* Allocate memory for all the arrays. */

    x = (PLFLT *) malloc(N * sizeof(PLFLT));
    if (x == NULL)
	error("Out of memory!");
    y = (PLFLT **) malloc(M * sizeof(PLFLT *));
    if (y == NULL)
	error("Out of memory!");
    for (i = 0; i < M; i++) {
	y[i] = (PLFLT *) malloc(N * sizeof(PLFLT));
	if (y[i] == NULL)
	    error("Out of memory!");
    }

/* Now read in all the data. */

    for (i = 0; i < N; i++) {	/* N points */
	k = fscanf(datafile, "%f", &x[i]);
	if (k != 1)
	    error("Error while reading data file.");
	for (j = 0; j < M; j++) {	/* M lines */
	    k = fscanf(datafile, "%f", &y[j][i]);
	    if (k != 1)
		error("Error while reading data file.");
	}
    }

/* ==============  Graph the data. ============= */

/* Set graph to portrait orientation. (Default is landscape.) */
/* (Portrait is usually desired for inclusion in TeX documents.) */

    plsori(1);

/* Initialize plplot */

    plinit();

/* 
 * We must call pladv() to advance to the first (and only) subpage.
 * You might want to use plenv() instead of the pladv(), plvpor(),
 * plwind() sequence.
 */

    pladv(0);

/*
 * Set up the viewport.  This is the window into which the data is
 * plotted.  The size of the window can be set with a call to
 * plvpor(), which sets the size in terms of normalized subpage
 * coordinates.  I want to plot the lines on the upper half of the
 * page and I want to leave room to the right of the figure for
 * labelling the lines. We must also leave room for the title and
 * labels with plvpor().  Normally a call to plvsta() can be used
 * instead.
 */

    plvpor(0.15, 0.70, 0.5, 0.9);

/*
 * We now need to define the size of the window in user coordinates.
 * To do this, we first need to determine the range of the data
 * values.
 */

    xmin = xmax = x[0];
    ymin = ymax = y[0][0];
    for (i = 0; i < N; i++) {
	if (x[i] < xmin)
	    xmin = x[i];
	if (x[i] > xmax)
	    xmax = x[i];
	for (j = 0; j < M; j++) {
	    if (y[j][i] < ymin)
		ymin = y[j][i];
	    if (y[j][i] > ymax)
		ymax = y[j][i];
	}
    }

/* 
 * Now set the size of the window. Leave a small border around the
 * data.
 */

    xdiff = (xmax - xmin) / 20.;
    ydiff = (ymax - ymin) / 20.;
    plwind(xmin - xdiff, xmax + xdiff, ymin - ydiff, ymax + ydiff);

/* 
 * Call plbox() to draw the axes (see the PLPLOT manual for
 * information about the option strings.)
 */

    plbox("bcnst", 0.0, 0, "bcnstv", 0.0, 0);

/* 
 * Label the axes and title the graph.  The string "#gm" plots the
 * Greek letter mu, all the Greek letters are available, see the
 * PLplot manual.
 */

    pllab("Time (weeks)", "Height (#gmparsecs)", "Specimen Growth Rate");

/*
 * Plot the data.  plpoin() draws a symbol at each point.  plline()
 * connects all the points.
 */

    for (i = 0; i < M; i++) {
	plpoin(N, x, y[i], i + OFFSET);
	plline(N, x, y[i]);
    }

/*
 * Draw legend to the right of the chart.  Things get a little messy
 * here.  You may want to remove this section if you don't want a
 * legend drawn.  First find length of longest string.
 */

    leglen = 0;
    for (i = 0; i < M; i++) {
	if (legend[i] == NULL)
	    break;
	j = strlen(legend[i]);
	if (j > leglen)
	    leglen = j;
    }

/* 
 * Now build the string.  The string consists of an element from the
 * legend string array, padded with spaces, followed by one of the
 * symbols used in plpoin above.
 */

    for (i = 0; i < M; i++) {
	if (legend[i] == NULL)
	    break;
	strcpy(string, legend[i]);
	j = strlen(string);
	if (j < leglen) {	/* pad string with spaces */
	    for (k = j; k < leglen; k++)
		string[k] = ' ';
	    string[k] = '\0';
	}

    /* pad an extra space */

	strcat(string, " ");
	j = strlen(string);

    /* insert the ASCII value of the symbol plotted with plpoin() */

	string[j] = i + OFFSET;
	string[j + 1] = '\0';

    /* plot the string */

	plmtex("rv", 1., 1. - (double) (i + 1) / (M + 1), 0., string);
    }

/*  Tell plplot we are done with this page. */

    pladv(0);			/* advance page */

/* Don't forget to call plend() to finish off! */

    plend();
    exit(0);
}

static int
error(char *str)
{
    fprintf(stderr, "%s\n", str);
    exit(1);
}
