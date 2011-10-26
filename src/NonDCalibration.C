/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDCalibration
//- Description: Base class for nondeterministic calibration
//- Owner:       Laura Swiler
//- Checked by:
//- Version:

#include "NonDCalibration.H"
#include "DakotaModel.H"
#include "ProblemDescDB.H"

static const char rcsId[]="@(#) $Id$";


namespace Dakota {

/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDCalibration::NonDCalibration(Model& model): NonD(model),
  expStdDeviations(probDescDB.get_rdv("method.exp_std_deviations")),
  expDataFileName(probDescDB.get_string("method.exp_data_filename")),
  expDataFileAnnotated(probDescDB.get_bool("method.exp_data_file_annotated")),
  numExperiments(probDescDB.get_int("method.num_experiments")),
  numExpConfigVars(probDescDB.get_int("method.num_exp_config_vars")),
  expDataReadStdDeviations(probDescDB.get_bool("method.exp_data_read_std_deviations"))
{ 
  bool found_error = false;

  if (expStdDeviations.length() != 0 && expStdDeviations.length() != 1 && 
      expStdDeviations.length() != numFunctions) {
    Cerr << "\nError (NonDCalibration): vector of experimental standard "
	 << "deviations must have length 0, 1, or " << numFunctions 
	 << "(number of responses)." << std::endl;
    found_error = true;
  }

  // for backward compatibility with single experiment data files
  if (!expDataFileName.empty() && numExperiments == 0)
    numExperiments = 1;

  if (numExpConfigVars > 0 && numExpConfigVars != numStateVars) {
    Cerr << "\nError (NonDCalibration): experimental_config_variables = "
	 << numExpConfigVars << " read from file must equal total state "
	 << "variables = " << numStateVars << std::endl;
    found_error = true;
  }

  if (found_error)
    abort_handler(-1);
}


NonDCalibration::~NonDCalibration()
{ }


//void NonDCalibration::print_results(std::ostream& s)
//{ Cout << "Posterior sample results " << '\n'; }

} // namespace Dakota
