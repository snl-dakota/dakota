/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        PecosApproximation
//- Description:  Base Class for Pecos polynomial approximations
//-               
//- Owner:        Mike Eldred

#ifndef PECOS_APPROXIMATION_H
#define PECOS_APPROXIMATION_H

#include "DakotaApproximation.H"
#include "OrthogPolyApproximation.hpp"


namespace Dakota {


/// Derived approximation class for global basis polynomials.

/** The PecosApproximation class provides a global approximation
    based on basis polynomials.  This includes orthogonal polynomials
    used for polynomial chaos expansions and interpolation polynomials
    used for stochastic collocation. */

class PecosApproximation: public Approximation
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// default constructor
  PecosApproximation();
  /// alternate constructor
  PecosApproximation(const String& approx_type, const UShortArray& approx_order,
		     size_t num_vars, short data_order);
  /// standard ProblemDescDB-driven constructor
  PecosApproximation(ProblemDescDB& problem_db, size_t num_vars);
  /// destructor
  ~PecosApproximation();

  //
  //- Heading: Member functions
  //

  /// increment OrthogPolyApproximation::approxOrder uniformly
  void increment_order();

  /// set pecosBasisApprox.configOptions.expCoeffsSolnApproach
  void solution_approach(short soln_approach);
  /// get pecosBasisApprox.configOptions.expCoeffsSolnApproach
  short solution_approach() const;

  /// set pecosBasisApprox.configOptions.expansionCoeffFlag
  void expansion_coefficient_flag(bool coeff_flag);
  /// get pecosBasisApprox.configOptions.expansionCoeffFlag
  bool expansion_coefficient_flag() const;

  /// set pecosBasisApprox.configOptions.expansionGradFlag
  void expansion_gradient_flag(bool grad_flag);
  /// get pecosBasisApprox.configOptions.expansionGradFlag
  bool expansion_gradient_flag() const;

  // set pecosBasisApprox.configOptions.refinementType
  //void refinement_type(short refine_type);
  // get pecosBasisApprox.configOptions.refinementType
  //short refinement_type() const;

  /// set pecosBasisApprox.configOptions.refinementControl
  void refinement_control(short refine_cntl);
  /// get pecosBasisApprox.configOptions.refinementControl
  short refinement_control() const;

  /// set pecosBasisApprox.configOptions.vbdControl
  void vbd_control(short vbd_cntl);
  /// get pecosBasisApprox.configOptions.vbdControl
  short vbd_control() const;

  /// Performs global sensitivity analysis using Sobol' Indices by
  /// computing component (main and interaction) effects
  void compute_component_effects();
  /// Performs global sensitivity analysis using Sobol' Indices by
  /// computing total effects
  void compute_total_effects();
  /// return polyApproxRep->sobolIndexMap
  const Pecos::IntIntMap& sobol_index_map() const;
  /// return polyApproxRep->sobolIndices
  const Pecos::RealVector& sobol_indices() const;
  /// return polyApproxRep->totalSobolIndices
  const Pecos::RealVector& total_sobol_indices() const;

  /// return OrthogPolyApproximation::decayRates
  const Pecos::RealVector& dimension_decay_rates() const;

  /// set pecosBasisApprox.randomVarsKey
  void random_variables_key(const Pecos::BoolDeque& random_vars_key);

  /// set pecosBasisApprox.driverRep
  void integration_iterator(const Iterator& iterator);

  /// invoke Pecos::OrthogPolyApproximation::distribution_types()
  void distribution_types(const Pecos::ShortArray& u_types);
  /// invoke Pecos::OrthogPolyApproximation::distributions()
  void distributions(const Pecos::ShortArray& u_types,
		     const Pecos::DistributionParams& dp);

  /// set Pecos::OrthogPolyApproximation::basisTypes
  void basis_types(const Pecos::ShortArray& basis_types);
  /// get Pecos::OrthogPolyApproximation::basisTypes
  const Pecos::ShortArray& basis_types() const;

  /// set Pecos::OrthogPolyApproximation::polynomialBasis
  void polynomial_basis(const std::vector<Pecos::BasisPolynomial>& poly_basis);
  /// get Pecos::OrthogPolyApproximation::polynomialBasis
  const std::vector<Pecos::BasisPolynomial>& polynomial_basis() const;

  /// invoke Pecos::OrthogPolyApproximation::coefficients_norms_flag()
  void coefficients_norms_flag(bool flag);
  /// invoke Pecos::OrthogPolyApproximation::expansion_terms()
  void expansion_terms(size_t terms);
  /// return Pecos::OrthogPolyApproximation::expansion_terms()
  size_t expansion_terms() const;
  /// invoke Pecos::PolynomialApproximation::allocate_arrays()
  void allocate_arrays();

  /// return the mean of the expansion, treating all variables as random
  Real mean();
  /// return the mean of the expansion for a given parameter vector,
  /// treating a subset of the variables as random
  Real mean(const Pecos::RealVector& x);
  /// return the gradient of the expansion mean for a given parameter
  /// vector, treating all variables as random
  const Pecos::RealVector& mean_gradient();
  /// return the gradient of the expansion mean for a given parameter vector
  /// and given DVV, treating a subset of the variables as random
  const Pecos::RealVector& mean_gradient(const Pecos::RealVector& x,
					 const Pecos::SizetArray& dvv);

  /// return the variance of the expansion, treating all variables as random
  Real variance();
  /// return the variance of the expansion for a given parameter vector,
  /// treating a subset of the variables as random
  Real variance(const Pecos::RealVector& x);
  /// return the gradient of the expansion variance for a given parameter
  /// vector, treating all variables as random
  const Pecos::RealVector& variance_gradient();
  /// return the gradient of the expansion variance for a given parameter
  /// vector and given DVV, treating a subset of the variables as random
  const Pecos::RealVector& variance_gradient(const Pecos::RealVector& x,
					     const Pecos::SizetArray& dvv);

  /// return the variance of the expansion, treating all variables as random
  Real covariance(PecosApproximation* pecos_approx_2);
  /// return the variance of the expansion, treating a subset of the
  /// variables as random
  Real covariance(const Pecos::RealVector& x,
		      PecosApproximation* pecos_approx_2);

  /// compute moments up to the order supported by the Pecos
  /// polynomial approximation
  void compute_moments();
  /// compute moments in all-variables mode up to the order supported
  /// by the Pecos polynomial approximation
  void compute_moments(const Pecos::RealVector& x);
  /// return virtual Pecos::PolynomialApproximation::moments()
  const RealVector& moments() const;
  /// return Pecos::PolynomialApproximation::expansionMoments
  const RealVector& expansion_moments() const;
  /// return Pecos::PolynomialApproximation::numericalMoments
  const RealVector& numerical_moments() const;

  /// return pecosBasisApprox
  Pecos::BasisApproximation& pecos_basis_approximation();

protected:

  //
  //- Heading: Virtual function redefinitions
  //
  
  // redocumenting these since they use Pecos:: qualification

  /// retrieve the approximate function value for a given parameter vector
  Real                        get_value(const Pecos::RealVector& x);
  /// retrieve the approximate function gradient for a given parameter vector
  const Pecos::RealVector&    get_gradient(const Pecos::RealVector& x);
  /// retrieve the approximate function Hessian for a given parameter vector
  const Pecos::RealSymMatrix& get_hessian(const Pecos::RealVector& x);

  int min_coefficients() const;
  //int num_constraints() const; // use default implementation

  void build();
  void rebuild();
  void pop(bool save_data);
  //size_t pop_count();
  void restore();
  bool restore_available();
  size_t restoration_index();
  void finalize();
  size_t finalization_index(size_t i);
  void store();
  void combine();

  void print_coefficients(std::ostream& s) const;

  const RealVector& approximation_coefficients() const;
  void approximation_coefficients(const RealVector& approx_coeffs);

  //
  //- Heading: Data
  //

private:

  //
  //- Heading: Convenience member functions
  //

  /// utility to convert Dakota type string to Pecos type enumeration
  void approx_type_to_basis_type(const String& approx_type, short& basis_type);

  //
  //- Heading: Data
  //

  /// the Pecos basis approximation, encompassing OrthogPolyApproximation
  /// and InterpPolyApproximation
  Pecos::BasisApproximation pecosBasisApprox;

  /// convenience pointer to representation
  Pecos::PolynomialApproximation* polyApproxRep;
};


inline PecosApproximation::PecosApproximation(): polyApproxRep(NULL)
{ }


inline PecosApproximation::~PecosApproximation()
{ }


inline void PecosApproximation::increment_order()
{ polyApproxRep->increment_order(); }


inline void PecosApproximation::solution_approach(short soln_approach)
{ polyApproxRep->solution_approach(soln_approach); }


inline short PecosApproximation::solution_approach() const
{ return polyApproxRep->solution_approach(); }


inline void PecosApproximation::expansion_coefficient_flag(bool coeff_flag)
{ polyApproxRep->expansion_coefficient_flag(coeff_flag); }


inline bool PecosApproximation::expansion_coefficient_flag() const
{ return polyApproxRep->expansion_coefficient_flag(); }


inline void PecosApproximation::expansion_gradient_flag(bool grad_flag)
{ polyApproxRep->expansion_coefficient_gradient_flag(grad_flag); }


inline bool PecosApproximation::expansion_gradient_flag() const
{ return polyApproxRep->expansion_coefficient_gradient_flag(); }


//inline void PecosApproximation::refinement_type(short refine_type)
//{ polyApproxRep->refinement_type(refine_type); }


//inline short PecosApproximation::refinement_type() const
//{ return polyApproxRep->refinement_type(); }


inline void PecosApproximation::refinement_control(short refine_cntl)
{ polyApproxRep->refinement_control(refine_cntl); }


inline short PecosApproximation::refinement_control() const
{ return polyApproxRep->refinement_control(); }


inline void PecosApproximation::vbd_control(short vbd_cntl)
{ polyApproxRep->vbd_control(vbd_cntl); }


inline short PecosApproximation::vbd_control() const
{ return polyApproxRep->vbd_control(); }


inline void PecosApproximation::compute_component_effects()
{ polyApproxRep->compute_component_effects(); }


inline void PecosApproximation::compute_total_effects()
{ polyApproxRep->compute_total_effects(); }


inline const Pecos::IntIntMap& PecosApproximation::sobol_index_map() const
{ return polyApproxRep->sobol_index_map(); }


inline const Pecos::RealVector& PecosApproximation::sobol_indices() const
{ return polyApproxRep->sobol_indices(); }


inline const Pecos::RealVector& PecosApproximation::total_sobol_indices() const
{ return polyApproxRep->total_sobol_indices(); }


inline const Pecos::RealVector& PecosApproximation::
dimension_decay_rates() const
{ return polyApproxRep->dimension_decay_rates(); }


inline void PecosApproximation::
random_variables_key(const Pecos::BoolDeque& random_vars_key)
{ polyApproxRep->random_variables_key(random_vars_key); }


inline void PecosApproximation::
distribution_types(const Pecos::ShortArray& u_types)
{
  ((Pecos::OrthogPolyApproximation*)polyApproxRep)->distribution_types(u_types);
}


inline void PecosApproximation::
distributions(const Pecos::ShortArray& u_types,
	      const Pecos::DistributionParams& dp)
{
  ((Pecos::OrthogPolyApproximation*)polyApproxRep)->distributions(u_types, dp);
}


inline void PecosApproximation::
basis_types(const Pecos::ShortArray& basis_types)
{ ((Pecos::OrthogPolyApproximation*)polyApproxRep)->basis_types(basis_types); }


inline const Pecos::ShortArray& PecosApproximation::basis_types() const
{ return ((Pecos::OrthogPolyApproximation*)polyApproxRep)->basis_types(); }


inline void PecosApproximation::
polynomial_basis(const std::vector<Pecos::BasisPolynomial>& poly_basis)
{
  ((Pecos::OrthogPolyApproximation*)polyApproxRep)->
    polynomial_basis(poly_basis);
}


inline const std::vector<Pecos::BasisPolynomial>& PecosApproximation::
polynomial_basis() const
{ return ((Pecos::OrthogPolyApproximation*)polyApproxRep)->polynomial_basis(); }


inline void PecosApproximation::coefficients_norms_flag(bool flag)
{
  ((Pecos::OrthogPolyApproximation*)polyApproxRep)->
    coefficients_norms_flag(flag);
}


inline void PecosApproximation::expansion_terms(size_t terms)
{ ((Pecos::OrthogPolyApproximation*)polyApproxRep)->expansion_terms(terms); }


inline size_t PecosApproximation::expansion_terms() const
{ return ((Pecos::OrthogPolyApproximation*)polyApproxRep)->expansion_terms(); }


inline void PecosApproximation::allocate_arrays()
{ polyApproxRep->allocate_arrays(); }


inline Real PecosApproximation::mean()
{ return polyApproxRep->mean(); }


inline Real PecosApproximation::mean(const Pecos::RealVector& x)
{ return polyApproxRep->mean(x); }


inline const Pecos::RealVector& PecosApproximation::mean_gradient()
{ return polyApproxRep->mean_gradient(); }


inline const Pecos::RealVector& PecosApproximation::
mean_gradient(const Pecos::RealVector& x, const Pecos::SizetArray& dvv)
{ return polyApproxRep->mean_gradient(x, dvv); }


inline Real PecosApproximation::variance()
{ return polyApproxRep->variance(); }


inline Real PecosApproximation::variance(const Pecos::RealVector& x)
{ return polyApproxRep->variance(x); }


inline const Pecos::RealVector& PecosApproximation::variance_gradient()
{ return polyApproxRep->variance_gradient(); }


inline const Pecos::RealVector& PecosApproximation::
variance_gradient(const Pecos::RealVector& x, const Pecos::SizetArray& dvv)
{ return polyApproxRep->variance_gradient(x, dvv); }


inline Real PecosApproximation::
covariance(PecosApproximation* pecos_approx_2)
{ return polyApproxRep->covariance(pecos_approx_2->polyApproxRep); }


inline Real PecosApproximation::
covariance(const Pecos::RealVector& x, PecosApproximation* pecos_approx_2)
{ return polyApproxRep->covariance(x, pecos_approx_2->polyApproxRep); }


inline void PecosApproximation::compute_moments()
{ polyApproxRep->compute_moments(); }


inline void PecosApproximation::compute_moments(const Pecos::RealVector& x)
{ polyApproxRep->compute_moments(x); }


inline const RealVector& PecosApproximation::moments() const
{ return polyApproxRep->moments(); }


inline const RealVector& PecosApproximation::expansion_moments() const
{ return polyApproxRep->expansion_moments(); }


inline const RealVector& PecosApproximation::numerical_moments() const
{ return polyApproxRep->numerical_moments(); }


inline Pecos::BasisApproximation& PecosApproximation::
pecos_basis_approximation()
{ return pecosBasisApprox; }


inline Real PecosApproximation::get_value(const Pecos::RealVector& x)
{ return pecosBasisApprox.value(x); }


inline const Pecos::RealVector& PecosApproximation::
get_gradient(const Pecos::RealVector& x)
{ return pecosBasisApprox.gradient(x); }


inline const Pecos::RealSymMatrix& PecosApproximation::
get_hessian(const Pecos::RealVector& x)
{ return pecosBasisApprox.hessian(x); }


inline int PecosApproximation::min_coefficients() const
{ return pecosBasisApprox.min_coefficients(); }


inline void PecosApproximation::build()
{
  // base class implementation checks data set against min required
  Approximation::build();
  // map to Pecos::BasisApproximation
  pecosBasisApprox.compute_coefficients();
}


inline void PecosApproximation::rebuild()
{
  // base class default invokes build() for derived Approximations
  // that do not supply rebuild()
  //Approximation::rebuild();

  // TO DO: increment_coefficients() below covers current usage of
  // append_approximation() in NonDExpansion.  For more general
  // support of both update and append, need a mechanism to detect
  // the +/- direction of discrepancy between data and coefficients.

  //size_t curr_pts  = approxData.size(),
  //  curr_pecos_pts = polyApproxRep->data_size();
  //if (curr_pts > curr_pecos_pts)
    pecosBasisApprox.increment_coefficients();
  //else if (curr_pts < curr_pecos_pts)
    //pecosBasisApprox.decrement_coefficients();
  // else, if number of points is consistent, leave as is
}


inline void PecosApproximation::pop(bool save_data)
{
  // base class implementation removes data from currentPoints
  Approximation::pop(save_data);
  // map to Pecos::BasisApproximation
  pecosBasisApprox.decrement_coefficients();
}


//inline size_t PecosApproximation::pop_count()
//{ return polyApproxRep->pop_count(); }


inline void PecosApproximation::restore()
{
  // base class implementation updates currentPoints
  Approximation::restore();
  // map to Pecos::BasisApproximation
  pecosBasisApprox.restore_coefficients();
}


inline bool PecosApproximation::restore_available()
{ return pecosBasisApprox.restore_available(); }


inline size_t PecosApproximation::restoration_index()
{ return pecosBasisApprox.restoration_index(); }


inline void PecosApproximation::finalize()
{
  // base class implementation appends currentPoints with savedSDPArrays
  Approximation::finalize();
  // map to Pecos::BasisApproximation
  pecosBasisApprox.finalize_coefficients();
}


inline void PecosApproximation::store()
{
  // base class implementation manages approx data
  Approximation::store();
  // map to Pecos::BasisApproximation
  pecosBasisApprox.store_coefficients();
}


inline void PecosApproximation::combine()
{
  // base class implementation manages approxData state
  //Approximation::combine();
  // map to Pecos::BasisApproximation
  pecosBasisApprox.combine_coefficients();
}


inline size_t PecosApproximation::finalization_index(size_t i)
{ return pecosBasisApprox.finalization_index(i); }


inline void PecosApproximation::print_coefficients(std::ostream& s) const
{ pecosBasisApprox.print_coefficients(s); }


inline const RealVector& PecosApproximation::approximation_coefficients() const
{ return pecosBasisApprox.approximation_coefficients(); }


inline void PecosApproximation::
approximation_coefficients(const RealVector& approx_coeffs)
{ pecosBasisApprox.approximation_coefficients(approx_coeffs); }

} // namespace Dakota

#endif
