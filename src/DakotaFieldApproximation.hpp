/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_FIELD_APPROXIMATION_H
#define DAKOTA_FIELD_APPROXIMATION_H

#include "DakotaApproximation.hpp"

namespace Dakota {

/// Base class for the field-based approximation class hierarchy.

class FieldApproximation: public Approximation
{
public:

  //
  //- Heading: Constructors, destructor, assignment operator
  //

  /// constructor initializes the base class part of letter classes
  /// (BaseConstructor overloading avoids infinite recursion in the
  /// derived class constructors - Coplien, p. 139)
  FieldApproximation(BaseConstructor, const ProblemDescDB& problem_db,
                     const SharedApproxData& shared_data, 
                     const StringArray& approx_label);

  /// default constructor
  FieldApproximation();
  /// standard constructor for envelope
  FieldApproximation(ProblemDescDB& problem_db,
                     const SharedApproxData& shared_data,
                     const StringArray& approx_labels);
  // /// alternate constructor
  //FieldApproximation(const SharedApproxData& shared_data);
  ///// copy constructor
  //FieldApproximation(const FieldApproximation& approx);

  /// destructor
  virtual ~FieldApproximation();

  ///// assignment operator
  //FieldApproximation operator=(const FieldApproximation& approx);

  //
  //- Heading: Virtual functions
  //

  /// builds the approximation from scratch
  void build() override;

  /// set active field component
  void active_component(int comp)
  { activeComponent = comp; }

  /// get active field component
  int active_component(int comp) const
  { return activeComponent; }

  ///// exports the approximation; if export_format > NO_MODEL_FORMAT,
  ///// uses all 3 parameters, otherwise extracts these from the
  ///// Approximation's sharedDataRep to build a filename
  //virtual void export_model(const StringArray& var_labels = StringArray(),
  //      		    const String& fn_label = "",
  //      		    const String& export_prefix = "",
  //      		    const unsigned short export_format = NO_MODEL_FORMAT);
  ///// approximation export that generates labels from the passed
  ///// Variables, since only the derived classes know how the variables
  ///// are ordered w.r.t. the surrogate build; if export_format >
  ///// NO_MODEL_FORMAT, uses all 3 parameters, otherwise extracts these
  ///// from the Approximation's sharedDataRep to build a filename
  //virtual void export_model(const Variables& vars,
  //      		    const String& fn_label = "",
  //      		    const String& export_prefix = "",
  //      		    const unsigned short export_format = NO_MODEL_FORMAT);

  ///// rebuilds the approximation incrementally
  //virtual void rebuild();

  ///// replace the response data 
  //virtual void replace(const IntResponsePair& response_pr, size_t fn_index);
  ///// removes entries from end of SurrogateData::{vars,resp}Data
  ///// (last points appended, or as specified in args)
  //virtual void pop_coefficients(bool save_data);
  ///// restores state prior to previous pop()
  //virtual void push_coefficients();
  ///// finalize approximation by applying all remaining trial sets
  //virtual void finalize_coefficients();

  ///// clear current build data in preparation for next build
  //virtual void clear_current_active_data();

  ///// combine all level approximations into a single aggregate approximation
  //virtual void combine_coefficients();
  ///// promote combined approximation into active approximation
  //virtual void combined_to_active_coefficients(bool clear_combined = true);
  ///// prune inactive coefficients following combination and promotion to active
  //virtual void clear_inactive_coefficients();

  ///// retrieve the approximate function value for a given parameter vector
  //virtual Real value(const Variables& vars);
  ///// retrieve the approximate function gradient for a given parameter vector
  //virtual const RealVector& gradient(const Variables& vars);
  ///// retrieve the approximate function Hessian for a given parameter vector
  //virtual const RealSymMatrix& hessian(const Variables& vars);
  ///// retrieve the variance of the predicted value for a given parameter vector
  //virtual Real prediction_variance(const Variables& vars);
  //  
  ///// retrieve the approximate function value for a given parameter vector
  //virtual Real value(const RealVector& c_vars);
  ///// retrieve the approximate function gradient for a given parameter vector
  //virtual const RealVector& gradient(const RealVector& c_vars);
  ///// retrieve the approximate function Hessian for a given parameter vector
  //virtual const RealSymMatrix& hessian(const RealVector& c_vars);
  ///// retrieve the variance of the predicted value for a given parameter vector
  //virtual Real prediction_variance(const RealVector& c_vars);

  ///// return the mean of the expansion, where all active vars are random
  //virtual Real mean();
  ///// return the mean of the expansion for a given parameter vector,
  ///// where a subset of the active variables are random
  //virtual Real mean(const RealVector& x);
  ///// return the mean of the combined expansion, where all active vars
  ///// are random
  //virtual Real combined_mean();
  ///// return the mean of the combined expansion for a given parameter vector,
  ///// where a subset of the active variables are random
  //virtual Real combined_mean(const RealVector& x);
  ///// return the gradient of the expansion mean
  //virtual const RealVector& mean_gradient();
  ///// return the gradient of the expansion mean
  //virtual const RealVector& mean_gradient(const RealVector& x,
  //      				  const SizetArray& dvv);

  ///// return the variance of the expansion, where all active vars are random
  //virtual Real variance();
  ///// return the variance of the expansion for a given parameter vector,
  ///// where a subset of the active variables are random
  //virtual Real variance(const RealVector& x);           
  //virtual const RealVector& variance_gradient();      
  //virtual const RealVector& variance_gradient(const RealVector& x,
  //      				      const SizetArray& dvv);
  ///// return the covariance between two response expansions, treating
  ///// all variables as random
  //virtual Real covariance(Approximation& approx_2);
  ///// return the covariance between two response expansions, treating
  ///// a subset of the variables as random
  //virtual Real covariance(const RealVector& x, Approximation& approx_2);
  ///// return the covariance between two combined response expansions,
  ///// where all active variables are random
  //virtual Real combined_covariance(Approximation& approx_2);
  ///// return the covariance between two combined response expansions,
  ///// where a subset of the active variables are random
  //virtual Real combined_covariance(const RealVector& x,
  //      			   Approximation& approx_2);

  //virtual void compute_moments(bool full_stats = true,
  //      		       bool combined_stats = false);
  //virtual void compute_moments(const RealVector& x, bool full_stats = true,
  //      		       bool combined_stats = false);

  //virtual const RealVector& moments() const;
  //virtual const RealVector& expansion_moments() const;
  //virtual const RealVector& numerical_integration_moments() const;
  //virtual const RealVector& combined_moments() const;

  //virtual Real moment(size_t i) const;
  //virtual void moment(Real mom, size_t i);
  //virtual Real combined_moment(size_t i) const;
  //virtual void combined_moment(Real mom, size_t i);

  //virtual void clear_component_effects();
  //virtual void compute_component_effects();
  //virtual void compute_total_effects();
  //virtual const RealVector& sobol_indices() const;
  //virtual const RealVector& total_sobol_indices() const;
  //virtual ULongULongMap sparse_sobol_index_map() const;

  ///// check if resolution advancement (e.g., order, rank) is available
  ///// for this approximation instance
  //virtual bool advancement_available();

  ///// check if diagnostics are available for this approximation type
  //virtual bool diagnostics_available();
  ///// retrieve a single diagnostic metric for the diagnostic type specified
  //virtual Real diagnostic(const String& metric_type);
  ///// retrieve diagnostic metrics for the diagnostic types specified, applying 
  //// num_folds-cross validation
  //virtual RealArray cv_diagnostic(const StringArray& metric_types,
  //      			  unsigned num_folds);
  ///// compute and print all requested diagnostics and cross-validation 
  //virtual void primary_diagnostics(size_t fn_index);
  ///// compute requested diagnostics for user provided challenge pts
  //virtual RealArray challenge_diagnostic(const StringArray& metric_types,
  //      		    const RealMatrix& challenge_points,
  //                          const RealVector& challenge_responses);
  ///// compute and print all requested diagnostics for user provided
  ///// challenge pts
  //virtual void challenge_diagnostics(size_t fn_index, 
  //      			     const RealMatrix& challenge_points, 
  //                                   const RealVector& challenge_responses);
  //// TODO: private implementation of cross-validation:
  ////  void cross_validate(metrics, folds)

  ///// return the coefficient array computed by build()/rebuild()
  //virtual RealVector approximation_coefficients(bool normalized) const;
  ///// set the coefficient array from external sources, rather than
  ///// computing with build()/rebuild()
  //virtual void approximation_coefficients(const RealVector& approx_coeffs,
  //      				  bool normalized);

  //// link more than once approxData instance for aggregated response data
  //// (PecosApproximation)
  ////virtual void link_multilevel_surrogate_data();

  ///// print the coefficient array computed in build()/rebuild()
  //virtual void coefficient_labels(std::vector<std::string>& coeff_labels) const;

  ///// print the coefficient array computed in build()/rebuild()
  //virtual void print_coefficients(std::ostream& s, bool normalized);

  ///// return the minimum number of samples (unknowns) required to
  ///// build the derived class approximation type in numVars dimensions
  //virtual int min_coefficients() const;

  ///// return the recommended number of samples (unknowns) required to
  ///// build the derived class approximation type in numVars dimensions
  //virtual int recommended_coefficients() const;

  ///// return the number of constraints to be enforced via an anchor point
  //virtual int num_constraints() const;

  ///* *** Additions for C3 ***
  ///// clear current build data in preparation for next build
  //virtual void clear_current();
  //*/
  //virtual void expansion_coefficient_flag(bool);
  //virtual bool expansion_coefficient_flag() const;    
  //virtual void expansion_gradient_flag(bool);
  //virtual bool expansion_gradient_flag() const;

  ///// clear tracking of computed moments, due to (expansion) change
  ///// that invalidates previous results
  //virtual void clear_computed_bits();

  ///// if needed, map passed all variable labels to approximation's labels
  //virtual void map_variable_labels(const Variables& dfsm_vars);

  //
  //- Heading: Member functions
  //


protected:

  //
  //- Heading: Constructors
  //

  ///// constructor initializes the base class part of letter classes
  ///// (BaseConstructor overloading avoids infinite recursion in the
  ///// derived class constructors - Coplien, p. 139)
  //FieldApproximation(BaseConstructor, const ProblemDescDB& problem_db,
  //      	const SharedApproxData& shared_data, 
  //              const String& approx_label);

  ///// constructor initializes the base class part of letter classes
  ///// (BaseConstructor overloading avoids infinite recursion in the
  ///// derived class constructors - Coplien, p. 139)
  //FieldApproximation(NoDBBaseConstructor, const SharedApproxData& shared_data);

  //
  //- Heading: Member functions
  //

  //
  //- Heading: Data
  //

  int activeComponent;

private:

  //
  //- Heading: Member functions
  //

  /// Used only by the standard envelope constructor to initialize
  /// approxRep to the appropriate derived type.
  std::shared_ptr<FieldApproximation>
  get_field_approx(ProblemDescDB& problem_db, const SharedApproxData& shared_data,
	           const StringArray& approx_label);

  //
  //- Heading: Data
  //
};

} // namespace Dakota

#endif
