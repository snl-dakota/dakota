/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
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

#include "NonDSampling.H"
#include "DataMethod.H"

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

  NonDGPImpSampling(Model& model); ///< standard constructor

  /// alternate constructor for sample generation and evaluation "on the fly"
  //NonDGPImpSampling(Model& model, const String& sample_type,
  //		  int samples, int seed, const String& rng,
  //		  short sampling_vars_mode = ACTIVE, 
  //                  const RealVector& lower_bnds, const RealVector& upper_bnds
  //                             );

  ~NonDGPImpSampling(); ///< destructor

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  /// perform the GP importance sampling and return probability of failure.
  void quantify_uncertainty();
  
  /// returns the probability calculated by the importance sampling
  const Real& get_probability();
 

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
  
  /// the number of points added to the original set of LHS samples
  int numPtsAdd;
  /// the total number of points
  int numPtsTotal;
  /// the number of points evaluated by the GP each iteration
  int numEmulEval;
  /// rhoDrawThis
  RealVector rhoDrawThis; 
  /// the final calculated probability (p)
  Real finalProb;

};


inline const Real& NonDGPImpSampling::get_probability()
{ return finalProb; }


} // namespace Dakota

#endif
