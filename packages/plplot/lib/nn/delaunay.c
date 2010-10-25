/******************************************************************************
 *
 * File:           delaunay.c
 *
 * Created:        04/08/2000
 *
 * Author:         Pavel Sakov
 *                 CSIRO Marine Research
 *
 * Purpose:        Delaunay triangulation - a wrapper to triangulate()
 *
 * Description:    None
 *
 * Revisions:      10/06/2003 PS: delaunay_build(); delaunay_destroy();
 *                 struct delaunay: from now on, only shallow copy of the
 *                 input data is contained in struct delaunay. This saves
 *                 memory and is consistent with libcsa.
 *
 * Modified:       Joao Cardoso, 4/2/2003
 *                 Adapted for use with Qhull instead of "triangle".
 *
 *****************************************************************************/

#define USE_QHULL

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#ifdef USE_QHULL
#include <qhull/qhull_a.h>
#else
#include "triangle.h"
#endif
#include "istack.h"
#include "nan.h"
#include "delaunay.h"

int circle_build(circle* c, point* p0, point* p1, point* p2);
int circle_contains(circle* c, point* p);

#ifdef USE_QHULL
static int cw(delaunay *d, triangle *t);
#endif

#ifndef USE_QHULL
static void tio_init(struct triangulateio* tio)
{
    tio->pointlist = NULL;
    tio->pointattributelist = NULL;
    tio->pointmarkerlist = NULL;
    tio->numberofpoints = 0;
    tio->numberofpointattributes = 0;
    tio->trianglelist = NULL;
    tio->triangleattributelist = NULL;
    tio->trianglearealist = NULL;
    tio->neighborlist = NULL;
    tio->numberoftriangles = 0;
    tio->numberofcorners = 0;
    tio->numberoftriangleattributes = 0;
    tio->segmentlist = 0;
    tio->segmentmarkerlist = NULL;
    tio->numberofsegments = 0;
    tio->holelist = NULL;
    tio->numberofholes = 0;
    tio->regionlist = NULL;
    tio->numberofregions = 0;
    tio->edgelist = NULL;
    tio->edgemarkerlist = NULL;
    tio->normlist = NULL;
    tio->numberofedges = 0;
}

static void tio_destroy(struct triangulateio* tio)
{
    if (tio->pointlist != NULL)
        free(tio->pointlist);
    if (tio->pointattributelist != NULL)
        free(tio->pointattributelist);
    if (tio->pointmarkerlist != NULL)
        free(tio->pointmarkerlist);
    if (tio->trianglelist != NULL)
        free(tio->trianglelist);
    if (tio->triangleattributelist != NULL)
        free(tio->triangleattributelist);
    if (tio->trianglearealist != NULL)
        free(tio->trianglearealist);
    if (tio->neighborlist != NULL)
        free(tio->neighborlist);
    if (tio->segmentlist != NULL)
        free(tio->segmentlist);
    if (tio->segmentmarkerlist != NULL)
        free(tio->segmentmarkerlist);
    if (tio->holelist != NULL)
        free(tio->holelist);
    if (tio->regionlist != NULL)
        free(tio->regionlist);
    if (tio->edgelist != NULL)
        free(tio->edgelist);
    if (tio->edgemarkerlist != NULL)
        free(tio->edgemarkerlist);
    if (tio->normlist != NULL)
        free(tio->normlist);
}

static delaunay* delaunay_create()
{
    delaunay* d = malloc(sizeof(delaunay));

    d->npoints = 0;
    d->points = NULL;
    d->xmin = DBL_MAX;
    d->xmax = -DBL_MAX;
    d->ymin = DBL_MAX;
    d->ymax = -DBL_MAX;
    d->ntriangles = 0;
    d->triangles = NULL;
    d->circles = NULL;
    d->neighbours = NULL;
    d->n_point_triangles = NULL;
    d->point_triangles = NULL;
    d->nedges = 0;
    d->edges = NULL;
    d->flags = NULL;
    d->first_id = -1;
    d->t_in = NULL;
    d->t_out = NULL;

    return d;
}

static void tio2delaunay(struct triangulateio* tio_out, delaunay* d)
{
    int i, j;

    /*
     * I assume that all input points appear in tio_out in the same order as 
     * they were written to tio_in. I have seen no exceptions so far, even
     * if duplicate points were presented. Just in case, let us make a couple
     * of checks. 
     */
    assert(tio_out->numberofpoints == d->npoints);
    assert(tio_out->pointlist[2 * d->npoints - 2] == d->points[d->npoints - 1].x && tio_out->pointlist[2 * d->npoints - 1] == d->points[d->npoints - 1].y);

    for (i = 0, j = 0; i < d->npoints; ++i) {
        point* p = &d->points[i];

        if (p->x < d->xmin)
            d->xmin = p->x;
        if (p->x > d->xmax)
            d->xmax = p->x;
        if (p->y < d->ymin)
            d->ymin = p->y;
        if (p->y > d->ymax)
            d->ymax = p->y;
    }
    if (nn_verbose) {
        fprintf(stderr, "input:\n");
        for (i = 0, j = 0; i < d->npoints; ++i) {
            point* p = &d->points[i];

            fprintf(stderr, "  %d: %15.7g %15.7g %15.7g\n", i, p->x, p->y, p->z);
        }
    }

    d->ntriangles = tio_out->numberoftriangles;
    if (d->ntriangles > 0) {
        d->triangles = malloc(d->ntriangles * sizeof(triangle));
        d->neighbours = malloc(d->ntriangles * sizeof(triangle_neighbours));
        d->circles = malloc(d->ntriangles * sizeof(circle));
        d->n_point_triangles = calloc(d->npoints, sizeof(int));
        d->point_triangles = malloc(d->npoints * sizeof(int*));
        d->flags = calloc(d->ntriangles, sizeof(int));
    }

    if (nn_verbose)
        fprintf(stderr, "triangles:\n");
    for (i = 0; i < d->ntriangles; ++i) {
        int offset = i * 3;
        triangle* t = &d->triangles[i];
        triangle_neighbours* n = &d->neighbours[i];
        circle* c = &d->circles[i];

        t->vids[0] = tio_out->trianglelist[offset];
        t->vids[1] = tio_out->trianglelist[offset + 1];
        t->vids[2] = tio_out->trianglelist[offset + 2];

        n->tids[0] = tio_out->neighborlist[offset];
        n->tids[1] = tio_out->neighborlist[offset + 1];
        n->tids[2] = tio_out->neighborlist[offset + 2];

        circle_build(c, &d->points[t->vids[0]], &d->points[t->vids[1]], &d->points[t->vids[2]]);

        if (nn_verbose)
            fprintf(stderr, "  %d: (%d,%d,%d)\n", i, t->vids[0], t->vids[1], t->vids[2]);
    }

    for (i = 0; i < d->ntriangles; ++i) {
        triangle* t = &d->triangles[i];

        for (j = 0; j < 3; ++j)
            d->n_point_triangles[t->vids[j]]++;
    }
    if (d->ntriangles > 0) {
        for (i = 0; i < d->npoints; ++i) {
            if (d->n_point_triangles[i] > 0)
                d->point_triangles[i] = malloc(d->n_point_triangles[i] * sizeof(int));
            else
                d->point_triangles[i] = NULL;
            d->n_point_triangles[i] = 0;
        }
    }
    for (i = 0; i < d->ntriangles; ++i) {
        triangle* t = &d->triangles[i];

        for (j = 0; j < 3; ++j) {
            int vid = t->vids[j];

            d->point_triangles[vid][d->n_point_triangles[vid]] = i;
            d->n_point_triangles[vid]++;
        }
    }

    if (tio_out->edgelist != NULL) {
        d->nedges = tio_out->numberofedges;
        d->edges = malloc(d->nedges * 2 * sizeof(int));
        memcpy(d->edges, tio_out->edgelist, d->nedges * 2 * sizeof(int));
    }
}
#endif

/* Builds Delaunay triangulation of the given array of points.
 *
 * @param np Number of points
 * @param points Array of points [np] (input)
 * @param ns Number of forced segments
 * @param segments Array of (forced) segment endpoint indices [2*ns]
 * @param nh Number of holes
 * @param holes Array of hole (x,y) coordinates [2*nh]
 * @return Delaunay triangulation structure with triangulation results
 */
delaunay* delaunay_build(int np, point points[], int ns, int segments[], int nh, double holes[])
#ifndef USE_QHULL
{
    delaunay* d = delaunay_create();
    struct triangulateio tio_in;
    struct triangulateio tio_out;
    char cmd[64] = "eznC";
    int i, j;

    assert(sizeof(REAL) == sizeof(double));

    tio_init(&tio_in);

    if (np == 0) {
        free(d);
        return NULL;
    }

    tio_in.pointlist = malloc(np * 2 * sizeof(double));
    tio_in.numberofpoints = np;
    for (i = 0, j = 0; i < np; ++i) {
        tio_in.pointlist[j++] = points[i].x;
        tio_in.pointlist[j++] = points[i].y;
    }

    if (ns > 0) {
        tio_in.segmentlist = malloc(ns * 2 * sizeof(int));
        tio_in.numberofsegments = ns;
        memcpy(tio_in.segmentlist, segments, ns * 2 * sizeof(int));
    }

    if (nh > 0) {
        tio_in.holelist = malloc(nh * 2 * sizeof(double));
        tio_in.numberofholes = nh;
        memcpy(tio_in.holelist, holes, nh * 2 * sizeof(double));
    }

    tio_init(&tio_out);

    if (!nn_verbose)
        strcat(cmd, "Q");
    else if (nn_verbose > 1)
        strcat(cmd, "VV");
    if (ns != 0)
        strcat(cmd, "p");

    if (nn_verbose)
        fflush(stderr);

    /*
     * climax 
     */
    triangulate(cmd, &tio_in, &tio_out, NULL);

    if (nn_verbose)
        fflush(stderr);

    d->npoints = np;
    d->points = points;

    tio2delaunay(&tio_out, d);

    tio_destroy(&tio_in);
    tio_destroy(&tio_out);

    return d;
}
#else /* USE_QHULL */
{
  delaunay* d = malloc(sizeof(delaunay));

  coordT *qpoints;                     /* array of coordinates for each point */
  boolT ismalloc = False;              /* True if qhull should free points */
  char flags[64] = "qhull d Qbb Qt";   /* option flags for qhull */
  facetT *facet,*neighbor,**neighborp; /* variables to walk through facets */
  vertexT *vertex, **vertexp;          /* variables to walk through vertex */

  int curlong, totlong;                /* memory remaining after qh_memfreeshort */
  FILE *outfile = stdout;
  FILE *errfile = stderr;              /* error messages from qhull code */

  int i, j;
  int exitcode;
  int dim, ntriangles;
  int numfacets, numsimplicial, numridges, totneighbors, numcoplanars, numtricoplanars;	 
    
  dim = 2;

  assert(sizeof(realT) == sizeof(double)); /* Qhull was compiled with doubles? */

  if (np == 0 || ns > 0 || nh > 0) {
    fprintf(stderr, "segments=%d holes=%d\n, aborting Qhull implementation, use 'triangle' instead.\n", ns, nh);
    free(d);
    return NULL;
  }

  qpoints = (coordT *) malloc(np * (dim+1) * sizeof(coordT));

  for (i=0; i<np; i++) {
    qpoints[i*dim] = points[i].x;
    qpoints[i*dim+1] = points[i].y;
  }
   
  if (!nn_verbose)
    outfile = NULL;
  if (nn_verbose)
    strcat(flags, " s");
  if (nn_verbose > 1)
    strcat(flags, " Ts");

  if (nn_verbose)
    fflush(stderr);

  /*
   * climax 
   */

  exitcode = qh_new_qhull (dim, np, qpoints, ismalloc,
			   flags, outfile, errfile);

  if(!exitcode) {

    if (nn_verbose)
      fflush(stderr);

    d->xmin = DBL_MAX;
    d->xmax = -DBL_MAX;
    d->ymin = DBL_MAX;
    d->ymax = -DBL_MAX;

    d->npoints = np;
    d->points = malloc(np * sizeof(point));
    for (i = 0; i < np; ++i) {
      point* p = &d->points[i];

      p->x = points[i].x;
      p->y = points[i].y;
      p->z = points[i].z;

      if (p->x < d->xmin)
	d->xmin = p->x;
      if (p->x > d->xmax)
	d->xmax = p->x;
      if (p->y < d->ymin)
	d->ymin = p->y;
      if (p->y > d->ymax)
	d->ymax = p->y;
    }

    if (nn_verbose) {
      fprintf(stderr, "input:\n");
      for (i = 0; i < np; ++i) {
	point* p = &d->points[i];

	fprintf(stderr, "  %d: %15.7g %15.7g %15.7g\n",
		i, p->x, p->y, p->z);
      }
    }

    qh_findgood_all (qh facet_list);
    qh_countfacets (qh facet_list, NULL, !qh_ALL, &numfacets,
		    &numsimplicial, &totneighbors, &numridges,
		    &numcoplanars, &numtricoplanars);

    ntriangles = 0;
    FORALLfacets {
      if (!facet->upperdelaunay && facet->simplicial)
	ntriangles++;
    }

    d->ntriangles = ntriangles;
    d->triangles = malloc(d->ntriangles * sizeof(triangle));
    d->neighbours = malloc(d->ntriangles * sizeof(triangle_neighbours));
    d->circles = malloc(d->ntriangles * sizeof(circle));

    if (nn_verbose)
      fprintf(stderr, "triangles:\tneighbors:\n");

    i = 0;      
    FORALLfacets {
      if (!facet->upperdelaunay && facet->simplicial) {
	triangle* t = &d->triangles[i];        
	triangle_neighbours* n = &d->neighbours[i];
	circle* c = &d->circles[i];

	j = 0;
	FOREACHvertex_(facet->vertices)
	  t->vids[j++] = qh_pointid(vertex->point);

	j = 0;
	FOREACHneighbor_(facet)
	  n->tids[j++] = neighbor->visitid ? neighbor->visitid - 1 : - 1;

	/* Put triangle vertices in counterclockwise order, as
	 * 'triangle' do.
	 * The same needs to be done with the neighbors.
	 *
	 * The following works, i.e., it seems that Qhull maintains a
	 * relationship between the vertices and the neighbors
	 * triangles, but that is not said anywhere, so if this stop
	 * working in a future Qhull release, you know what you have
	 * to do, reorder the neighbors.
	 */

	if(cw(d, t)) {
	  int tmp = t->vids[1];
	  t->vids[1] = t->vids[2];
	  t->vids[2] = tmp;

	  tmp = n->tids[1];
	  n->tids[1] = n->tids[2];
	  n->tids[2] = tmp;
	}

	circle_build(c, &d->points[t->vids[0]], &d->points[t->vids[1]],
		     &d->points[t->vids[2]]);

	if (nn_verbose)
            fprintf(stderr, "  %d: (%d,%d,%d)\t(%d,%d,%d)\n",
		    i, t->vids[0], t->vids[1], t->vids[2], n->tids[0],
		    n->tids[1], n->tids[2]);

	i++;
      }
    }

    d->flags = calloc(d->ntriangles, sizeof(int));

    d->n_point_triangles = calloc(d->npoints, sizeof(int));
    for (i = 0; i < d->ntriangles; ++i) {
      triangle* t = &d->triangles[i];

      for (j = 0; j < 3; ++j)
	d->n_point_triangles[t->vids[j]]++;
    }
    d->point_triangles = malloc(d->npoints * sizeof(int*));
    for (i = 0; i < d->npoints; ++i) {
      if (d->n_point_triangles[i] > 0)
	d->point_triangles[i] = malloc(d->n_point_triangles[i] * sizeof(int));
      else
	d->point_triangles[i] = NULL;
      d->n_point_triangles[i] = 0;
    }
    for (i = 0; i < d->ntriangles; ++i) {
      triangle* t = &d->triangles[i];

      for (j = 0; j < 3; ++j) {
	int vid = t->vids[j];

	d->point_triangles[vid][d->n_point_triangles[vid]] = i;
	d->n_point_triangles[vid]++;
      }
    }

    d->nedges = 0;
    d->edges = NULL;

    d->t_in = NULL;
    d->t_out = NULL;
    d->first_id = -1;

  } else {
    free(d);
    d = NULL;
  }

  free(qpoints);
  qh_freeqhull(!qh_ALL);                 /* free long memory */
  qh_memfreeshort (&curlong, &totlong);  /* free short memory and memory allocator */
  if (curlong || totlong) 
    fprintf (errfile,
	     "qhull: did not free %d bytes of long memory (%d pieces)\n",
	     totlong, curlong);

  return d;
}

 /* returns 1 if a,b,c are clockwise ordered */
static int cw(delaunay *d, triangle *t)
{
  point* pa = &d->points[t->vids[0]];
  point* pb = &d->points[t->vids[1]];
  point* pc = &d->points[t->vids[2]];

  return ((pb->x - pa->x)*(pc->y - pa->y) <
	  (pc->x - pa->x)*(pb->y - pa->y));
}

#endif

/* Releases memory engaged in the Delaunay triangulation structure.
 *
 * @param d Structure to be destroyed
 */
void delaunay_destroy(delaunay* d)
{
    if (d == NULL)
        return;

    if (d->point_triangles != NULL) {
        int i;

        for (i = 0; i < d->npoints; ++i)
            if (d->point_triangles[i] != NULL)
                free(d->point_triangles[i]);
        free(d->point_triangles);
    }
    if (d->nedges > 0)
        free(d->edges);
#ifdef USE_QHULL
    /* This is a shallow copy if we're not using qhull so we don't
     * need to free it */
    if (d->points != NULL)
        free(d->points);
#endif
    if (d->n_point_triangles != NULL)
        free(d->n_point_triangles);
    if (d->flags != NULL)
        free(d->flags);
    if (d->circles != NULL)
        free(d->circles);
    if (d->neighbours != NULL)
        free(d->neighbours);
    if (d->triangles != NULL)
        free(d->triangles);
    if (d->t_in != NULL)
        istack_destroy(d->t_in);
    if (d->t_out != NULL)
        istack_destroy(d->t_out);
    free(d);
}

/* Returns whether the point p is on the right side of the vector (p0, p1).
 */
static int on_right_side(point* p, point* p0, point* p1)
{
    return (p1->x - p->x) * (p0->y - p->y) > (p0->x - p->x) * (p1->y - p->y);
}

/* Finds triangle specified point belongs to (if any).
 *
 * @param d Delaunay triangulation
 * @param p Point to be mapped
 * @param seed Triangle index to start with
 * @return Triangle id if successful, -1 otherwhile
 */
int delaunay_xytoi(delaunay* d, point* p, int id)
{
    triangle* t;
    int i;

    if (p->x < d->xmin || p->x > d->xmax || p->y < d->ymin || p->y > d->ymax)
        return -1;

    if (id < 0 || id > d->ntriangles)
        id = 0;
    t = &d->triangles[id];
    do {
        for (i = 0; i < 3; ++i) {
            int i1 = (i + 1) % 3;

            if (on_right_side(p, &d->points[t->vids[i]], &d->points[t->vids[i1]])) {
                id = d->neighbours[id].tids[(i + 2) % 3];
                if (id < 0)
                    return id;
                t = &d->triangles[id];
                break;
            }
        }
    } while (i < 3);

    return id;
}

/* Finds all tricircles specified point belongs to.
 *
 * @param d Delaunay triangulation
 * @param p Point to be mapped
 * @param n Pointer to the number of tricircles within `d' containing `p'
 *          (output)
 * @param out Pointer to an array of indices of the corresponding triangles 
 *            [n] (output)
 *
 * There is a standard search procedure involving search through triangle
 * neighbours (not through vertex neighbours). It must be a bit faster due to
 * the smaller number of triangle neighbours (3 per triangle) but can fail
 * for a point outside convex hall.
 *
 * We may wish to modify this procedure in future: first check if the point
 * is inside the convex hall, and depending on that use one of the two
 * search algorithms. It not 100% clear though whether this will lead to a
 * substantial speed gains because of the check on convex hall involved.
 */
void delaunay_circles_find(delaunay* d, point* p, int* n, int** out)
{
    int i;

    if (d->t_in == NULL) {
        d->t_in = istack_create();
        d->t_out = istack_create();
    }

    /*
     * It is important to have a reasonable seed here. If the last search
     * was successful -- start with the last found tricircle, otherwhile (i) 
     * try to find a triangle containing (x,y); if fails then (ii) check
     * tricircles from the last search; if fails then (iii) make linear
     * search through all tricircles 
     */
    if (d->first_id < 0 || !circle_contains(&d->circles[d->first_id], p)) {
        /*
         * if any triangle contains (x,y) -- start with this triangle 
         */
        d->first_id = delaunay_xytoi(d, p, d->first_id);

        /*
         * if no triangle contains (x,y), there still is a chance that it is
         * inside some of circumcircles 
         */
        if (d->first_id < 0) {
            int nn = d->t_out->n;
            int tid = -1;

            /*
             * first check results of the last search 
             */
            for (i = 0; i < nn; ++i) {
                tid = d->t_out->v[i];
                if (circle_contains(&d->circles[tid], p))
                    break;
            }
            /*
             * if unsuccessful, search through all circles 
             */
            if (tid < 0 || tid == nn) {
                double nt = d->ntriangles;

                for (tid = 0; tid < nt; ++tid) {
                    if (circle_contains(&d->circles[tid], p))
                        break;
                }
                if (tid == nt) {
                    istack_reset(d->t_out);
                    *n = 0;
                    *out = NULL;
                    return;     /* failed */
                }
            }
            d->first_id = tid;
        }
    }

    istack_reset(d->t_in);
    istack_reset(d->t_out);

    istack_push(d->t_in, d->first_id);
    d->flags[d->first_id] = 1;

    /*
     * main cycle 
     */
    while (d->t_in->n > 0) {
        int tid = istack_pop(d->t_in);
        triangle* t = &d->triangles[tid];

        if (circle_contains(&d->circles[tid], p)) {
            istack_push(d->t_out, tid);
            for (i = 0; i < 3; ++i) {
                int vid = t->vids[i];
                int nt = d->n_point_triangles[vid];
                int j;

                for (j = 0; j < nt; ++j) {
                    int ntid = d->point_triangles[vid][j];

                    if (d->flags[ntid] == 0) {
                        istack_push(d->t_in, ntid);
                        d->flags[ntid] = 1;
                    }
                }
            }
        }
    }

    *n = d->t_out->n;
    *out = d->t_out->v;
}
