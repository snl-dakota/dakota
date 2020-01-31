/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//
// File: fbeamGV.cpp
//
// MATLAB Coder version            : 3.4
// C/C++ source code generated on  : 23-Feb-2018 13:38:03
//

// Include Files
#include "rt_nonfinite.h"
#include "fbeamGV.h"
#include "gbeamGV.h"
#include "fbeamGV_emxutil.h"
#include "sum.h"
#include "rdivide.h"
#include "power.h"

// Function Definitions

//
// FBEAMGV Cantilever beam with N design variables
// Arguments    : const emxArray_real_T *x
//                double *f
//                emxArray_real_T *g
// Return Type  : void
//
void fbeamGV(const emxArray_real_T *x, double *f, emxArray_real_T *g)
{
  double nelm;
  emxArray_real_T *bi;
  int i0;
  int ndbl;
  emxArray_real_T *hi;
  double apnd;
  int n;
  double cdiff;
  int absb;
  double u0;
  emxArray_real_T *b;
  double u1;
  emxArray_real_T *h;
  emxArray_real_T *Inertia;
  emxArray_real_T *sigma;
  emxArray_real_T *y;
  emxArray_real_T *ll;
  emxArray_real_T *suml;
  emxArray_real_T *r0;
  emxArray_real_T *r1;
  emxArray_real_T *yp;
  emxArray_real_T *r2;
  emxArray_real_T *b_sigma;
  emxArray_real_T *b_yp;
  short tmp_data[25000];
  emxArray_int32_T *r3;

  //         from Vanderplaats (1984) Example 5-1, pp. 147-150.
  nelm = (double)x->size[0] / 2.0;
  emxInit_real_T(&bi, 2);
  if (nelm < 1.0) {
    i0 = bi->size[0] * bi->size[1];
    bi->size[0] = 1;
    bi->size[1] = 0;
    emxEnsureCapacity_real_T(bi, i0);
  } else {
    i0 = bi->size[0] * bi->size[1];
    bi->size[0] = 1;
    bi->size[1] = (int)std::floor(nelm - 1.0) + 1;
    emxEnsureCapacity_real_T(bi, i0);
    ndbl = (int)std::floor(nelm - 1.0);
    for (i0 = 0; i0 <= ndbl; i0++) {
      bi->data[bi->size[0] * i0] = 1.0 + (double)i0;
    }
  }

  emxInit_real_T(&hi, 2);
  if (x->size[0] < nelm + 1.0) {
    i0 = hi->size[0] * hi->size[1];
    hi->size[0] = 1;
    hi->size[1] = 0;
    emxEnsureCapacity_real_T(hi, i0);
  } else if (std::floor(nelm + 1.0) == nelm + 1.0) {
    i0 = x->size[0];
    n = hi->size[0] * hi->size[1];
    hi->size[0] = 1;
    hi->size[1] = (int)std::floor((double)i0 - (nelm + 1.0)) + 1;
    emxEnsureCapacity_real_T(hi, n);
    ndbl = (int)std::floor((double)i0 - (nelm + 1.0));
    for (i0 = 0; i0 <= ndbl; i0++) {
      hi->data[hi->size[0] * i0] = (nelm + 1.0) + (double)i0;
    }
  } else {
    ndbl = (int)std::floor(((double)x->size[0] - (nelm + 1.0)) + 0.5);
    apnd = (nelm + 1.0) + (double)ndbl;
    cdiff = apnd - (double)x->size[0];
    absb = x->size[0];
    u0 = nelm + 1.0;
    u1 = absb;
    if (u0 > u1) {
      u1 = u0;
    }

    if (std::abs(cdiff) < 4.4408920985006262E-16 * u1) {
      ndbl++;
      apnd = x->size[0];
    } else if (cdiff > 0.0) {
      apnd = (nelm + 1.0) + ((double)ndbl - 1.0);
    } else {
      ndbl++;
    }

    if (ndbl >= 0) {
      n = ndbl;
    } else {
      n = 0;
    }

    i0 = hi->size[0] * hi->size[1];
    hi->size[0] = 1;
    hi->size[1] = n;
    emxEnsureCapacity_real_T(hi, i0);
    if (n > 0) {
      hi->data[0] = nelm + 1.0;
      if (n > 1) {
        hi->data[n - 1] = apnd;
        ndbl = (n - 1) / 2;
        for (absb = 1; absb < ndbl; absb++) {
          hi->data[absb] = (nelm + 1.0) + (double)absb;
          hi->data[(n - absb) - 1] = apnd - (double)absb;
        }

        if (ndbl << 1 == n - 1) {
          hi->data[ndbl] = ((nelm + 1.0) + apnd) / 2.0;
        } else {
          hi->data[ndbl] = (nelm + 1.0) + (double)ndbl;
          hi->data[ndbl + 1] = apnd - (double)ndbl;
        }
      }
    }
  }

  emxInit_real_T1(&b, 1);
  i0 = b->size[0];
  b->size[0] = bi->size[1];
  emxEnsureCapacity_real_T1(b, i0);
  ndbl = bi->size[1];
  for (i0 = 0; i0 < ndbl; i0++) {
    b->data[i0] = x->data[(int)bi->data[bi->size[0] * i0] - 1];
  }

  emxInit_real_T1(&h, 1);

  //  base width of each element in meters from cm
  i0 = h->size[0];
  h->size[0] = hi->size[1];
  emxEnsureCapacity_real_T1(h, i0);
  ndbl = hi->size[1];
  for (i0 = 0; i0 < ndbl; i0++) {
    h->data[i0] = x->data[(int)hi->data[hi->size[0] * i0] - 1];
  }

  emxFree_real_T(&hi);
  emxInit_real_T1(&Inertia, 1);
  emxInit_real_T1(&sigma, 1);

  //  height of each element in meters from cm
  //  Newtons
  //  Newtons / cm^2 (Pascal x10^-4)
  //  Length in centimeters
  //  N/cm^2
  // 0.03457; % centimeters
  power(h, sigma);
  i0 = Inertia->size[0];
  Inertia->size[0] = b->size[0];
  emxEnsureCapacity_real_T1(Inertia, i0);
  ndbl = b->size[0];
  for (i0 = 0; i0 < ndbl; i0++) {
    Inertia->data[i0] = b->data[i0] * sigma->data[i0] / 12.0;
  }

  emxInit_real_T1(&y, 1);
  apnd = 500.0 / nelm;
  i0 = y->size[0];
  y->size[0] = (int)nelm;
  emxEnsureCapacity_real_T1(y, i0);
  ndbl = (int)nelm;
  for (i0 = 0; i0 < ndbl; i0++) {
    y->data[i0] = apnd;
  }

  emxInit_real_T1(&ll, 1);
  i0 = ll->size[0];
  ll->size[0] = (int)nelm;
  emxEnsureCapacity_real_T1(ll, i0);
  ndbl = (int)nelm;
  for (i0 = 0; i0 < ndbl; i0++) {
    ll->data[i0] = apnd;
  }

  if (nelm < 1.0) {
    i0 = bi->size[0] * bi->size[1];
    bi->size[0] = 1;
    bi->size[1] = 0;
    emxEnsureCapacity_real_T(bi, i0);
  } else {
    i0 = bi->size[0] * bi->size[1];
    bi->size[0] = 1;
    bi->size[1] = (int)std::floor(nelm - 1.0) + 1;
    emxEnsureCapacity_real_T(bi, i0);
    ndbl = (int)std::floor(nelm - 1.0);
    for (i0 = 0; i0 <= ndbl; i0++) {
      bi->data[bi->size[0] * i0] = 1.0 + (double)i0;
    }
  }

  emxInit_real_T1(&suml, 1);
  i0 = suml->size[0];
  suml->size[0] = bi->size[1];
  emxEnsureCapacity_real_T1(suml, i0);
  ndbl = bi->size[1];
  for (i0 = 0; i0 < ndbl; i0++) {
    suml->data[i0] = bi->data[bi->size[0] * i0] * 500.0 / nelm;
  }

  emxFree_real_T(&bi);
  emxInit_real_T1(&r0, 1);

  //  Calculate lateral slopes and deflections
  //
  //  yp = zeros(size(ll)); y=zeros(size(ll)); M=zeros(size(ll));
  //  yp(1) = P*ll(1)/(E*Inertia(1)) * (L - ll(1)/2);
  //  y(1) = P*ll(1)^2/(2*E*Inertia(1)) * (L - ll(1)/3);
  //  for i=2:nelm
  //  	suml = sum(ll(1:i));
  //  	yp(i) = yp(i-1) +                 P*ll(i)  /  (E*Inertia(i)) * (L +   ll(i)/2 - suml); 
  //  	y(i)  = y(i-1)  + yp(i-1)*ll(i) + P*ll(i)^2/(2*E*Inertia(i)) * (L + 2*ll(i)/3 - suml); 
  //  end
  //  g(nelm+1) = y(nelm) / tip_max - 1; % tip deflection constraint
  b_power(y, sigma);
  i0 = r0->size[0];
  r0->size[0] = sigma->size[0];
  emxEnsureCapacity_real_T1(r0, i0);
  ndbl = sigma->size[0];
  for (i0 = 0; i0 < ndbl; i0++) {
    r0->data[i0] = 50000.0 * sigma->data[i0];
  }

  emxInit_real_T1(&r1, 1);
  i0 = r1->size[0];
  r1->size[0] = Inertia->size[0];
  emxEnsureCapacity_real_T1(r1, i0);
  ndbl = Inertia->size[0];
  for (i0 = 0; i0 < ndbl; i0++) {
    r1->data[i0] = 4.0E+7 * Inertia->data[i0];
  }

  rdivide(r0, r1, sigma);
  i0 = r0->size[0];
  r0->size[0] = y->size[0];
  emxEnsureCapacity_real_T1(r0, i0);
  ndbl = y->size[0];
  for (i0 = 0; i0 < ndbl; i0++) {
    r0->data[i0] = 50000.0 * y->data[i0];
  }

  i0 = r1->size[0];
  r1->size[0] = Inertia->size[0];
  emxEnsureCapacity_real_T1(r1, i0);
  ndbl = Inertia->size[0];
  for (i0 = 0; i0 < ndbl; i0++) {
    r1->data[i0] = 2.0E+7 * Inertia->data[i0];
  }

  emxInit_real_T1(&yp, 1);
  emxInit_real_T1(&r2, 1);
  rdivide(r0, r1, r2);
  i0 = yp->size[0];
  yp->size[0] = r2->size[0];
  emxEnsureCapacity_real_T1(yp, i0);
  ndbl = r2->size[0];
  emxFree_real_T(&r1);
  for (i0 = 0; i0 < ndbl; i0++) {
    yp->data[i0] = r2->data[i0] * ((500.0 - suml->data[i0]) + ll->data[i0] / 2.0);
  }

  emxFree_real_T(&r2);
  for (ndbl = 0; ndbl <= (int)nelm - 2; ndbl++) {
    yp->data[ndbl + 1] += yp->data[ndbl];
  }

  if (1 > yp->size[0] - 1) {
    ndbl = 0;
  } else {
    ndbl = yp->size[0] - 1;
  }

  i0 = !(2 > ll->size[0]);
  emxInit_real_T1(&b_sigma, 1);
  n = b_sigma->size[0];
  b_sigma->size[0] = sigma->size[0];
  emxEnsureCapacity_real_T1(b_sigma, n);
  absb = sigma->size[0];
  for (n = 0; n < absb; n++) {
    b_sigma->data[n] = sigma->data[n] * ((500.0 - suml->data[n]) +
      0.66666666666666663 * y->data[n]);
  }

  emxInit_real_T1(&b_yp, 1);
  n = b_yp->size[0];
  b_yp->size[0] = ndbl;
  emxEnsureCapacity_real_T1(b_yp, n);
  for (n = 0; n < ndbl; n++) {
    b_yp->data[n] = yp->data[n] * ll->data[i0 + n];
  }

  emxFree_real_T(&yp);
  emxFree_real_T(&ll);
  apnd = sum(b_sigma) + sum(b_yp);
  i0 = r0->size[0];
  r0->size[0] = suml->size[0];
  emxEnsureCapacity_real_T1(r0, i0);
  ndbl = suml->size[0];
  emxFree_real_T(&b_yp);
  emxFree_real_T(&b_sigma);
  for (i0 = 0; i0 < ndbl; i0++) {
    r0->data[i0] = 50000.0 * ((500.0 - suml->data[i0]) + y->data[i0]) * (h->
      data[i0] / 2.0);
  }

  emxFree_real_T(&suml);
  rdivide(r0, Inertia, sigma);
  i0 = Inertia->size[0];
  Inertia->size[0] = b->size[0];
  emxEnsureCapacity_real_T1(Inertia, i0);
  ndbl = b->size[0];
  emxFree_real_T(&r0);
  for (i0 = 0; i0 < ndbl; i0++) {
    Inertia->data[i0] = b->data[i0] * h->data[i0] * y->data[i0];
  }

  emxFree_real_T(&y);
  cdiff = sum(Inertia);

  //  Volume of beam in cm^3
  i0 = g->size[0];
  g->size[0] = ((int)nelm << 1) + 1;
  emxEnsureCapacity_real_T1(g, i0);
  ndbl = (int)nelm << 1;
  emxFree_real_T(&Inertia);
  for (i0 = 0; i0 <= ndbl; i0++) {
    g->data[i0] = 0.0;
  }

  g->data[(int)nelm] = apnd / 2.5 - 1.0;

  //  tip deflection constraint
  if (1.0 > nelm) {
    i0 = 0;
  } else {
    i0 = (int)nelm;
  }

  ndbl = (short)((short)i0 - 1);
  for (n = 0; n <= ndbl; n++) {
    tmp_data[n] = (short)n;
  }

  ndbl = (short)((short)i0 - 1) + 1;
  for (i0 = 0; i0 < ndbl; i0++) {
    g->data[tmp_data[i0]] = sigma->data[i0] / 14000.0 - 1.0;
  }

  emxFree_real_T(&sigma);

  //  stresses
  i0 = ((int)nelm << 1) + 1;
  if ((int)nelm + 2 > i0) {
    n = 1;
    i0 = 0;
  } else {
    n = (int)nelm + 2;
  }

  emxInit_int32_T(&r3, 2);
  ndbl = r3->size[0] * r3->size[1];
  r3->size[0] = 1;
  r3->size[1] = (i0 - n) + 1;
  emxEnsureCapacity_int32_T(r3, ndbl);
  ndbl = (i0 - n) + 1;
  for (i0 = 0; i0 < ndbl; i0++) {
    r3->data[r3->size[0] * i0] = (n + i0) - 1;
  }

  ndbl = r3->size[0] * r3->size[1];
  for (i0 = 0; i0 < ndbl; i0++) {
    g->data[r3->data[i0]] = h->data[i0] - 20.0 * b->data[i0];
  }

  emxFree_int32_T(&r3);
  emxFree_real_T(&h);
  emxFree_real_T(&b);

  //  aspect ratios (cm)
  //  matfile = ['Comparison\mca_comp_' num2str(length(x)) '.mat'];
  //  if ~exist(matfile,'file')
  //      Store.X = x;
  //      Store.F = f;
  //      Store.MG = max(g);
  //      Store.G = g;
  //      save(matfile,'-struct','Store')
  //  else
  //      Store=load(matfile,'X','F','MG','G');
  //      Store.X = [Store.X x];
  //      Store.F = [Store.F f];
  //      Store.MG = [Store.MG max(g)];
  //      Store.G = [Store.G g];
  //      save(matfile,'-struct','Store','-append')
  //  end
  *f = cdiff;
}

//
// File trailer for fbeamGV.cpp
//
// [EOF]
//
