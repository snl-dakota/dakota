/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef C3_APPROXIMATION_H
#define C3_APPROXIMATION_H

#include "DakotaApproximation.hpp"
#include "DakotaVariables.hpp"
#include "C3FnTrainData.hpp"

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

  /// return the active C3FnTrainData instance in levelApprox
  C3FnTrainData& active_ftd();
  /// return combinedC3FTData
  C3FnTrainData& combined_ftd();

  // *** IMPORTANT NOTE: these regression_size() implementations utilize after-
  // *** build per-QoI details (subject to CV adapt_{rank,order}), not shared
  // *** before-build input specification (subject to increment/decrement)

  size_t regression_size(); // uses active ftd
  size_t regression_size(const SizetVector& ranks,  size_t max_rank,
			 const UShortArray& orders, unsigned short max_order);

  void recover_function_train_ranks(struct FunctionTrain * ft);
  void recover_function_train_orders(const std::vector<OneApproxOpts*>& a_opts);

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
			       const Pecos::ActiveKey& active_key,
			       short combine_type);

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void active_model_key(const Pecos::ActiveKey& key);
  void clear_model_keys();

  //void link_multilevel_surrogate_data();

  Real                 value(const Variables& vars);
  const RealVector&    gradient(const Variables& vars);
  const RealSymMatrix& hessian(const Variables& vars);

  bool advancement_available();

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

  bool max_rank_advancement_available();
  bool max_order_advancement_available();

  Real stored_value(const RealVector& c_vars, const Pecos::ActiveKey& key);

  void compute_derived_statistics(C3FnTrainData& ftd, size_t num_mom,
				  bool overwrite = false);
  void compute_derived_statistics_av(C3FnTrainData& ftd, size_t num_mom,
				     bool overwrite = false);

  /// differentiate the ft to form its gradient, if not previously performed
  void check_function_gradient();
  /// differentiate the ftg to form the ft Hessian, if not previously performed
  void check_function_hessian();

  /// compute mean corresponding to the passed FT expansion
  Real mean(C3FnTrainData& ftd);
  /// compute mean corresponding to the passed FT expansion
  Real mean(const RealVector &x, C3FnTrainData& ftd);
  /// compute variance corresponding to the passed FT expansion
  Real variance(C3FnTrainData& ftd);
  /// compute variance corresponding to the passed FT expansion
  Real variance(const RealVector &x, C3FnTrainData& ftd);
  /// compute variance corresponding to the passed FT expansion
  Real covariance(C3FnTrainData& ftd1, C3FnTrainData& ftd2);
  /// compute variance corresponding to the passed FT expansion
  Real covariance(const RealVector &x, C3FnTrainData& ftd1,C3FnTrainData& ftd2);
  /// compute 3rd central moment corresponding to the passed FT expansion
  Real third_central(C3FnTrainData& ftd);
  /// compute 4th central moment corresponding to the passed FT expansion
  Real fourth_central(C3FnTrainData& ftd);
  /// compute skewness corresponding to the passed FT expansion
  Real skewness(C3FnTrainData& ftd);
  /// compute excess kurtosis corresponding to the passed FT expansion
  Real kurtosis(C3FnTrainData& ftd);

  //
  //- Heading: Data
  //

  /// set of pointers to QoI approximation data for each model key
  std::map<Pecos::ActiveKey, C3FnTrainData> levelApprox;
  /// iterator to active levelApprox
  std::map<Pecos::ActiveKey, C3FnTrainData>::iterator levApproxIter;

  /// the previous approximation, cached for restoration
  C3FnTrainData prevC3FTData;
  /// bookkeeping for previously evaluated FT approximations that may
  /// be restored
  std::map<Pecos::ActiveKey, std::deque<C3FnTrainData> > poppedLevelApprox;
  /// the combined approximation, summed across model keys
  C3FnTrainData combinedC3FTData;

  /// secondary (numerical) moments: inactive
  RealVector secondaryMoments;
  /// combined moments from multilevel-multifidelity FT rollup
  RealVector combinedMoments;

  /// flag indicating need to build a fn train approximation for this QoI
  bool expansionCoeffFlag;
  /// flag indicating need to build a fn train gradient approx for this QoI
  bool expansionCoeffGradFlag;
};


inline void C3Approximation::active_model_key(const Pecos::ActiveKey& key)
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
    std::pair<Pecos::ActiveKey, C3FnTrainData> ftd_pair(key, C3FnTrainData());
    levApproxIter = levelApprox.insert(ftd_pair).first;
  }
}


inline void C3Approximation::clear_model_keys()
{
  // clears approxData keys
  Approximation::clear_model_keys();

  levelApprox.clear(); levApproxIter = levelApprox.end();
}


inline C3FnTrainData& C3Approximation::active_ftd()
{ return levApproxIter->second; }


inline C3FnTrainData& C3Approximation::combined_ftd()
{ return combinedC3FTData; }


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
//    combinedMoments.length() : levApproxIter->second.moments().length();
//}


inline const RealVector& C3Approximation::moments() const
{ return levApproxIter->second.moments(); }


inline const RealVector& C3Approximation::expansion_moments() const
{ return levApproxIter->second.moments(); }


inline const RealVector& C3Approximation::numerical_integration_moments() const
{ return secondaryMoments; } // empty


inline const RealVector& C3Approximation::combined_moments() const
{ return combinedMoments; }


inline Real C3Approximation::moment(size_t i) const
{ return levApproxIter->second.moment(i); }


inline void C3Approximation::moment(Real mom, size_t i)
{ levApproxIter->second.moment(mom, i); }


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
{ return mean(combinedC3FTData); }


inline Real C3Approximation::combined_mean(const RealVector &x)
{ return mean(x, combinedC3FTData); }


inline Real C3Approximation::combined_variance()
{ return variance(combinedC3FTData); }


inline Real C3Approximation::combined_variance(const RealVector &x)
{ return variance(x, combinedC3FTData); }


inline Real C3Approximation::covariance(Approximation& approx_2)
{
  std::shared_ptr<C3Approximation> c3_approx_rep_2 =
    std::static_pointer_cast<C3Approximation>(approx_2.approx_rep());
  return covariance(levApproxIter->second, c3_approx_rep_2->active_ftd());
}


inline Real C3Approximation::combined_covariance(Approximation& approx_2)
{
  std::shared_ptr<C3Approximation> c3_approx_rep_2 =
    std::static_pointer_cast<C3Approximation>(approx_2.approx_rep());
  return covariance(combinedC3FTData, c3_approx_rep_2->combined_ftd());
}


inline Real C3Approximation::
covariance(const RealVector &x, Approximation& approx_2)
{
  std::shared_ptr<C3Approximation> c3_approx_rep_2 =
    std::static_pointer_cast<C3Approximation>(approx_2.approx_rep());
  return covariance(x, levApproxIter->second, c3_approx_rep_2->active_ftd());
}


inline Real C3Approximation::
combined_covariance(const RealVector &x, Approximation& approx_2)
{
  std::shared_ptr<C3Approximation> c3_approx_rep_2 =
    std::static_pointer_cast<C3Approximation>(approx_2.approx_rep());
  return covariance(x, combinedC3FTData, c3_approx_rep_2->combined_ftd());
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
{ return third_central(combinedC3FTData); }


inline Real C3Approximation::combined_fourth_central()
{ return fourth_central(combinedC3FTData); }


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
