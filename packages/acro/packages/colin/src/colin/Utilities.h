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

/**
 * \file Utilities.h
 *
 *  This file defines a variety of utility functions for COLIN.
 */

#ifndef colin_Utilities_h
#define colin_Utilities_h

#include <acro_config.h>
//#include <utilib/pvector.h>
//#include <colin/Problem.h>
#include <colin/real.h>

namespace colin
{

#if 0
/// WEH - unused

template <class A, class B>
double dist(const A& a, const B& b)
{
   double ans = 0.0;
   for (unsigned int i = 0; i < a.size(); i++)
      ans += (a[i] - b[i]) * (a[i] - b[i]);
   return sqrt(ans);
}
#endif


template <class A>
real l2_norm_sq(const A& a)
{
   real ans = 0.0;
   for (size_t i = 0; i < a.size(); i++)
      ans += a[i] * a[i];
   return ans;
}


#if 0
/// WEH - unused

template <class ProblemT>
void summarize_integer_bounds(Problem<ProblemT>& prob)
{
   if (prob->enforcing_bounds() && (prob->num_int_params() > 0))
   {
      utilib::pvector<int> lb, ub;
      prob->get_int_bounds(lb, ub);
      ucout << "int lb: " << lb << std::endl;
      ucout << "int ub: " << ub << std::endl;
   }
}
#endif

}

#endif
