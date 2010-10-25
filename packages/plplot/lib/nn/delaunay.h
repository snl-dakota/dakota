/******************************************************************************
 *
 * File:           delaunay.h
 *
 * Created:        04/08/2000
 *
 * Author:         Pavel Sakov
 *                 CSIRO Marine Research
 *
 * Purpose:        Header for delaunay triangulation wrapper
 *
 * Description:    None
 *
 * Revisions:      None
 *
 *****************************************************************************/

#if !defined(_DELAUNAY_H)
#define _DELAUNAY_H

#include "nn.h"

typedef struct {
    int vids[3];
} triangle;

typedef struct {
    int tids[3];
} triangle_neighbours;

typedef struct {
    double x;
    double y;
    double r;
} circle;

#if !defined(_ISTACK_H)
struct istack;
typedef struct istack istack;
#endif

struct delaunay {
    int npoints;
    point* points;
    double xmin;
    double xmax;
    double ymin;
    double ymax;

    int ntriangles;
    triangle* triangles;
    circle* circles;
    triangle_neighbours* neighbours;    /* for delaunay_xytoi() */

    int* n_point_triangles;     /* n_point_triangles[i] is number of
                                 * triangles i-th point belongs to */
    int** point_triangles;      /* point_triangles[i][j] is index of j-th
                                 * triangle i-th point belongs to */

    int nedges;
    int* edges;                 /* n-th edge is formed by points[edges[n*2]]
                                 * and points[edges[n*2+1]] */

    /*
     * Work data for delaunay_circles_find(). Placed here for efficiency
     * reasons. Should be moved to the procedure if parallelizable code
     * needed. 
     */
    int* flags;
    int first_id;               /* last search result, used in start up of a
                                 * new search */
    istack* t_in;
    istack* t_out;
};

#endif
