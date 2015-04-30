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

/* math.c
 *
 */


#include <utilib_config.h>
#include <stdlib.h>
#include <utilib/math_basic.h>
#include <math.h>

#ifndef UTILIB_HAVE_LROUND
extern long int lround(double x);
#endif
void setup_bufexp(int tabsz, double xmin, double xmax);
double bufexp(double x);


#ifndef UTILIB_HAVE_LROUND
long int lround(double x)
{
double temp;

temp = floor(x);
if ((x - temp) < 0.5)
   return( (long int) temp );
else
   return( (long int) ceil(x) );
}
#endif

/* We hope this is a portable method for rounding a double to num_digits
   number of decimal digits after the decimal point. We may want to add
   some tolerances. */

double d_round(double to_round, unsigned int num_digits)
{
double intpart;
double shift_factor;
double val;

intpart = floor(to_round);
to_round -= intpart;
shift_factor = pow(10, num_digits);

val = to_round * shift_factor;
val += 0.5;
val = floor(val);
val /= shift_factor;
val += intpart;

return(val);
}




/*  Table for 'exp'
 *
 *  I don't know if this is a good way to do this.
 */

static int exp_tabsz=0;
static double exp_xmin=-999.0;
static double exp_xmax=-999.0;
static double exp_step=-999.0;
static double* exp_tab=0;

#if 1
void setup_bufexp(int tabsz, double xmin, double xmax)
{
abort();
exp_tabsz=tabsz;
exp_xmin=xmin;
exp_xmax=xmax;
}
#else
void setup_bufexp(int tabsz, double xmin, double xmax)
{
static int flag=1;

if (tabsz < 1) return;

if (flag == 1) {
   double temp;
   int i;

   exp_tabsz = tabsz;
   exp_xmin = xmin;
   exp_xmax = xmax;

   exp_step = (xmax - xmin)/(double)(tabsz-1.0);

   exp_tab = dvector(0,tabsz-1);
   for (i=0, temp=exp_xmin; i<tabsz; i++, temp+=exp_step) {
     exp_tab[i] = exp(temp);
/*
     printf("%d\t%lf\t%lf\n",i,temp, exp_tab[i]);
*/
     }
   }
}
#endif


double bufexp(double x)
{
double ndx;
int lndx;

if (exp_tabsz == 0)
   setup_bufexp(5000,-70.0,70.0);

ndx = (x - exp_xmin)/exp_step;
if (ndx <= 0)
   return exp_tab[0];
if (ndx >= exp_tabsz-1)
   return exp_tab[exp_tabsz-1];
lndx = (int) floor(ndx);
return exp_tab[lndx] + (ndx - (double)lndx)*(exp_tab[lndx+1] - exp_tab[lndx]);
}


/// Calculates how many bits are needed to hold an unsigned value.
/// If the argument is 0, it returns 0.

unsigned int bitWidth(unsigned int x)
{
  unsigned int result;

  result = 0;

  while (x != 0)
    {
      result++;
      x >>= 1;
    }

  return result;
}


/* Computes the greatest common divisor of two integers using the */
/* classical Euclidean algorithm (remainder version).  Got this   */
/* particularly tight loop from Wikipedia!                        */

unsigned int gcd(unsigned int a, unsigned int b)
{
  while (b != 0)
    {
      register unsigned int temp = b;
      b = a % b;
      a = temp;
    }
  return a;
}

