/*  _________________________________________________________________________
 *
 *  UTILIB: A utility library for developing portable C++ codes.
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README file in the top UTILIB directory.
 *  _________________________________________________________________________
 */

/**
 * \file Random.h
 *
 * Provide externs and headers for the routines in RANLIB.C
 */

#ifndef utilib_Random_h
#define utilib_Random_h

#if !defined(DOXYGEN)

#include <utilib_config.h>
#if defined(UTILIB_HAVE_STD) || defined(__cplusplus)
#define _(args) args
#else
#define _(args) ()
#endif

#ifdef __cplusplus
extern "C" {
#endif


/* #define OLDSTYLE_RANLIB 	Define this in the Makefile */

/* Routines which generate random deviates */
extern double genbet _((double aa,double bb));
extern double genchi _((double df));
extern double genexp _((double av));
extern double sexpo _((void));
#if HAVE_FSIGN
extern double genf _((double dfn, double dfd));
extern double gengam _((double a,double r));
extern double sgamma _((double a));
#endif
extern void genmn _((double *parm,double *x,double *work));
#if HAVE_FSIGN
extern double gennch _((double df,double xnonc));
extern double gennf _((double dfn, double dfd, double xnonc));
#endif
extern double gennor _((double av,double sd));
extern double snorm1 _((void));
extern double snorm2 _((void));
extern double snorm3 _((void));
extern double genunf _((double low,double high));
extern long ignbin _((long n,double pp));
#if HAVE_FSIGN
extern long ignpoi _((double mu));
#endif
#ifdef HAVE_IGNUIN
extern long ignuin _((long low,long high));
#endif
extern double rcauchy _((double alpha, double beta));
extern double scauchy1 _((void));

/* #ifndef OLDSTYLE_RANLIB */
#if 0
#define rbeta(a,b)	genbet(a,b)
#define rchi(a)		genchi(a)
#define rexp(a)		genexp(a)
#define rF(a,b)		genf(a,b)
#define rgamma(a,b)	gengam(a,b)
#define rmnorm(a,b,c)	genmn(a,b,c)
#define rnchi(a,b)	gennch(a,b)
#define rnF(a,b,c)	gennf(a,b,c)
#define rnorm(a,b)	gennor(a,b)
#define runif(a,b)	genunf(a,b)
#define rbinomial(a,b)	ignbin(a,b)
#define rpoisson(a)	ignpoi(a)
#define runifint(a,b)	ignuin(a,b)
#endif


#ifdef __cplusplus
};
#endif


#define snorm() snorm1()

#define ABS(x) ((x) >= 0 ? (x) : -(x))
#ifndef MIN
#define MIN(a,b) ((a) <= (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a,b) ((a) >= (b) ? (a) : (b))
#endif


/* Setup stuff for the global rng */
#include <utilib/LEC_rng.h>

/*
** TODO - What's OLDSTYLE_RANLIB?
*/
#ifdef OLDSTYLE_RANLIB
#define ranf()	LEC_runif()
#else
#define ranf()	(*global_runif)()
#ifdef __cplusplus
extern "C" {
#endif
extern double (*global_runif) _((void));
#ifdef __cplusplus
};
#endif
#define set_global_runif(func)	global_runif = func
#endif

#endif
#endif
