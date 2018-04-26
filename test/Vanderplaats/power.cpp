/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//
// File: power.cpp
//
// MATLAB Coder version            : 3.4
// C/C++ source code generated on  : 23-Feb-2018 13:38:03
//

// Include Files
#include "rt_nonfinite.h"
#include "fbeamGV.h"
#include "gbeamGV.h"
#include "power.h"
#include "fbeamGV_emxutil.h"

// Function Declarations
static double rt_powd_snf(double u0, double u1);

// Function Definitions

//
// Arguments    : double u0
//                double u1
// Return Type  : double
//
static double rt_powd_snf(double u0, double u1)
{
  double y;
  double d0;
  double d1;
  if (rtIsNaN(u0) || rtIsNaN(u1)) {
    y = rtNaN;
  } else {
    d0 = std::abs(u0);
    d1 = std::abs(u1);
    if (rtIsInf(u1)) {
      if (d0 == 1.0) {
        y = 1.0;
      } else if (d0 > 1.0) {
        if (u1 > 0.0) {
          y = rtInf;
        } else {
          y = 0.0;
        }
      } else if (u1 > 0.0) {
        y = 0.0;
      } else {
        y = rtInf;
      }
    } else if (d1 == 0.0) {
      y = 1.0;
    } else if (d1 == 1.0) {
      if (u1 > 0.0) {
        y = u0;
      } else {
        y = 1.0 / u0;
      }
    } else if (u1 == 2.0) {
      y = u0 * u0;
    } else if ((u1 == 0.5) && (u0 >= 0.0)) {
      y = std::sqrt(u0);
    } else if ((u0 < 0.0) && (u1 > std::floor(u1))) {
      y = rtNaN;
    } else {
      y = pow(u0, u1);
    }
  }

  return y;
}

//
// Arguments    : const emxArray_real_T *a
//                emxArray_real_T *y
// Return Type  : void
//
void b_power(const emxArray_real_T *a, emxArray_real_T *y)
{
  unsigned short a_idx_0;
  unsigned short b_a_idx_0;
  int k;
  a_idx_0 = (unsigned short)a->size[0];
  b_a_idx_0 = (unsigned short)a->size[0];
  k = y->size[0];
  y->size[0] = b_a_idx_0;
  emxEnsureCapacity_real_T1(y, k);
  for (k = 0; k + 1 <= a_idx_0; k++) {
    y->data[k] = a->data[k] * a->data[k];
  }
}

//
// Arguments    : const emxArray_real_T *a
//                emxArray_real_T *y
// Return Type  : void
//
void power(const emxArray_real_T *a, emxArray_real_T *y)
{
  unsigned short a_idx_0;
  unsigned short b_a_idx_0;
  int k;
  a_idx_0 = (unsigned short)a->size[0];
  b_a_idx_0 = (unsigned short)a->size[0];
  k = y->size[0];
  y->size[0] = b_a_idx_0;
  emxEnsureCapacity_real_T1(y, k);
  for (k = 0; k + 1 <= a_idx_0; k++) {
    y->data[k] = rt_powd_snf(a->data[k], 3.0);
  }
}

//
// File trailer for power.cpp
//
// [EOF]
//
