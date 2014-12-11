/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "ExperimentData.hpp"
#include "DataMethod.hpp"

namespace Dakota {


/* For the historical case:
     One experiment replicate per line, containing all responses in one line
     can't allow different sigma for each response and the 
     config vars are repeated per experiment. */
inline void
read_historical_data(const std::string& expDataFileName,
		     const std::string& context_message,
		     size_t numExperiments,
		     size_t numExpConfigVars,
		     size_t numFunctions,
		     size_t numExpStdDeviationsRead,
		     bool expDataFileAnnotated,
		     bool calc_sigma_from_data,
		     RealMatrix& xObsData,
		     RealMatrix& yObsData, 
		     RealMatrix& yStdData)
{
  //using boost::multi_array;
  //using boost::extents;
  size_t i,j,k, total_num_rows=0;
  //for now, numExperiments are the same for all functions
  total_num_rows=numExperiments;
  Cout << "\nReading Experimental Data." << '\n';
  Cout << "Total number of rows " << total_num_rows << '\n';
  Cout << "Number of Response Functions " << numFunctions << '\n';
  //yObsFull(extents[numFunctions][numExperiments][max_replicates]);

  // Read from a matrix with numExperiments rows and a number of cols
  // columns:  numExpConfigVars X, numFunctions Y, [numFunctions Sigma]
  RealMatrix experimental_data;

  size_t num_cols = numExpConfigVars + numFunctions + numExpStdDeviationsRead;

  TabularIO::read_data_tabular(expDataFileName, context_message, 
			       experimental_data, total_num_rows, num_cols, 
			       expDataFileAnnotated);

  // Get views of the data in 3 matrices for convenience

  size_t start_row, start_col;
  if (numExpConfigVars > 0) {
    start_row = 0;
    start_col = 0;
    RealMatrix x_obs_data(Teuchos::View, experimental_data,
			  total_num_rows, numExpConfigVars,
			  start_row, start_col);
    xObsData.reshape(total_num_rows, numExpConfigVars);
    for (i=0; i<total_num_rows; i++)
      for (j=0; j<numExpConfigVars; j++)
        xObsData(i,j) = x_obs_data(i,j);
  }
 
  start_row = 0;
  start_col = numExpConfigVars;
  RealMatrix y_obs_data(Teuchos::View, experimental_data,
			total_num_rows, numFunctions,
			start_row, start_col);
  yObsData.reshape(numExperiments,numFunctions); 
  for (i=0; i<numExperiments; i++){
    for (j=0; j<numFunctions; j++){
        yObsData(i,j) = y_obs_data(i,j);
        Cout << yObsData(i,j) << "\n";
    }
  }
  // BMA TODO: The number of experimental functions may not match the
  // user functions, so can't assume numFunctions
  yStdData.reshape(numExperiments,numFunctions);
  if (numExpStdDeviationsRead > 0) {
    start_row = 0;
    start_col = numExpConfigVars + numFunctions;
    RealMatrix y_std_data(Teuchos::View, experimental_data,
			  total_num_rows, numExpStdDeviationsRead,
			  start_row, start_col);
    // We allow 1 or numFunctions sigmas
    for (i=0; i<numExperiments; i++){
      for (j=0; j<numFunctions; j++){
	  if (numExpStdDeviationsRead == 1)
            yStdData(i,j) = y_std_data(i,0);
          else 
            yStdData(i,j) = y_std_data(i,j);
          //Cout << yStdData[j];
      }
    }
  }
    // user values?  Commenting out as we don't currently support
    // input file-specified errors.
    // if (expStdDeviations.length()==1) {
    //   for (int i=0; i<numExperiments; i++)
    //     for (int j=0; j<numFunctions; j++)
    //       yStdData(i,j) = expStdDeviations(0);
    // }
    // else if (expStdDeviations.length()==numFunctions) {
    //   for (int i=0; i<numExperiments; i++)
    //     for (int j=0; j<numFunctions; j++)
    //       yStdData(i,j) = expStdDeviations(j);
    // }
  else if (calc_sigma_from_data) {
    // calculate sigma terms
    Real mean_est, var_est;
    for (j=0; j<numFunctions; j++){
      mean_est = 0.0;
      for (i=0; i<numExperiments; i++) 
        mean_est += yObsData(i,j);
      mean_est = mean_est / ((Real)(numExperiments));
      var_est = 0;
      for (i=0; i<numExperiments; i++) 
        var_est += (yObsData(i,j)-mean_est)*(yObsData(i,j)-mean_est); 
      for (i=0; i<numExperiments; i++) 
        yStdData(i,j) = (numExperiments > 1) ? 
	  std::sqrt(var_est/(Real)(numExperiments-1)) : 1.0;
      for (i=0; i<numExperiments; i++) 
        Cout << yStdData(i,j) << "\n";
    }
  }
  else {
    // Default: use 1.0 in the likelihood (no weight)
    for (i=0; i<numExperiments; i++)
      for (j=0; j<numFunctions; j++){
        yStdData(i,j) = 1.0;
        Cout << yStdData(i,j) << "\n";
      }
  }
}

void ExperimentData:: 
load_data(const std::string& expDataFileName,
	    const std::string& context_message,
	    size_t numExperiments,
	    size_t numExpConfigVars,
	    size_t numFunctions,
	    size_t numExpStdDeviationsRead,
	    bool expDataFileAnnotated,
	    bool calc_sigma_from_data,
	    short verbosity, 
            const SharedResponseData& sim_shared_data)
{
  
  // allExperiments.resize(numExperiments);
  allConfigVars.resize(numExperiments);

  //TO DO:  Change the argument list to load_data, account for reading 
  //scalar data and field data.  This currently reads only scalar data 
  //in the old-style format.
  RealMatrix xObsData;
  RealMatrix yObsData, yStdData;
  
  read_historical_data(expDataFileName, context_message,
		       numExperiments,
		       numExpConfigVars, numFunctions, numExpStdDeviationsRead,
		       expDataFileAnnotated, calc_sigma_from_data,
		       xObsData, yObsData, yStdData);
  sigmaScalarValues=yStdData;
  
  if (verbosity > VERBOSE_OUTPUT) {
    Cout << "xobs_data" << xObsData << '\n';
      Cout << "yobs_data" << yObsData << '\n';
      Cout << "ystd_data" << yStdData << '\n';
      Cout << "ytemp_sigma" << sigmaScalarValues << '\n';
  }
  // Get a copy of the simulation SRD to use in contructing the
  // experiment response
  SharedResponseData exp_srd = sim_shared_data.copy();

  // change the type of response
  exp_srd.response_type(EXPERIMENT_RESPONSE);
  std::cout << "Construct experiment response" << std::endl;
  Response exp_resp(exp_srd);
  exp_resp.write(std::cout);

  // this is the main loop which we will augment as we add capability
  for (size_t exp_index = 0; exp_index < numExperiments; ++exp_index) {
    //READ ONE ROW OF SCALAR DATA 
    //allConfigVars[exp_index]=xObsData[exp_index];
   
    for (size_t fn_index = 0; fn_index < numFunctions; ++fn_index) {
      exp_resp.function_value(yObsData(exp_index, fn_index),fn_index); 
    }
    //NEED TO SET COVARIANCE MATRIX CORRECTLY.  The following case is for scalars
    //exp_resp.set_scalar_covariance(curr_sigmas);

    // READ ONE FIELD DATA RECORD and set the corresponding field data values
    //for (size_t fn_index = 0; fn_index < numFields; ++fn_index) {
    //  exp_resp.field_value(proper indexing);
    //}yObsData(exp_index, fn_index),fn_index); 
    Cout << "CurrExp " << exp_resp.function_values() << '\n';
    allExperiments.push_back(exp_resp.copy());
  }
  // verify that the two experiments have different data
  for (size_t i=0; i<numExperiments; ++i) {
     std::cout << "vec_exp #" << i << std::endl;
     allExperiments[i].write(std::cout);
  }

 
}

const RealVector& ExperimentData::
config_vars(size_t experiment)
{
  return(allConfigVars[experiment]);
}

Real ExperimentData::
scalar_data(size_t response, size_t experiment)
{
  //if (allExperiments[response].experimentType != SCALAR_DATA) {
  //  Cerr << "Error (ExperimentData): invalid query of scalar data." << std::endl;
  //  abort_handler(-1);
  //}
  return(allExperiments[experiment].function_value(response));
}

Real ExperimentData::
scalar_sigma(size_t response, size_t experiment)
{
  //if (allExperiments[response].experimentType != SCALAR_DATA) {
  //  Cerr << "Error (ExperimentData): invalid query of scalar data." << std::endl;
  //  abort_handler(-1);
  //}
  //return(allExperiments[experiment].get_scalar_covariance(response));
  return(sigmaScalarValues(experiment, response));
}


}  // namespace Dakota
