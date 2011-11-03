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
  expStdDeviations(probDescDB.get_rdv("responses.exp_std_deviations")),
  expDataFileName(probDescDB.get_string("responses..exp_data_filename")),
  expDataFileAnnotated(probDescDB.get_bool("responses.exp_data_file_annotated")),
  numExperiments(probDescDB.get_sizet("responses.num_experiments")),
  numExpConfigVars(probDescDB.get_sizet("responses.num_exp_config_vars")),
  numExpStdDeviationsRead(probDescDB.get_sizet("method.num_exp_std_deviations")),
  continuousConfigVars(0), discreteIntConfigVars(0), discreteRealConfigVars(0),
  continuousConfigStart(0), discreteIntConfigStart(0),
  discreteRealConfigStart(0)
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

  short active_view = iteratedModel.current_variables().view().first;

  if (numExpConfigVars > 0) {

    // the difference between these are any left over design/state variables
    // all uncertain should be active
    //size_t total_active_vars = numDesignVars+numUncertainVars+numStateVars;
    size_t total_vars = iteratedModel.current_variables().tv();

    if (active_view == MERGED_ALL || active_view == MIXED_ALL) {
      // all-variables mode
      // in this case total_vars = total_active_vars so none are left for config
      Cerr << "\nError (NonDCalibration): cannot use "
	   << "experimental_config_variables = " << numExpConfigVars 
	   << " read from file since all " << total_vars << " variables are "
	   << "active.  Probably don't want \"all_variables\" mode."
	   << std::endl;
      found_error = true;
    }
    else {
      // we might have valid configuration vars X
      // don't need this check; for debugging
      //if (total_vars - total_active_vars < numExpConfigVars)
      //Cerr << "Not enough left over variables!" << std::endl;

      // available config var counts
      continuousConfigVars =
	probDescDB.get_sizet("variables.continuous_state"); 
      discreteIntConfigVars =
	probDescDB.get_sizet("variables.discrete_state_range") + 
	probDescDB.get_sizet("variables.discrete_state_set_int");
      discreteRealConfigVars =
	probDescDB.get_sizet("variables.discrete_state_set_real"); 

      size_t total_state_config_vars = 
	continuousConfigVars + discreteIntConfigVars + discreteRealConfigVars;

      if (numExpConfigVars > 0 && numExpConfigVars != total_state_config_vars) {
	Cerr << "\nError (NonDCalibration): experimental_config_variables = "
	     << numExpConfigVars << " read from file must equal total state "
	     << "variables = " << total_state_config_vars << std::endl;
	found_error = true;
      }
      else {
	// set indices of the state variables within the all* variable arrays
	// want to skip any design that might be present;
	// plus any uncertain that are active

	// continuous
	size_t cd = probDescDB.get_sizet("variables.continuous_design"); 
	// discrete int
	size_t ddr = probDescDB.get_sizet("variables.discrete_design_range"); 
	size_t ddsi = probDescDB.get_sizet("variables.discrete_design_set_int");
	// discrete real
	size_t ddsr 
	  = probDescDB.get_sizet("variables.discrete_design_set_real");

	continuousConfigStart = 
	  cd + numContAleatUncVars + numContEpistUncVars; 
	discreteIntConfigStart = 
	  ddr + ddsi + numDiscIntAleatUncVars + numDiscIntEpistUncVars; 
	discreteRealConfigStart =
	  ddsr + numDiscRealAleatUncVars + numDiscRealEpistUncVars;
      }

    }

  }

  if (found_error)
    abort_handler(-1);
}


NonDCalibration::~NonDCalibration()
{ }


//void NonDCalibration::print_results(std::ostream& s)
//{ Cout << "Posterior sample results " << '\n'; }

void NonDCalibration::
set_configuration_vars(Model& model, const RealVector& config_vars) {

  // config_vars consists of [continuous, discreteInt, discreteReal] in order

  // current index into configuration variables
  size_t config_ind = 0;
  // where to insert in the all variables array
  size_t state_ins = continuousConfigStart;
  for ( ; config_ind < continuousConfigVars; ++config_ind, ++state_ins)
    model.all_continuous_variable(config_vars[config_ind], state_ins);

  // don't reset the config index
  state_ins = discreteIntConfigStart;
  // TODO: read integer config vars as integers instead of rounding
  for ( ; config_ind < discreteIntConfigVars; ++config_ind, ++state_ins) {
    int int_config_var = (int) std::floor(config_vars[config_ind]+0.5);
    model.all_discrete_int_variable(int_config_var, state_ins);
  }

  // don't reset the config index
  state_ins = discreteRealConfigStart;
  for ( ; config_ind < discreteRealConfigVars; ++config_ind, ++state_ins)
    model.all_discrete_real_variable(config_vars[config_ind], state_ins);

}


} // namespace Dakota
