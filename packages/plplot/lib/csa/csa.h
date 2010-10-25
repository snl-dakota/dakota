/******************************************************************************
 *
 * File:           csa.h
 *
 * Created:        16/10/2002
 *
 * Author:         Pavel Sakov
 *                 CSIRO Marine Research
 *
 * Purpose:        A header for csa library (2D data approximation with
 *                 bivariate C1 cubic spline)
 *
 * Revisions:      None
 *
 *****************************************************************************/

#if !defined(_CSA_H)
#define _CSA_H

#if !defined(_POINT_STRUCT)
#define _POINT_STRUCT
typedef struct {
    double x;
    double y;
    double z;
} point;
#endif

extern int csa_verbose;
extern char* csa_version;

struct csa;
typedef struct csa csa;

csa* csa_create();
void csa_destroy(csa* a);
void csa_addpoints(csa* a, int n, point points[]);
void csa_calculatespline(csa* a);
void csa_approximate_point(csa* a, point* p);
void csa_approximate_points(csa* a, int n, point* points);

void csa_setnpmin(csa* a, int npmin);
void csa_setnpmax(csa* a, int npmax);
void csa_setk(csa* a, int k);
void csa_setnpps(csa* a, double npps);

#endif
