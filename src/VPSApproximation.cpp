/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 VPSApproximation
//- Description: Class implementation for Voronoi Piecewise Surrogate Approximation
//- Owner:       Mohamed Ebeida
//- Checked by:
//- Version:

#include "VPSApproximation.hpp"
#include "dakota_data_types.hpp"
#include "DakotaIterator.hpp"
#include "DakotaResponse.hpp"
//#include "NPSOLOptimizer.hpp"
#ifdef HAVE_NCSU
#include "NCSUOptimizer.hpp"
#endif
#include "ProblemDescDB.hpp"

#ifdef HAVE_OPTPP
#include "SNLLOptimizer.hpp"
using OPTPP::NLPFunction;
using OPTPP::NLPGradient;
#endif

#include "Teuchos_LAPACK.hpp"
#include "Teuchos_SerialDenseSolver.hpp"
#include "Teuchos_SerialDenseHelpers.hpp"

//#define DEBUG
//#define DEBUG_FULL


namespace Dakota {

using Teuchos::rcp;

typedef Teuchos::SerialDenseSolver<int, Real>    RealSolver;
typedef Teuchos::SerialSpdDenseSolver<int, Real> RealSpdSolver;

//initialization of statics
VPSApproximation* VPSApproximation::VPSinstance(NULL);

VPSApproximation::
VPSApproximation(const ProblemDescDB& problem_db,
		       const SharedApproxData& shared_data):
  Approximation(BaseConstructor(), problem_db, shared_data)
{

}


int VPSApproximation::min_coefficients() const
{
  // min number of samples required to build the network is equal to
  // the number of design variables + 1

  // Note: Often this is too few samples.  It is better to have about
  // O(n^2) samples, where 'n' is the number of variables.

  return sharedDataRep->numVars + 1;
}


int VPSApproximation::num_constraints() const
{ return (approxData.anchor()) ? 1 : 0; }


void VPSApproximation::build()
{
  // base class implementation checks data set against min required
  Approximation::build();

  size_t i, j, offset = 0, num_v = sharedDataRep->numVars;
  numObs = approxData.points();
  // GaussProcApproximation does not directly handle anchorPoint
  // -> treat it as another currentPoint
  if (approxData.anchor()) {
    offset  = 1;
    numObs += 1;
  }

  // Transfer the training data to the Teuchos arrays used by the GP
  trainPoints.shapeUninitialized(numObs, num_v);
  trainValues.shapeUninitialized(numObs, 1);
  // process anchorPoint, if present
  if (approxData.anchor()) {
    const RealVector& c_vars = approxData.anchor_continuous_variables();
    for (j=0; j<num_v; ++j)
      trainPoints(0,j) = c_vars[j];
    trainValues(0,0) = approxData.anchor_function();
  }
  // process currentPoints
  for (i=offset; i<numObs; ++i) {
    const RealVector& c_vars = approxData.continuous_variables(i);
    for (j=0; j<num_v; j++)
      trainPoints(i,j) = c_vars[j];
    trainValues(i,0) = approxData.response_function(i);
  }

  // Build a VPS surrogate model using the sampled data
  VPSmodel_build();
}


Real VPSApproximation::value(const Variables& vars)
{ VPSmodel_apply(vars.continuous_variables(),false,false); return approxValue; }


const RealVector& VPSApproximation::gradient(const Variables& vars)
{ VPSmodel_apply(vars.continuous_variables(),false,true); return approxGradient;}


Real VPSApproximation::prediction_variance(const Variables& vars)
{ VPSmodel_apply(vars.continuous_variables(),true,false); return approxVariance;}


void VPSApproximation::VPSmodel_build()
{
}


void VPSApproximation::
VPSmodel_apply(const RealVector& approx_pt, bool variance_flag,
	      bool gradients_flag)
{
   approxValue = 5.0;
}


} // namespace Dakota
