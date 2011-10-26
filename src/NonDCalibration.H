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

  //
  //- Heading: Data
  //

private:

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
  int numExperiments;
  /// number of columns in data file which are state variables
  int numExpConfigVars;
  /// whether to read numFunctions sigmas from the data file
  bool expDataReadStdDeviations;

};

} // namespace Dakota

#endif
