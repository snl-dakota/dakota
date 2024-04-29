/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

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

  //void print_results(std::ostream& s, short results_state = FINAL_RESULTS);

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


inline NonDCalibration::~NonDCalibration()
{ }

} // namespace Dakota

#endif
