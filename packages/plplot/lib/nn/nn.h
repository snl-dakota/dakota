/******************************************************************************
 *
 * File:           nn.h
 *
 * Created:        04/08/2000
 *
 * Author:         Pavel Sakov
 *                 CSIRO Marine Research
 *
 * Purpose:        Header file for nn library
 *
 * Description:    None
 *
 * Revisions:      None
 *
 *****************************************************************************/

#if !defined(_NN_H)
#define _NN_H

typedef enum { SIBSON, NON_SIBSONIAN } NN_RULE;

#if !defined(_POINT_STRUCT)
#define _POINT_STRUCT
typedef struct {
    double x;
    double y;
    double z;
} point;
#endif

/** Smoothes the input point array by averaging the input x,y and z values
 ** for each cell within virtual rectangular nx by ny grid. The corners of the
 ** grid are created from min and max values of the input array. It also frees
 ** the original array and returns results and new dimension via original
 ** data and size pointers.
 *
 * @param pn Pointer to number of points (input/output)
 * @param ppoints Pointer to array of points (input/output) [*pn]
 * @param nx Number of x nodes in decimation
 * @param ny Number of y nodes in decimation
 */
void points_thin(int* n, point** points, int nx, int ny);

/** Generates rectangular grid nx by ny using min and max x and y values from
 ** the input point array. Allocates space for the output point array, be sure
 ** to free it when necessary!
 *
 * @param n Number of points
 * @param points Array of points [n]
 * @param nx Number of x nodes
 * @param ny Number of y nodes
 * @param nout Pointer to number of output points
 * @param pout Ppointer to array of output points [*nout]
 */
void points_generate1(int n, point points[], int nx, int ny, double zoom, int* nout, point** pout);

/** Generates rectangular grid nx by ny using specified min and max x and y 
 ** values. Allocates space for the output point array, be sure to free it
 ** when necessary!
 *
 * @param xmin Min x value
 * @param xmax Max x value
 * @param ymin Min y value
 * @param ymax Max y value
 * @param nx Number of x nodes
 * @param ny Number of y nodes
 * @param zoom Zoom coefficient
 * @param nout Pointer to number of output points
 * @param pout Pointer to array of output points [*nout]
 */
void points_generate2(double xmin, double xmax, double ymin, double ymax, int nx, int ny, int* nout, point** pout);

/** Reads array of points from a columnar file.
 *
 * @param fname File name (can be "stdin" dor stndard input)
 * @param dim Number of dimensions (must be 2 or 3)
 * @param n Pointer to number of points (output)
 * @param points Pointer to array of points [*n] (output)
 */
void points_read(char* fname, int dim, int* n, point** points);

/** Scales Y coordinate so that the resulting set fits into square:
 ** xmax - xmin = ymax - ymin
 *
 * @param n Number of points
 * @param points The points to scale
 * @return Y axis compression coefficient
 */
double points_scaletosquare(int n, point* points);

/** Compresses Y domain by a given multiple.
 *
 * @param n Number of points
 * @param points The points to scale
 * @param Y axis compression coefficient as returned by points_scaletosquare()
 */
void points_scale(int n, point* points, double k);

/** Structure to perform the Delaunay triangulation of a given array of points.
 *
 * Contains a deep copy of the input array of points.
 * Contains triangles, circles and edges resulted from the triangulation.
 * Contains neighbour triangles for each triangle.
 * Contains point to triangle map.
 */
struct delaunay;
typedef struct delaunay delaunay;

/** Builds Delaunay triangulation of the given array of points.
 *
 * @param np Number of points
 * @param points Array of points [np] (input)
 * @param ns Number of forced segments
 * @param segments Array of (forced) segment endpoint indices [2*ns]
 * @param nh Number of holes
 * @param holes Array of hole (x,y) coordinates [2*nh]
 * @return Delaunay triangulation with triangulation results
 */
delaunay* delaunay_build(int np, point points[], int ns, int segments[], int nh, double holes[]);

/** Destroys Delaunay triangulation.
 *
 * @param d Structure to be destroyed
 */
void delaunay_destroy(delaunay* d);

/** `lpi' -- "linear point interpolator" is a structure for
 * conducting linear interpolation on a given data on a "point-to-point" basis.
 * It interpolates linearly within each triangle resulted from the Delaunay
 * triangluation of input data. `lpi' is much faster than all
 * Natural Neighbours interpolators below.
 */
struct lpi;
typedef struct lpi lpi;

/** Builds linear interpolator.
 *
 * @param d Delaunay triangulation
 * @return Linear interpolator
 */
lpi* lpi_build(delaunay* d);

/** Destroys linear interpolator.
 *
 * @param l Structure to be destroyed
 */
void lpi_destroy(lpi* l);

/** Finds linearly interpolated value in a point.
 *
 * @param l Linear point interpolator
 * @param p Point to be interpolated (p->x, p->y -- input; p->z -- output)
 */
void lpi_interpolate_point(lpi* l, point* p);

/* Linearly interpolates data from one array of points for another array of
 * points.
 *
 * @param nin Number of input points
 * @param pin Array of input points [pin]
 * @param nout Number of ouput points
 * @param pout Array of output points [nout]
 */
void lpi_interpolate_points(int nin, point pin[], int nout, point pout[]);

/** `nnpi' -- "Natural Neighbours point interpolator" is a
 * structure for conducting Natural Neighbours interpolation on a given data on
 * a "point-to-point" basis. Because it involves weight calculation for each
 * next output point, it is not particularly suitable for consequitive
 * interpolations on the same set of observation points -- use 
 * `nnhpi' or `nnai' in these cases.
 */
struct nnpi;
typedef struct nnpi nnpi;

/** Creates Natural Neighbours point interpolator.
 *
 * @param d Delaunay triangulation
 * @return Natural Neighbours interpolation
 */
nnpi* nnpi_create(delaunay* d);

/** Destroys Natural Neighbours point interpolation.
 *
 * @param nn Structure to be destroyed
 */
void nnpi_destroy(nnpi* nn);

/** Finds Natural Neighbours-interpolated value in a point.
 *
 * @param nn NN point interpolator
 * @param p Point to be interpolated (p->x, p->y -- input; p->z -- output)
 */
void nnpi_interpolate_point(nnpi* nn, point* p);

/** Natural Neighbours-interpolates data in one array of points for another
 ** array of points.
 *
 * @param nin Number of input points
 * @param pin Array of input points [pin]
 * @param wmin Minimal allowed weight
 * @param nout Number of output points
 * @param pout Array of output points [nout]
 */
void nnpi_interpolate_points(int nin, point pin[], double wmin, int nout, point pout[]);

/** Sets minimal allowed weight for Natural Neighbours interpolation.
 * @param nn Natural Neighbours point interpolator
 * @param wmin Minimal allowed weight
 */
void nnpi_setwmin(nnpi* nn, double wmin);

/** `nnhpi' is a structure for conducting consequitive
 * Natural Neighbours interpolations on a given spatial data set in a random
 * sequence of points from a set of finite size, taking advantage of repeated
 * interpolations in the same point. It allows to modify Z
 * coordinate of data between interpolations.
 */
struct nnhpi;
typedef struct nnhpi nnhpi;

/** Creates Natural Neighbours hashing point interpolator.
 *
 * @param d Delaunay triangulation
 * @param size Hash table size (should be of order of number of output points)
 * @return Natural Neighbours interpolation
 */
nnhpi* nnhpi_create(delaunay* d, int size);

/** Destroys Natural Neighbours hashing point interpolation.
 *
 * @param nn Structure to be destroyed
 */
void nnhpi_destroy(nnhpi* nn);

/** Finds Natural Neighbours-interpolated value in a point.
 *
 * @param nnhpi NN hashing point interpolator
 * @param p Point to be interpolated (p->x, p->y -- input; p->z -- output)
 */
void nnhpi_interpolate(nnhpi* nn, point* p);

/** Modifies interpolated data.
 * Finds point* pd in the underlying Delaunay triangulation such that
 * pd->x = p->x and pd->y = p->y, and copies p->z to pd->z. Exits with error
 * if the point is not found.
 *
 * @param nn Natural Neighbours hashing point interpolator
 * @param p New data
 */
void nnhpi_modify_data(nnhpi* nn, point* p);

/** Sets minimal allowed weight for Natural Neighbours interpolation.
 * @param nn Natural Neighbours point hashing interpolator
 * @param wmin Minimal allowed weight
 */
void nnhpi_setwmin(nnhpi* nn, double wmin);

/* `nnai' is a tructure for conducting consequitive Natural
 * Neighbours interpolations on a given spatial data set in a given array of
 * points. It allows to modify Z coordinate of data between interpolations.
 * `nnai' is the fastest of the three Natural Neighbours
 * interpolators here.
 */
struct nnai;
typedef struct nnai nnai;

/** Builds Natural Neighbours array interpolator. This includes calculation of
 * weights used in nnai_interpolate().
 *
 * @param d Delaunay triangulation
 * @return Natural Neighbours interpolation
 */
nnai* nnai_build(delaunay* d, int n, double* x, double* y);

/** Destroys Natural Neighbours array interpolator.
 *
 * @param nn Structure to be destroyed
 */
void nnai_destroy(nnai* nn);

/** Conducts NN interpolation in a fixed array of output points using 
 * data specified for a fixed array of input points. Uses pre-calculated
 * weights.
 *
 * @param nn NN array interpolator
 * @param zin input data [nn->d->npoints]
 * @param zout output data [nn->n]. Must be pre-allocated!
 */
void nnai_interpolate(nnai* nn, double* zin, double* zout);

/** Sets minimal allowed weight for Natural Neighbours interpolation.
 * @param nn Natural Neighbours array interpolator
 * @param wmin Minimal allowed weight
 */
void nnai_setwmin(nnai* nn, double wmin);

/* Sets the verbosity level within nn package.
 * 0 (default) - silent
 * 1 - verbose
 * 2 - very verbose
 */
extern int nn_verbose;

/* Switches between weight calculation methods.
 * SIBSON -- classic Sibson method
 * NON_SIBSONIAN -- simpler and (I think) more robust method
 */
extern NN_RULE nn_rule;

/* Contains version string for the nn package.
 */
extern char* nn_version;

/* Limits verbose information to a particular vertex (used mainly for
 * debugging purposes).
 */
extern int nn_test_vertice;

#endif                          /* _NN_H */
