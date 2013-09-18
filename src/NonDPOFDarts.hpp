/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDPOFDarts
//- Description: Class for the Probability of Failure DARTS approach
//- Owner:	 Mohamed Ebeida and Laura Swiler
//- Checked by:
//- Version:

#ifndef NOND_POFDARTS_H
#define NOND_POFDARTS_H

#include "dakota_data_types.hpp"
#include "DakotaNonD.hpp"

namespace Dakota {


/// Base class for POF Dart methods within DAKOTA/UQ

/** The NonDPOFDart class implements the calculation of a failure 
    probability for a specified threshold for a specified response
    function using the concepts developed by Mohamed Ebeida. 
    The approach works by throwing down a number of Poisson disk samples 
    of varying radii, and identifying each disk as either in the  
    failure or safe region. The center of each disk represents a "true" 
    function evaluation. kd-darts are used to place additional 
    points, in such a way to target the failure region.  When the 
    disks cover the space sufficiently, Monte Carlo methods or a 
    box volume approach is used to calculate both the lower and 
    upper bounds on the failure probability. */ 

class NonDPOFDarts: public NonD
{
public:

  //
  //- Heading: Constructors and destructor
  //

  NonDPOFDarts(Model& model); ///< constructor
  ~NonDPOFDarts();            ///< destructor

  //
  //- Heading: Member functions
  //

  /// perform POFDart analysis and return probability of failure 
  void quantify_uncertainty(); // called by run_iterator

protected: 
  //
  //- Heading: Convenience functions
  //
  
  //
  //- Heading: Data
  //
  // number of samples of true function
  int samples; 
  // user-specified seed 
  int seed;

};

} // namespace Dakota

#endif
