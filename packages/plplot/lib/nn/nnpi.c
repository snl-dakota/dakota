 /*****************************************************************************
 *
 * File:           nnpi.c
 *
 * Created:        15/11/2002
 *
 * Author:         Pavel Sakov
 *                 CSIRO Marine Research
 *
 * Purpose:        Code for:
 *                 -- Natural Neighbours Point Interpolator
 *                 -- Natural Neighbours Point Hashing Interpolator
 *
 * Description:    `nnpi' -- "Natural Neighbours Point
 *                 Interpolator" -- is a structure for conducting Natural
 *                 Neighbours interpolation on a given data on a
 *                 "point-to-point" basis. Because it involves weight
 *                 calculation for each next output point, it is not
 *                 particularly suitable for consequitive interpolations on
 *                 the same set of observation points -- use
 *                 `nnhpi' or `nnai'
 *                 in these cases.
 *
 *                 `nnhpi' is a structure for
 *                 conducting consequitive Natural Neighbours interpolations
 *                 on a given spatial data set in a random sequence of points
 *                 from a set of finite size, taking advantage of repeated
 *                 interpolations in the same point. It allows to modify Z
 *                 coordinate of data in between interpolations.
 *
 *
 * Revisions:      01/04/2003 PS: modified nnpi_triangle_process(): for
 *                   Sibson interpolation, if circle_build fails(), now a
 *                   local copy of a point is moved slightly rather than the
 *                   data point itself. The later approach have found leading
 *                   to inconsistencies of the new point position with the 
 *                   earlier built triangulation.
 *
 *****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <float.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "nn.h"
#include "delaunay.h"
#include "nan.h"
#include "hash.h"

struct nnpi {
    delaunay* d;
    point* p;
    double wmin;
    /*
     * work variables 
     */
    int nvertices;
    int nallocated;
    int* vertices;              /* vertex indices */
    double* weights;
    int n;                      /* number of points processed */
};

int circle_build(circle* c, point* p0, point* p1, point* p2);
int circle_contains(circle* c, point* p);
void delaunay_circles_find(delaunay* d, point* p, int* n, int** out);
int delaunay_xytoi(delaunay* d, point* p, int seed);
void nn_quit(char* format, ...);

#define NSTART 10
#define NINC 10
#define EPS_SHIFT 1.0e-9
#define N_SEARCH_TURNON 20
#define BIGNUMBER 1.0e+100

#define min(x,y) ((x) < (y) ? (x) : (y))
#define max(x,y) ((x) > (y) ? (x) : (y))

/* Creates Natural Neighbours point interpolator.
 *
 * @param d Delaunay triangulation
 * @return Natural Neighbours interpolation
 */
nnpi* nnpi_create(delaunay* d)
{
    nnpi* nn = malloc(sizeof(nnpi));

    nn->d = d;
    nn->wmin = -DBL_MAX;
    nn->vertices = calloc(NSTART, sizeof(int));
    nn->weights = calloc(NSTART, sizeof(double));
    nn->nvertices = 0;
    nn->nallocated = NSTART;
    nn->p = NULL;
    nn->n = 0;

    return nn;
}

/* Destroys Natural Neighbours point interpolator.
 *
 * @param nn Structure to be destroyed
 */
void nnpi_destroy(nnpi* nn)
{
    free(nn->weights);
    free(nn->vertices);
    free(nn);
}

void nnpi_reset(nnpi* nn)
{
    nn->nvertices = 0;
    nn->p = NULL;
    memset(nn->d->flags, 0, nn->d->ntriangles * sizeof(int));
}

static void nnpi_add_weight(nnpi* nn, int vertex, double w)
{
    int i;

    /*
     * find whether the vertex is already in the list 
     */
    for (i = 0; i < nn->nvertices; ++i)
        if (nn->vertices[i] == vertex)
            break;

    if (i == nn->nvertices) {   /* not in the list */
        /*
         * get more memory if necessary 
         */
        if (nn->nvertices == nn->nallocated) {
            nn->vertices = realloc(nn->vertices, (nn->nallocated + NINC) * sizeof(int));
            nn->weights = realloc(nn->weights, (nn->nallocated + NINC) * sizeof(double));
            nn->nallocated += NINC;
        }

        /*
         * add the vertex to the list 
         */
        nn->vertices[i] = vertex;
        nn->weights[i] = w;
        nn->nvertices++;
    } else {                    /* in the list */

        if (nn_rule == SIBSON)
            nn->weights[i] += w;
        else if (w > nn->weights[i])
            nn->weights[i] = w;
    }
}

static double triangle_scale_get(delaunay* d, triangle* t)
{
    double x0 = d->points[t->vids[0]].x;
    double x1 = d->points[t->vids[1]].x;
    double x2 = d->points[t->vids[2]].x;
    double y0 = d->points[t->vids[0]].y;
    double y1 = d->points[t->vids[1]].y;
    double y2 = d->points[t->vids[2]].y;
    double xmin = min(min(x0, x1), x2);
    double xmax = max(max(x0, x1), x2);
    double ymin = min(min(y0, y1), y2);
    double ymax = max(max(y0, y1), y2);

    return xmax - xmin + ymax - ymin;
}

/* This is a central procedure for the Natural Neighbours interpolation. It
 * uses the Watson's algorithm for the required areas calculation and implies
 * that the vertices of the delaunay triangulation are listed in uniform
 * (clockwise or counterclockwise) order.
 */
static void nnpi_triangle_process(nnpi* nn, point* p, int i)
{
    delaunay* d = nn->d;
    triangle* t = &d->triangles[i];
    circle* c = &d->circles[i];
    circle cs[3];
    int j;

    assert(circle_contains(c, p));

    if (nn_rule == SIBSON) {
        point pp;

        pp.x = p->x;
        pp.y = p->y;
        /*
         * Sibson interpolation by using Watson's algorithm 
         */
        do {
            for (j = 0; j < 3; ++j) {
                int j1 = (j + 1) % 3;
                int j2 = (j + 2) % 3;
                int v1 = t->vids[j1];
                int v2 = t->vids[j2];

                if (!circle_build(&cs[j], &d->points[v1], &d->points[v2], &pp)) {
                    double scale = triangle_scale_get(d, t);

                    if (d->points[v1].y == d->points[v2].y)
                        pp.y += EPS_SHIFT * scale;
                    else
                        pp.x += EPS_SHIFT * scale;
                    break;
                }
            }
        } while (j != 3);

        for (j = 0; j < 3; ++j) {
            int j1 = (j + 1) % 3;
            int j2 = (j + 2) % 3;
            double det = ((cs[j1].x - c->x) * (cs[j2].y - c->y) - (cs[j2].x - c->x) * (cs[j1].y - c->y));

            nnpi_add_weight(nn, t->vids[j], det);
        }
    } else if (nn_rule == NON_SIBSONIAN) {
        double d1 = c->r - hypot(p->x - c->x, p->y - c->y);

        for (i = 0; i < 3; ++i) {
            int vid = t->vids[i];
            point* pp = &d->points[vid];
            double d2 = hypot(p->x - pp->x, p->y - pp->y);

            if (d2 == 0.0)
                nnpi_add_weight(nn, vid, BIGNUMBER);
            else
                nnpi_add_weight(nn, vid, d1 / d2);
        }
    } else
        nn_quit("unknown rule\n");
}

void nnpi_calculate_weights(nnpi* nn)
{
    point* p = nn->p;
    int n = nn->d->ntriangles;
    int i;

    if (n > N_SEARCH_TURNON) {
        int* tids;

        delaunay_circles_find(nn->d, p, &n, &tids);
        for (i = 0; i < n; ++i)
            nnpi_triangle_process(nn, p, tids[i]);
    } else
        for (i = 0; i < n; ++i)
            if (circle_contains(&nn->d->circles[i], p))
                nnpi_triangle_process(nn, p, i);
}

void nnpi_normalize_weights(nnpi* nn)
{
    int n = nn->nvertices;
    double sum = 0.0;
    int i;

    for (i = 0; i < n; ++i)
        sum += nn->weights[i];

    for (i = 0; i < n; ++i)
        nn->weights[i] /= sum;
}

/* Finds Natural Neighbours-interpolated value for a point.
 *
 * @param nn NN interpolation
 * @param p Point to be interpolated (p->x, p->y -- input; p->z -- output)
 */
void nnpi_interpolate_point(nnpi* nn, point* p)
{
    delaunay* d = nn->d;
    int i;

    nnpi_reset(nn);
    nn->p = p;
    nnpi_calculate_weights(nn);
    nnpi_normalize_weights(nn);

    if (nn_verbose) {
        if (nn_test_vertice == -1) {
            if (nn->n == 0)
                fprintf(stderr, "weights:\n");
            fprintf(stderr, "  %d: {", nn->n);
            for (i = 0; i < nn->nvertices; ++i) {
                fprintf(stderr, "(%d,%.5g)", nn->vertices[i], nn->weights[i]);
                if (i < nn->nvertices - 1)
                    fprintf(stderr, ", ");
            }
            fprintf(stderr, "}\n");
        } else {
            double w = 0.0;

            if (nn->n == 0)
                fprintf(stderr, "weights for vertex %d:\n", nn_test_vertice);
            for (i = 0; i < nn->nvertices; ++i) {
                if (nn->vertices[i] == nn_test_vertice) {
                    w = nn->weights[i];
                    break;
                }
            }
            fprintf(stderr, "%15.7g %15.7g %15.7g\n", p->x, p->y, w);
        }
    }

    nn->n++;

    if (nn->nvertices == 0) {
        p->z = NaN;
        return;
    }

    p->z = 0.0;
    for (i = 0; i < nn->nvertices; ++i) {
        double weight = nn->weights[i];

        if (weight < nn->wmin) {
            p->z = NaN;
            return;
        }
        p->z += d->points[nn->vertices[i]].z * weight;
    }
}

/* Performs Natural Neighbours interpolation for an array of points.
 *
 * @param nin Number of input points
 * @param pin Array of input points [pin]
 * @param wmin Minimal allowed weight
 * @param nout Number of output points
 * @param pout Array of output points [nout]
 */
void nnpi_interpolate_points(int nin, point pin[], double wmin, int nout, point pout[])
{
    delaunay* d = delaunay_build(nin, pin, 0, NULL, 0, NULL);
    nnpi* nn = nnpi_create(d);
    int seed = 0;
    int i;

    nn->wmin = wmin;

    if (nn_verbose) {
        fprintf(stderr, "xytoi:\n");
        for (i = 0; i < nout; ++i) {
            point* p = &pout[i];

            fprintf(stderr, "(%.7g,%.7g) -> %d\n", p->x, p->y, delaunay_xytoi(d, p, seed));
        }
    }

    for (i = 0; i < nout; ++i)
        nnpi_interpolate_point(nn, &pout[i]);

    if (nn_verbose) {
        fprintf(stderr, "output:\n");
        for (i = 0; i < nout; ++i) {
            point* p = &pout[i];

            fprintf(stderr, "  %d:%15.7g %15.7g %15.7g\n", i, p->x, p->y, p->z);
        }
    }

    nnpi_destroy(nn);
    delaunay_destroy(d);
}

/* Sets minimal allowed weight for Natural Neighbours interpolation.
 * @param nn Natural Neighbours point interpolator
 * @param wmin Minimal allowed weight
 */
void nnpi_setwmin(nnpi* nn, double wmin)
{
    nn->wmin = wmin;
}

/* Sets point to interpolate in.
 * @param nn Natural Neighbours point interpolator
 * @param p Point to interpolate in
 */
void nnpi_set_point(nnpi* nn, point* p)
{
    nn->p = p;
}

/* Gets number of data points involved in current interpolation.
 * @return Number of data points involved in current interpolation
 */
int nnpi_get_nvertices(nnpi* nn)
{
    return nn->nvertices;
}

/* Gets indices of data points involved in current interpolation.
 * @return indices of data points involved in current interpolation
 */
int* nnpi_get_vertices(nnpi* nn)
{
    return nn->vertices;
}

/* Gets weights of data points involved in current interpolation.
 * @return weights of data points involved in current interpolation
 */
double* nnpi_get_weights(nnpi* nn)
{
    return nn->weights;
}

/*
 * nnhpi
 */

struct nnhpi {
    nnpi* nnpi;
    hashtable* ht_data;
    hashtable* ht_weights;
    int n;                      /* number of points processed */
};

typedef struct {
    int nvertices;
    int* vertices;              /* vertex indices [nvertices] */
    double* weights;            /* vertex weights [nvertices] */
} nn_weights;

/* Creates Natural Neighbours hashing point interpolator.
 *
 * @param d Delaunay triangulation
 * @param size Hash table size (should be of order of number of output points)
 * @return Natural Neighbours interpolation
 */
nnhpi* nnhpi_create(delaunay* d, int size)
{
    nnhpi* nn = malloc(sizeof(nnhpi));
    int i;

    nn->nnpi = nnpi_create(d);

    nn->ht_data = ht_create_d2(d->npoints);
    nn->ht_weights = ht_create_d2(size);
    nn->n = 0;

    for (i = 0; i < d->npoints; ++i)
        ht_insert(nn->ht_data, &d->points[i], &d->points[i]);

    return nn;
}

static void free_nn_weights(void* data)
{
    nn_weights* weights = (nn_weights*) data;

    free(weights->vertices);
    free(weights->weights);
    free(weights);
}

/* Destroys Natural Neighbours hashing point interpolation.
 *
 * @param nn Structure to be destroyed
 */
void nnhpi_destroy(nnhpi* nn)
{
    ht_destroy(nn->ht_data);
    ht_process(nn->ht_weights, free_nn_weights);
    ht_destroy(nn->ht_weights);
    nnpi_destroy(nn->nnpi);
}

/* Finds Natural Neighbours-interpolated value in a point.
 *
 * @param nnhpi NN point hashing interpolator
 * @param p Point to be interpolated (p->x, p->y -- input; p->z -- output)
 */
void nnhpi_interpolate(nnhpi* nnhpi, point* p)
{
    nnpi* nnpi = nnhpi->nnpi;
    delaunay* d = nnpi->d;
    hashtable* ht_weights = nnhpi->ht_weights;
    nn_weights* weights;
    int i;

    if (ht_find(ht_weights, p) != NULL) {
        weights = ht_find(ht_weights, p);
        if (nn_verbose)
            fprintf(stderr, "  <hashtable>\n");
    } else {
        nnpi_reset(nnpi);
        nnpi->p = p;
        nnpi_calculate_weights(nnpi);
        nnpi_normalize_weights(nnpi);

        weights = malloc(sizeof(nn_weights));
        weights->vertices = malloc(sizeof(int) * nnpi->nvertices);
        weights->weights = malloc(sizeof(double) * nnpi->nvertices);

        weights->nvertices = nnpi->nvertices;

        for (i = 0; i < nnpi->nvertices; ++i) {
            weights->vertices[i] = nnpi->vertices[i];
            weights->weights[i] = nnpi->weights[i];
        }

        ht_insert(ht_weights, p, weights);

        if (nn_verbose) {
            if (nn_test_vertice == -1) {
                if (nnpi->n == 0)
                    fprintf(stderr, "weights:\n");
                fprintf(stderr, "  %d: {", nnpi->n);

                for (i = 0; i < nnpi->nvertices; ++i) {
                    fprintf(stderr, "(%d,%.5g)", nnpi->vertices[i], nnpi->weights[i]);

                    if (i < nnpi->nvertices - 1)
                        fprintf(stderr, ", ");
                }
                fprintf(stderr, "}\n");
            } else {
                double w = 0.0;

                if (nnpi->n == 0)
                    fprintf(stderr, "weights for vertex %d:\n", nn_test_vertice);
                for (i = 0; i < nnpi->nvertices; ++i) {
                    if (nnpi->vertices[i] == nn_test_vertice) {
                        w = nnpi->weights[i];

                        break;
                    }
                }
                fprintf(stderr, "%15.7g %15.7g %15.7g\n", p->x, p->y, w);
            }
        }

        nnpi->n++;
    }

    nnhpi->n++;

    if (weights->nvertices == 0) {
        p->z = NaN;
        return;
    }

    p->z = 0.0;
    for (i = 0; i < weights->nvertices; ++i) {
        if (weights->weights[i] < nnpi->wmin) {
            p->z = NaN;
            return;
        }
        p->z += d->points[weights->vertices[i]].z * weights->weights[i];
    }
}

/* Modifies interpolated data.
 * Finds point* pd in the underlying Delaunay triangulation such that
 * pd->x = p->x and pd->y = p->y, and copies p->z to pd->z. Exits with error
 * if the point is not found.
 *
 * @param nnhpi Natural Neighbours hashing point interpolator
 * @param p New data
 */
void nnhpi_modify_data(nnhpi* nnhpi, point* p)
{
    point* orig = ht_find(nnhpi->ht_data, p);

    assert(orig != NULL);
    orig->z = p->z;
}

/* Sets minimal allowed weight for Natural Neighbours interpolation.
 * @param nn Natural Neighbours point hashing interpolator
 * @param wmin Minimal allowed weight
 */
void nnhpi_setwmin(nnhpi* nn, double wmin)
{
    nn->nnpi->wmin = wmin;
}

#if defined(NNPHI_TEST)

#include <sys/time.h>

#define NPOINTSIN 10000
#define NMIN 10
#define NX 101
#define NXMIN 1

#define SQ(x) ((x) * (x))

static double franke(double x, double y)
{
    x *= 9.0;
    y *= 9.0;
    return 0.75 * exp((-SQ(x - 2.0) - SQ(y - 2.0)) / 4.0)
        + 0.75 * exp(-SQ(x - 2.0) / 49.0 - (y - 2.0) / 10.0)
        + 0.5 * exp((-SQ(x - 7.0) - SQ(y - 3.0)) / 4.0)
        - 0.2 * exp(-SQ(x - 4.0) - SQ(y - 7.0));
}

static void usage()
{
    printf("Usage: nnhpi_test [-a] [-n <nin> <nxout>] [-v|-V]\n");
    printf("Options:\n");
    printf("  -a              -- use non-Sibsonian interpolation rule\n");
    printf("  -n <nin> <nout>:\n");
    printf("            <nin> -- number of input points (default = 10000)\n");
    printf("           <nout> -- number of output points per side (default = 64)\n");
    printf("  -v              -- verbose\n");
    printf("  -V              -- very verbose\n");

    exit(0);
}

int main(int argc, char* argv[])
{
    int nin = NPOINTSIN;
    int nx = NX;
    int nout = 0;
    point* pin = NULL;
    delaunay* d = NULL;
    point* pout = NULL;
    nnhpi* nn = NULL;
    int cpi = -1;               /* control point index */
    struct timeval tv0, tv1;
    struct timezone tz;
    int i;

    i = 1;
    while (i < argc) {
        switch (argv[i][1]) {
        case 'a':
            i++;
            nn_rule = NON_SIBSONIAN;
            break;
        case 'n':
            i++;
            if (i >= argc)
                nn_quit("no number of data points found after -n\n");
            nin = atoi(argv[i]);
            i++;
            if (i >= argc)
                nn_quit("no number of ouput points per side found after -i\n");
            nx = atoi(argv[i]);
            i++;
            break;
        case 'v':
            i++;
            nn_verbose = 1;
            break;
        case 'V':
            i++;
            nn_verbose = 2;
            break;
        default:
            usage();
            break;
        }
    }

    if (nin < NMIN)
        nin = NMIN;
    if (nx < NXMIN)
        nx = NXMIN;

    printf("\nTest of Natural Neighbours hashing point interpolator:\n\n");
    printf("  %d data points\n", nin);
    printf("  %d output points\n", nx * nx);

    /*
     * generate data 
     */
    printf("  generating data:\n");
    fflush(stdout);
    pin = malloc(nin * sizeof(point));
    for (i = 0; i < nin; ++i) {
        point* p = &pin[i];

        p->x = (double) random() / RAND_MAX;
        p->y = (double) random() / RAND_MAX;
        p->z = franke(p->x, p->y);
        if (nn_verbose)
            printf("    (%f, %f, %f)\n", p->x, p->y, p->z);
    }

    /*
     * triangulate
     */
    printf("  triangulating:\n");
    fflush(stdout);
    d = delaunay_build(nin, pin, 0, NULL, 0, NULL);

    /*
     * generate output points 
     */
    points_generate2(-0.1, 1.1, -0.1, 1.1, nx, nx, &nout, &pout);
    cpi = (nx / 2) * (nx + 1);

    gettimeofday(&tv0, &tz);

    /*
     * create interpolator 
     */
    printf("  creating interpolator:\n");
    fflush(stdout);
    nn = nnhpi_create(d, nout);

    fflush(stdout);
    gettimeofday(&tv1, &tz);
    {
        long dt = 1000000 * (tv1.tv_sec - tv0.tv_sec) + tv1.tv_usec - tv0.tv_usec;

        printf("    interpolator creation time = %ld us (%.2f us / point)\n", dt, (double) dt / nout);
    }

    /*
     * interpolate 
     */
    printf("  interpolating:\n");
    fflush(stdout);
    gettimeofday(&tv1, &tz);
    for (i = 0; i < nout; ++i) {
        point* p = &pout[i];

        nnhpi_interpolate(nn, p);
        if (nn_verbose)
            printf("    (%f, %f, %f)\n", p->x, p->y, p->z);
    }

    fflush(stdout);
    gettimeofday(&tv0, &tz);
    {
        long dt = 1000000.0 * (tv0.tv_sec - tv1.tv_sec) + tv0.tv_usec - tv1.tv_usec;

        printf("    interpolation time = %ld us (%.2f us / point)\n", dt, (double) dt / nout);
    }

    if (!nn_verbose)
        printf("    control point: (%f, %f, %f) (expected z = %f)\n", pout[cpi].x, pout[cpi].y, pout[cpi].z, franke(pout[cpi].x, pout[cpi].y));

    printf("  interpolating one more time:\n");
    fflush(stdout);
    gettimeofday(&tv0, &tz);
    for (i = 0; i < nout; ++i) {
        point* p = &pout[i];

        nnhpi_interpolate(nn, p);
        if (nn_verbose)
            printf("    (%f, %f, %f)\n", p->x, p->y, p->z);
    }

    fflush(stdout);
    gettimeofday(&tv1, &tz);
    {
        long dt = 1000000.0 * (tv1.tv_sec - tv0.tv_sec) + tv1.tv_usec - tv0.tv_usec;

        printf("    interpolation time = %ld us (%.2f us / point)\n", dt, (double) dt / nout);
    }

    if (!nn_verbose)
        printf("    control point: (%f, %f, %f) (expected z = %f)\n", pout[cpi].x, pout[cpi].y, pout[cpi].z, franke(pout[cpi].x, pout[cpi].y));

    printf("  entering new data:\n");
    fflush(stdout);
    for (i = 0; i < nin; ++i) {
        point* p = &pin[i];

        p->z = p->x * p->x - p->y * p->y;
        nnhpi_modify_data(nn, p);
        if (nn_verbose)
            printf("    (%f, %f, %f)\n", p->x, p->y, p->z);
    }

    printf("  interpolating:\n");
    fflush(stdout);
    gettimeofday(&tv1, &tz);
    for (i = 0; i < nout; ++i) {
        point* p = &pout[i];

        nnhpi_interpolate(nn, p);
        if (nn_verbose)
            printf("    (%f, %f, %f)\n", p->x, p->y, p->z);
    }

    fflush(stdout);
    gettimeofday(&tv0, &tz);
    {
        long dt = 1000000.0 * (tv0.tv_sec - tv1.tv_sec) + tv0.tv_usec - tv1.tv_usec;

        printf("    interpolation time = %ld us (%.2f us / point)\n", dt, (double) dt / nout);
    }

    if (!nn_verbose)
        printf("    control point: (%f, %f, %f) (expected z = %f)\n", pout[cpi].x, pout[cpi].y, pout[cpi].z, pout[cpi].x * pout[cpi].x - pout[cpi].y * pout[cpi].y);

    printf("  restoring data:\n");
    fflush(stdout);
    for (i = 0; i < nin; ++i) {
        point* p = &pin[i];

        p->z = franke(p->x, p->y);
        nnhpi_modify_data(nn, p);
        if (nn_verbose)
            printf("    (%f, %f, %f)\n", p->x, p->y, p->z);
    }

    printf("  interpolating:\n");
    fflush(stdout);
    gettimeofday(&tv0, &tz);
    for (i = 0; i < nout; ++i) {
        point* p = &pout[i];

        nnhpi_interpolate(nn, p);
        if (nn_verbose)
            printf("    (%f, %f, %f)\n", p->x, p->y, p->z);
    }

    fflush(stdout);
    gettimeofday(&tv1, &tz);
    {
        long dt = 1000000.0 * (tv1.tv_sec - tv0.tv_sec) + tv1.tv_usec - tv0.tv_usec;

        printf("    interpolation time = %ld us (%.2f us / point)\n", dt, (double) dt / nout);
    }

    if (!nn_verbose)
        printf("    control point: (%f, %f, %f) (expected z = %f)\n", pout[cpi].x, pout[cpi].y, pout[cpi].z, franke(pout[cpi].x, pout[cpi].y));

    printf("  hashtable stats:\n");
    fflush(stdout);
    {
        hashtable* ht = nn->ht_data;

        printf("    input points: %d entries, %d table elements, %d filled elements\n", ht->n, ht->size, ht->nhash);
        ht = nn->ht_weights;
        printf("    weights: %d entries, %d table elements, %d filled elements\n", ht->n, ht->size, ht->nhash);
    }
    printf("\n");

    nnhpi_destroy(nn);
    free(pout);
    delaunay_destroy(d);
    free(pin);

    return 0;
}

#endif
