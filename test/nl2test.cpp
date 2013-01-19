/* nl2test.f -- translated by f2c (version 20030320).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include <cmath>

/* #include "f2c.h" */
typedef int integer;
typedef float real;
typedef double doublereal;
typedef int /* Unknown procedure type */ (*U_fp)();
#define min(a,b) ((a) <= (b) ? (a) : (b))

/* Table of constant values */

static integer c__2 = 2;
static integer c__5 = 5;

/* Subroutine */ int testj_(integer *n, integer *p, doublereal *x, integer *
	nfcall, doublereal *j, integer *uiparm, doublereal *urparm, U_fp
	ufparm)
{
    /* Initialized data */

    static doublereal twopi = 6.283185307179586;
    static doublereal zero = 0.;
    static doublereal ukow[11] = { 4.,2.,1.,.5,.25,.167,.125,.1,.0833,.0714,
	    .0625 };
    static doublereal expmin = 0.;
    static doublereal uftolg = 0.;

    /* System generated locals */
    integer j_dim1, j_offset, i__1, i__2;
    doublereal d__1, d__2;

    /* Local variables */
    static doublereal e;
    static integer i__, k;
    static doublereal t, u, v, w, z__, r2, ti;
    static integer nm1, nex;
    static doublereal tpi, tim1, tip1, tpim1, tpip1, theta;
    extern doublereal rmdcon_(integer *);


/*  ***  parameters  *** */


/*     .................................................................. */
/*     .................................................................. */

/*     *****purpose. */
/*     this routine evaluates the jacobian matrix  j  for the various */
/*     test problems listed in references (1), (2), and (3). */

/*     *****parameter description. */
/*     on input. */

/*        nn is the row dimension of  j  as declared in the calling */
/*             program. */
/*        n is the actual number of rows in  j  and is the length of  r. */
/*        p is the number of parameters being estimated and hence is */
/*             the length of x. */
/*        x is the vector of parameters at which the jacobian matrix  j */
/*             is to be computed. */
/*        nfcall is the invocation count of  testr  at the time when  r */
/*             was evaluated at  x.  testr ignores nfcall. */
/*        r is the residual vector at  x  (and is ignored). */
/*        nex = uiparm(1) is the index of the problem currently being */
/*             solved. */
/*        urparm is a user parameter vector (and is ignored). */
/*        ufparm is a user entry point parameter (and is ignored). */
/*        testr is the subroutine that computes  r  (and is ignored). */

/*     on output. */

/*        j is the jacobian matrix at x. */

/*     *****application and usage restrictions. */
/*     these test problems may be used to test least-squares solvers */
/*     such as nl2sol.  in particular, these problems may be used to */
/*     check whether  nl2sol  has been successfully transported to */
/*     a particular machine. */

/*     *****algorithm notes. */
/*     none */

/*     *****subroutines and functions called. */
/*     none */

/*     *****references */
/*     (1) gill, p.e.; & murray, w. (1976), algorithms for the solution */
/*        of the non-linear least-squares problem, npl report nac71. */

/*     (2) meyer, r.r. (1970), theoretical and computational aspects */
/*        of nonlinear regression, pp. 465-486 of nonlinear programming, */
/*        edited by j.b. rosen, o.l.mangasarian, and k. ritter, */
/*        academic press, new york. */

/*     (3) brown, k.m. (1969), a quadratically convergent newton- */
/*        like method based upon gaussian elimination, */
/*        siam j. numer. anal. 6, pp. 560-569. */

/*     *****general. */

/*     this subroutine was written in connection with research */
/*     supported by the national science foundation under grants */
/*     mcs-7600324, dcr75-10143, 76-14311dss, and mcs76-11989. */

/*     .................................................................. */
/*     .................................................................. */

/*  ***  local variables and constants  *** */

/*  ***  intrinsic functions  *** */
/* /+ */
/* / */

/* /6 */
/* /6                                                                    t */
    /* Parameter adjustments */
    j_dim1 = *n;
    j_offset = 1 + j_dim1;
    j -= j_offset;
    --x;
    --uiparm;
    --urparm;

    /* Function Body */
/* /7 */
/*     parameter (twopi=6.283185307179586d+0, zero=0.d+0) */
/* / */
/* /6 */
/* /7 */
/*     save expmin, uftolg */
/* / */
/*  ***  machine dependent constant  *** */


/* ----------------------------------------------------------------------- */

    nex = uiparm[1];
    switch (nex) {
	case 1:  goto L100;
	case 2:  goto L200;
	case 3:  goto L300;
	case 4:  goto L400;
	case 5:  goto L500;
	case 6:  goto L600;
	case 7:  goto L700;
	case 8:  goto L800;
	case 9:  goto L900;
	case 10:  goto L1000;
	case 11:  goto L1100;
	case 12:  goto L1200;
	case 13:  goto L1300;
	case 14:  goto L1400;
	case 15:  goto L1500;
	case 16:  goto L1600;
	case 17:  goto L1700;
	case 18:  goto L1800;
	case 19:  goto L1900;
	case 20:  goto L2000;
	case 21:  goto L2100;
	case 22:  goto L2200;
	case 23:  goto L2300;
	case 24:  goto L2400;
	case 25:  goto L2500;
	case 26:  goto L2600;
	case 27:  goto L2700;
	case 28:  goto L2800;
	case 29:  goto L2900;
	case 30:  goto L1900;
	case 31:  goto L2100;
	case 32:  goto L2500;
	case 33:  goto L1300;
	case 34:  goto L1400;
	case 35:  goto L1500;
	case 36:  goto L1600;
    }

/*  ***  rosenbrock  *** */
L100:
    j[j_dim1 + 1] = x[1] * -20.;
    j[(j_dim1 << 1) + 1] = 10.;
    j[j_dim1 + 2] = -1.;
    j[(j_dim1 << 1) + 2] = 0.;
    goto L9999;
/*  ***  helix  *** */
L200:
/* Computing 2nd power */
    d__1 = x[1];
/* Computing 2nd power */
    d__2 = x[2];
    t = d__1 * d__1 + d__2 * d__2;
    ti = 100. / (twopi * t);
    j[j_dim1 + 1] = ti * x[2];
    t = 10. / std::sqrt(t);
    j[j_dim1 + 2] = x[1] * t;
    j[j_dim1 + 3] = 0.;
    j[(j_dim1 << 1) + 1] = -ti * x[1];
    j[(j_dim1 << 1) + 2] = x[2] * t;
    j[(j_dim1 << 1) + 3] = 0.;
    j[j_dim1 * 3 + 1] = 10.;
    j[j_dim1 * 3 + 2] = 0.;
    j[j_dim1 * 3 + 3] = 1.;
    goto L9999;
/*  ***  singular  *** */
L300:
    for (k = 1; k <= 4; ++k) {
	for (i__ = 1; i__ <= 4; ++i__) {
/* L301: */
	    j[i__ + k * j_dim1] = 0.;
	}
    }
    j[j_dim1 + 1] = 1.;
    j[(j_dim1 << 1) + 1] = 10.;
    j[j_dim1 * 3 + 2] = std::sqrt(5.);
    j[(j_dim1 << 2) + 2] = -j[j_dim1 * 3 + 2];
    j[(j_dim1 << 1) + 3] = (x[2] - x[3] * 2.) * 2.;
    j[j_dim1 * 3 + 3] = j[(j_dim1 << 1) + 3] * -2.;
    j[j_dim1 + 4] = std::sqrt(40.) * (x[1] - x[4]);
    j[(j_dim1 << 2) + 4] = -j[j_dim1 + 4];
    goto L9999;
/*  ***  woods  *** */
L400:
    for (k = 1; k <= 4; ++k) {
	for (i__ = 1; i__ <= 7; ++i__) {
/* L401: */
	    j[i__ + k * j_dim1] = 0.;
	}
    }
    j[j_dim1 + 1] = x[1] * -20.;
    j[(j_dim1 << 1) + 1] = 10.;
    j[j_dim1 + 2] = -1.;
    j[(j_dim1 << 2) + 3] = std::sqrt(90.);
    j[j_dim1 * 3 + 3] = x[3] * -2. * j[(j_dim1 << 2) + 3];
    j[j_dim1 * 3 + 4] = -1.;
    j[(j_dim1 << 1) + 5] = std::sqrt(9.9);
    j[(j_dim1 << 2) + 5] = j[(j_dim1 << 1) + 5];
    j[(j_dim1 << 1) + 6] = std::sqrt(.2);
    j[(j_dim1 << 2) + 7] = j[(j_dim1 << 1) + 6];
    goto L9999;
/*  ***  zangwill  *** */
L500:
    for (k = 1; k <= 3; ++k) {
	for (i__ = 1; i__ <= 3; ++i__) {
/* L501: */
	    j[i__ + k * j_dim1] = 1.;
	}
    }
    j[(j_dim1 << 1) + 1] = -1.;
    j[j_dim1 + 2] = -1.;
    j[j_dim1 * 3 + 3] = -1.;
    goto L9999;
/*  ***  engvall  *** */
L600:
    j[j_dim1 + 1] = x[1] * 2.;
    j[(j_dim1 << 1) + 1] = x[2] * 2.;
    j[j_dim1 * 3 + 1] = x[3] * 2.;
    j[j_dim1 + 2] = j[j_dim1 + 1];
    j[(j_dim1 << 1) + 2] = j[(j_dim1 << 1) + 1];
    j[j_dim1 * 3 + 2] = (x[3] - 2.) * 2.;
    j[j_dim1 + 3] = 1.;
    j[(j_dim1 << 1) + 3] = 1.;
    j[j_dim1 * 3 + 3] = 1.;
    j[j_dim1 + 4] = 1.;
    j[(j_dim1 << 1) + 4] = 1.;
    j[j_dim1 * 3 + 4] = -1.;
    t = (x[3] * 5. - x[1] + 1.) * 2.;
/* Computing 2nd power */
    d__1 = x[1];
    j[j_dim1 + 5] = d__1 * d__1 * 3. - t;
    j[(j_dim1 << 1) + 5] = x[2] * 6.;
    j[j_dim1 * 3 + 5] = t * 5.;
    goto L9999;
/*  ***  branin  *** */
L700:
    j[j_dim1 + 1] = 4.;
    j[(j_dim1 << 1) + 1] = 4.;
    j[j_dim1 + 2] = (x[1] - 2.) * (x[1] * 3. - x[2] * 2. - 2.) + 3. + x[2] *
	    x[2];
/* Computing 2nd power */
    d__1 = x[1] - x[2];
    j[(j_dim1 << 1) + 2] = (x[1] * 2. - x[2] * x[2]) * 2. + 1. - d__1 * d__1;
    goto L9999;
/*  ***  beale  *** */
L800:
    j[j_dim1 + 1] = x[2] - 1.;
    j[(j_dim1 << 1) + 1] = x[1];
/* Computing 2nd power */
    d__1 = x[2];
    j[j_dim1 + 2] = d__1 * d__1 - 1.;
    j[(j_dim1 << 1) + 2] = x[1] * 2. * x[2];
/* Computing 3rd power */
    d__1 = x[2];
    j[j_dim1 + 3] = d__1 * (d__1 * d__1) - 1.;
/* Computing 2nd power */
    d__1 = x[2];
    j[(j_dim1 << 1) + 3] = x[1] * 3. * (d__1 * d__1);
    goto L9999;
/*  ***  cragg & levy  *** */
L900:
    for (i__ = 1; i__ <= 5; ++i__) {
	for (k = 1; k <= 4; ++k) {
/* L901: */
	    j[i__ + k * j_dim1] = 0.;
	}
    }
    t = std::exp(x[1]);
    j[(j_dim1 << 1) + 1] = (t - x[2]) * -2.;
    j[j_dim1 + 1] = -t * j[(j_dim1 << 1) + 1];
/* Computing 2nd power */
    d__1 = x[2] - x[3];
    j[(j_dim1 << 1) + 2] = d__1 * d__1 * 30.;
    j[j_dim1 * 3 + 2] = -j[(j_dim1 << 1) + 2];
/* Computing 3rd power */
    d__1 = std::cos(x[3] - x[4]);
    j[j_dim1 * 3 + 3] = std::sin(x[3] - x[4]) * 2. / (d__1 * (d__1 * d__1));
    j[(j_dim1 << 2) + 3] = -j[j_dim1 * 3 + 3];
/* Computing 3rd power */
    d__1 = x[1];
    j[j_dim1 + 4] = d__1 * (d__1 * d__1) * 4.;
    j[(j_dim1 << 2) + 5] = 1.;
    goto L9999;
/*  ***  box  *** */
L1000:
    if (expmin == zero) {
	expmin = std::log(rmdcon_(&c__2)) * 1.999;
    }
    for (i__ = 1; i__ <= 10; ++i__) {
	ti = (doublereal) ((real) i__) * -.1;
	e = zero;
	t = x[1] * ti;
	if (t >= expmin) {
	    e = std::exp(t);
	}
	j[i__ + j_dim1] = ti * e;
	e = zero;
	t = x[2] * ti;
	if (t >= expmin) {
	    e = std::exp(t);
	}
	j[i__ + (j_dim1 << 1)] = -ti * e;
	j[i__ + j_dim1 * 3] = std::exp(ti * 10.) - std::exp(ti);
/* L1001: */
    }
    goto L9999;
/*  ***  davidon 1  *** */
L1100:
    nm1 = *n - 1;
    i__1 = nm1;
    for (i__ = 1; i__ <= i__1; ++i__) {
	ti = (doublereal) ((real) i__);
	t = 1.;
	i__2 = *p;
	for (k = 1; k <= i__2; ++k) {
	    j[i__ + k * j_dim1] = t;
	    t *= ti;
/* L1101: */
	}
    }
    j[*n + j_dim1] = 1.;
    i__2 = *p;
    for (k = 2; k <= i__2; ++k) {
/* L1102: */
	j[*n + k * j_dim1] = 0.;
    }
    goto L9999;
/*  ***  freudenstein & roth  *** */
L1200:
    j[j_dim1 + 1] = 1.;
    j[(j_dim1 << 1) + 1] = x[2] * (10. - x[2] * 3.) - 2.;
    j[j_dim1 + 2] = 1.;
    j[(j_dim1 << 1) + 2] = x[2] * (x[2] * 3. + 2.) - 14.;
    goto L9999;
/*  ***  watson  *** */
L1300:
L1400:
L1500:
L1600:
    for (i__ = 1; i__ <= 29; ++i__) {
	ti = (doublereal) ((real) i__) / 29.;
	r2 = x[1];
	t = 1.;
	i__2 = *p;
	for (k = 2; k <= i__2; ++k) {
	    t *= ti;
	    r2 += t * x[k];
/* L1601: */
	}
	r2 *= -2.;
	j[i__ + j_dim1] = r2;
	t = 1.;
	r2 = ti * r2;
	i__2 = *p;
	for (k = 2; k <= i__2; ++k) {
	    i__1 = k - 1;
	    j[i__ + k * j_dim1] = t * ((doublereal) ((real) i__1) + r2);
	    t *= ti;
/* L1602: */
	}
/* L1603: */
    }
    for (i__ = 30; i__ <= 31; ++i__) {
	i__2 = *p;
	for (k = 2; k <= i__2; ++k) {
/* L1604: */
	    j[i__ + k * j_dim1] = 0.;
	}
    }
    j[j_dim1 + 30] = 1.;
    j[j_dim1 + 31] = x[1] * -2.;
    j[(j_dim1 << 1) + 31] = 1.;
    goto L9999;
/*  ***  chebyquad  *** */
L1700:
    i__2 = *n;
    for (k = 1; k <= i__2; ++k) {
	tim1 = -1. / (doublereal) ((real) (*n));
	z__ = x[k] * 2. - 1.;
	ti = z__ * tim1;
	tpim1 = 0.;
	tpi = tim1 * 2.;
	z__ += z__;
	i__1 = *n;
	for (i__ = 1; i__ <= i__1; ++i__) {
	    j[i__ + k * j_dim1] = tpi;
	    tpip1 = ti * 4. + z__ * tpi - tpim1;
	    tpim1 = tpi;
	    tpi = tpip1;
	    tip1 = z__ * ti - tim1;
	    tim1 = ti;
	    ti = tip1;
/* L1701: */
	}
    }
    goto L9999;
/*  ***  brown and dennis  *** */
L1800:
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
	ti = (doublereal) ((real) i__) * .2;
	j[i__ + j_dim1] = (x[1] + x[2] * ti - std::exp(ti)) * 2.;
	j[i__ + (j_dim1 << 1)] = ti * j[i__ + j_dim1];
	t = std::sin(ti);
	j[i__ + j_dim1 * 3] = (x[3] + x[4] * t - std::cos(ti)) * 2.;
	j[i__ + (j_dim1 << 2)] = t * j[i__ + j_dim1 * 3];
/* L1801: */
    }
    goto L9999;
/*  ***  bard  *** */
L1900:
    for (i__ = 1; i__ <= 15; ++i__) {
	j[i__ + j_dim1] = -1.;
	u = (doublereal) ((real) i__);
	v = 16. - u;
	w = min(u,v);
/* Computing 2nd power */
	d__1 = x[2] * v + x[3] * w;
	t = u / (d__1 * d__1);
	j[i__ + (j_dim1 << 1)] = v * t;
	j[i__ + j_dim1 * 3] = w * t;
/* L1901: */
    }
    goto L9999;
/*  *** jennrich & sampson  *** */
L2000:
    for (i__ = 1; i__ <= 10; ++i__) {
	ti = (doublereal) ((real) i__);
	j[i__ + j_dim1] = -ti * std::exp(ti * x[1]);
	j[i__ + (j_dim1 << 1)] = -ti * std::exp(ti * x[2]);
/* L2001: */
    }
    goto L9999;
/*  ***  kowalik & osborne  *** */
L2100:
    for (i__ = 1; i__ <= 11; ++i__) {
/* Computing 2nd power */
	d__1 = ukow[i__ - 1];
	t = -1. / (d__1 * d__1 + x[3] * ukow[i__ - 1] + x[4]);
/* Computing 2nd power */
	d__1 = ukow[i__ - 1];
	j[i__ + j_dim1] = t * (d__1 * d__1 + x[2] * ukow[i__ - 1]);
	j[i__ + (j_dim1 << 1)] = x[1] * ukow[i__ - 1] * t;
	t = t * j[i__ + j_dim1] * x[1];
	j[i__ + j_dim1 * 3] = ukow[i__ - 1] * t;
	j[i__ + (j_dim1 << 2)] = t;
/* L2101: */
    }
    goto L9999;
/*  ***  osborne 1  *** */
L2200:
    for (i__ = 1; i__ <= 33; ++i__) {
	i__1 = 1 - i__;
	ti = (doublereal) ((real) i__1) * 10.;
	j[i__ + j_dim1] = -1.;
	j[i__ + (j_dim1 << 1)] = -std::exp(x[4] * ti);
	j[i__ + j_dim1 * 3] = -std::exp(x[5] * ti);
	j[i__ + (j_dim1 << 2)] = ti * x[2] * j[i__ + (j_dim1 << 1)];
	j[i__ + j_dim1 * 5] = ti * x[3] * j[i__ + j_dim1 * 3];
/* L2201: */
    }
    goto L9999;
/*  ***  osborne 2  *** */
/*     ***  uftolg is a machine-dependent constant.  it is just slightly */
/*     ***  larger than the log of the smallest positive machine number. */
L2300:
    if (uftolg == 0.) {
	uftolg = std::log(rmdcon_(&c__2)) * 1.999;
    }
    for (i__ = 1; i__ <= 65; ++i__) {
	i__1 = 1 - i__;
	ti = (doublereal) ((real) i__1) * .1;
	j[i__ + j_dim1] = -std::exp(x[5] * ti);
	j[i__ + j_dim1 * 5] = x[1] * ti * j[i__ + j_dim1];
	for (k = 2; k <= 4; ++k) {
	    t = x[k + 7] + ti;
	    r2 = 0.;
	    theta = -x[k + 4] * t * t;
	    if (theta > uftolg) {
		r2 = -std::exp(theta);
	    }
	    j[i__ + k * j_dim1] = r2;
	    r2 = -t * r2 * x[k];
	    j[i__ + (k + 4) * j_dim1] = r2 * t;
	    j[i__ + (k + 7) * j_dim1] = x[k + 4] * 2. * r2;
/* L2301: */
	}
/* L2302: */
    }
    goto L9999;
/*  ***  madsen  *** */
L2400:
    j[j_dim1 + 1] = x[1] * 2. + x[2];
    j[(j_dim1 << 1) + 1] = x[2] * 2. + x[1];
    j[j_dim1 + 2] = std::cos(x[1]);
    j[(j_dim1 << 1) + 2] = 0.;
    j[j_dim1 + 3] = 0.;
    j[(j_dim1 << 1) + 3] = -std::sin(x[2]);
    goto L9999;
/*  ***  meyer  *** */
L2500:
    for (i__ = 1; i__ <= 16; ++i__) {
	i__1 = i__ * 5 + 45;
	ti = (doublereal) ((real) i__1);
	u = ti + x[3];
	t = std::exp(x[2] / u);
	j[i__ + j_dim1] = t;
	j[i__ + (j_dim1 << 1)] = x[1] * t / u;
	j[i__ + j_dim1 * 3] = -x[1] * x[2] * t / (u * u);
/* L2501: */
    }
    goto L9999;
/*  ***  brown  *** */
L2600:
L2700:
L2800:
L2900:
    nm1 = *n - 1;
    i__1 = *n;
    for (k = 1; k <= i__1; ++k) {
	i__2 = nm1;
	for (i__ = 1; i__ <= i__2; ++i__) {
	    j[i__ + k * j_dim1] = 1.;
	    if (i__ == k) {
		j[i__ + k * j_dim1] = 2.;
	    }
/* L2901: */
	}
    }
    i__2 = *n;
    for (k = 1; k <= i__2; ++k) {
	t = 1.;
	i__1 = *n;
	for (i__ = 1; i__ <= i__1; ++i__) {
	    if (i__ != k) {
		t *= x[i__];
	    }
/* L2902: */
	}
	j[*n + k * j_dim1] = t;
/* L2903: */
    }
    goto L9999;


L9999:
    return 0;
} /* testj_ */

/* Subroutine */ int testr_(integer *n, integer *p, doublereal *x, integer *
	nfcall, doublereal *r__, integer *uiparm, doublereal *urparm, U_fp
	ufparm)
{
    /* Initialized data */

    static doublereal twopi = 6.283185307179586;
    static doublereal ybard[15] = { .14,.18,.22,.25,.29,.32,.35,.39,.37,.58,
	    .73,.96,1.34,2.1,4.39 };
    static doublereal ykow[11] = { .1957,.1947,.1735,.16,.0844,.0627,.0456,
	    .0342,.0323,.0235,.0246 };
    static doublereal ukow[11] = { 4.,2.,1.,.5,.25,.167,.125,.1,.0833,.0714,
	    .0625 };
    static doublereal yosb1[33] = { .844,.908,.932,.936,.925,.908,.881,.85,
	    .818,.784,.751,.718,.685,.658,.628,.603,.58,.558,.538,.522,.506,
	    .49,.478,.467,.457,.448,.438,.431,.424,.42,.414,.411,.406 };
    static doublereal yosb2[65] = { 1.366,1.191,1.112,1.013,.991,.885,.831,
	    .847,.786,.725,.746,.679,.608,.655,.616,.606,.602,.626,.651,.724,
	    .649,.649,.694,.644,.624,.661,.612,.558,.533,.495,.5,.423,.395,
	    .375,.372,.391,.396,.405,.428,.429,.523,.562,.607,.653,.672,.708,
	    .633,.668,.645,.632,.591,.559,.597,.625,.739,.71,.729,.72,.636,
	    .581,.428,.292,.162,.098,.054 };
    static doublereal ymeyer[16] = { 34780.,28610.,23650.,19630.,16370.,
	    13720.,11540.,9744.,8261.,7030.,6005.,5147.,4427.,3820.,3307.,
	    2872. };
    static doublereal expmax = 0.;
    static doublereal uftolg = 0.;

    /* System generated locals */
    integer i__1, i__2;
    doublereal d__1, d__2, d__3;

    /* Local variables */
    static integer i__, j;
    static doublereal t, u, v, w, z__, e1, e2, r1, r2, t1, t2, ri, ti;
    static integer nm1, nex;
    static doublereal tim1, tip1, theta;
    extern doublereal rmdcon_(integer *);
    static doublereal floatn, expmin;


/*     *****parameters. */


/*     .................................................................. */
/*     .................................................................. */

/*     *****purpose. */
/*     this routine evaluates  r  for the various test functions in */
/*        references (1), (2), and (3), as well as for some variations */
/*        suggested by jorge more (private communication) on some of */
/*        these test problems (for nex .ge. 30). */

/*     *****parameter description. */
/*     on input. */

/*        n is the length of r. */
/*        p is the length of x. */
/*        x is the point at which the residual vector r is to be */
/*             computed. */
/*        nfcall is the invocation count of testr. */
/*        nex = uiparm(1) is the index of the problem currently being */
/*             solved. */
/*        urparm is a user parameter vector (and is ignored). */
/*        ufparm is a user entry point parameter (and is ignored). */

/*     on output. */

/*        r is the residual vector at x. */

/*     *****application and usage restrictions. */
/*     these test problems may be used to test least-squares solvers */
/*     such as nl2sol.  in particular, these problems may be used to */
/*     check whether  nl2sol  has been successfully transported to */
/*     a particular machine. */

/*     *****algorithm notes. */
/*     none */

/*     *****subroutines and functions called. */
/*     none */

/*     *****references */
/*     (1) gill, p.e.. & murray, w. (1976), algorithms for the solution */
/*        of the non-linear least-squares problem, npl report nac71. */

/*     (2) meyer, r.r. (1970), theoretical and computational aspects */
/*        of nonlinear regression, pp. 465-486 of nonlinear programming, */
/*        edited by j.b. rosen, o.l.mangasarian, and k. ritter, */
/*        academic press, new york. */

/*     (3) brown, k.m. (1969), a quadratically convergent newton- */
/*        like method based upon gaussian elimination, */
/*        siam j. numer. anal. 6, pp. 560-569. */

/*     *****general. */

/*     this subroutine was written in connection with research */
/*     supported by the national science foundation under grants */
/*     mcs-7600324, dcr75-10143, 76-14311dss, and mcs76-11989. */

/*     .................................................................. */
/*     .................................................................. */

/*  ***  local variables and constants  *** */

/*  ***  intrinsic functions  *** */
/* /+ */
/* / */
/* /6 */
    /* Parameter adjustments */
    --r__;
    --x;
    --uiparm;
    --urparm;

    /* Function Body */
/* /7 */
/*     parameter (twopi=6.283185307179586d+0) */
/* / */
/* /6 */
/* /7 */
/*     save expmax, expmin, uftolg */
/* / */



/* ----------------------------------------------------------------------- */

    nex = uiparm[1];
    switch (nex) {
	case 1:  goto L100;
	case 2:  goto L200;
	case 3:  goto L300;
	case 4:  goto L400;
	case 5:  goto L500;
	case 6:  goto L600;
	case 7:  goto L700;
	case 8:  goto L800;
	case 9:  goto L900;
	case 10:  goto L1000;
	case 11:  goto L1100;
	case 12:  goto L1200;
	case 13:  goto L1300;
	case 14:  goto L1400;
	case 15:  goto L1500;
	case 16:  goto L1600;
	case 17:  goto L1700;
	case 18:  goto L1800;
	case 19:  goto L1900;
	case 20:  goto L2000;
	case 21:  goto L2100;
	case 22:  goto L2200;
	case 23:  goto L2300;
	case 24:  goto L2400;
	case 25:  goto L2500;
	case 26:  goto L2600;
	case 27:  goto L2700;
	case 28:  goto L2800;
	case 29:  goto L2900;
	case 30:  goto L1900;
	case 31:  goto L2100;
	case 32:  goto L2500;
	case 33:  goto L1300;
	case 34:  goto L1400;
	case 35:  goto L1500;
	case 36:  goto L1600;
    }

/*  ***  rosenbrock   *** */
L100:
/* Computing 2nd power */
    d__1 = x[1];
    r__[1] = (x[2] - d__1 * d__1) * 10.;
    r__[2] = 1. - x[1];
    goto L9999;
/*  ***  helix   *** */
L200:
    theta = std::atan2(x[2], x[1]) / twopi;
    if (x[1] <= 0. && x[2] <= 0.) {
	theta += 1.;
    }
    r__[1] = (x[3] - theta * 10.) * 10.;
/* Computing 2nd power */
    d__1 = x[1];
/* Computing 2nd power */
    d__2 = x[2];
    r__[2] = (std::sqrt(d__1 * d__1 + d__2 * d__2) - 1.) * 10.;
    r__[3] = x[3];
    goto L9999;
/*  ***  singular   *** */
L300:
    r__[1] = x[1] + x[2] * 10.;
    r__[2] = std::sqrt(5.) * (x[3] - x[4]);
/* Computing 2nd power */
    d__1 = x[2] - x[3] * 2.;
    r__[3] = d__1 * d__1;
/* Computing 2nd power */
    d__1 = x[1] - x[4];
    r__[4] = std::sqrt(10.) * (d__1 * d__1);
    goto L9999;
/*  ***  woods   *** */
L400:
/* Computing 2nd power */
    d__1 = x[1];
    r__[1] = (x[2] - d__1 * d__1) * 10.;
    r__[2] = 1. - x[1];
/* Computing 2nd power */
    d__1 = x[3];
    r__[3] = std::sqrt(90.) * (x[4] - d__1 * d__1);
    r__[4] = 1. - x[3];
    r__[5] = std::sqrt(9.9) * (x[2] + x[4] - 2.);
    t = std::sqrt(.2);
    r__[6] = t * (x[2] - 1.);
    r__[7] = t * (x[4] - 1.);
    goto L9999;
/*  ***  zangwill */
L500:
    r__[1] = x[1] - x[2] + x[3];
    r__[2] = -x[1] + x[2] + x[3];
    r__[3] = x[1] + x[2] - x[3];
    goto L9999;
/*  ***  engvall   *** */
L600:
/* Computing 2nd power */
    d__1 = x[1];
/* Computing 2nd power */
    d__2 = x[2];
/* Computing 2nd power */
    d__3 = x[3];
    r__[1] = d__1 * d__1 + d__2 * d__2 + d__3 * d__3 - 1.;
/* Computing 2nd power */
    d__1 = x[1];
/* Computing 2nd power */
    d__2 = x[2];
/* Computing 2nd power */
    d__3 = x[3] - 2.;
    r__[2] = d__1 * d__1 + d__2 * d__2 + d__3 * d__3 - 1.;
    r__[3] = x[1] + x[2] + x[3] - 1.;
    r__[4] = x[1] + x[2] - x[3] + 1.;
/* Computing 3rd power */
    d__1 = x[1];
/* Computing 2nd power */
    d__2 = x[2];
/* Computing 2nd power */
    d__3 = x[3] * 5. - x[1] + 1.;
    r__[5] = d__1 * (d__1 * d__1) + d__2 * d__2 * 3. + d__3 * d__3 - 36.;
    goto L9999;
/*  ***  branin *** */
L700:
    r__[1] = (x[1] + x[2]) * 4.;
/* Computing 2nd power */
    d__1 = x[1] - 2.;
/* Computing 2nd power */
    d__2 = x[2];
    r__[2] = r__[1] + (x[1] - x[2]) * (d__1 * d__1 + d__2 * d__2 - 1.);
    goto L9999;
/*  ***  beale  *** */
L800:
    r__[1] = 1.5 - x[1] * (1. - x[2]);
/* Computing 2nd power */
    d__1 = x[2];
    r__[2] = 2.25 - x[1] * (1. - d__1 * d__1);
/* Computing 3rd power */
    d__1 = x[2];
    r__[3] = 2.625 - x[1] * (1. - d__1 * (d__1 * d__1));
    goto L9999;
/*  ***  cragg and levy  *** */
L900:
/* Computing 2nd power */
    d__1 = std::exp(x[1]) - x[2];
    r__[1] = d__1 * d__1;
/* Computing 3rd power */
    d__1 = x[2] - x[3];
    r__[2] = d__1 * (d__1 * d__1) * 10.;
/* Computing 2nd power */
    d__1 = std::sin(x[3] - x[4]) / std::cos(x[3] - x[4]);
    r__[3] = d__1 * d__1;
/* Computing 4th power */
    d__1 = x[1], d__1 *= d__1;
    r__[4] = d__1 * d__1;
    r__[5] = x[4] - 1.;
    goto L9999;
/*  ***  box  *** */
L1000:
    if (expmax > 0.) {
	goto L1001;
    }
    expmax = std::log(rmdcon_(&c__5)) * 1.999;
    expmin = std::log(rmdcon_(&c__2)) * 1.999;
L1001:
/* Computing MIN */
    d__1 = min(x[1],x[2]);
    if (-expmax >= min(d__1,x[3])) {
	goto L1003;
    }
    for (i__ = 1; i__ <= 10; ++i__) {
	ti = (doublereal) ((real) i__) * -.1;
	t1 = ti * x[1];
	e1 = 0.;
	if (t1 > expmin) {
	    e1 = std::exp(t1);
	}
	t2 = ti * x[2];
	e2 = 0.;
	if (t2 > expmin) {
	    e2 = std::exp(t2);
	}
	r__[i__] = e1 - e2 - x[3] * (std::exp(ti) - std::exp(ti * 10.));
/* L1002: */
    }
    goto L9999;
L1003:
    *nfcall = -1;
    goto L9999;
/*  ***  davidon 1  *** */
L1100:
    nm1 = *n - 1;
    i__1 = nm1;
    for (i__ = 1; i__ <= i__1; ++i__) {
	r1 = 0.;
	ti = (doublereal) ((real) i__);
	t = 1.;
	i__2 = *p;
	for (j = 1; j <= i__2; ++j) {
	    r1 += t * x[j];
	    t *= ti;
/* L1101: */
	}
	r__[i__] = r1;
/* L1102: */
    }
    r__[*n] = x[1] - 1.;
    goto L9999;
/*  ***  freudenstein and roth  *** */
L1200:
/* Computing 2nd power */
    d__1 = x[2];
/* Computing 3rd power */
    d__2 = x[2];
    r__[1] = x[1] - 13. - x[2] * 2. + d__1 * d__1 * 5. - d__2 * (d__2 * d__2);
/* Computing 2nd power */
    d__1 = x[2];
/* Computing 3rd power */
    d__2 = x[2];
    r__[2] = x[1] - 29. - x[2] * 14. + d__1 * d__1 + d__2 * (d__2 * d__2);
    goto L9999;
/*  ***  watson  *** */
L1300:
L1400:
L1500:
L1600:
    for (i__ = 1; i__ <= 29; ++i__) {
	ti = (doublereal) ((real) i__) / 29.;
	r1 = 0.;
	r2 = x[1];
	t = 1.;
	i__1 = *p;
	for (j = 2; j <= i__1; ++j) {
	    i__2 = j - 1;
	    r1 += (doublereal) ((real) i__2) * t * x[j];
	    t *= ti;
	    r2 += t * x[j];
/* L1601: */
	}
	r__[i__] = r1 - r2 * r2 - 1.;
	if (nex >= 33 && nex <= 36) {
	    r__[i__] += 10.;
	}
/* L1602: */
    }
    r__[30] = x[1];
/* Computing 2nd power */
    d__1 = x[1];
    r__[31] = x[2] - d__1 * d__1 - 1.;
    if (nex < 33 || nex > 36) {
	goto L9999;
    }
    r__[30] += 10.;
    r__[31] += 10.;
    goto L9999;
/*  ***  chebyquad  *** */
L1700:
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
/* L1701: */
	r__[i__] = 0.;
    }
    i__1 = *n;
    for (j = 1; j <= i__1; ++j) {
	tim1 = 1.;
	ti = x[j] * 2. - 1.;
	z__ = ti + ti;
	i__2 = *n;
	for (i__ = 1; i__ <= i__2; ++i__) {
	    r__[i__] += ti;
	    tip1 = z__ * ti - tim1;
	    tim1 = ti;
	    ti = tip1;
/* L1702: */
	}
    }
    floatn = (doublereal) ((real) (*n));
    i__2 = *n;
    for (i__ = 1; i__ <= i__2; ++i__) {
	ti = 0.;
	if (i__ % 2 == 0) {
	    i__1 = i__ * i__ - 1;
	    ti = -1. / (doublereal) ((real) i__1);
	}
	r__[i__] = ti - r__[i__] / floatn;
/* L1703: */
    }
    goto L9999;
/*  ***  brown and dennis  *** */
L1800:
    i__2 = *n;
    for (i__ = 1; i__ <= i__2; ++i__) {
	ti = (doublereal) ((real) i__) * .2;
/* Computing 2nd power */
	d__1 = x[1] + x[2] * ti - std::exp(ti);
/* Computing 2nd power */
	d__2 = x[3] + x[4] * std::sin(ti) - std::cos(ti);
	r__[i__] = d__1 * d__1 + d__2 * d__2;
/* L1801: */
    }
    goto L9999;
/*  ***  bard  *** */
L1900:
    for (i__ = 1; i__ <= 15; ++i__) {
	u = (doublereal) ((real) i__);
	v = 16. - u;
	w = min(u,v);
	r__[i__] = ybard[i__ - 1] - (x[1] + u / (x[2] * v + x[3] * w));
	if (nex == 30) {
	    r__[i__] += 10.;
	}
/* L1901: */
    }
    goto L9999;
/*  ***  jennrich and sampson  *** */
L2000:
    for (i__ = 1; i__ <= 10; ++i__) {
	ti = (doublereal) ((real) i__);
	r__[i__] = ti * 2. + 2. - (std::exp(ti * x[1]) + std::exp(ti * x[2]));
/* L2001: */
    }
    goto L9999;
/*  ***  kowalik and osborne  *** */
L2100:
    for (i__ = 1; i__ <= 11; ++i__) {
/* Computing 2nd power */
	d__1 = ukow[i__ - 1];
/* Computing 2nd power */
	d__2 = ukow[i__ - 1];
	r__[i__] = ykow[i__ - 1] - x[1] * (d__1 * d__1 + x[2] * ukow[i__ - 1])
		 / (d__2 * d__2 + x[3] * ukow[i__ - 1] + x[4]);
	if (nex == 31) {
	    r__[i__] += 10.;
	}
/* L2101: */
    }
    goto L9999;
/*  ***  osborne 1  *** */
L2200:
    for (i__ = 1; i__ <= 33; ++i__) {
	i__2 = 1 - i__;
	ti = (doublereal) ((real) i__2) * 10.;
	r__[i__] = yosb1[i__ - 1] - (x[1] + x[2] * std::exp(x[4] * ti) + x[3] *
		std::exp(x[5] * ti));
/* L2201: */
    }
    goto L9999;
/*  ***  osborne 2  *** */
/*     ***  uftolg is a machine-dependent constant.  it is just slightly */
/*     ***  larger than the log of the smallest positive machine number. */
L2300:
    if (uftolg == 0.) {
	uftolg = std::log(rmdcon_(&c__2)) * 1.999;
    }
    for (i__ = 1; i__ <= 65; ++i__) {
	i__2 = 1 - i__;
	ti = (doublereal) ((real) i__2) * .1;
	ri = x[1] * std::exp(x[5] * ti);
	for (j = 2; j <= 4; ++j) {
	    t = 0.;
/* Computing 2nd power */
	    d__1 = ti + x[j + 7];
	    theta = -x[j + 4] * (d__1 * d__1);
	    if (theta > uftolg) {
		t = std::exp(theta);
	    }
	    ri += x[j] * t;
/* L2301: */
	}
	r__[i__] = yosb2[i__ - 1] - ri;
/* L2302: */
    }
    goto L9999;
/*  ***  madsen  *** */
L2400:
/* Computing 2nd power */
    d__1 = x[1];
/* Computing 2nd power */
    d__2 = x[2];
    r__[1] = d__1 * d__1 + d__2 * d__2 + x[1] * x[2];
    r__[2] = std::sin(x[1]);
    r__[3] = std::cos(x[2]);
    goto L9999;
/*  ***  meyer  *** */
L2500:
    for (i__ = 1; i__ <= 16; ++i__) {
	i__2 = i__ * 5 + 45;
	ti = (doublereal) ((real) i__2);
	r__[i__] = x[1] * std::exp(x[2] / (ti + x[3])) - ymeyer[i__ - 1];
	if (nex == 32) {
	    r__[i__] += 10.;
	}
/* L2501: */
    }
    goto L9999;
/*  ***  brown  *** */
L2600:
L2700:
L2800:
L2900:
    i__2 = *n + 1;
    t = x[1] - (doublereal) ((real) i__2);
    i__2 = *n;
    for (i__ = 2; i__ <= i__2; ++i__) {
/* L2901: */
	t += x[i__];
    }
    nm1 = *n - 1;
    i__2 = nm1;
    for (i__ = 1; i__ <= i__2; ++i__) {
/* L2902: */
	r__[i__] = t + x[i__];
    }
    t = x[1];
    i__2 = *n;
    for (i__ = 2; i__ <= i__2; ++i__) {
/* L2903: */
	t *= x[i__];
    }
    r__[*n] = t - 1.;
    goto L9999;

L9999:
    return 0;
/*     ..... last card of testr ......................................... */
} /* testr_ */

/* Subroutine */ int xinit_(integer *p, doublereal *x, integer *nex)
{
    /* System generated locals */
    integer i__1;

    /* Local variables */
    static integer i__;
    static doublereal pp1inv;


/*     *****parameters... */


/*     .................................................................. */

/*     *****purpose... */
/*     this routine initializes the solution vector x according to */
/*     the initial values for the various test functions given in */
/*     references (1), (2), and (3). */
/*     subroutines testr and testj.  (see testr for references.) */

/*     *****parameter description... */
/*     on input... */

/*        nex is the test problem number. */

/*        p is the number of parameters. */

/*     on output... */

/*        x is the initial guess to the solution. */

/*     *****application and usage restrictions... */
/*     this routine is called by nltest. */

/*     .................................................................. */

/*     *****local variables... */
/*     *****intrinsic functions... */
/* /+ */
/* / */

    /* Parameter adjustments */
    --x;

    /* Function Body */
    switch (*nex) {
	case 1:  goto L100;
	case 2:  goto L200;
	case 3:  goto L300;
	case 4:  goto L400;
	case 5:  goto L500;
	case 6:  goto L600;
	case 7:  goto L700;
	case 8:  goto L800;
	case 9:  goto L900;
	case 10:  goto L1000;
	case 11:  goto L1100;
	case 12:  goto L1200;
	case 13:  goto L1300;
	case 14:  goto L1400;
	case 15:  goto L1500;
	case 16:  goto L1600;
	case 17:  goto L1700;
	case 18:  goto L1800;
	case 19:  goto L1900;
	case 20:  goto L2000;
	case 21:  goto L2100;
	case 22:  goto L2200;
	case 23:  goto L2300;
	case 24:  goto L2400;
	case 25:  goto L2500;
	case 26:  goto L2600;
	case 27:  goto L2700;
	case 28:  goto L2800;
	case 29:  goto L2900;
	case 30:  goto L1900;
	case 31:  goto L2100;
	case 32:  goto L2500;
	case 33:  goto L1300;
	case 34:  goto L1400;
	case 35:  goto L1500;
	case 36:  goto L1600;
    }

/*  ***  rosenbrock  *** */
L100:
    x[1] = -1.2;
    x[2] = 1.;
    goto L9999;
/*  ***  helix  *** */
L200:
    x[1] = -1.;
    x[2] = 0.;
    x[3] = 0.;
    goto L9999;
/*  *** singular  *** */
L300:
    x[1] = 3.;
    x[2] = -1.;
    x[3] = 0.;
    x[4] = 1.;
    goto L9999;
/*  ***  woods  *** */
L400:
    x[1] = -3.;
    x[2] = -1.;
    x[3] = -3.;
    x[4] = -1.;
    goto L9999;
/*  ***  zangwill  *** */
L500:
    x[1] = 100.;
    x[2] = -1.;
    x[3] = 2.5;
    goto L9999;
/*  ***  engvall  *** */
L600:
    x[1] = 1.;
    x[2] = 2.;
    x[3] = 0.;
    goto L9999;
/*  *** branin  *** */
L700:
    x[1] = 2.;
    x[2] = 0.;
    goto L9999;
/*  ***  beale  *** */
L800:
    x[1] = .1;
    x[2] = .1;
    goto L9999;
/*  *** cragg and levy  *** */
L900:
    x[1] = 1.;
    x[2] = 2.;
    x[3] = 2.;
    x[4] = 2.;
    goto L9999;
/*  ***  box  *** */
L1000:
    x[1] = 0.;
    x[2] = 10.;
    x[3] = 20.;
    goto L9999;
/*  ***  davidon 1  *** */
L1100:
    i__1 = *p;
    for (i__ = 1; i__ <= i__1; ++i__) {
/* L1101: */
	x[i__] = 0.;
    }
    goto L9999;
/*  ***  freudenstein and roth  *** */
L1200:
    x[1] = 15.;
    x[2] = -2.;
    goto L9999;
/*  ***  watson  *** */
L1300:
L1400:
L1500:
L1600:
    i__1 = *p;
    for (i__ = 1; i__ <= i__1; ++i__) {
/* L1601: */
	x[i__] = 0.;
    }
    goto L9999;
/*  ***  chebyquad  *** */
L1700:
    i__1 = *p + 1;
    pp1inv = 1. / (real) ((doublereal) ((real) i__1));
    i__1 = *p;
    for (i__ = 1; i__ <= i__1; ++i__) {
/* L1701: */
	x[i__] = (real) ((doublereal) ((real) i__)) * pp1inv;
    }
    goto L9999;
/*  *** brown and dennis  *** */
L1800:
    x[1] = 25.;
    x[2] = 5.;
    x[3] = -5.;
    x[4] = -1.;
    goto L9999;
/*  ***  bard  *** */
L1900:
    x[1] = 1.;
    x[2] = 1.;
    x[3] = 1.;
    goto L9999;
/*  ***  jennrich and sampson  *** */
L2000:
    x[1] = .3;
    x[2] = .4;
    goto L9999;
/*  ***  kowalik and osborne  *** */
L2100:
    x[1] = .25;
    x[2] = .39;
    x[3] = .415;
    x[4] = .39;
    goto L9999;
/*  ***  osborne 1  *** */
L2200:
    x[1] = .5;
    x[2] = 1.5;
    x[3] = -1.;
    x[4] = .01;
    x[5] = .02;
    goto L9999;
/*  ***  osborne 2  *** */
L2300:
    x[1] = 1.3;
    x[2] = .65;
    x[3] = .65;
    x[4] = .7;
    x[5] = .6;
    x[6] = 3.;
    x[7] = 5.;
    x[8] = 7.;
    x[9] = 2.;
    x[10] = 4.5;
    x[11] = 5.5;
    goto L9999;
/*  ***  madsen  *** */
L2400:
    x[1] = 3.;
    x[2] = 1.;
    goto L9999;
/*  ***  meyer  ** */
L2500:
    x[1] = .02;
    x[2] = 4e3;
    x[3] = 250.;
    goto L9999;
/*  ***  brown  *** */
L2600:
L2700:
L2800:
L2900:
    i__1 = *p;
    for (i__ = 1; i__ <= i__1; ++i__) {
/* L2901: */
	x[i__] = .5;
    }
    goto L9999;


L9999:
    return 0;
} /* xinit_ */

doublereal rmdcon_(integer *k)
{
    /* Initialized data */

    static doublereal one001 = 1.001;
    static doublereal pt999 = .999;
    static doublereal big = 1.79769e308;
    static doublereal eta = 2.22508e-308;
    static doublereal machep = 2.22044605e-16;

    /* System generated locals */
    doublereal ret_val;

/*  ***  return machine dependent constants used by nl2sol  *** */

/* +++  comments below contain data statements for various machines.  +++ */
/* +++  to convert to another machine, place a c in column 1 of the   +++ */
/* +++  data statement line(s) that correspond to the current machine +++ */
/* +++  and remove the c from column 1 of the data statement line(s)  +++ */
/* +++  that correspond to the new machine.                           +++ */


/*  ***  the constant returned depends on k... */

/*  ***        k = 1... smallest pos. eta such that -eta exists. */
/*  ***        k = 2... square root of 1.001*eta. */
/*  ***        k = 3... unit roundoff = smallest pos. no. machep such */
/*  ***                 that 1 + machep .gt. 1 .and. 1 - machep .lt. 1. */
/*  ***        k = 4... square root of 0.999*machep. */
/*  ***        k = 5... square root of 0.999*big (see k = 6). */
/*  ***        k = 6... largest machine no. big such that -big exists. */

/* /+ */
/* / */


/*  +++  IEEE  +++ */


/*  +++  ibm 360, ibm 370, or xerox  +++ */

/*     data big/z7fffffffffffffff/, eta/z0010000000000000/, */
/*    1     machep/z3410000000000000/ */

/*  +++  data general  +++ */

/*     data big/0.7237005577d+76/, eta/0.5397605347d-78/, */
/*    1     machep/2.22044605d-16/ */

/*  +++  dec 11  +++ */

/*     data big/1.7d+38/, eta/2.938735878d-39/, machep/2.775557562d-17/ */

/*  +++  hp3000  +++ */

/*     data big/1.157920892d+77/, eta/8.636168556d-78/, */
/*    1     machep/5.551115124d-17/ */

/*  +++  honeywell  +++ */

/*     data big/1.69d+38/, eta/5.9d-39/, machep/2.1680435d-19/ */

/*  +++  dec10  +++ */

/*     data big/"377777100000000000000000/, */
/*    1     eta/"002400400000000000000000/, */
/*    2     machep/"104400000000000000000000/ */

/*  +++  burroughs  +++ */

/*     data big/o0777777777777777,o7777777777777777/, */
/*    1     eta/o1771000000000000,o7770000000000000/, */
/*    2     machep/o1451000000000000,o0000000000000000/ */

/*  +++  control data  +++ */


/*     data big/37767777777777777777b,37167777777777777777b/, */
/*    1     eta/00014000000000000000b,00000000000000000000b/, */
/*    2     machep/15614000000000000000b,15010000000000000000b/ */

/*  +++  prime  +++ */

/*     data big/1.0d+9786/, eta/1.0d-9860/, machep/1.4210855d-14/ */

/*  +++  univac  +++ */

/*     data big/8.988d+307/, eta/1.2d-308/, machep/1.734723476d-18/ */

/*  +++  vax  +++ */

/*     data big/1.7d+38/, eta/2.939d-39/, machep/1.3877788d-17/ */

/* -------------------------------  body  -------------------------------- */

    switch (*k) {
	case 1:  goto L10;
	case 2:  goto L20;
	case 3:  goto L30;
	case 4:  goto L40;
	case 5:  goto L50;
	case 6:  goto L60;
    }

L10:
    ret_val = eta;
    goto L999;

L20:
    ret_val = std::sqrt(one001 * eta);
    goto L999;

L30:
    ret_val = machep;
    goto L999;

L40:
    ret_val = std::sqrt(pt999 * machep);
    goto L999;

L50:
    ret_val = std::sqrt(pt999 * big);
    goto L999;

L60:
    ret_val = big;

L999:
    return ret_val;
/*  ***  last card of rmdcon follows  *** */
} /* rmdcon_ */

