/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDCalibration
//- Description: Base class for generic ian inference
//- Owner:       Laura Swiler
//- Checked by:
//- Version:

#ifndef NOND_CALIBRATION_H
#define NOND_CALIBRATION_H

#include "DakotaNonD.H"


namespace Dakota {


/// 

/** This class ...  */

class NonDCalibration: public NonD
{
public:

  //
  //- Heading: Constructors and destructor
  //

  NonDCalibration(Model& model); ///< standard constructor
  ~NonDCalibration();            ///< destructor

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  //void print_results(std::ostream& s);

  /// set the passed configuration variables into the model's state vars
  void set_configuration_vars(Model& model, const RealVector& x);

  //
  //- Heading: Data
  //

  /// 1 or numFunctions standard deviations
  RealVector expStdDeviations;
  /// filename from which to read experimental data; optionally
  /// configuration vars x and standard deviations sigma
  String expDataFileName;
  /// whether the data file is in annotated format
  bool expDataFileAnnotated;
  /// number of experiments to read from data file
  size_t numExperiments;
  /// number of columns in data file which are state variables
  size_t numExpConfigVars;
  /// how many sigmas to read from the data file (1 or numFunctions)
  size_t numExpStdDeviationsRead;

private:

  //
  //- Heading: Data
  //

  // number of configuration variables of each type
  size_t continuousConfigVars;
  size_t discreteIntConfigVars;
  size_t discreteRealConfigVars;
  
  // start indices for each type of config variable
  size_t continuousConfigStart;
  size_t discreteIntConfigStart;
  size_t discreteRealConfigStart;

};

} // namespace Dakota

#endif
