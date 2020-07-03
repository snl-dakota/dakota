/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020 National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef C3_APPROXIMATION_H
#define C3_APPROXIMATION_H

#include "DakotaApproximation.hpp"
#include "DakotaVariables.hpp"

// NOTE: Do not include C3 headers here to maintain isolation from
// Dakota::Iterator header chain

// forward declares from dakota_c3_include.hpp
struct FunctionTrain;
struct FT1DArray;
struct FT1DArray;
struct C3SobolSensitivity;
struct MultiApproxOpts;

namespace Pecos {
class SurrogateData;
}
    
namespace Dakota {
    
// fwd declares previously defined in this header now in dakota_c3_include.hpp
struct FTDerivedFunctions;
// now in separate implementation file
class C3FnTrainPtrsRep;

// BMA: Perhaps make this a typedef, avoiding class with all the forwards:
//typedef std::shared_ptr<C3FnTrainPtrsRep> C3FnTrainPtrs;
// BMA: (Would change calling code from . to ->)

/// Handle for reference-counted pointer to C3FnTrainPtrsRep body
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

  /// perform a deep copy (copy ctor and operator= use shallow copies)
  C3FnTrainPtrs copy() const;
  /// swap ftpReps between two envelopes
  void swap(C3FnTrainPtrs& ftp);

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

  //
  //- Heading: Data
  //

  /// (shared) pointer to body instance
  std::shared_ptr<C3FnTrainPtrsRep> ftpRep;
};




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

  /// return the active C3FnTrainPtrs instance in levelApprox
  C3FnTrainPtrs& active_ftp();
  /// return combinedC3FTPtrs
  C3FnTrainPtrs& combined_ftp();

  // *** IMPORTANT NOTE: these regression_size() implementations utilize after-
  // *** build per-QoI details (subject to CV adapt_{rank,order}), not shared
  // *** before-build input specification (subject to increment/decrement)

  size_t regression_size(); // uses active ftp
  size_t regression_size(const SizetVector& ranks,  size_t max_rank,
			 const UShortArray& orders, unsigned short max_order);

  void recover_function_train_ranks(SizetVector& ft_ranks);
  void recover_function_train_orders(UShortArray& ft_orders);

  //size_t average_rank();
  //size_t maximum_rank();

  void expansion_coefficient_flag(bool coeff_flag);
  bool expansion_coefficient_flag() const;
  void expansion_gradient_flag(bool grad_flag);
  bool expansion_gradient_flag() const;

  //size_t moment_size() const;

  void compute_moments(bool full_stats = true, bool combined_stats = false);
  void compute_moments(const Pecos::RealVector& x, bool full_stats = true,
		       bool combined_stats = false);

  const RealVector& moments() const;
  const RealVector& expansion_moments() const;
  const RealVector& numerical_integration_moments() const;
  const RealVector& combined_moments() const;

  Real moment(size_t i) const;
  void moment(Real mom, size_t i);
  Real combined_moment(size_t i) const;
  void combined_moment(Real mom, size_t i);

  void compute_component_effects();
  void compute_total_effects();

  void compute_all_sobol_indices(size_t); // computes total and interactions
  Real total_sobol_index(size_t);         // returns total sobol index
  Real main_sobol_index(size_t);          // returns main sobol index
  // iterate over sobol indices and apply a function
  void sobol_iterate_apply(void (*)(double, size_t, size_t*,void*), void*); 
    
  Real mean();                   // expectation with respect to all variables
  Real mean(const RealVector &); // expectation with respect to random vars
  const RealVector& mean_gradient();      // NOT SURE
  const RealVector& mean_gradient(const RealVector &, const SizetArray &); 
    
  Real variance();
  Real variance(const RealVector &);
  const RealVector& variance_gradient();      // NOT SURE
  const RealVector& variance_gradient(const RealVector &, const SizetArray &); 
  Real covariance(Approximation& approx_2);
  Real covariance(const RealVector& x, Approximation& approx_2);

  Real skewness();
  Real kurtosis();
  Real third_central();
  Real fourth_central();

  Real combined_mean();
  Real combined_mean(const RealVector &);
  Real combined_variance();
  Real combined_variance(const RealVector &);
  Real combined_covariance(Approximation& approx_2);
  Real combined_covariance(const RealVector& x, Approximation& approx_2);
  Real combined_third_central();
  Real combined_fourth_central();

  /// update surrData to define aggregated data from raw data, when indicated
  /// by an active aggregated key
  void synchronize_surrogate_data();
  /// generate synthetic data for the surrogate QoI prediction corresponding
  /// to the level key preceding active key; for use in surplus estimation
  /// for new level data relative to a previous level's surrogate prediction
  void generate_synthetic_data(Pecos::SurrogateData& surr_data,
			       const UShortArray& active_key,
			       short combine_type);

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

  Real stored_value(const RealVector& c_vars, const UShortArray& key);

  void compute_derived_statistics(C3FnTrainPtrs& ftp, size_t num_mom,
				  bool overwrite = false);
  void compute_derived_statistics_av(C3FnTrainPtrs& ftp, size_t num_mom,
				     bool overwrite = false);

  /// differentiate the ft to form its gradient, if not previously performed
  void check_function_gradient();
  /// differentiate the ftg to form the ft Hessian, if not previously performed
  void check_function_hessian();

  /// compute mean corresponding to the passed FT expansion
  Real mean(C3FnTrainPtrs& ftp);
  /// compute mean corresponding to the passed FT expansion
  Real mean(const RealVector &x, C3FnTrainPtrs& ftp);
  /// compute variance corresponding to the passed FT expansion
  Real variance(C3FnTrainPtrs& ftp);
  /// compute variance corresponding to the passed FT expansion
  Real variance(const RealVector &x, C3FnTrainPtrs& ftp);
  /// compute variance corresponding to the passed FT expansion
  Real covariance(C3FnTrainPtrs& ftp1, C3FnTrainPtrs& ftp2);
  /// compute variance corresponding to the passed FT expansion
  Real covariance(const RealVector &x, C3FnTrainPtrs& ftp1,C3FnTrainPtrs& ftp2);
  /// compute 3rd central moment corresponding to the passed FT expansion
  Real third_central(C3FnTrainPtrs& ftp);
  /// compute 4th central moment corresponding to the passed FT expansion
  Real fourth_central(C3FnTrainPtrs& ftp);
  /// compute skewness corresponding to the passed FT expansion
  Real skewness(C3FnTrainPtrs& ftp);
  /// compute excess kurtosis corresponding to the passed FT expansion
  Real kurtosis(C3FnTrainPtrs& ftp);

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

  /// flag indicating need to build a fn train approximation for this QoI
  bool expansionCoeffFlag;
  /// flag indicating need to build a fn train gradient approx for this QoI
  bool expansionCoeffGradFlag;
    
  /// mean and central moments 2/3/4 computed from either the expansion form
  std::map<UShortArray, RealVector> primaryMoments;
  /// iterator to active entry in primaryMoments
  std::map<UShortArray, RealVector>::iterator primaryMomIter;
  /// secondary (numerical) moments: inactive
  RealVector secondaryMoments;
  /// combined moments from multilevel-multifidelity FT rollup
  RealVector combinedMoments;
};


inline void C3Approximation::active_model_key(const UShortArray& key)
{
  // sets approxData keys
  // Note: this may be required even if levApproxIter->first is consistent
  // with incoming key due to enumeration of multiple approx data sets by
  // ApproximationInterface::*_add()
  Approximation::active_model_key(key);

  // Test for change
  if (levApproxIter != levelApprox.end() && levApproxIter->first == key)
    return;

  levApproxIter = levelApprox.find(key);
  if (levApproxIter == levelApprox.end()) {
    // Note: C3FT pointers not allocated until build()
    std::pair<UShortArray, C3FnTrainPtrs> ftp_pair(key, C3FnTrainPtrs());
    levApproxIter = levelApprox.insert(ftp_pair).first;
  }

  primaryMomIter = primaryMoments.find(key);
  if (primaryMomIter == primaryMoments.end()) {
    std::pair<UShortArray, RealVector> rv_pair(key, RealVector());
    primaryMomIter = primaryMoments.insert(rv_pair).first;
  }
}


inline void C3Approximation::clear_model_keys()
{
  // clears approxData keys
  Approximation::clear_model_keys();

  levelApprox.clear();    levApproxIter  = levelApprox.end();
  primaryMoments.clear(); primaryMomIter = primaryMoments.end();
}


inline C3FnTrainPtrs& C3Approximation::active_ftp()
{ return levApproxIter->second; }


inline C3FnTrainPtrs& C3Approximation::combined_ftp()
{ return combinedC3FTPtrs; }


inline void C3Approximation::expansion_coefficient_flag(bool coeff_flag)
{ expansionCoeffFlag = coeff_flag; }


inline bool C3Approximation::expansion_coefficient_flag() const
{ return expansionCoeffFlag; }


inline void C3Approximation::expansion_gradient_flag(bool grad_flag)
{ expansionCoeffGradFlag = grad_flag; }


inline bool C3Approximation::expansion_gradient_flag() const
{ return expansionCoeffGradFlag; }


//inline size_t C3Approximation::moment_size() const
//{
//  return (data_rep->refineStatsType == Pecos::COMBINED_EXPANSION_STATS) ?
//    combinedMoments.length() : primaryMomIter->second.length();
//}


inline const RealVector& C3Approximation::moments() const
{ return primaryMomIter->second; }


inline const RealVector& C3Approximation::expansion_moments() const
{ return primaryMomIter->second; }


inline const RealVector& C3Approximation::numerical_integration_moments() const
{ return secondaryMoments; } // empty


inline const RealVector& C3Approximation::combined_moments() const
{ return combinedMoments; }


inline Real C3Approximation::moment(size_t i) const
{ return primaryMomIter->second[i]; }


inline void C3Approximation::moment(Real mom, size_t i)
{ primaryMomIter->second[i] = mom; }


inline Real C3Approximation::combined_moment(size_t i) const
{ return combinedMoments[i]; }


inline void C3Approximation::combined_moment(Real mom, size_t i)
{ combinedMoments[i] = mom; }


inline Real C3Approximation::mean()
{ return mean(levApproxIter->second); }


inline Real C3Approximation::mean(const RealVector &x)
{ return mean(x, levApproxIter->second); }


inline Real C3Approximation::variance()
{ return variance(levApproxIter->second); }


inline Real C3Approximation::variance(const RealVector &x)
{ return variance(x, levApproxIter->second); }


inline Real C3Approximation::combined_mean()
{ return mean(combinedC3FTPtrs); }


inline Real C3Approximation::combined_mean(const RealVector &x)
{ return mean(x, combinedC3FTPtrs); }


inline Real C3Approximation::combined_variance()
{ return variance(combinedC3FTPtrs); }


inline Real C3Approximation::combined_variance(const RealVector &x)
{ return variance(x, combinedC3FTPtrs); }


inline Real C3Approximation::covariance(Approximation& approx_2)
{
  C3Approximation* c3_approx_rep_2 = (C3Approximation*)approx_2.approx_rep();
  return covariance(levApproxIter->second, c3_approx_rep_2->active_ftp());
}


inline Real C3Approximation::combined_covariance(Approximation& approx_2)
{
  C3Approximation* c3_approx_rep_2 = (C3Approximation*)approx_2.approx_rep();
  return covariance(combinedC3FTPtrs, c3_approx_rep_2->combined_ftp());
}


inline Real C3Approximation::
covariance(const RealVector &x, Approximation& approx_2)
{
  C3Approximation* c3_approx_rep_2 = (C3Approximation*)approx_2.approx_rep();
  return covariance(x, levApproxIter->second, c3_approx_rep_2->active_ftp());
}


inline Real C3Approximation::
combined_covariance(const RealVector &x, Approximation& approx_2)
{
  C3Approximation* c3_approx_rep_2 = (C3Approximation*)approx_2.approx_rep();
  return covariance(x, combinedC3FTPtrs, c3_approx_rep_2->combined_ftp());
}


inline Real C3Approximation::skewness()
{ return skewness(levApproxIter->second); }


inline Real C3Approximation::kurtosis()
{ return kurtosis(levApproxIter->second); }


inline Real C3Approximation::third_central()
{ return third_central(levApproxIter->second); }


inline Real C3Approximation::fourth_central()
{ return fourth_central(levApproxIter->second); }


inline Real C3Approximation::combined_third_central()
{ return third_central(combinedC3FTPtrs); }


inline Real C3Approximation::combined_fourth_central()
{ return fourth_central(combinedC3FTPtrs); }


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


} // end namespace

#endif
