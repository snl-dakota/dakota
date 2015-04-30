/* This is third-party software that is distributed with UTILIB.
 * For licensing information concerning this file, see the UTILIB home page:
 * http://software.sandia.gov/trac/utilib
 */
/* genbet.c
 *
 */

#include <utilib_config.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <utilib/Random.h>


double genbet(double aa,double bb)
/*
**********************************************************************
     double genbet(double aa,double bb)
               GeNerate BETa random deviate
                              Function
     Returns a single random deviate from the beta distribution with
     parameters A and B.  The density of the beta is
               x^(a-1) * (1-x)^(b-1) / B(a,b) for 0 < x < 1
                              Arguments
     aa --> First parameter of the beta distribution

     bb --> Second parameter of the beta distribution

                              Method
     R. C. H. Cheng
     Generating Beta Variatew with Nonintegral Shape Parameters
     Communications of the ACM, 21:317-322  (1978)
     (Algorithms BB and BC)
**********************************************************************
*/
{
#define expmax 89.0
#define infnty 1.0E38
static double olda = -1.0;
static double oldb = -1.0;
static double genbet,a,alpha,b,beta,delta,gamma,k1,k2,r,s,t,u1,u2,v,w,y,z;
static long qsame;

    qsame = olda == aa && oldb == bb;
    if(qsame) goto S20;
    if(!(aa <= 0.0 || bb <= 0.0)) goto S10;
    puts(" AA or BB <= 0 in GENBET - Abort!");
    printf(" AA: %16.6E BB %16.6E\n",aa,bb);
    exit(1);
S10:
    olda = aa;
    oldb = bb;
S20:
    if(!(MIN(aa,bb) > 1.0)) goto S100;
/*
     Alborithm BB
     Initialize
*/
    if(qsame) goto S30;
    a = MIN(aa,bb);
    b = MAX(aa,bb);
    alpha = a+b;
    beta = sqrt((alpha-2.0)/(2.0*a*b-alpha));
    gamma = a+1.0/beta;
S30:
S40:
    u1 = ranf();
/*
     Step 1
*/
    u2 = ranf();
    v = beta*log(u1/(1.0-u1));
    if(!(v > expmax)) goto S50;
    w = infnty;
    goto S60;
S50:
    w = a*exp(v);
S60:
    z = pow(u1,2.0)*u2;
    r = gamma*v-1.3862944;
    s = a+r-w;
/*
     Step 2
*/
    if(s+2.609438 >= 5.0*z) goto S70;
/*
     Step 3
*/
    t = log(z);
    if(s > t) goto S70;
/*
     Step 4
*/
    if(r+alpha*log(alpha/(b+w)) < t) goto S40;
S70:
/*
     Step 5
*/
    if(!(aa == a)) goto S80;
    genbet = w/(b+w);
    goto S90;
S80:
    genbet = b/(b+w);
S90:
    goto S230;
S100:
/*
     Algorithm BC
     Initialize
*/
    if(qsame) goto S110;
    a = MAX(aa,bb);
    b = MIN(aa,bb);
    alpha = a+b;
    beta = 1.0/b;
    delta = 1.0+a-b;
    k1 = delta*(1.38889E-2+4.16667E-2*b)/(a*beta-0.777778);
    k2 = 0.25+(0.5+0.25/delta)*b;
S110:
S120:
    u1 = ranf();
/*
     Step 1
*/
    u2 = ranf();
    if(u1 >= 0.5) goto S130;
/*
     Step 2
*/
    y = u1*u2;
    z = u1*y;
    if(0.25*u2+z-y >= k1) goto S120;
    goto S170;
S130:
/*
     Step 3
*/
    z = pow(u1,2.0)*u2;
    if(!(z <= 0.25)) goto S160;
    v = beta*log(u1/(1.0-u1));
    if(!(v > expmax)) goto S140;
    w = infnty;
    goto S150;
S140:
    w = a*exp(v);
S150:
    goto S200;
S160:
    if(z >= k2) goto S120;
S170:
/*
     Step 4
     Step 5
*/
    v = beta*log(u1/(1.0-u1));
    if(!(v > expmax)) goto S180;
    w = infnty;
    goto S190;
S180:
    w = a*exp(v);
S190:
    if(alpha*(log(alpha/(b+w))+v)-1.3862944 < log(z)) goto S120;
S200:
/*
     Step 6
*/
    if(!(a == aa)) goto S210;
    genbet = w/(b+w);
    goto S220;
S210:
    genbet = b/(b+w);
S230:
S220:
    return genbet;
#undef expmax
#undef infnty
}

