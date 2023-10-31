/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef C3_FN_TRAIN_DATA_REP_H
#define C3_FN_TRAIN_DATA_REP_H

#include "dakota_c3_include.hpp"
#include "dakota_data_types.hpp"
//#include <vector>

namespace Dakota {

// fwd declares previously defined in this header now in dakota_c3_include.hpp
struct FTDerivedFunctions;

// BMA: Perhaps make this a typedef, avoiding class with all the forwards:
//typedef std::shared_ptr<C3FnTrainDataRep> C3FnTrainData;
// BMA: (Would change calling code from . to ->)


// C3FnTrainData already resembles a handle managing pointers.  However, the
// ref count has to be shared/managed along with the shared representation(s).
// So resorting to the standard handle-body approach using a C3FnTrainDataRep.

class C3FnTrainDataRep
{
public:

  //
  //- Heading: Constructor and destructor
  //

  C3FnTrainDataRep();  ///< default constructor
  ~C3FnTrainDataRep(); ///< destructor

  //
  //- Heading: Member functions
  //

  //void copy(const C3FnTrainDataRep& ptrs);

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

  // when training data is scaled to [0,1], the resulting ft is unscaled
  // using ft * scalingMultiplier + scalingOffset
  //Real scalingMultiplier;
  // when training data is scaled to [0,1], the resulting ft is unscaled
  // using ft * scalingMultiplier + scalingOffset
  //Real scalingOffset;

  /// moment vector retrieved from ft_derived_fns
  RealVector primaryMoments;
  /// set of polynomial basis orders recovered following order selection
  /// based on cross validation; these values precede any scaling or other
  /// post-processing of the regression result (which may be reflected in the
  /// stored ft), keeping them consistent with {start,kick,max} order controls
  UShortArray recoveredOrders;
  /// set of expansion ranks recovered following rank selection based on cross
  /// validation; these values precede any scaling or other post-processing
  /// of the regression result (which may be reflected in the stored ft),
  /// keeping them consistent with {start,kick,max} rank controls
  SizetVector recoveredRanks;
};


inline C3FnTrainDataRep::C3FnTrainDataRep():
  ft(NULL), ft_gradient(NULL), ft_hessian(NULL), ft_sobol(NULL)
{ ft_derived_functions_init_null(); }


inline struct FunctionTrain * C3FnTrainDataRep::
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


inline void C3FnTrainDataRep::free_ft()
{
  if (ft)          { function_train_free(ft);      ft          = NULL; }
  if (ft_gradient) { ft1d_array_free(ft_gradient); ft_gradient = NULL; }
  if (ft_hessian)  { ft1d_array_free(ft_hessian);  ft_hessian  = NULL; }
}


inline void C3FnTrainDataRep::free_all()
{
  free_ft();
  if (ft_derived_fns.allocated)  ft_derived_functions_free();
  if (ft_sobol)
    { c3_sobol_sensitivity_free(ft_sobol); ft_sobol = NULL; }
}


inline C3FnTrainDataRep::~C3FnTrainDataRep()
{ free_all(); }


////////////////////////////////////////////////////////////////////////////////


/// Handle for reference-counted pointer to C3FnTrainDataRep body

class C3FnTrainData
{
public:

  //
  //- Heading: Constructor and destructor
  //

  C3FnTrainData();                         ///< default constructor
  C3FnTrainData(const C3FnTrainData& ftd); ///< copy constructor
  ~C3FnTrainData();                        ///< destructor

  /// assignment operator
  C3FnTrainData& operator=(const C3FnTrainData& ftd);

  //
  //- Heading: Member functions
  //

  /// perform a deep copy (copy ctor and operator= use shallow copies)
  C3FnTrainData copy() const;
  /// swap ftdReps between two envelopes
  void swap(C3FnTrainData& ftd);

  /// free FT storage for value, gradient, and Hessian expansions
  void free_ft();
  /// augment free_ft() with derived functions and global sensitivities
  void free_all();

  // Manage stats (FTDerivedFunctions) computed from FT approximation

  /// initialize derived funtions pointers to NULL
  void ft_derived_functions_init_null();
  /// allocate derived funtions pointers (standard mode)
  void ft_derived_functions_create(struct MultiApproxOpts* opts,
				   size_t num_mom, Real round_tol);
  /// allocate derived funtions pointers (all variables mode)
  void ft_derived_functions_create_av(struct MultiApproxOpts* opts,
				      const SizetArray& rand_indices,
				      Real round_tol);
  /// deallocate derived funtions pointers
  void ft_derived_functions_free();

  /// get pointer to the FunctionTrain approximation
  struct FunctionTrain * function_train();
  /// set pointer to the FunctionTrain approximation
  void function_train(struct FunctionTrain * ft);

  /// get pointer to the FunctionTrain gradient
  struct FT1DArray * ft_gradient();
  /// set pointer to the FunctionTrain gradient
  void ft_gradient(struct FT1DArray * ftg);

  /// get pointer to the FunctionTrain Hessian
  struct FT1DArray * ft_hessian();
  /// set pointer to the FunctionTrain Hessian
  void ft_hessian(struct FT1DArray * fth);

  /// return reference to the FTDerivedFunctions instance
  const struct FTDerivedFunctions& derived_functions();

  /// get pointer to the Sobol' indices object
  struct C3SobolSensitivity * sobol();
  /// set pointer to the Sobol' indices object
  void sobol(struct C3SobolSensitivity * ss);

  const UShortArray& recovered_orders() const;
  UShortArray& recovered_orders();
  void recovered_orders(const UShortArray& ft_orders);

  const SizetVector& recovered_ranks() const;
  SizetVector& recovered_ranks();
  void recovered_ranks(const SizetVector& ft_ranks);

  const RealVector& moments() const;
  RealVector& moments();
  Real moment(size_t i) const;
  void moment(Real mom, size_t i);

  //
  //- Heading: Data
  //

  /// (shared) pointer to body instance
  std::shared_ptr<C3FnTrainDataRep> ftdRep;
};


inline void C3FnTrainData::swap(C3FnTrainData& ftd)
{ ftdRep.swap(ftd.ftdRep); }

// Note: the following functions init/create/free ft memory within an ftdRep
//       but do not alter ftdRep accounting

inline void C3FnTrainData::free_ft()
{ ftdRep->free_ft(); }


inline void C3FnTrainData::free_all()
{ ftdRep->free_all(); }


inline void C3FnTrainData::ft_derived_functions_init_null()
{ ftdRep->ft_derived_functions_init_null(); }


inline void C3FnTrainData::
ft_derived_functions_create(struct MultiApproxOpts * ma_opts, size_t num_mom,
			    Real round_tol)
{ ftdRep->ft_derived_functions_create(ma_opts, num_mom, round_tol); }


inline void C3FnTrainData::
ft_derived_functions_create_av(struct MultiApproxOpts * ma_opts,
			       const SizetArray& rand_indices, Real round_tol)
{ ftdRep->ft_derived_functions_create_av(ma_opts, rand_indices, round_tol); }


inline void C3FnTrainData::ft_derived_functions_free()
{ ftdRep->ft_derived_functions_free(); }


inline struct FunctionTrain * C3FnTrainData::function_train()
{ return ftdRep->ft; }


inline void C3FnTrainData::function_train(struct FunctionTrain * ft)
{ ftdRep->ft = ft; }


inline struct FT1DArray * C3FnTrainData::ft_gradient()
{ return ftdRep->ft_gradient; }


inline void C3FnTrainData::ft_gradient(struct FT1DArray * ftg)
{ ftdRep->ft_gradient = ftg; }


inline struct FT1DArray * C3FnTrainData::ft_hessian()
{ return ftdRep->ft_hessian; }


inline void C3FnTrainData::ft_hessian(struct FT1DArray * fth)
{ ftdRep->ft_hessian = fth; }


inline const struct FTDerivedFunctions& C3FnTrainData::derived_functions()
{ return ftdRep->ft_derived_fns; }


inline struct C3SobolSensitivity * C3FnTrainData::sobol()
{ return ftdRep->ft_sobol; }


inline void C3FnTrainData::sobol(struct C3SobolSensitivity * ss)
{ ftdRep->ft_sobol = ss; }


inline const UShortArray& C3FnTrainData::recovered_orders() const
{ return ftdRep->recoveredOrders; }


inline UShortArray& C3FnTrainData::recovered_orders()
{ return ftdRep->recoveredOrders; }


inline void C3FnTrainData::recovered_orders(const UShortArray& ft_orders)
{ ftdRep->recoveredOrders = ft_orders; }


inline const SizetVector& C3FnTrainData::recovered_ranks() const
{ return ftdRep->recoveredRanks; }


inline SizetVector& C3FnTrainData::recovered_ranks()
{ return ftdRep->recoveredRanks; }


inline void C3FnTrainData::recovered_ranks(const SizetVector& ft_ranks)
{ ftdRep->recoveredRanks = ft_ranks; }


inline const RealVector& C3FnTrainData::moments() const
{ return ftdRep->primaryMoments; }


inline RealVector& C3FnTrainData::moments()
{ return ftdRep->primaryMoments; }


inline Real C3FnTrainData::moment(size_t i) const
{ return ftdRep->primaryMoments[i]; }


inline void C3FnTrainData::moment(Real mom, size_t i)
{ ftdRep->primaryMoments[i] = mom; }

} // namespace

#endif
