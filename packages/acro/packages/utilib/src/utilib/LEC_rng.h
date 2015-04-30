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
 * \file LEC_rng.h
 *
 * Provide headers and externs for the routines in RANLIB.C
 */

#ifndef utilib_LEC_rng_h
#define utilib_LEC_rng_h

#if !defined(DOXYGEN)

#include <utilib_config.h>
#if defined(__cplusplus)
extern "C" {
#endif

extern long mltmod(long a,long s,long m);
extern void advnst(long k);
extern void phrtsd(char* phrase,long* seed1,long* seed2);

extern double LEC_runif(void);

extern void setall(long iseed1,long iseed2);
extern void setant(long qvalue);
extern void setgmn(double *meanv,double *covm,long p,double *parm);
extern void setsd(long iseed1,long iseed2);
extern void getsd(long *iseed1,long *iseed2);

extern void initgn(long isdtyp);
extern void gscgn(long getset,long *g);

#ifdef __cplusplus
};
#endif

#endif

#endif
