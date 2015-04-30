/*  _________________________________________________________________________
 *
 *  Acro: A Common Repository for Optimizers
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README.txt file in the top Acro directory.
 *  _________________________________________________________________________
 */

#ifdef ACRO_USING_AMPL

#include "stdio1.h"
#include "math.h"       /* for sqrt */
#include "funcadd.h"    /* includes "stdio1.h" */

char *ix_details_ASL[] = {0};  /* no -i command-line option */

static int initval = 0;

static real
ncall(arglist *al)      /* generalized inverse of a single argument */
{
initval++;
return (real) initval;
}

static real
constfn(arglist *al)
{
return (real) 1.0;
}

static real
ginv(arglist *al)       /* generalized inverse of a single argument */
{
        real x = al->ra[0];
        x = x ? 1./x : 0.;
        if (al->derivs) {
                *al->derivs = -x*x;
                if (al->hes)
                        *al->hes = -2.*x * *al->derivs;
                }
        return x;
        }

 static real
myhypot(arglist *al)    /* sqrt(x*x + y*y) */
{
        real *d, *h, rv, x, x0, y, y0;

        x = x0 = al->ra[0];
        y = y0 = al->ra[1];

        if (x < 0.)
                x = -x;
        if (y < 0.)
                y = -y;
        rv = x;
        if (x < y) {
                rv = y;
                y = x;
                x = rv;
                }
        if (rv) {
                y /= x;
                rv = x * sqrt(1. + y*y);
                if ((d = al->derivs)) {
                        d[0] = x0 / rv;
                        d[1] = y0 / rv;
                        if ((h = al->hes)) {
                                h[0] =  d[1]*d[1] / rv;
                                h[1] = -d[0]*d[1] / rv;
                                h[2] =  d[0]*d[0] / rv;
                                }
                        }
                }
        else if ((d = al->derivs)) {
                d[0] = d[1] = 0;
                if ((h = al->hes))
                        h[0] = h[1] = h[2] = 0;
                }
        return rv;
        }

 static real
mean(register arglist *al)      /* mean of arbitrarily many arguments */
{
        real x, z;
        real *d, *de, *ra;
        int *at, i, j, n;
        char *se, *sym;
        AmplExports *ae = al->AE; /* for fprintf and strtod */

        if ((n = al->n) <= 0)
                return 0;
        at = al->at;
        ra = al->ra;
        d = de = al->derivs;
        x = 0.;
        for(i = 0; i < n;)
                if ((j = at[i++]) >= 0) {
                        x += ra[j];
                        ++de;
                        }
                else {
                        x += z = strtod(sym = (char*)(al->sa[-(j+1)]), &se);
                        if (*se) {
                                fprintf(Stderr,
                                "mean treating arg %d = \"%s\" as %.g\n",
                                        i, sym, z);
                                }
                        }
        if (d) {
                z = 1. / n;
                while(d < de)
                        *d++ = z;
                /* The Hessian is == 0 and, if needed, has been */
                /* initialized to 0. */
                }
        return x / n;
        }

void
funcadd(AmplExports *ae){
        /* Insert calls on addfunc here... */
        /* Arg 3, called argtype, can be 0 or 1:
         *      0 ==> force all arguments to be numeric
         *      1 ==> pass both symbolic and numeric arguments.
         *
         * Arg 4, called nargs, is interpretted as follows:
         *      >=  0 ==> the function has exactly nargs arguments
         *      <= -1 ==> the function has >= -(nargs+1) arguments.
         *
         * Arg 5, funcinfo, is passed to the functions in struct arglist;
         *      it is not used in these examples, so we just pass 0.
         */

        addfunc("ginv", (ufunc*)ginv, 0, 1, 0);
        addfunc("hypot", (ufunc*)myhypot, 0, 2, 0);
        addfunc("mean", (ufunc*)mean, 1, -1, 0);
        addfunc("ncall", (ufunc*)ncall, 0, -1, 0);
        addfunc("constfn", (ufunc*)constfn, 0, -1, 0);
        }

#endif // ACRO_USING_AMPL
