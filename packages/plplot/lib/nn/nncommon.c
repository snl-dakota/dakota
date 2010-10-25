/******************************************************************************
 *
 * File:           nncommon.c
 *
 * Created:        04/08/2000
 *
 * Author:         Pavel Sakov
 *                 CSIRO Marine Research
 *
 * Purpose:        Common stuff for NN interpolation library
 *
 * Description:    None
 *
 * Revisions:      15/11/2002 PS: Changed name from "utils.c"
 *                 28/02/2003 PS: Modified points_read() to do the job without
 *                   rewinding the file. This allows to read from stdin when
 *                   necessary.
 *                 09/04/2003 PS: Modified points_read() to read from a
 *                   file specified by name, not by handle.
 *
 *****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <math.h>
#include <limits.h>
#include <float.h>
#include <string.h>
#include <errno.h>
#include "nan.h"
#include "delaunay.h"

#define BUFSIZE 1024

int nn_verbose = 0;
int nn_test_vertice = -1;
NN_RULE nn_rule = SIBSON;

#include "version.h"

void nn_quit(char* format, ...)
{
    va_list args;

    fflush(stdout);             /* just in case, to have the exit message
                                 * last */

    fprintf(stderr, "error: nn: ");
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    exit(1);
}

int circle_build(circle* c, point* p1, point* p2, point* p3)
{
    double x1sq = p1->x * p1->x;
    double x2sq = p2->x * p2->x;
    double x3sq = p3->x * p3->x;
    double y1sq = p1->y * p1->y;
    double y2sq = p2->y * p2->y;
    double y3sq = p3->y * p3->y;
    double t1 = x3sq - x2sq + y3sq - y2sq;
    double t2 = x1sq - x3sq + y1sq - y3sq;
    double t3 = x2sq - x1sq + y2sq - y1sq;
    double D = (p1->x * (p2->y - p3->y) + p2->x * (p3->y - p1->y) + p3->x * (p1->y - p2->y)) * 2.0;

    if (D == 0.0)
        return 0;

    c->x = (p1->y * t1 + p2->y * t2 + p3->y * t3) / D;
    c->y = -(p1->x * t1 + p2->x * t2 + p3->x * t3) / D;
    c->r = hypot(c->x - p1->x, c->y - p1->y);

    return 1;
}

/* This procedure has taken it final shape after a number of tries. The problem
 * was to have the calculated and stored radii being the same if (x,y) is
 * exactly on the circle border (i.e. not to use FCPU extended precision in
 * the radius calculation). This may have little effect in practice but was
 * important in some tests when both input and output data were placed
 * in rectangular grid nodes.
 */
int circle_contains(circle* c, point* p)
{
    return hypot(c->x - p->x, c->y - p->y) <= c->r;
}

/* Smoothes the input point array by averaging the input x,y and z values
 * for each cell within virtual rectangular nx by ny grid. The corners of the
 * grid are created from min and max values of the input array. It also frees
 * the original array and returns results and new dimension via original
 * data and size pointers. 
 *
 * @param pn Pointer to number of points (input/output)
 * @param ppoints Pointer to array of points (input/output) [*pn]
 * @param nx Number of x nodes in decimation
 * @param ny Number of y nodes in decimation
 */
void points_thin(int* pn, point** ppoints, int nx, int ny)
{
    int n = *pn;
    point* points = *ppoints;
    double xmin = DBL_MAX;
    double xmax = -DBL_MAX;
    double ymin = DBL_MAX;
    double ymax = -DBL_MAX;
    int nxy = nx * ny;
    double* sumx = calloc(nxy, sizeof(double));
    double* sumy = calloc(nxy, sizeof(double));
    double* sumz = calloc(nxy, sizeof(double));
    int* count = calloc(nxy, sizeof(int));
    double stepx = 0.0;
    double stepy = 0.0;
    int nnew = 0;
    point* pointsnew = NULL;
    int i, j, ii;

    if (nn_verbose)
        fprintf(stderr, "thinned: %d points -> ", *pn);

    if (nx < 1 || ny < 1) {
        free(points);
        *ppoints = NULL;
        *pn = 0;
        if (nn_verbose)
            fprintf(stderr, "0 points");
        return;
    }

    for (ii = 0; ii < n; ++ii) {
        point* p = &points[ii];

        if (p->x < xmin)
            xmin = p->x;
        if (p->x > xmax)
            xmax = p->x;
        if (p->y < ymin)
            ymin = p->y;
        if (p->y > ymax)
            ymax = p->y;
    }

    stepx = (nx > 1) ? (xmax - xmin) / nx : 0.0;
    stepy = (ny > 1) ? (ymax - ymin) / ny : 0.0;

    for (ii = 0; ii < n; ++ii) {
        point* p = &points[ii];
        int index;

        /*
         * Following is the portion of the code which really depends on the
         * floating point particulars. Do not be surprised if different
         * compilers/options give different results here. 
         */
        i = (nx == 1) ? 0 : (p->x - xmin) / stepx;
        j = (ny == 1) ? 0 : (p->y - ymin) / stepy;

        if (i == nx)
            i--;
        if (j == ny)
            j--;
        index = i + j * nx;
        sumx[index] += p->x;
        sumy[index] += p->y;
        sumz[index] += p->z;
        count[index]++;
    }

    for (j = 0; j < ny; ++j) {
        for (i = 0; i < nx; ++i) {
            int index = i + j * nx;

            if (count[index] > 0)
                nnew++;
        }
    }

    pointsnew = malloc(nnew * sizeof(point));

    ii = 0;
    for (j = 0; j < ny; ++j) {
        for (i = 0; i < nx; ++i) {
            int index = i + j * nx;
            int nn = count[index];

            if (nn > 0) {
                point* p = &pointsnew[ii];

                p->x = sumx[index] / nn;
                p->y = sumy[index] / nn;
                p->z = sumz[index] / nn;
                ii++;
            }
        }
    }

    if (nn_verbose)
        fprintf(stderr, "%d points\n", nnew);

    free(sumx);
    free(sumy);
    free(sumz);
    free(count);

    free(points);
    *ppoints = pointsnew;
    *pn = nnew;
}

/* Generates rectangular grid nx by ny using min and max x and y values from
 * the input point array. Allocates space for the output point array, be sure
 * to free it when necessary!
 *
 * @param n Number of points
 * @param points Array of points [n]
 * @param nx Number of x nodes
 * @param ny Number of y nodes
 * @param zoom Zoom coefficient
 * @param nout Pointer to number of output points
 * @param pout Pointer to array of output points [*nout]
 */
void points_generate1(int nin, point pin[], int nx, int ny, double zoom, int* nout, point** pout)
{
    double xmin = DBL_MAX;
    double xmax = -DBL_MAX;
    double ymin = DBL_MAX;
    double ymax = -DBL_MAX;
    double stepx, stepy;
    double x0, xx, yy;
    int i, j, ii;

    if (nx < 1 || ny < 1) {
        *pout = NULL;
        *nout = 0;
        return;
    }

    for (ii = 0; ii < nin; ++ii) {
        point* p = &pin[ii];

        if (p->x < xmin)
            xmin = p->x;
        if (p->x > xmax)
            xmax = p->x;
        if (p->y < ymin)
            ymin = p->y;
        if (p->y > ymax)
            ymax = p->y;
    }

    if (isnan(zoom) || zoom <= 0.0)
        zoom = 1.0;

    if (zoom != 1.0) {
        double xdiff2 = (xmax - xmin) / 2.0;
        double ydiff2 = (ymax - ymin) / 2.0;
        double xav = (xmax + xmin) / 2.0;
        double yav = (ymax + ymin) / 2.0;

        xmin = xav - xdiff2 * zoom;
        xmax = xav + xdiff2 * zoom;
        ymin = yav - ydiff2 * zoom;
        ymax = yav + ydiff2 * zoom;
    }

    *nout = nx * ny;
    *pout = malloc(*nout * sizeof(point));

    stepx = (nx > 1) ? (xmax - xmin) / (nx - 1) : 0.0;
    stepy = (ny > 1) ? (ymax - ymin) / (ny - 1) : 0.0;
    x0 = (nx > 1) ? xmin : (xmin + xmax) / 2.0;
    yy = (ny > 1) ? ymin : (ymin + ymax) / 2.0;

    ii = 0;
    for (j = 0; j < ny; ++j) {
        xx = x0;
        for (i = 0; i < nx; ++i) {
            point* p = &(*pout)[ii];

            p->x = xx;
            p->y = yy;
            xx += stepx;
            ii++;
        }
        yy += stepy;
    }
}

/* Generates rectangular grid nx by ny using specified min and max x and y 
 * values. Allocates space for the output point array, be sure to free it
 * when necessary!
 *
 * @param xmin Min x value
 * @param xmax Max x value
 * @param ymin Min y value
 * @param ymax Max y value
 * @param nx Number of x nodes
 * @param ny Number of y nodes
 * @param nout Pointer to number of output points
 * @param pout Pointer to array of output points [*nout]
 */
void points_generate2(double xmin, double xmax, double ymin, double ymax, int nx, int ny, int* nout, point** pout)
{
    double stepx, stepy;
    double x0, xx, yy;
    int i, j, ii;

    if (nx < 1 || ny < 1) {
        *pout = NULL;
        *nout = 0;
        return;
    }

    *nout = nx * ny;
    *pout = malloc(*nout * sizeof(point));

    stepx = (nx > 1) ? (xmax - xmin) / (nx - 1) : 0.0;
    stepy = (ny > 1) ? (ymax - ymin) / (ny - 1) : 0.0;
    x0 = (nx > 1) ? xmin : (xmin + xmax) / 2.0;
    yy = (ny > 1) ? ymin : (ymin + ymax) / 2.0;

    ii = 0;
    for (j = 0; j < ny; ++j) {
        xx = x0;
        for (i = 0; i < nx; ++i) {
            point* p = &(*pout)[ii];

            p->x = xx;
            p->y = yy;
            xx += stepx;
            ii++;
        }
        yy += stepy;
    }
}

static int str2double(char* token, double* value)
{
    char* end = NULL;

    if (token == NULL) {
        *value = NaN;
        return 0;
    }

    *value = strtod(token, &end);

    if (end == token) {
        *value = NaN;
        return 0;
    }

    return 1;
}

#define NALLOCATED_START 1024

/* Reads array of points from a columnar file.
 *
 * @param fname File name (can be "stdin" for standard input)
 * @param dim Number of dimensions (must be 2 or 3)
 * @param n Pointer to number of points (output)
 * @param points Pointer to array of points [*n] (output) (to be freed)
 */
void points_read(char* fname, int dim, int* n, point** points)
{
    FILE* f = NULL;
    int nallocated = NALLOCATED_START;
    char buf[BUFSIZE];
    char seps[] = " ,;\t";
    char* token;

    if (dim < 2 || dim > 3) {
        *n = 0;
        *points = NULL;
        return;
    }

    if (fname == NULL)
        f = stdin;
    else {
        if (strcmp(fname, "stdin") == 0 || strcmp(fname, "-") == 0)
            f = stdin;
        else {
            f = fopen(fname, "r");
            if (f == NULL)
                nn_quit("%s: %s\n", fname, strerror(errno));
        }
    }

    *points = malloc(nallocated * sizeof(point));
    *n = 0;
    while (fgets(buf, BUFSIZE, f) != NULL) {
        point* p;

        if (*n == nallocated) {
            nallocated *= 2;
            *points = realloc(*points, nallocated * sizeof(point));
        }

        p = &(*points)[*n];

        if (buf[0] == '#')
            continue;
        if ((token = strtok(buf, seps)) == NULL)
            continue;
        if (!str2double(token, &p->x))
            continue;
        if ((token = strtok(NULL, seps)) == NULL)
            continue;
        if (!str2double(token, &p->y))
            continue;
        if (dim == 2)
            p->z = NaN;
        else {
            if ((token = strtok(NULL, seps)) == NULL)
                continue;
            if (!str2double(token, &p->z))
                continue;
        }
        (*n)++;
    }

    if (*n == 0) {
        free(*points);
        *points = NULL;
    } else
        *points = realloc(*points, *n * sizeof(point));

    if (f != stdin)
        if (fclose(f) != 0)
            nn_quit("%s: %s\n", fname, strerror(errno));
}

/** Scales Y coordinate so that the resulting set fits into square:
 ** xmax - xmin = ymax - ymin
 *
 * @param n Number of points
 * @param points The points to scale
 * @return Y axis compression coefficient
 */
double points_scaletosquare(int n, point* points)
{
    double xmin, ymin, xmax, ymax;
    double k;
    int i;

    if (n <= 0)
        return NaN;

    xmin = xmax = points[0].x;
    ymin = ymax = points[0].y;

    for (i = 1; i < n; ++i) {
        point* p = &points[i];

        if (p->x < xmin)
            xmin = p->x;
        else if (p->x > xmax)
            xmax = p->x;
        if (p->y < ymin)
            ymin = p->y;
        else if (p->y > ymax)
            ymax = p->y;
    }

    if (xmin == xmax || ymin == ymax)
        return NaN;
    else
        k = (ymax - ymin) / (xmax - xmin);

    for (i = 0; i < n; ++i)
        points[i].y /= k;

    return k;
}

/** Compresses Y domain by a given multiple.
 *
 * @param n Number of points
 * @param points The points to scale
 * @param Y axis compression coefficient as returned by points_scaletosquare()
 */
void points_scale(int n, point* points, double k)
{
    int i;

    for (i = 0; i < n; ++i)
        points[i].y /= k;
}
