/* This is third-party software that is distributed with UTILIB.
 * For licensing information concerning this file, see the UTILIB home page:
 * http://software.sandia.gov/trac/utilib
 */

/* LEC_misc.c
 *
 */

#include <utilib_config.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "LEC_rng.h"


long mltmod(long a,long s,long m)
/*
**********************************************************************
     long mltmod(long a,long s,long m)
                    Returns (A*S) MOD M
     This is a transcription from Pascal to Fortran of routine
     MULtMod_Decompos from the paper
     L'Ecuyer, P. and Cote, S. "Implementing a Random Number Package
     with Splitting Facilities." ACM Transactions on Mathematical
     Software, 17:98-111 (1991)
                              Arguments
     a, s, m  -->
**********************************************************************
*/
{
#define h 32768L
static long mltmod,a0,a1,k,p,q,qh,rh;
/*
     H = 2**((b-2)/2) where b = 32 because we are using a 32 bit
      machine. On a different machine recompute H
*/
    if(!(a <= 0 || a >= m || s <= 0 || s >= m)) goto S10;
    puts(" a, m, s out of order in mltmod - ABORT!");
    printf(" a = %12ld s = %12ld m = %12ld\n",a,s,m);
    puts(" mltmod requires: 0 < a < m; 0 < s < m");
    exit(1);
S10:
    if(!(a < h)) goto S20;
    a0 = a;
    p = 0;
    goto S120;
S20:
    a1 = a/h;
    a0 = a-h*a1;
    qh = m/h;
    rh = m-h*qh;
    if(!(a1 >= h)) goto S50;
    a1 -= h;
    k = s/qh;
    p = h*(s-k*qh)-k*rh;
S30:
    if(!(p < 0)) goto S40;
    p += m;
    goto S30;
S40:
    goto S60;
S50:
    p = 0;
S60:
/*
     P = (A2*S*H)MOD M
*/
    if(!(a1 != 0)) goto S90;
    q = m/a1;
    k = s/q;
    p -= (k*(m-a1*q));
    if(p > 0) p -= m;
    p += (a1*(s-k*q));
S70:
    if(!(p < 0)) goto S80;
    p += m;
    goto S70;
S90:
S80:
    k = p/qh;
/*
     P = ((A2*H + A1)*S)MOD M
*/
    p = h*(p-k*qh)-k*rh;
S100:
    if(!(p < 0)) goto S110;
    p += m;
    goto S100;
S120:
S110:
    if(!(a0 != 0)) goto S150;
/*
     P = ((A2*H + A1)*H*S)MOD M
*/
    q = m/a0;
    k = s/q;
    p -= (k*(m-a0*q));
    if(p > 0) p -= m;
    p += (a0*(s-k*q));
S130:
    if(!(p < 0)) goto S140;
    p += m;
    goto S130;
S150:
S140:
    mltmod = p;
    return mltmod;
#undef h
}


void advnst(long k)
/*
**********************************************************************
     void advnst(long k)
               ADV-a-N-ce ST-ate
     Advances the state  of  the current  generator  by 2^K values  and
     resets the initial seed to that value.
     This is  a  transcription from   Pascal to  Fortran    of  routine
     Advance_State from the paper
     L'Ecuyer, P. and  Cote, S. "Implementing  a  Random Number Package
     with  Splitting   Facilities."  ACM  Transactions  on Mathematical
     Software, 17:98-111 (1991)
                              Arguments
     k -> The generator is advanced by2^K values
**********************************************************************
*/
{
#define numg 32L
extern void gsrgs(long getset,long *qvalue);
extern void gscgn(long getset,long *g);
extern long Xm1,Xm2,Xa1,Xa2,Xcg1[],Xcg2[];
static long g,i,ib1,ib2;
static long qrgnin;
/*
     Abort unless random number generator initialized
*/
    gsrgs(0L,&qrgnin);
    if(qrgnin) goto S10;
    puts(" ADVNST called before random generator initialized - ABORT");
    exit(1);
S10:
    gscgn(0L,&g);
    ib1 = Xa1;
    ib2 = Xa2;
    for(i=1; i<=k; i++) {
        ib1 = mltmod(ib1,ib1,Xm1);
        ib2 = mltmod(ib2,ib2,Xm2);
    }
    setsd(mltmod(ib1,*(Xcg1+g-1),Xm1),mltmod(ib2,*(Xcg2+g-1),Xm2));
/*
     NOW, IB1 = A1**K AND IB2 = A2**K
*/
#undef numg
}


static long lennob(char *str);

void phrtsd(char* phrase,long *seed1,long *seed2)
/*
**********************************************************************
     void phrtsd(char* phrase,long *seed1,long *seed2)
               PHRase To SeeDs

                              Function

     Uses a phrase (character string) to generate two seeds for the RGN
     random number generator.
                              Arguments
     phrase --> Phrase to be used for random number generation

     seed1 <-- First seed for generator

     seed2 <-- Second seed for generator

                              Note

     Trailing blanks are eliminated before the seeds are generated.
     Generated seed values will fall in the range 1..2^30
     (1..1,073,741,824)
**********************************************************************
*/
{

static char table[] =
"abcdefghijklmnopqrstuvwxyz\
ABCDEFGHIJKLMNOPQRSTUVWXYZ\
0123456789\
!@#$%^&*()_+[];:'\\\"<>?,./";

long ix;

static long twop30 = 1073741824L;
static long shift[5] = {
    1L,64L,4096L,262144L,16777216L
};
static long i,ichr,j,lphr,values[5];

    *seed1 = 1234567890L;
    *seed2 = 123456789L;
    lphr = lennob(phrase);
    if(lphr < 1) return;
    for(i=0; i<=(lphr-1); i++) {
        for (ix=0; table[ix]; ix++) if (*(phrase+i) == table[ix]) break;
        if (!table[ix]) ix = 0;
        ichr = ix % 64;
        if(ichr == 0) ichr = 63;
        for(j=1; j<=5; j++) {
            *(values+j-1) = ichr-j;
            if(*(values+j-1) < 1) *(values+j-1) += 63;
        }
        for(j=1; j<=5; j++) {
            *seed1 = ( *seed1+*(shift+j-1)**(values+j-1) ) % twop30;
            *seed2 = ( *seed2+*(shift+j-1)**(values+6-j-1) )  % twop30;
        }
    }    
#undef twop30
}

static long lennob( char *str )
/*
Returns the length of str ignoring trailing blanks but not
other white space.
*/
{
long i, i_nb;

for (i=0, i_nb= -1L; *(str+i); i++)
    if ( *(str+i) != ' ' ) i_nb = i;
return (i_nb+1);
    }

