/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2001, Sandia National Laboratories.
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

#include "data_types.h"
#include "DakotaNonD.H"
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

  /// increment SSG level/TPQ order
  virtual void increment_grid() = 0;

  /// increment SSG level/TPQ order and update anisotropy
  virtual void increment_grid(const RealVector& dim_pref) = 0;

  //
  //- Heading: Member functions
  //

  /// return numIntDriver
  const Pecos::IntegrationDriver& driver() const;

  // return product weights corresponding to each of the collocation points
  //const RealVector& weight_products() const;

  // return the number of unique points in the current grid
  //size_t grid_size();

protected:

  //
  //- Heading: Constructors and destructor
  //

  NonDIntegration(Model& model); ///< constructor

  /// alternate constructor for instantiations "on the fly"
  NonDIntegration(NoDBBaseConstructor, Model& model);

  ~NonDIntegration();            ///< destructor

  //
  //- Heading: Virtual function redefinitions
  //

  void quantify_uncertainty();

  //
  //- Heading: Member functions
  //

  /// verify self-consistency of variables data
  void check_variables(const Pecos::ShortArray& x_types);

  //
  //- Heading: Data
  //

  /// Pecos utlity class for managing interface to tensor-product grids
  /// and VPISparseGrid utilities for Smolyak sparse grids and cubature
  Pecos::IntegrationDriver numIntDriver;

  /// counter for number of integration executions for this object
  size_t numIntegrations;

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
