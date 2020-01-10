/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef C3_APPROXIMATION_H
#define C3_APPROXIMATION_H

#include "DakotaApproximation.hpp"
#include "DakotaVariables.hpp"
#include "dakota_c3_include.hpp"

namespace Dakota {
    
struct FTDerivedFunctions
{
  int set;
    
  struct FunctionTrain * ft_squared;
  struct FunctionTrain * ft_cubed;
  struct FunctionTrain * ft_constant_at_mean;
  struct FunctionTrain * ft_diff_from_mean;
  struct FunctionTrain * ft_diff_from_mean_squared;
  struct FunctionTrain * ft_diff_from_mean_cubed;    
  struct FunctionTrain * ft_diff_from_mean_tesseracted;// courtesy of dan 
  struct FunctionTrain * ft_diff_from_mean_normalized;
  struct FunctionTrain * ft_diff_from_mean_normalized_squared;
  struct FunctionTrain * ft_diff_from_mean_normalized_cubed;

  // raw moments
  double first_moment;
  double second_moment;
  double third_moment;

  // central moments
  double second_central_moment;
  double third_central_moment;
  double fourth_central_moment;

  // standardized moments
  double std_dev;
  double skewness;
  double kurtosis;
};


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

  void free_ft();
  void free_all();

  // Manage stats (FTDerivedFunctions) computed from approx (FunctionTrain):
  void ft_derived_functions_init_null();
  // pass in sharedC3DataRep->approxOpts
  void ft_derived_functions_create(struct MultiApproxOpts * opts);
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

  int referenceCount; ///< number of handle objects sharing pointers
};


inline C3FnTrainPtrsRep::C3FnTrainPtrsRep():
  ft(NULL), ft_gradient(NULL), ft_hessian(NULL), ft_sobol(NULL),
  referenceCount(1)
{ ft_derived_functions_init_null(); }


inline void C3FnTrainPtrsRep::free_ft()
{
  if (ft)          { function_train_free(ft);      ft          = NULL; }
  if (ft_gradient) { ft1d_array_free(ft_gradient); ft_gradient = NULL; }
  if (ft_hessian)  { ft1d_array_free(ft_hessian);  ft_hessian  = NULL; }
}


inline void C3FnTrainPtrsRep::free_all()
{
  free_ft();
  ft_derived_functions_free();
  if (ft_sobol)
    { c3_sobol_sensitivity_free(ft_sobol); ft_sobol = NULL; }
}


inline C3FnTrainPtrsRep::~C3FnTrainPtrsRep()
{ free_all(); }


class C3FnTrainPtrs
{
public:

  //
  //- Heading: Constructor and destructor
  //

  C3FnTrainPtrs();                         ///< default constructor
  C3FnTrainPtrs(const C3FnTrainPtrs& ftp); ///< copy constructor
  ~C3FnTrainPtrs();                        ///< destructor

  /// assignment operator
  C3FnTrainPtrs& operator=(const C3FnTrainPtrs& ftp);

  //
  //- Heading: Member functions
  //

  /// perform a deep copy (unlike copy ctor and operator=, which are shallow)
  C3FnTrainPtrs copy() const;
  /// swap ftpReps between two envelopes
  void swap(C3FnTrainPtrs& ftp);

  void free_ft();
  void free_all();

  // Manage stats (FTDerivedFunctions) computed from approx (FunctionTrain):
  void derived_functions_init_null();
  // pass in sharedC3DataRep->approxOpts
  void derived_functions_create(struct MultiApproxOpts* opts);
  void derived_functions_free();

  struct FunctionTrain * function_train();
  void function_train(struct FunctionTrain * ft);

  struct FT1DArray *     ft_gradient();
  void ft_gradient(struct FT1DArray * ftg);

  struct FT1DArray *     ft_hessian();
  void ft_hessian(struct FT1DArray * fth);

  const struct FTDerivedFunctions& derived_functions();

  struct C3SobolSensitivity * sobol();
  void sobol(struct C3SobolSensitivity * ss);

  //
  //- Heading: Data
  //

  C3FnTrainPtrsRep* ftpRep; ///< number of handle objects sharing pointers
};


inline C3FnTrainPtrs::C3FnTrainPtrs(): ftpRep(new C3FnTrainPtrsRep())
{ } // body allocated with null FT pointers


inline C3FnTrainPtrs C3FnTrainPtrs::copy() const
{
  C3FnTrainPtrs ftp; // new envelope with ftpRep default allocated

  ftp.ftpRep->ft          = function_train_copy(ftpRep->ft);
  ftp.ftpRep->ft_gradient = ft1d_array_copy(ftpRep->ft_gradient);
  ftp.ftpRep->ft_hessian  = ft1d_array_copy(ftpRep->ft_hessian);

  // ft_derived_fns,ft_sobol have been assigned NULL and can be allocated
  // downsteam when needed for stats,indices

  return ftp;
}


inline void C3FnTrainPtrs::swap(C3FnTrainPtrs& ftp)
{
  // reference counts for each ftpRep are unmodified by swap()
  C3FnTrainPtrsRep* save_rep = ftpRep;
  ftpRep                     = ftp.ftpRep;
  ftp.ftpRep                 = save_rep;
}


// TO DO: shallow copy would be better for this case, but requires ref counting
inline C3FnTrainPtrs::C3FnTrainPtrs(const C3FnTrainPtrs& ftp)
{
  // Increment new (no old to decrement)
  ftpRep = ftp.ftpRep;
  if (ftpRep) // Check for an assignment of NULL
    ++ftpRep->referenceCount;
}


inline C3FnTrainPtrs::~C3FnTrainPtrs()
{
  if (ftpRep) { // Check for NULL
    --ftpRep->referenceCount; // decrement
    if (ftpRep->referenceCount == 0)
      delete ftpRep;
  }
}


inline C3FnTrainPtrs& C3FnTrainPtrs::operator=(const C3FnTrainPtrs& ftp)
{
  if (ftpRep != ftp.ftpRep) { // prevent re-assignment of same rep
    // Decrement old
    if (ftpRep) // Check for NULL
      if ( --ftpRep->referenceCount == 0 ) 
	delete ftpRep;
    // Increment new
    ftpRep = ftp.ftpRep;
    if (ftpRep) // Check for an assignment of NULL
      ++ftpRep->referenceCount;
  }
  // else if assigning same rep, then leave referenceCount as is

  return *this;
}

// Note: the following functions init/create/free ft memory within an ftpRep
//       but do not alter ftpRep accounting

inline void C3FnTrainPtrs::free_ft()
{ ftpRep->free_ft(); }


inline void C3FnTrainPtrs::free_all()
{ ftpRep->free_all(); }


inline void C3FnTrainPtrs::derived_functions_init_null()
{ ftpRep->ft_derived_functions_init_null(); }


inline void C3FnTrainPtrs::
derived_functions_create(struct MultiApproxOpts * opts)
{ ftpRep->ft_derived_functions_create(opts); }


inline void C3FnTrainPtrs::derived_functions_free()
{ ftpRep->ft_derived_functions_free(); }


inline struct FunctionTrain * C3FnTrainPtrs::function_train()
{ return ftpRep->ft; }


inline void C3FnTrainPtrs::function_train(struct FunctionTrain * ft)
{ ftpRep->ft = ft; }


inline struct FT1DArray * C3FnTrainPtrs::ft_gradient()
{ return ftpRep->ft_gradient; }


inline void C3FnTrainPtrs::ft_gradient(struct FT1DArray * ftg)
{ ftpRep->ft_gradient = ftg; }


inline struct FT1DArray * C3FnTrainPtrs::ft_hessian()
{ return ftpRep->ft_hessian; }


inline void C3FnTrainPtrs::ft_hessian(struct FT1DArray * fth)
{ ftpRep->ft_hessian = fth; }


inline const struct FTDerivedFunctions& C3FnTrainPtrs::derived_functions()
{ return ftpRep->ft_derived_fns; }


inline struct C3SobolSensitivity * C3FnTrainPtrs::sobol()
{ return ftpRep->ft_sobol; }


inline void C3FnTrainPtrs::sobol(struct C3SobolSensitivity * ss)
{ ftpRep->ft_sobol = ss; }


class SharedC3ApproxData;  // forward declare


/// Derived approximation class for global basis polynomials.

/** The PecosApproximation class provides a global approximation
    based on basis polynomials.  This includes orthogonal polynomials
    used for polynomial chaos expansions and interpolation polynomials
    used for stochastic collocation. */

class C3Approximation: public Approximation
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// default constructor
  C3Approximation();
  /// standard ProblemDescDB-driven constructor
  C3Approximation(ProblemDescDB& problem_db,
		  const SharedApproxData& shared_data,
		  const String& approx_label);
  /// alternate constructor
  C3Approximation(const SharedApproxData& shared_data);
  ~C3Approximation(); // destructor

  //
  //- Heading: Member functions
  //

  size_t regression_size();
  size_t average_rank();
  size_t maximum_rank();

  /// I Dont know what the next 4 are for, but I will leave them in
  /// in case I ever find out!
    
  /// set pecosBasisApprox.configOptions.expansionCoeffFlag
  void expansion_coefficient_flag(bool coeff_flag);
  /// get pecosBasisApprox.configOptions.expansionCoeffFlag
  bool expansion_coefficient_flag() const;

  /// set pecosBasisApprox.configOptions.expansionGradFlag
  void expansion_gradient_flag(bool grad_flag);
  /// get pecosBasisApprox.configOptions.expansionGradFlag
  bool expansion_gradient_flag() const;

  void compute_moments(bool full_stats = true, bool combined_stats = false);
  void compute_moments(const Pecos::RealVector& x, bool full_stats = true,
		       bool combined_stats = false);
  const RealVector& moments() const;
  Real moment(size_t i) const;
  void moment(Real mom, size_t i);

  /// Performs global sensitivity analysis using Sobol' Indices by
  /// computing component (main and interaction) effects
  void compute_component_effects();
  /// Performs global sensitivity analysis using Sobol' Indices by
  /// computing total effects
  void compute_total_effects();

  void compute_all_sobol_indices(size_t); // computes total and interacting sobol indices
  Real total_sobol_index(size_t);         // returns total sobol index
  Real main_sobol_index(size_t);          // returns main sobol index
  // iterate over sobol indices and apply a function
  void sobol_iterate_apply(void (*)(double, size_t, size_t*,void*), void*); 
    
  Real mean();                            // expectation with respect to all variables
  Real mean(const RealVector &);          // expectation with respect to uncertain variables
  const RealVector& mean_gradient();      // NOT SURE
  // gradient with respect fixed variables
  const RealVector& mean_gradient(const RealVector &, const SizetArray &); 
    
  //     inline const Pecos::RealVector& PecosApproximation::
  // mean_gradient(const Pecos::RealVector& x, const Pecos::SizetArray& dvv)
  // { return polyApproxRep->mean_gradient(x, dvv); }

  Real variance();                        // variance with respect to all variables
  Real variance(const RealVector&);       // variance with respect to RV, others fixed
  const RealVector& variance_gradient();      // NOT SURE
  // gradient with respect fixed variables
  const RealVector& variance_gradient(const RealVector &, const SizetArray &); 

  Real covariance(Approximation& approx_2);                    // covariance between two functions
  Real covariance(const RealVector& x, Approximation& approx_2); // covariance with respect so subset

  Real skewness();
  Real kurtosis();
  Real third_central();
  Real fourth_central();

  const RealVector& expansion_moments() const;
  const RealVector& numerical_integration_moments() const;

  void synchronize_surrogate_data();
  //void response_data_to_surplus_data();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void active_model_key(const UShortArray& key);
  void clear_model_keys();

  //void link_multilevel_surrogate_data();

  Real                 value(const Variables& vars);
  const RealVector&    gradient(const Variables& vars);
  const RealSymMatrix& hessian(const Variables& vars);

  void build();
  void rebuild(); // build from scratch, but push C3 pointers to prev
  void pop_coefficients(bool save_data);
  void push_coefficients();
  //void finalize_coefficients();
  void combine_coefficients(); // use c3axpy
  void combined_to_active_coefficients(bool clear_combined = true);
  void clear_inactive_coefficients();

  int min_coefficients() const;

private:

  //
  //- Heading: Convenience member functions
  //

  void base_init();
    
  void compute_derived_statistics(bool overwrite);

  struct FunctionTrain * subtract_const(Real val);

  /// differentiate the ft to form its gradient, if not previously performed
  void check_function_gradient();
  /// differentiate the ftg to form the ft Hessian, if not previously performed
  void check_function_hessian();

  //
  //- Heading: Data
  //

  /// set of pointers to QoI approximation data for each model key
  std::map<UShortArray, C3FnTrainPtrs> levelApprox;
  /// iterator to active levelApprox
  std::map<UShortArray, C3FnTrainPtrs>::iterator levApproxIter;

  /// the previous approximation, cached for restoration
  C3FnTrainPtrs prevC3FTPtrs;
  /// bookkeeping for previously evaluated FT approximations that may
  /// be restored
  std::map<UShortArray, std::deque<C3FnTrainPtrs> > poppedLevelApprox;
  /// the combined approximation, summed across model keys
  C3FnTrainPtrs combinedC3FTPtrs;

  bool expansionCoeffFlag;     // build a fn train for the QoI
  bool expansionCoeffGradFlag; // build a fn train for the gradient of the QoI
    
  // containers allowing const ref return of latest result (active key)
  RealVector expansionMoments;
  RealVector numericalMoments;
};


inline void C3Approximation::active_model_key(const UShortArray& key)
{
  // Test for change
  if (levApproxIter != levelApprox.end() && levApproxIter->first == key)
    return;

  levApproxIter = levelApprox.find(key);
  if (levApproxIter == levelApprox.end()) {
    // Note: C3FT pointers not allocated until build()
    std::pair<UShortArray, C3FnTrainPtrs> ftp_pair(key, C3FnTrainPtrs());
    levApproxIter = levelApprox.insert(ftp_pair).first;
  }

  // sets approxData keys
  Approximation::active_model_key(key);
}


inline void C3Approximation::clear_model_keys()
{
  // clears approxData keys
  Approximation::clear_model_keys();

  levelApprox.clear();  levApproxIter = levelApprox.end();
}


/** this replaces the need to model data requirements as O(p r^2 d) */
inline size_t C3Approximation::regression_size()
{ return function_train_get_nparams(levApproxIter->second.function_train()); }


inline size_t C3Approximation::average_rank()
{ return function_train_get_avgrank(levApproxIter->second.function_train()); }


inline size_t C3Approximation::maximum_rank()
{ return function_train_get_maxrank(levApproxIter->second.function_train()); }


inline void C3Approximation::expansion_coefficient_flag(bool coeff_flag)
{ expansionCoeffFlag = coeff_flag; }


inline bool C3Approximation::expansion_coefficient_flag() const
{ return expansionCoeffFlag; }


inline void C3Approximation::expansion_gradient_flag(bool grad_flag)
{ expansionCoeffGradFlag = grad_flag; }


inline bool C3Approximation::expansion_gradient_flag() const
{ return expansionCoeffGradFlag; }


inline const RealVector& C3Approximation::moments() const
{ return expansionMoments; }


inline Real C3Approximation::moment(size_t i) const
{ return expansionMoments[i]; }


inline void C3Approximation::moment(Real mom, size_t i)
{ expansionMoments[i] = mom; }


inline const RealVector& C3Approximation::expansion_moments() const
{ return expansionMoments; } // populated


inline const RealVector& C3Approximation::numerical_integration_moments() const
{ return numericalMoments; } // empty


inline Real C3Approximation::third_central()
{
  compute_derived_statistics(false);
  return levApproxIter->second.derived_functions().third_central_moment;
}


inline Real C3Approximation::fourth_central()
{
  compute_derived_statistics(false);
  return levApproxIter->second.derived_functions().fourth_central_moment;
}


inline Real C3Approximation::skewness()
{
  compute_derived_statistics(false);
  return levApproxIter->second.derived_functions().skewness;
}


inline Real C3Approximation::kurtosis()
{
  compute_derived_statistics(false);
  return levApproxIter->second.derived_functions().kurtosis;
}


// Next two. Should access through compute_all_sobol_indices()
// Just need these two because NonDExpansion insists on computing
// all the analysis in one huge function (compute_analytic_statistcs)
// instead of smaller ones for different analysis.
// Need these two functions to reuse that code with no errors
inline void C3Approximation::compute_component_effects()
{ } // C3 does not distinguish so defer to fn below


inline void C3Approximation::compute_total_effects()
{
  // *** TO DO: mirror expConfigOptions.vbdOrderLimit in Pecos

  size_t interaction_order =//(vbdOrderLimit) ? vbdOrderLimit :
    sharedDataRep->numVars;
  compute_all_sobol_indices(interaction_order); 
}    


inline Real C3Approximation::main_sobol_index(size_t dim)
{ return c3_sobol_sensitivity_get_main(levApproxIter->second.sobol(),dim); }


inline Real C3Approximation::total_sobol_index(size_t dim)
{ return c3_sobol_sensitivity_get_total(levApproxIter->second.sobol(),dim); }


inline void C3Approximation::
sobol_iterate_apply(void (*f)(double val, size_t ninteract,
			      size_t*interactions,void* arg), void* args)
{ c3_sobol_sensitivity_apply_external(levApproxIter->second.sobol(),f,args); }

} // end namespace

#endif
