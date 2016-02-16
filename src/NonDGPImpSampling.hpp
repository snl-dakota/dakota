/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDGPImpSampling
//- Description: Wrapper class for Gaussian Process Importance Sampling
//- Owner:       Laura Swiler and Keith Dalbey
//- Checked by:
//- Version:

#ifndef NOND_GP_IMP_SAMPLING_H
#define NOND_GP_IMP_SAMPLING_H

#include "NonDSampling.hpp"
#include "DataMethod.hpp"

namespace Dakota {

/// Class for the Gaussian Process-based Importance Sampling method 

/** The NonDGPImpSampling implements a method developed by Keith Dalbey 
    that uses a Gaussian process surrogate in the calculation of the 
    importance density.  Specifically, the mean and variance of the 
    GP prediction are used to calculate an expected value that 
    a particular point fails, and that is used as part of the 
    computation of the "draw distribution."  The normalization constants 
    and the mixture distribution used are defined in (need to get 
    SAND report). */

class NonDGPImpSampling: public NonDSampling
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  NonDGPImpSampling(ProblemDescDB& problem_db, Model& model);

  // alternate constructor for sample generation and evaluation "on the fly"
  //NonDGPImpSampling(Model& model, const String& sample_type,
  //		      int samples, int seed, const String& rng,
  //		      short sampling_vars_mode = ACTIVE, 
  //                  const RealVector& lower_bnds,
  //                  const RealVector& upper_bnds);

  /// destructor
  ~NonDGPImpSampling();

  //
  //- Heading: Virtual function redefinitions
  //

  bool resize();
  void derived_init_communicators(ParLevLIter pl_iter);
  void derived_set_communicators(ParLevLIter pl_iter);
  void derived_free_communicators(ParLevLIter pl_iter);

  /// perform the GP importance sampling and return probability of failure
  void core_run();
  
  /// print the final statistics
  void print_results(std::ostream& s);

  //
  //- Heading: Member functions
  //

  /// returns the probability calculated by the importance sampling
  Real final_probability();

protected:
 
private:

  //
  //- Heading: Data
  //

  /// LHS iterator for building the initial GP
  Iterator gpBuild;
  /// LHS iterator for sampling on the GP
  Iterator gpEval;
  /// GP model of response, one approximation per response function
  Model gpModel;
  /// LHS iterator for sampling from the rhoOneDistribution 
  Iterator sampleRhoOne;
  
  /// the number of points added to the original set of LHS samples
  int numPtsAdd;
  /// the total number of points
  int numPtsTotal;
  /// the number of points evaluated by the GP each iteration
  int numEmulEval;
  /// the final calculated probability (p)
  Real finalProb;
  /// Vector to hold the current values of the current sample inputs on the GP
  RealVectorArray gpCvars;
  /// Vector to hold the current values of the current mean estimates 
  /// for the sample values on the GP
  RealVectorArray gpMeans;
  /// Vector to hold the current values of the current variance estimates 
  /// for the sample values on the GP
  RealVectorArray gpVar;
  /// Vector to hold the expected indicator values for the current GP samples
  RealVector expIndicator; 
  /// Vector to hold the rhoDraw values for the current GP samples
  RealVector rhoDraw; 
  /// Vector to hold the normalization constant calculated for each 
  /// point added
  RealVector normConst;
  /// IntVector to hold indicator for actual simulation values vs. threshold
  RealVector indicator;
  /// xDrawThis, appended to locally to hold the X values of emulator points chosen
  RealVectorArray xDrawThis; 
  /// expIndThis, appended locally to hold the expected indicator 
  RealVector expIndThis; 
  /// rhoDrawThis, appended locally to hold the rhoDraw density for calculating draws
  RealVector rhoDrawThis; 
  /// rhoMix, mixture density
  RealVector rhoMix; 
  /// rhoOne, original importance density
  RealVector rhoOne; 
  /// function to calculate the expected indicator probabilities
  RealVector calcExpIndicator(const int respFnCount, const Real respThresh);
  /// function to calculate the expected indicator probabilities for one point
  Real calcExpIndPoint(const int respFnCount, const Real respThresh, const RealVector this_mean, const RealVector this_var);
  /// function to update the rhoDraw data, adding x values and rho draw values
  void calcRhoDraw();
  /// function to pick the next X value to be evaluated by the Iterated model
  RealVector drawNewX(int this_k);
};


inline Real NonDGPImpSampling::final_probability()
{ return finalProb; }

} // namespace Dakota

#endif
