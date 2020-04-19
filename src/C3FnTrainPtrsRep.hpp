/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef C3_FNTRAIN_PTRSREP_H
#define C3_FNTRAIN_PTRSREP_H

#include "dakota_c3_include.hpp"
#include <vector>

namespace Dakota {


// C3FnTrainPtrs already resembles a handle managing pointers.  However, the
// ref count has to be shared/managed along with the shared representation(s).
// So resorting to the standard handle-body approach and adding a
// C3FnTrainPtrsRep class. (Note: could also consider C++ smart pointer.)

class C3FnTrainPtrsRep
{
public:

  //
  //- Heading: Constructor and destructor
  //

  C3FnTrainPtrsRep();  ///< default constructor
  ~C3FnTrainPtrsRep(); ///< destructor

  //
  //- Heading: Member functions
  //

  //void copy(const C3FnTrainPtrsRep& ptrs);

  static struct FunctionTrain *
    subtract_const(struct FunctionTrain * ft, double val,
		   struct MultiApproxOpts * opts);

  void free_ft();
  void free_all();

  // Manage stats (FTDerivedFunctions) computed from FT approximation
  void ft_derived_functions_init_null();
  void ft_derived_functions_create(struct MultiApproxOpts * opts,
				   size_t num_mom, double eps);
  void ft_derived_functions_create_av(struct MultiApproxOpts * opts,
				      const std::vector<size_t>& rand_indices,
				      double eps);
  void ft_derived_functions_free();

  //
  //- Heading: Data
  //

  struct FunctionTrain * ft;
  struct FT1DArray * ft_gradient;
  struct FT1DArray * ft_hessian;

  // allocated downstream in compute_derived_statistics() for storing stats
  struct FTDerivedFunctions ft_derived_fns;
  // allocated downstream in compute_all_sobol_indices() for storing indices
  struct C3SobolSensitivity * ft_sobol;
};


inline C3FnTrainPtrsRep::C3FnTrainPtrsRep():
  ft(NULL), ft_gradient(NULL), ft_hessian(NULL), ft_sobol(NULL)
{ ft_derived_functions_init_null(); }


inline struct FunctionTrain * C3FnTrainPtrsRep::
subtract_const(struct FunctionTrain * ft, double val,
	       struct MultiApproxOpts * opts)
{
  // two new FT are allocated, one is immediately deallocated, one is returned
  // (which must be eventually deallocated by client)

  struct FunctionTrain * ft_const   = function_train_constant(-val, opts);
  struct FunctionTrain * ft_updated = function_train_sum(ft, ft_const);
  function_train_free(ft_const); //ft_const = NULL;
  return ft_updated;
}


inline void C3FnTrainPtrsRep::free_ft()
{
  if (ft)          { function_train_free(ft);      ft          = NULL; }
  if (ft_gradient) { ft1d_array_free(ft_gradient); ft_gradient = NULL; }
  if (ft_hessian)  { ft1d_array_free(ft_hessian);  ft_hessian  = NULL; }
}


inline void C3FnTrainPtrsRep::free_all()
{
  free_ft();
  if (ft_derived_fns.allocated)  ft_derived_functions_free();
  if (ft_sobol)
    { c3_sobol_sensitivity_free(ft_sobol); ft_sobol = NULL; }
}


inline C3FnTrainPtrsRep::~C3FnTrainPtrsRep()
{ free_all(); }


} // namespace

#endif
