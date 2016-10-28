/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
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

#include "DakotaNonD.hpp"
#include "ExperimentData.hpp"

namespace Dakota {


/// 

/** This class ...  */

class NonDCalibration: public NonD
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  NonDCalibration(ProblemDescDB& problem_db, Model& model);
  /// destructor
  ~NonDCalibration();

  //
  //- Heading: Virtual function redefinitions
  //
  
  bool resize();

protected:

  //void print_results(std::ostream& s);

  //
  //- Heading: Data
  //

  /// flag indicating whether there is calibration data present
  bool calibrationData;

  /// Container for experimental data to which to calibrate model
  ExperimentData expData;

private:

  //
  //- Heading: Member functions
  //

  //
  //- Heading: Data
  //

};

} // namespace Dakota

#endif
