/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//
// File: sum.cpp
//
// MATLAB Coder version            : 3.4
// C/C++ source code generated on  : 23-Feb-2018 13:38:03
//

// Include Files
#include "rt_nonfinite.h"
#include "fbeamGV.h"
#include "gbeamGV.h"
#include "sum.h"

// Function Definitions

//
// Arguments    : const emxArray_real_T *x
// Return Type  : double
//
double sum(const emxArray_real_T *x)
{
  double y;
  int k;
  if (x->size[0] == 0) {
    y = 0.0;
  } else {
    y = x->data[0];
    for (k = 2; k <= x->size[0]; k++) {
      y += x->data[k - 1];
    }
  }

  return y;
}

//
// File trailer for sum.cpp
//
// [EOF]
//
