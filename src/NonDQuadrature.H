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
		 unsigned short order, const RealVector& dim_pref,
		 bool nested_rules = false, bool piecewise_basis = false,
		 bool use_derivs = false);

  /// alternate constructor for instantiations "on the fly" based on
  /// the size of a filtered tensor product sample set
  NonDQuadrature(Model& model, const Pecos::ShortArray& u_types,
		 int num_filt_samples, const RealVector& dim_pref,
		 bool nested_rules = false, bool piecewise_basis = false,
		 bool use_derivs = false);

  //
  //- Heading: Member functions
  //

  /// return Pecos::TensorProductDriver::quadOrder
  const Pecos::UShortArray& quadrature_order() const;

  /// set numFilteredSamples
  void filtered_samples(size_t samples);

  /// calculate smallest quadOrderRef with at least numFilteredSamples
  void compute_min_quadrature_order(size_t min_samples);

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

  /// initialize quad_order vector based on quad_order_spec scalar and
  /// dim_pref_spec vector
  void anisotropic_preference(unsigned short quad_order_spec,
			      const RealVector& dim_pref_spec,
			      UShortArray& quad_order);
  /// update quad_order_ref based on an updated dimension preference,
  /// enforcing previous values as a lower bound
  void anisotropic_preference(const RealVector& dim_pref,
			      UShortArray& quad_order_ref);

  /// initialize quadOrder{Spec,Ref} from quad_order_spec and dimPrefSpec
  void initialize_quadrature_order(unsigned short quad_order_spec);

  /// update Pecos::TensorProductDriver::quadOrder from quadOrderRef
  /// to account for nested rule constraints
  void nested_quadrature_order(const UShortArray& quad_order_ref);

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


/** Note: quadOrderSpec state includes dimPrefSpec. */
inline void NonDQuadrature::reset()
{ quadOrderRef = quadOrderSpec; tpqDriver->quadrature_order(quadOrderSpec); }


inline int NonDQuadrature::num_samples() const
{ return (numFilteredSamples) ? numFilteredSamples : tpqDriver->grid_size(); }

} // namespace Dakota

#endif
