/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//
// File: gbeamGV.cpp
//
// MATLAB Coder version            : 3.4
// C/C++ source code generated on  : 23-Feb-2018 13:38:03
//

// Include Files
#include "rt_nonfinite.h"
#include "fbeamGV.h"
#include "gbeamGV.h"
#include "fbeamGV_emxutil.h"
#include "eye.h"
#include "rdivide.h"
#include "power.h"

// Function Definitions

//
// GBEAMGV Gradients for Gary Vanderplaats Cantilever beam with N design variables
// Arguments    : const emxArray_real_T *x
//                emxArray_real_T *gradf
//                emxArray_real_T *gradg
// Return Type  : void
//
void gbeamGV(const emxArray_real_T *x, emxArray_real_T *gradf, emxArray_real_T
             *gradg)
{
  double nelm;
  emxArray_real_T *bi;
  int i2;
  int apnd;
  emxArray_real_T *hi;
  int ndbl;
  double b_apnd;
  int absb;
  double cdiff;
  double u0;
  emxArray_real_T *b;
  double u1;
  emxArray_real_T *h;
  emxArray_real_T *ll;
  emxArray_real_T *r4;
  int nm1d2;
  int k;
  short tmp_data[25000];
  emxArray_uint16_T *r5;
  emxArray_real_T *y;
  emxArray_real_T *suml;
  emxArray_real_T *M;
  emxArray_real_T *r6;
  emxArray_real_T *A;
  emxArray_real_T *r7;
  short b_tmp_data[25000];
  emxArray_real_T *b_A;
  emxArray_real_T *b_b;
  unsigned short unnamed_idx_0;
  unsigned short unnamed_idx_1;
  emxArray_uint16_T *r8;
  emxArray_real_T *c_b;
  emxArray_real_T *dyN;

  //         from Vanderplaats (1984) Example 5-1, pp. 147-150.
  nelm = (double)x->size[0] / 2.0;
  emxInit_real_T(&bi, 2);
  if (nelm < 1.0) {
    i2 = bi->size[0] * bi->size[1];
    bi->size[0] = 1;
    bi->size[1] = 0;
    emxEnsureCapacity_real_T(bi, i2);
  } else {
    i2 = bi->size[0] * bi->size[1];
    bi->size[0] = 1;
    bi->size[1] = (int)std::floor(nelm - 1.0) + 1;
    emxEnsureCapacity_real_T(bi, i2);
    apnd = (int)std::floor(nelm - 1.0);
    for (i2 = 0; i2 <= apnd; i2++) {
      bi->data[bi->size[0] * i2] = 1.0 + (double)i2;
    }
  }

  emxInit_real_T(&hi, 2);
  if (x->size[0] < nelm + 1.0) {
    i2 = hi->size[0] * hi->size[1];
    hi->size[0] = 1;
    hi->size[1] = 0;
    emxEnsureCapacity_real_T(hi, i2);
  } else if (std::floor(nelm + 1.0) == nelm + 1.0) {
    i2 = x->size[0];
    absb = hi->size[0] * hi->size[1];
    hi->size[0] = 1;
    hi->size[1] = (int)std::floor((double)i2 - (nelm + 1.0)) + 1;
    emxEnsureCapacity_real_T(hi, absb);
    apnd = (int)std::floor((double)i2 - (nelm + 1.0));
    for (i2 = 0; i2 <= apnd; i2++) {
      hi->data[hi->size[0] * i2] = (nelm + 1.0) + (double)i2;
    }
  } else {
    ndbl = (int)std::floor(((double)x->size[0] - (nelm + 1.0)) + 0.5);
    b_apnd = (nelm + 1.0) + (double)ndbl;
    cdiff = b_apnd - (double)x->size[0];
    absb = x->size[0];
    u0 = nelm + 1.0;
    u1 = absb;
    if (u0 > u1) {
      u1 = u0;
    }

    if (std::abs(cdiff) < 4.4408920985006262E-16 * u1) {
      ndbl++;
      b_apnd = x->size[0];
    } else if (cdiff > 0.0) {
      b_apnd = (nelm + 1.0) + ((double)ndbl - 1.0);
    } else {
      ndbl++;
    }

    if (ndbl >= 0) {
      absb = ndbl;
    } else {
      absb = 0;
    }

    i2 = hi->size[0] * hi->size[1];
    hi->size[0] = 1;
    hi->size[1] = absb;
    emxEnsureCapacity_real_T(hi, i2);
    if (absb > 0) {
      hi->data[0] = nelm + 1.0;
      if (absb > 1) {
        hi->data[absb - 1] = b_apnd;
        nm1d2 = (absb - 1) / 2;
        for (k = 1; k < nm1d2; k++) {
          hi->data[k] = (nelm + 1.0) + (double)k;
          hi->data[(absb - k) - 1] = b_apnd - (double)k;
        }

        if (nm1d2 << 1 == absb - 1) {
          hi->data[nm1d2] = ((nelm + 1.0) + b_apnd) / 2.0;
        } else {
          hi->data[nm1d2] = (nelm + 1.0) + (double)nm1d2;
          hi->data[nm1d2 + 1] = b_apnd - (double)nm1d2;
        }
      }
    }
  }

  emxInit_real_T1(&b, 1);
  i2 = b->size[0];
  b->size[0] = bi->size[1];
  emxEnsureCapacity_real_T1(b, i2);
  apnd = bi->size[1];
  for (i2 = 0; i2 < apnd; i2++) {
    b->data[i2] = x->data[(int)bi->data[bi->size[0] * i2] - 1];
  }

  emxInit_real_T1(&h, 1);

  //  base width of each element in meters from cm
  i2 = h->size[0];
  h->size[0] = hi->size[1];
  emxEnsureCapacity_real_T1(h, i2);
  apnd = hi->size[1];
  for (i2 = 0; i2 < apnd; i2++) {
    h->data[i2] = x->data[(int)hi->data[hi->size[0] * i2] - 1];
  }

  emxInit_real_T1(&ll, 1);
  emxInit_real_T1(&r4, 1);

  //  height of each element in meters from cm
  //  Newtons
  //  Newtons / cm^2 (Pascal x10^-4)
  //  Length in centimeters
  //  N/cm^2
  // 0.03457; % centimeters
  power(h, r4);
  b_apnd = 500.0 / nelm;
  i2 = ll->size[0];
  ll->size[0] = (int)nelm;
  emxEnsureCapacity_real_T1(ll, i2);
  apnd = (int)nelm;
  for (i2 = 0; i2 < apnd; i2++) {
    ll->data[i2] = b_apnd;
  }

  //  Objective (volume) gradient
  i2 = gradf->size[0];
  gradf->size[0] = x->size[0];
  emxEnsureCapacity_real_T1(gradf, i2);
  apnd = x->size[0];
  for (i2 = 0; i2 < apnd; i2++) {
    gradf->data[i2] = 0.0;
  }

  absb = bi->size[1];
  apnd = bi->size[0] * bi->size[1];
  for (i2 = 0; i2 < apnd; i2++) {
    tmp_data[i2] = (short)bi->data[i2];
  }

  for (i2 = 0; i2 < absb; i2++) {
    gradf->data[tmp_data[i2] - 1] = h->data[i2] * ll->data[i2];
  }

  emxInit_uint16_T(&r5, 2);
  i2 = r5->size[0] * r5->size[1];
  r5->size[0] = 1;
  r5->size[1] = hi->size[1];
  emxEnsureCapacity_uint16_T(r5, i2);
  apnd = hi->size[0] * hi->size[1];
  for (i2 = 0; i2 < apnd; i2++) {
    r5->data[i2] = (unsigned short)hi->data[i2];
  }

  apnd = r5->size[0] * r5->size[1];
  for (i2 = 0; i2 < apnd; i2++) {
    gradf->data[r5->data[i2] - 1] = b->data[i2] * ll->data[i2];
  }

  emxFree_uint16_T(&r5);

  //  Calculate gradients of lateral slopes, deflections and stresses
  i2 = gradg->size[0] * gradg->size[1];
  gradg->size[0] = x->size[0];
  gradg->size[1] = x->size[0] + 1;
  emxEnsureCapacity_real_T(gradg, i2);
  apnd = x->size[0] * (x->size[0] + 1);
  for (i2 = 0; i2 < apnd; i2++) {
    gradg->data[i2] = 0.0;
  }

  emxInit_real_T(&y, 2);
  if (nelm < 1.0) {
    i2 = y->size[0] * y->size[1];
    y->size[0] = 1;
    y->size[1] = 0;
    emxEnsureCapacity_real_T(y, i2);
  } else {
    i2 = y->size[0] * y->size[1];
    y->size[0] = 1;
    y->size[1] = (int)std::floor(nelm - 1.0) + 1;
    emxEnsureCapacity_real_T(y, i2);
    apnd = (int)std::floor(nelm - 1.0);
    for (i2 = 0; i2 <= apnd; i2++) {
      y->data[y->size[0] * i2] = 1.0 + (double)i2;
    }
  }

  emxInit_real_T1(&suml, 1);
  i2 = suml->size[0];
  suml->size[0] = y->size[1];
  emxEnsureCapacity_real_T1(suml, i2);
  apnd = y->size[1];
  for (i2 = 0; i2 < apnd; i2++) {
    suml->data[i2] = y->data[y->size[0] * i2] * 500.0 / nelm;
  }

  emxInit_real_T1(&M, 1);

  //  Stress constraint gradients
  i2 = M->size[0];
  M->size[0] = suml->size[0];
  emxEnsureCapacity_real_T1(M, i2);
  apnd = suml->size[0];
  for (i2 = 0; i2 < apnd; i2++) {
    M->data[i2] = 50000.0 * ((500.0 - suml->data[i2]) + ll->data[i2]);
  }

  emxInit_real_T1(&r6, 1);
  emxInit_real_T1(&A, 1);
  b_power(b, A);
  b_power(h, r6);
  apnd = bi->size[1];
  for (i2 = 0; i2 < apnd; i2++) {
    b_tmp_data[i2] = (short)((short)bi->data[bi->size[0] * i2] - 1);
  }

  emxInit_real_T1(&r7, 1);
  i2 = r7->size[0];
  r7->size[0] = M->size[0];
  emxEnsureCapacity_real_T1(r7, i2);
  apnd = M->size[0];
  for (i2 = 0; i2 < apnd; i2++) {
    r7->data[i2] = -6.0 * M->data[i2];
  }

  emxInit_real_T1(&b_A, 1);
  i2 = b_A->size[0];
  b_A->size[0] = A->size[0];
  emxEnsureCapacity_real_T1(b_A, i2);
  apnd = A->size[0];
  for (i2 = 0; i2 < apnd; i2++) {
    b_A->data[i2] = A->data[i2] * r6->data[i2];
  }

  rdivide(r7, b_A, A);
  i2 = A->size[0];
  emxEnsureCapacity_real_T1(A, i2);
  apnd = A->size[0];
  emxFree_real_T(&b_A);
  emxFree_real_T(&r7);
  for (i2 = 0; i2 < apnd; i2++) {
    A->data[i2] /= 14000.0;
  }

  emxInit_real_T(&b_b, 2);
  unnamed_idx_0 = (unsigned short)A->size[0];
  unnamed_idx_1 = (unsigned short)A->size[0];
  i2 = b_b->size[0] * b_b->size[1];
  b_b->size[0] = unnamed_idx_0;
  b_b->size[1] = unnamed_idx_1;
  emxEnsureCapacity_real_T(b_b, i2);
  apnd = unnamed_idx_0 * unnamed_idx_1;
  for (i2 = 0; i2 < apnd; i2++) {
    b_b->data[i2] = 0.0;
  }

  for (absb = 0; absb + 1 <= A->size[0]; absb++) {
    b_b->data[absb + b_b->size[0] * absb] = A->data[absb];
  }

  apnd = b_b->size[1];
  for (i2 = 0; i2 < apnd; i2++) {
    nm1d2 = b_b->size[0];
    for (absb = 0; absb < nm1d2; absb++) {
      gradg->data[b_tmp_data[absb] + gradg->size[0] * i2] = b_b->data[absb +
        b_b->size[0] * i2];
    }
  }

  emxInit_uint16_T1(&r8, 1);
  power(h, A);
  i2 = r8->size[0];
  r8->size[0] = hi->size[1];
  emxEnsureCapacity_uint16_T1(r8, i2);
  apnd = hi->size[1];
  for (i2 = 0; i2 < apnd; i2++) {
    r8->data[i2] = (unsigned short)((unsigned short)hi->data[hi->size[0] * i2] -
      1U);
  }

  i2 = r6->size[0];
  r6->size[0] = M->size[0];
  emxEnsureCapacity_real_T1(r6, i2);
  apnd = M->size[0];
  for (i2 = 0; i2 < apnd; i2++) {
    r6->data[i2] = -12.0 * M->data[i2];
  }

  emxFree_real_T(&M);
  emxInit_real_T1(&c_b, 1);
  i2 = c_b->size[0];
  c_b->size[0] = b->size[0];
  emxEnsureCapacity_real_T1(c_b, i2);
  apnd = b->size[0];
  for (i2 = 0; i2 < apnd; i2++) {
    c_b->data[i2] = b->data[i2] * A->data[i2];
  }

  rdivide(r6, c_b, A);
  i2 = A->size[0];
  emxEnsureCapacity_real_T1(A, i2);
  apnd = A->size[0];
  for (i2 = 0; i2 < apnd; i2++) {
    A->data[i2] /= 14000.0;
  }

  unnamed_idx_0 = (unsigned short)A->size[0];
  unnamed_idx_1 = (unsigned short)A->size[0];
  i2 = b_b->size[0] * b_b->size[1];
  b_b->size[0] = unnamed_idx_0;
  b_b->size[1] = unnamed_idx_1;
  emxEnsureCapacity_real_T(b_b, i2);
  apnd = unnamed_idx_0 * unnamed_idx_1;
  for (i2 = 0; i2 < apnd; i2++) {
    b_b->data[i2] = 0.0;
  }

  for (absb = 0; absb + 1 <= A->size[0]; absb++) {
    b_b->data[absb + b_b->size[0] * absb] = A->data[absb];
  }

  apnd = b_b->size[1];
  for (i2 = 0; i2 < apnd; i2++) {
    nm1d2 = b_b->size[0];
    for (absb = 0; absb < nm1d2; absb++) {
      gradg->data[r8->data[absb] + gradg->size[0] * i2] = b_b->data[absb +
        b_b->size[0] * i2];
    }
  }

  //  Tip displacement constraint
  if (nelm - 1.0 < 0.0) {
    i2 = y->size[0] * y->size[1];
    y->size[0] = 1;
    y->size[1] = 0;
    emxEnsureCapacity_real_T(y, i2);
  } else if (nelm - 1.0 == nelm - 1.0) {
    i2 = y->size[0] * y->size[1];
    y->size[0] = 1;
    y->size[1] = (int)std::floor(-(0.0 - (nelm - 1.0))) + 1;
    emxEnsureCapacity_real_T(y, i2);
    apnd = (int)std::floor(-(0.0 - (nelm - 1.0)));
    for (i2 = 0; i2 <= apnd; i2++) {
      y->data[y->size[0] * i2] = (nelm - 1.0) - (double)i2;
    }
  } else {
    ndbl = (int)std::floor((0.0 - (nelm - 1.0)) / -1.0 + 0.5);
    apnd = ((int)nelm - ndbl) - 1;
    absb = (int)std::abs(nelm - 1.0);
    if (std::abs(0.0 - (double)apnd) < 4.4408920985006262E-16 * (double)absb) {
      ndbl++;
      apnd = 0;
    } else if (-apnd > 0) {
      apnd = (int)nelm - ndbl;
    } else {
      ndbl++;
    }

    i2 = y->size[0] * y->size[1];
    y->size[0] = 1;
    y->size[1] = ndbl;
    emxEnsureCapacity_real_T(y, i2);
    if (ndbl > 0) {
      y->data[0] = nelm - 1.0;
      if (ndbl > 1) {
        y->data[ndbl - 1] = apnd;
        nm1d2 = (ndbl - 1) / 2;
        for (k = 1; k < nm1d2; k++) {
          y->data[k] = (nelm - 1.0) + -(double)k;
          y->data[(ndbl - k) - 1] = apnd + k;
        }

        if (nm1d2 << 1 == ndbl - 1) {
          y->data[nm1d2] = ((nelm - 1.0) + (double)apnd) / 2.0;
        } else {
          y->data[nm1d2] = (nelm - 1.0) + -(double)nm1d2;
          y->data[nm1d2 + 1] = apnd + nm1d2;
        }
      }
    }
  }

  //  for i = 2:nelm-1
  //  	 dyp(i) = dyp1(i) + dyp(i-1);
  //  end
  // dyp =                       (L - suml +   ll/2);    dyp(end)=0;
  // dyN = -P/E*ll.^2./Inertia.*((L - suml + 2*ll/3)/2 + dyp);
  b_power(ll, A);
  i2 = r6->size[0];
  r6->size[0] = A->size[0];
  emxEnsureCapacity_real_T1(r6, i2);
  apnd = A->size[0];
  for (i2 = 0; i2 < apnd; i2++) {
    r6->data[i2] = -0.0025 * A->data[i2];
  }

  i2 = c_b->size[0];
  c_b->size[0] = b->size[0];
  emxEnsureCapacity_real_T1(c_b, i2);
  apnd = b->size[0];
  for (i2 = 0; i2 < apnd; i2++) {
    c_b->data[i2] = b->data[i2] * r4->data[i2] / 12.0;
  }

  emxInit_real_T1(&dyN, 1);
  rdivide(r6, c_b, r4);
  i2 = dyN->size[0];
  dyN->size[0] = r4->size[0];
  emxEnsureCapacity_real_T1(dyN, i2);
  apnd = r4->size[0];
  emxFree_real_T(&c_b);
  emxFree_real_T(&r6);
  for (i2 = 0; i2 < apnd; i2++) {
    dyN->data[i2] = r4->data[i2] * (((500.0 - suml->data[i2]) + 2.0 * ll->
      data[i2] / 3.0) / 2.0 + ((500.0 - suml->data[i2]) + ll->data[i2] / 2.0) *
      y->data[y->size[0] * i2]);
  }

  emxFree_real_T(&y);
  emxFree_real_T(&r4);
  emxFree_real_T(&suml);
  emxFree_real_T(&ll);
  apnd = bi->size[1];
  for (i2 = 0; i2 < apnd; i2++) {
    b_tmp_data[i2] = (short)((short)bi->data[bi->size[0] * i2] - 1);
  }

  rdivide(dyN, b, A);
  apnd = A->size[0];
  emxFree_real_T(&b);
  for (i2 = 0; i2 < apnd; i2++) {
    gradg->data[b_tmp_data[i2] + gradg->size[0] * (int)nelm] = A->data[i2] / 2.5;
  }

  i2 = r8->size[0];
  r8->size[0] = hi->size[1];
  emxEnsureCapacity_uint16_T1(r8, i2);
  apnd = hi->size[1];
  for (i2 = 0; i2 < apnd; i2++) {
    r8->data[i2] = (unsigned short)((unsigned short)hi->data[hi->size[0] * i2] -
      1U);
  }

  rdivide(dyN, h, A);
  apnd = A->size[0];
  emxFree_real_T(&dyN);
  emxFree_real_T(&h);
  for (i2 = 0; i2 < apnd; i2++) {
    gradg->data[r8->data[i2] + gradg->size[0] * (int)nelm] = A->data[i2] * 3.0 /
      2.5;
  }

  emxFree_real_T(&A);

  //  Aspect ratio constraint gradients
  if ((int)nelm + 2 > gradg->size[1]) {
    i2 = 1;
  } else {
    i2 = (int)nelm + 2;
  }

  apnd = bi->size[1];
  for (absb = 0; absb < apnd; absb++) {
    b_tmp_data[absb] = (short)((short)bi->data[bi->size[0] * absb] - 1);
  }

  emxFree_real_T(&bi);
  eye(nelm, b_b);
  apnd = b_b->size[1];
  for (absb = 0; absb < apnd; absb++) {
    nm1d2 = b_b->size[0];
    for (k = 0; k < nm1d2; k++) {
      gradg->data[b_tmp_data[k] + gradg->size[0] * ((i2 + absb) - 1)] = -20.0 *
        b_b->data[k + b_b->size[0] * absb];
    }
  }

  if ((int)nelm + 2 > gradg->size[1]) {
    i2 = 1;
  } else {
    i2 = (int)nelm + 2;
  }

  absb = r8->size[0];
  r8->size[0] = hi->size[1];
  emxEnsureCapacity_uint16_T1(r8, absb);
  apnd = hi->size[1];
  for (absb = 0; absb < apnd; absb++) {
    r8->data[absb] = (unsigned short)((unsigned short)hi->data[hi->size[0] *
      absb] - 1U);
  }

  emxFree_real_T(&hi);
  eye(nelm, b_b);
  apnd = b_b->size[1];
  for (absb = 0; absb < apnd; absb++) {
    nm1d2 = b_b->size[0];
    for (k = 0; k < nm1d2; k++) {
      gradg->data[r8->data[k] + gradg->size[0] * ((i2 + absb) - 1)] = b_b->
        data[k + b_b->size[0] * absb];
    }
  }

  emxFree_real_T(&b_b);
  emxFree_uint16_T(&r8);

  //
  //  matfile = ['Comparison\mca_comp_' num2str(length(x)) '.mat'];
  //      Store=load(matfile);
  //      if size(Store.X,2)>1
  //          Store.gradf = [Store.gradf gradf];
  //          Store.gradg = [Store.gradg gradg];
  //          save(matfile,'-struct','Store','-append')
  //      else
  //          Store.gradf = gradf;
  //          Store.gradg = gradg;
  //          save(matfile,'-struct','Store')
  //      end
}

//
// File trailer for gbeamGV.cpp
//
// [EOF]
//
