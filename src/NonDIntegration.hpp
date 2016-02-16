/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDIntegration
//- Description: Base class for multidimensional integration using
//-              tensor-product quadrature or Smolyak sparse grids.
//- Owner:       Mike Eldred
//- Revised by:  
//- Version:

#ifndef NOND_INTEGRATION_H
#define NOND_INTEGRATION_H

#include "dakota_data_types.hpp"
#include "DakotaNonD.hpp"
#include "IntegrationDriver.hpp"

namespace Dakota {


/// Derived nondeterministic class that generates N-dimensional
/// numerical integration points for evaluation of expectation integrals

/** This class provides a base class for shared code among
    NonDQuadrature and NonDSparseGrid. */

class NonDIntegration: public NonD
{
public:

  //
  //- Heading: Virtual functions
  //

  /// initialize integration grid by drawing from polynomial basis settings
  virtual void initialize_grid(
    const std::vector<Pecos::BasisPolynomial>& poly_basis) = 0;

  /// increment SSG level/TPQ order
  virtual void increment_grid() = 0;
  /// increment SSG level/TPQ order and update anisotropy
  virtual void increment_grid_preference(const RealVector& dim_pref);
  /// increment SSG level/TPQ order and update anisotropy
  virtual void increment_grid_weights(const RealVector& aniso_wts);
  /// increment sequenceIndex and update active orders/levels
  virtual void increment_specification_sequence();

  //
  //- Heading: Member functions
  //

  /// convert scalar_order_spec and vector dim_pref_spec to vector aniso_order
  static void dimension_preference_to_anisotropic_order(
    unsigned short scalar_order_spec, const RealVector& dim_pref_spec,
    size_t num_v, UShortArray& aniso_order);
  /// convert vector aniso_order to scalar_order and vector dim_pref
  static void anisotropic_order_to_dimension_preference(
    const UShortArray& aniso_order, unsigned short& scalar_order,
    RealVector& dim_pref);

  /// return numIntDriver
  const Pecos::IntegrationDriver& driver() const;

  // return product weights corresponding to each of the collocation points
  //const RealVector& weight_products() const;

  // return the number of unique points in the current grid
  //size_t grid_size();

  //
  //- Heading: Virtual function redefinitions
  //

  bool resize();

protected:

  //
  //- Heading: Constructors and destructor
  //

  /// constructor
  NonDIntegration(ProblemDescDB& problem_db, Model& model);
  /// alternate constructor for instantiations "on the fly"
  NonDIntegration(unsigned short method_name, Model& model);
  /// alternate constructor for instantiations "on the fly"
  NonDIntegration(unsigned short method_name, Model& model,
		  const RealVector& dim_pref);
  /// destructor
  ~NonDIntegration();

  //
  //- Heading: Virtual function redefinitions
  //

  void core_run();

  //
  //- Heading: Member functions
  //

  /// verify self-consistency of variables data
  void check_variables(const std::vector<Pecos::RandomVariable>& x_ran_vars);

  /// output integration points and weights to a tabular file
  void print_points_weights(const String& tabular_name);

  //
  //- Heading: Data
  //

  /// Pecos utlity class for managing interface to tensor-product grids
  /// and VPISparseGrid utilities for Smolyak sparse grids and cubature
  Pecos::IntegrationDriver numIntDriver;

  /// counter for number of integration executions for this object
  size_t numIntegrations;
  /// index into NonDQuadrature::quadOrderSpec and NonDSparseGrid::ssgLevelSpec
  /// that defines the current instance of several possible refinement levels
  size_t sequenceIndex;

  /// the user specification for anisotropic dimension preference
  RealVector dimPrefSpec;

private:

  //
  //- Heading: Data
  //
};


inline const Pecos::IntegrationDriver& NonDIntegration::driver() const
{ return numIntDriver; }


//inline const Pecos::RealVector& NonDIntegration::weight_products() const
//{ return numIntDriver.weight_sets(); }


//inline size_t NonDIntegration::grid_size()
//{ return numIntDriver.grid_size(); }

} // namespace Dakota

#endif
