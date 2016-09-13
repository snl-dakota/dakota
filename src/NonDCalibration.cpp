/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDCalibration
//- Description: Base class for nondeterministic calibration
//- Owner:       Laura Swiler
//- Checked by:
//- Version:

#include "NonDCalibration.hpp"
#include "DakotaModel.hpp"
#include "ProblemDescDB.hpp"

static const char rcsId[]="@(#) $Id$";


namespace Dakota {

/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDCalibration::NonDCalibration(ProblemDescDB& problem_db, Model& model):
  NonD(problem_db, model),
  calibrationData(probDescDB.get_bool("responses.calibration_data") ||
    !probDescDB.get_string("responses.scalar_data_filename").empty()),
  expData(problem_db, iteratedModel.current_response().shared_data(), 
	  outputLevel)
{ 
  // Read in all of the experimental data, including any x configuration 
  // variables, y observations, and covariance information if available 
  //if (outputLevel > NORMAL_OUTPUT)
  //  Cout << "Read data from file " << calibrationData << '\n';
  if (calibrationData)
    expData.load_data("NonDCalibration");
  else if (outputLevel > SILENT_OUTPUT)
    Cout << "No experiment data from files.\nCalibration is assuming the "
	 << "simulation is returning the residuals" << std::endl;
}


NonDCalibration::~NonDCalibration()
{ }

bool NonDCalibration::resize()
{
  bool parent_reinit_comms = NonD::resize();

  Cerr << "\nError: Resizing is not yet supported in method "
       << method_enum_to_string(methodName) << "." << std::endl;
  abort_handler(METHOD_ERROR);

  return parent_reinit_comms;
}


//void NonDCalibration::print_results(std::ostream& s)
//{ Cout << "Posterior sample results " << '\n'; }


} // namespace Dakota
