/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDQuadrature
//- Description: Projects 1-D Gaussian quadratures in a tensor-product approach
//- Owner:       Mike Eldred
//- Revised by:  
//- Version:

#ifndef NOND_QUADRATURE_H
#define NOND_QUADRATURE_H

#include "data_types.h"
#include "NonDIntegration.H"
#include "TensorProductDriver.hpp"

namespace Dakota {


/// Derived nondeterministic class that generates N-dimensional
/// numerical quadrature points for evaluation of expectation
/// integrals over uncorrelated standard
/// normals/uniforms/exponentials/betas/gammas.

/** This class is used by NonDPolynomialChaos, but could also be used
    for general numerical integration of moments.  It employs
    Gauss-Hermite, Gauss-Legendre, Gauss-Laguerre, Gauss-Jacobi and
    generalized Gauss-Laguerre quadrature for use with normal,
    uniform, exponential, beta, and gamma density functions and
    integration bounds.  The abscissas and weights for one-dimensional
    integration are extracted from the appropriate
    OrthogonalPolynomial class and are extended to n-dimensions using
    a tensor product approach. */

class NonDQuadrature: public NonDIntegration
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// alternate constructor for instantiations "on the fly" based on a
  /// quadrature order specification
  NonDQuadrature(Model& model, const Pecos::ShortArray& u_types,
		 const UShortArray& order, bool nested_rules = false);

  /// alternate constructor for instantiations "on the fly" based on
  /// the size of a filtered tensor product sample set
  NonDQuadrature(Model& model, const Pecos::ShortArray& u_types,
		 int num_filt_samples, bool nested_rules = false);

  //
  //- Heading: Member functions
  //

  /// return quadOrder
  const Pecos::UShortArray& quadrature_order() const;

  /// set numFilteredSamples
  void filtered_samples(size_t samples);

  /// calculate smallest quadOrderRef with at least numFilteredSamples
  void compute_min_order();

protected:

  //
  //- Heading: Constructors and destructor
  //

  NonDQuadrature(Model& model); ///< constructor
  ~NonDQuadrature();            ///< destructor

  //
  //- Heading: Virtual function redefinitions
  //

  void get_parameter_sets(Model& model);

  void reset();

  void sampling_reset(int min_samples,bool all_data_flag, bool stats_flag);

  void increment_grid();

  void increment_grid_preference(const RealVector& dim_pref);

  int num_samples() const;

private:

  //
  //- Heading: Convenience functions
  //

  /// prune allSamples back to size numFilteredSamples, retaining points
  /// with highest product weight
  void filter_parameter_sets();

  /// update quadOrder based on an updated dimension preference
  void anisotropic_preference(const RealVector& dim_pref);

  /// verify self-consistency of integration specification
  void check_integration(const UShortArray& quad_order_spec);

  /// update quadOrder from quadOrderRef to account for nested rule constraints
  void nested_quadrature_order(const UShortArray& quad_order_ref);

  /// return Product_i(quadOrderRef[i])
  size_t reference_grid_size() const;

  /// increment each quadOrderRef entry by 1
  void increment_reference();
  /// increment the quadOrderRef entry with maximum preference by 1
  void increment_reference(const RealVector& dim_pref);

  //
  //- Heading: Data
  //

  /// convenience pointer to the numIntDriver representation
  Pecos::TensorProductDriver* tpqDriver;

  /// for studies involving refinement strategies, allow for use of nested
  /// quadrature rules such as Gauss-Patterson
  bool nestedRules;

  /// the user specification for the number of Gauss points per dimension
  UShortArray quadOrderSpec;
  /// reference point for Pecos::TensorProductDriver::quadOrder: the original
  /// user specification for the number of Gauss points per dimension, plus
  /// any refinements posted by increment_grid()
  UShortArray quadOrderRef;

  /// size of a filtered set of tensor quadrature points; used by the
  /// regression PCE approach known as "probabilistic collocation"
  size_t numFilteredSamples;
};


inline const Pecos::UShortArray& NonDQuadrature::quadrature_order() const
{ return tpqDriver->quadrature_order(); }


inline void NonDQuadrature::filtered_samples(size_t samples)
{ numFilteredSamples = samples; }


inline void NonDQuadrature::reset()
{ quadOrderRef = quadOrderSpec; tpqDriver->quadrature_order(quadOrderSpec); }


inline size_t NonDQuadrature::reference_grid_size() const
{
  size_t i, size = 1;
  for (i=0; i<numContinuousVars; ++i)
    size *= quadOrderRef[i];
  return size;
}


inline int NonDQuadrature::num_samples() const
{ return (numFilteredSamples) ? numFilteredSamples : reference_grid_size(); }


inline void NonDQuadrature::increment_reference()
{
  // quadOrderRef is a reference point for quadOrder, e.g., a lower bound
  for (size_t i=0; i<numContinuousVars; ++i)
    quadOrderRef[i] += 1;
}


inline void NonDQuadrature::increment_reference(const RealVector& dim_pref)
{
  // determine the dimension with max preference
  Real max_dim_pref = dim_pref[0]; size_t max_dim_pref_index = 0;
  for (size_t i=1; i<numContinuousVars; ++i)
    if (dim_pref[i] > max_dim_pref)
      { max_dim_pref = dim_pref[i]; max_dim_pref_index = i; }
  // increment only the dimension with max preference by 1
  quadOrderRef[max_dim_pref_index] += 1;
  // now balance the other dims relative to this new increment, preserving
  // previous resolution
  anisotropic_preference(dim_pref);
}

} // namespace Dakota

#endif
