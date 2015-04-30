/* This is third-party software that is distributed with UTILIB.
 * For licensing information concerning this file, see the UTILIB home page:
 * http://software.sandia.gov/trac/utilib
 */

/* LEC_gen.c
 *
 */

#include <utilib_config.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "LEC_rng.h"


void initgn(long isdtyp)
/*
**********************************************************************
     void initgn(long isdtyp)
          INIT-ialize current G-e-N-erator
     Reinitializes the state of the current generator
     This is a transcription from Pascal to Fortran of routine
     Init_Generator from the paper
     L'Ecuyer, P. and Cote, S. "Implementing a Random Number Package
     with Splitting Facilities." ACM Transactions on Mathematical
     Software, 17:98-111 (1991)
                              Arguments
     isdtyp -> The state to which the generator is to be set
          isdtyp = -1  => sets the seeds to their initial value
          isdtyp =  0  => sets the seeds to the first value of
                          the current block
          isdtyp =  1  => sets the seeds to the first value of
                          the next block
**********************************************************************
*/
{
#define numg 32L
extern void gsrgs(long getset,long *qvalue);
extern void gscgn(long getset,long *g);
extern long Xm1,Xm2,Xa1w,Xa2w,Xig1[],Xig2[],Xlg1[],Xlg2[],Xcg1[],Xcg2[];
static long g;
static long qrgnin;
/*
     Abort unless random number generator initialized
*/
    gsrgs(0L,&qrgnin);
    if(qrgnin) goto S10;
    printf(
      " INITGN called before random number generator  initialized -- abort!\n");    exit(1);
S10:
    gscgn(0L,&g);
    if(-1 != isdtyp) goto S20;
    *(Xlg1+g-1) = *(Xig1+g-1);
    *(Xlg2+g-1) = *(Xig2+g-1);
    goto S50;
S20:
    if(0 != isdtyp) goto S30;
    goto S50;
S30:
/*
     do nothing
*/
    if(1 != isdtyp) goto S40;
    *(Xlg1+g-1) = mltmod(Xa1w,*(Xlg1+g-1),Xm1);
    *(Xlg2+g-1) = mltmod(Xa2w,*(Xlg2+g-1),Xm2);
    goto S50;
S40:
    printf("isdtyp not in range in INITGN");
    exit(1);
S50:
    *(Xcg1+g-1) = *(Xlg1+g-1);
    *(Xcg2+g-1) = *(Xlg2+g-1);
#undef numg
}


void inrgcm(void)
/*
**********************************************************************
     void inrgcm(void)
          INitialize Random number Generator CoMmon
                              Function
     Initializes common area  for random number  generator.  This saves
     the  nuisance  of  a  BLOCK DATA  routine  and the  difficulty  of
     assuring that the routine is loaded with the other routines.
**********************************************************************
*/
{
#define numg 32L
extern void gsrgs(long getset,long *qvalue);
extern long Xm1,Xm2,Xa1,Xa2,Xa1w,Xa2w,Xa1vw,Xa2vw;
extern long Xqanti[];
static long T1;
static long i;
/*
     V=20;                            W=30;
     A1W = MOD(A1**(2**W),M1)         A2W = MOD(A2**(2**W),M2)
     A1VW = MOD(A1**(2**(V+W)),M1)    A2VW = MOD(A2**(2**(V+W)),M2)
   If V or W is changed A1W, A2W, A1VW, and A2VW need to be recomputed.
    An efficient way to precompute a**(2*j) MOD m is to start with
    a and square it j times modulo m using the function MLTMOD.
*/
    Xm1 = 2147483563L;
    Xm2 = 2147483399L;
    Xa1 = 40014L;
    Xa2 = 40692L;
    Xa1w = 1033780774L;
    Xa2w = 1494757890L;
    Xa1vw = 2082007225L;
    Xa2vw = 784306273L;
    for(i=0; i<numg; i++) *(Xqanti+i) = 0;
    T1 = 1;
/*
     Tell the world that common has been initialized
*/
    gsrgs(1L,&T1);
#undef numg
}



void gscgn(long getset,long *g)
/*
**********************************************************************
     void gscgn(long getset,long *g)
                         Get/Set GeNerator
     Gets or returns in G the number of the current generator
                              Arguments
     getset --> 0 Get
                1 Set
     g <-- Number of the current random number generator (1..32)
**********************************************************************
*/
{
#define numg 32L
static long curntg = 1;
    if(getset == 0) *g = curntg;
    else  {
        if(*g < 0 || *g > numg) {
            puts(" Generator number out of range in GSCGN");
            exit(0);
        }
        curntg = *g;
    }
#undef numg
}



void gsrgs(long getset,long *qvalue)
/*
**********************************************************************
     void gsrgs(long getset,long *qvalue)
               Get/Set Random Generators Set
     Gets or sets whether random generators set (initialized).
     Initially (data statement) state is not set
     If getset is 1 state is set to qvalue
     If getset is 0 state returned in qvalue
**********************************************************************
*/
{
static long qinit = 0;

    if(getset == 0) *qvalue = qinit;
    else qinit = *qvalue;
}


void gssst(long getset,long *qset)
/*
**********************************************************************
     void gssst(long getset,long *qset)
          Get or Set whether Seed is Set
     Initialize to Seed not Set
     If getset is 1 sets state to Seed Set
     If getset is 0 returns T in qset if Seed Set
     Else returns F in qset
**********************************************************************
*/
{
static long qstate = 0;
    if(getset != 0) qstate = 1;
    else  *qset = qstate;
}

