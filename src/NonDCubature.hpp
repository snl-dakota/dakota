/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDCubature
//- Description: Stroud cubature rules and extensions
//- Owner:       Mike Eldred
//- Revised by:  
//- Version:

#ifndef NOND_CUBATURE_H
#define NOND_CUBATURE_H

#include "dakota_data_types.hpp"
#include "NonDIntegration.hpp"
#include "CubatureDriver.hpp"

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
  NonDCubature(Model& model, const Pecos::ShortArray& u_types,
	       unsigned short cub_int_order);

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
  ~NonDCubature();                                       ///< destructor

  //
  //- Heading: Virtual function redefinitions
  //

  void initialize_grid(const std::vector<Pecos::BasisPolynomial>& poly_basis);

  void get_parameter_sets(Model& model);

  void sampling_reset(int min_samples,bool all_data_flag, bool stats_flag);

  void increment_grid();
  void increment_grid_preference(const RealVector& dim_pref);

  int num_samples() const;

private:

  //
  //- Heading: Convenience functions
  //

  /// verify self-consistency of integration specification
  void check_integration(const Pecos::ShortArray& u_types,
			 const Pecos::AleatoryDistParams& adp);

  /// increment each cubIntOrderRef entry by 1
  void increment_reference();

  //
  //- Heading: Data
  //

  /// convenience pointer to the numIntDriver representation
  Pecos::CubatureDriver* cubDriver;

  // the user specification for the number of Gauss points per dimension
  //UShortArray cubIntOrderSpec;
  /// reference point for Pecos::CubatureDriver::cubIntOrder: the original
  /// user specification for the number of Gauss points per dimension, plus
  /// any refinements posted by increment_grid()
  unsigned short cubIntOrderRef;
  /// the isotropic cubature integration rule
  unsigned short cubIntRule;
};


inline unsigned short NonDCubature::integrand_order() const
{ return cubDriver->integrand_order(); }


/** cubIntOrderRef is a reference point for CubatureDriver::cubIntOrder,
    e.g., a lower bound */
inline void NonDCubature::increment_reference()
{ cubIntOrderRef += 1; }


/** Should not be used, but pure virtual must be defined. */
inline void NonDCubature::increment_grid_preference(const RealVector& dim_pref)
{ increment_grid(); } // ignore dim_pref


inline int NonDCubature::num_samples() const
{ return cubDriver->grid_size(); }

} // namespace Dakota

#endif
