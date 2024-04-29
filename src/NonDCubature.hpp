/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef NOND_CUBATURE_H
#define NOND_CUBATURE_H

#include "dakota_data_types.hpp"
#include "NonDIntegration.hpp"
#include "CubatureDriver.hpp"

namespace Pecos {
class MultivariateDistribution; // fwd declaration
}

namespace Dakota {


/// Derived nondeterministic class that generates N-dimensional
/// numerical cubature points for evaluation of expectation integrals.

/** This class is used by NonDPolynomialChaos, but could also be used
    for general numerical integration of moments.  It employs
    Stroud cubature rules and extensions by D. Xiu. */

class NonDCubature: public NonDIntegration
{
public:

  //
  //- Heading: Constructors and destructor
  //

  // alternate constructor for instantiations "on the fly"
  NonDCubature(Model& model, unsigned short cub_int_order);

  ~NonDCubature();                                       ///< destructor

  //
  //- Heading: Member functions
  //

  /// return cubIntOrder
  unsigned short integrand_order() const;

protected:

  //
  //- Heading: Constructors and destructor
  //

  NonDCubature(ProblemDescDB& problem_db, Model& model); ///< constructor

  //
  //- Heading: Virtual function redefinitions
  //

  void initialize_grid(const std::vector<Pecos::BasisPolynomial>& poly_basis);

  void get_parameter_sets(Model& model);

  void sampling_reset(size_t min_samples, bool all_data_flag, bool stats_flag);

  void increment_grid();
  void increment_grid_preference(const RealVector& dim_pref);
  void increment_grid_preference();
  void decrement_grid();

  void reset();

  size_t num_samples() const;

private:

  //
  //- Heading: Convenience functions
  //

  /// define cubIntRule from random variable type
  void assign_rule(const Pecos::MultivariateDistribution& mvd);

  //
  //- Heading: Data
  //

  /// convenience pointer to the numIntDriver representation
  std::shared_ptr<Pecos::CubatureDriver> cubDriver;

  // the user specification for the number of Gauss points per dimension
  //UShortArray cubIntOrderSpec;
  /// reference point for Pecos::CubatureDriver::cubIntOrder: the original
  /// user specification for the number of Gauss points per dimension, plus
  /// any refinements posted by increment_grid()
  unsigned short cubIntOrderRef;
  /// the isotropic cubature integration rule
  unsigned short cubIntRule;
};


inline void NonDCubature::reset()
{
  // reset dimensional quadrature order to specification
  //cubIntOrderRef = cubIntOrderSpec;
  // clear dist param update trackers
  cubDriver->reset();
}


inline unsigned short NonDCubature::integrand_order() const
{ return cubDriver->integrand_order(); }


/** Should not be used, but one of virtual function pair must be defined. */
inline void NonDCubature::increment_grid_preference(const RealVector& dim_pref)
{ increment_grid(); } // ignore dim_pref


/** Should not be used, but one of virtual function pair must be defined. */
inline void NonDCubature::increment_grid_preference()
{ increment_grid(); } // ignore dim_pref


inline size_t NonDCubature::num_samples() const
{ return cubDriver->grid_size(); }

} // namespace Dakota

#endif
