/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//
// File: rdivide.cpp
//
// MATLAB Coder version            : 3.4
// C/C++ source code generated on  : 23-Feb-2018 13:38:03
//

// Include Files
#include "rt_nonfinite.h"
#include "fbeamGV.h"
#include "gbeamGV.h"
#include "rdivide.h"
#include "fbeamGV_emxutil.h"

// Function Definitions

//
// Arguments    : const emxArray_real_T *x
//                const emxArray_real_T *y
//                emxArray_real_T *z
// Return Type  : void
//
void rdivide(const emxArray_real_T *x, const emxArray_real_T *y, emxArray_real_T
             *z)
{
  int i1;
  int loop_ub;
  i1 = z->size[0];
  z->size[0] = x->size[0];
  emxEnsureCapacity_real_T1(z, i1);
  loop_ub = x->size[0];
  for (i1 = 0; i1 < loop_ub; i1++) {
    z->data[i1] = x->data[i1] / y->data[i1];
  }
}

//
// File trailer for rdivide.cpp
//
// [EOF]
//
