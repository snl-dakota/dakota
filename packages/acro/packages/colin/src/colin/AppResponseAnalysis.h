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
 * \file AppResponseAnalysis.h
 *
 * Defines the colin::AppResponseAnalysis class.
 */

#ifndef colin_AppResponseAnalysis_h
#define colin_AppResponseAnalysis_h

#include <acro_config.h>
#include <utilib/std_headers.h>
#include <utilib/ParameterSet.h>
#include <colin/real.h>

namespace colin
{

/// Computes the constraint violation for an array of constraint
/// values, given upper and lower bounds on those values.
template <class ArrayT, class CArrayT>
void constraint_violation(ArrayT& cvals,
                          CArrayT& clower,
                          CArrayT& cupper,
                          real& constr_violation,
                          double tolerance)
{
   if (cvals.size() != clower.size())
      EXCEPTION_MNGR(std::runtime_error, "constraint_violation - cvals has length " << cvals.size() << " but clower has length " << clower.size());
      if (cvals.size() != cupper.size())
         EXCEPTION_MNGR(std::runtime_error, "constraint_violation - cvals has length " << cvals.size() << " but cupper has length " << cupper.size());
         if (cvals.size() == 0)
         {
            constr_violation = 0.0;
            return;
         }

   constr_violation = 0.0;
   for (unsigned int k = 0; k < cvals.size(); k++)
   {
#ifdef _MSC_VER
      if (_finite(clower[k]) &&
#else
      if (::finite(clower[k]) &&
#endif
            (cvals[k] < clower[k] - tolerance))
         constr_violation += (cvals[k] - clower[k]) * (cvals[k] - clower[k]);
#ifdef _MSC_VER
      else if (_finite(cupper[k]) &&
#else
      else if (::finite(cupper[k]) &&
#endif
               (cvals[k] > cupper[k] + tolerance))
         constr_violation += (cvals[k] - cupper[k]) * (cvals[k] - cupper[k]);
   }
}



#if 0
/// Defines mechanisms for analyzing an AppResponse, and in particular for
/// computing an objective function value using various penalty
/// functions.
class AppResponseAnalysis : virtual public utilib::ParameterSet
{
public:

   /// Constructor.
   AppResponseAnalysis();

   /// Compute the constraint violation (L2 norm).
   template <class ArrayT, class CArrayT>
   void constraint_violation(ArrayT& cvals, CArrayT& clower, CArrayT& cupper,
                             real& constr_violation)
   {
      colin::constraint_violation(cvals, clower, cupper, constr_violation,
                                  _constraint_tolerance);
   }

   /// Compute an augmented objective function with a penalty that is
   /// weighted by a convergence factor.
   real compute_penalty_function(real& fval, real& constr_violation,
                                 double convergence_factor = 1.0)
   {
      if (ignore_convergence_factor)
         return fval + constraint_penalty * constr_violation;

      //
      // If the convergence factor is effectively infinity but we're
      // satisfying our constraints, then ignore them
      //
      real tmp = convergence_factor;
      if ((tmp == real :: positive_infinity) && (constr_violation == 0.0))
         return fval;
      return fval + constraint_penalty * convergence_factor * constr_violation;
   }

   /// Compute an augmented objective function and constraint violation.
   template <class ResponseT, class ArrayT>
   void compute_response_info(ResponseT& response,
                              ArrayT& clower, ArrayT& cupper,
                              double convergence_factor)
   {
      constraint_violation(response.constraint_values(),
                           clower, cupper,
                           response.l2_constraint_violation());
      response.augmented_function_value() = compute_penalty_function(
                                               response.function_value(),
                                               response.l2_constraint_violation(),
                                               convergence_factor);
   }

   /// Compute an augmented objective function and constraint violation.
   template <class ResponseT, class ArrayT>
   void compute_response_info(ResponseT& response,
                              ArrayT& clower, ArrayT& cupper,
                              double convergence_factor,
                              real& value,
                              real& cvalue)
   {
      compute_response_info(response, clower, cupper, convergence_factor);
      value = response.augmented_function_value();
      cvalue = response.l2_constraint_violation();
   }

   /// Compute an augmented objective function and constraint violation.
   template <class ResponseT, class ArrayT>
   void compute_response_info(ResponseT& response,
                              ArrayT& clower,
                              ArrayT& cupper,
                              real& value,
                              real& constr_violation)
   {
      compute_response_info(response, clower, cupper, 1.0, value,
                            constr_violation);
   }

   /// The constraint penalty coefficient.
   double constraint_penalty;

   /// If true, then ignore the convergence factor.
   bool ignore_convergence_factor;

   /// Initialize this object.
   void initialize(unsigned int numNonlinearIneqConstraints_, double constraint_tolerance_)
   {
      _numNonlinearIneqConstraints = numNonlinearIneqConstraints_;
      _constraint_tolerance = constraint_tolerance_;
   }

private:

   /// The number of nonlinear inequality constraints.
   unsigned int _numNonlinearIneqConstraints;

   /// The tolerance for infeasibility for constraints.
   double _constraint_tolerance;

};
#endif



} // namespace colin

#endif

