/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDQUESOBayesCalibration
//- Description: Derived class for Bayesian inference using QUESO
//- Owner:       Laura Swiler
//- Checked by:
//- Version:

#include "NonDQUESOBayesCalibration.H"
#include "ProblemDescDB.H"
#include "DakotaModel.H"

#include "/home/lpswile/bin/queso42/include/uqStatisticalInverseProblem.h"


static const char rcsId[]="@(#) $Id$";


namespace Dakota {

/* Old approach for a function object (now using multiple inheritance):
struct
likelihoodRoutine_DataType
{
  const uqGslVectorClass* meanVector;
  const uqGslMatrixClass* covMatrix;
};

double likelihoodRoutine(
  const uqGslVectorClass& paramValues,
  const uqGslVectorClass* paramDirection,
  const void*             functionDataPtr,
  uqGslVectorClass*       gradVector,
  uqGslMatrixClass*       hessianMatrix,
  uqGslVectorClass*       hessianEffect)
{
  //const uqGslVectorClass& meanVector =
  //  *((likelihoodRoutine_DataType *) functionDataPtr)->meanVector;
  //const uqGslMatrixClass& covMatrix  =
  //  *((likelihoodRoutine_DataType *) functionDataPtr)->covMatrix;

  //uqGslVectorClass diffVec(paramValues - meanVector);

  //double resultValue = -0.5*scalarProduct(diffVec, covMatrix.invertMultiply(diffVec));

  double x1 = paramValues[0];

  double mean1  = 10.;
  double sigma1 = 1.;
  double y1 = -(x1-mean1)*(x1-mean1)/(2.*sigma1*sigma1);
  //double z1 = (1./sigma1/sqrt(2*M_PI))*exp(y1);
  //double resultValue = log(z1);
  double resultValue = y1;

  if (resultValue == INFINITY) {
    //std::cerr << "WARNING In likelihoodRoutine"
    //          << ", fullRank "       << paramValues.env().fullRank()
    //          << ", subEnvironment " << paramValues.env().subId()
    //          << ", subRank "        << paramValues.env().subRank()
    //          << ", inter0Rank "     << paramValues.env().inter0Rank()
    //          << ": x1 = "           << x1
    //          << ", z1 = "           << z1
    //          << ", resultValue = "  << resultValue
    //          << std::endl;
    resultValue = 1040.;
  }

  return resultValue;
}
*/

/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDQUESOBayesCalibration::NonDQUESOBayesCalibration(Model& model):
  NonDBayesCalibration(model)
{ }


NonDQUESOBayesCalibration::~NonDQUESOBayesCalibration()
{ }


/** Perform the uncertainty quantification */
void NonDQUESOBayesCalibration::quantify_uncertainty()
{
  // instantiate QUESO objects and execute

  //QuesoRV prior, posterior;
  //Likelihood like_fn;
  //Queso::StatisticalInverseProblem sip(prior, like_fn, posterior);
  //sip.run();
  uqFullEnvironmentClass* env =
    new uqFullEnvironmentClass(MPI_COMM_WORLD,"sip.inp","");

  uqVectorSpaceClass<uqGslVectorClass,uqGslMatrixClass>
    paramSpace(*env, "param_", 2, NULL);

  ////////////////////////////////////////////////////////
  // Step 2 of 5: Instantiate the parameter domain
  ////////////////////////////////////////////////////////
  uqGslVectorClass paramMins(paramSpace.zeroVector());
  paramMins.cwSet(-250.);
  uqGslVectorClass paramMaxs(paramSpace.zeroVector());
  paramMaxs.cwSet( 250.);
  uqBoxSubsetClass<uqGslVectorClass,uqGslMatrixClass>
    paramDomain("param_",paramSpace,paramMins,paramMaxs);

  /*
  ////////////////////////////////////////////////////////
  // Step 3 of 5: Instantiate the likelihood function object
  ////////////////////////////////////////////////////////
  uqGslVectorClass meanVector(paramSpace.zeroVector());
  meanVector[0] = 10.;
  meanVector[1] = 10.;
  uqGslMatrixClass covMatrix(paramSpace.zeroVector());
  covMatrix(0,0) = 1.;
  covMatrix(0,1) = 0.;
  covMatrix(1,0) = 0.;
  covMatrix(1,1) = 1.;
  likelihoodRoutine_DataType likelihoodRoutine_Data;
  likelihoodRoutine_Data.meanVector = &meanVector;
  likelihoodRoutine_Data.covMatrix  = &covMatrix;
  uqGenericScalarFunctionClass<uqGslVectorClass,uqGslMatrixClass>
    likelihoodFunctionObj("like_",
                          paramDomain,
                          likelihoodRoutine,
                          (void *) &likelihoodRoutine_Data,
                          true); // routine computes [ln(function)]
  */

  ////////////////////////////////////////////////////////
  // Step 4 of 5: Instantiate the inverse problem
  ////////////////////////////////////////////////////////
  uqUniformVectorRVClass<uqGslVectorClass,uqGslMatrixClass>
    priorRv("prior_", paramDomain);
  uqGenericVectorRVClass<uqGslVectorClass,uqGslMatrixClass>
    postRv("post_", paramSpace);
  uqStatisticalInverseProblemClass<uqGslVectorClass,uqGslMatrixClass>
    ip("", priorRv, *this, postRv);//, likelihoodFunctionObj, postRv);

  ////////////////////////////////////////////////////////
  // Step 5 of 5: Solve the inverse problem
  ////////////////////////////////////////////////////////
  uqGslVectorClass paramInitials(paramSpace.zeroVector());
  paramInitials[0] = 45.;
  paramInitials[1] = 45.;
  uqGslMatrixClass proposalCovMatrix(paramSpace.zeroVector());
  proposalCovMatrix(0,0) = 1600.;
  proposalCovMatrix(0,1) = 0.;
  proposalCovMatrix(1,0) = 0.;
  proposalCovMatrix(1,1) = 1600.;
  ip.solveWithBayesMetropolisHastings(paramInitials, &proposalCovMatrix);

  // Return
  delete env;
  return;

}


//void NonDQUESOBayesCalibration::print_results(std::ostream& s)
//{
//  NonDBayesCalibration::print_results(s);
//
//  additional QUESO output
//}


Real NonDQUESOBayesCalibration::lnValue(...x...)
{
  // This likelihood aggregates all observables.  For now, one observable at a
  // time (multiple independent inferences) is supported only through multiple
  // independent DAKOTA executions (with a modified analysis_driver to pick out
  // a different metric).  In the future, we might support a loop over response
  // fns as an option.

  //emulatorModel.continuous_variables(x); // TO DO: continuous to start
  emulatorModel.compute_response();
  const RealVector& fn_vals = current_response().function_values();
  // TO DO: triple product -(fn_vals - fn_data)^T C^{-1} (fn_vals - fn_data)
  return -fn_vals.dot(fn_vals); / no data to start
}

} // namespace Dakota
