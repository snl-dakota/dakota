/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 VPSApproximation
//- Description: Class implementation of Voronoi Piecewise Surrogate Approximation
//- Owner:       Mohamed Ebeida
//- Checked by:
//- Version:

#ifndef VPS_APPROXIMATION_H
#define VPS_APPROXIMATION_H

#include "dakota_data_types.hpp"
#include "DakotaApproximation.hpp"

#include "Teuchos_SerialSpdDenseSolver.hpp"

#ifdef HAVE_OPTPP
namespace Teuchos { 
  template<typename OrdinalType, typename ScalarType> class SerialDenseVector;
  template<typename OrdinalType, typename ScalarType> class SerialDenseMatrix;
}
#endif // HAVE_OPTPP


namespace Dakota {

class ProblemDescDB;

/// Derived approximation class for VPS implementation

/** The VPSApproximation class provides a set of piecewise surrogate approximations 
    each of which is valid within a Voronoi cell.  */

class VPSApproximation: public Approximation
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// default constructor
  VPSApproximation();
  /// alternate constructor
  VPSApproximation(const SharedApproxData& shared_data);
  /// standard constructor
  VPSApproximation(const ProblemDescDB& problem_db,
			 const SharedApproxData& shared_data);
  /// destructor
  ~VPSApproximation();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  /// return the minimum number of samples (unknowns) required to
  /// build the derived class approximation type in numVars dimensions
  int min_coefficients() const;

  /// return the number of constraints to be enforced via an anchor point
  int num_constraints()  const;

  /// builds the approximation from scratch
  void build();

  /// retrieve the predicted function value for a given parameter set
  Real value(const Variables& vars);

  /// retrieve the function gradient at the predicted value 
  /// for a given parameter set
  const RealVector& gradient(const Variables& vars);

  /// retrieve the variance of the predicted value for a given parameter set
  Real prediction_variance(const Variables& vars);

private: 

  //
  //- Heading: Member functions
  //

  /// Function to compute coefficients governing the VPS surrogates.
  void VPSmodel_build();

  /// Function returns a response value using the VPS surface.
  /** The response value is computed at the design point specified
      by the RealVector function argument.*/
  void VPSmodel_apply(const RealVector& new_x, bool variance_flag,
		     bool gradients_flag);

  //
  //- Heading: Data
  //

  /// pointer to the active object instance used within the static evaluator
  static VPSApproximation* VPSinstance;

  /// value of the approximation returned by value()
  Real approxValue;
  /// value of the approximation returned by prediction_variance()
  Real approxVariance;
  /// A 2-D array (num sample sites = rows, num vars = columns) 
  /// used to create the Gaussian process
  RealMatrix trainPoints;
  /// An array of response values; one response value per sample site
  RealMatrix trainValues;
  /// The number of observations on which the GP surface is built.
  size_t numObs;
 

};


/** alternate constructor */

inline VPSApproximation::VPSApproximation()
{
}

inline VPSApproximation::
VPSApproximation(const SharedApproxData& shared_data):
  Approximation(NoDBBaseConstructor(), shared_data)
{ }

inline VPSApproximation::~VPSApproximation()
{ }

} // namespace Dakota

#endif
