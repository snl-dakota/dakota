/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "ExperimentData.hpp"
#include "DataMethod.hpp"

namespace Dakota {


/* For the historical case:
     One experiment per line, containing all responses in one line
     no replicates
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

  // Read from a matrix with numExperiments rows and a number of cols
  // columns:  numExpConfigVars X, numFunctions Y, [numFunctions Sigma]
  RealMatrix experimental_data;

  size_t num_cols = numExpConfigVars + numFunctions + numExpStdDeviationsRead;

  TabularIO::read_data_tabular(expDataFileName, context_message, 
			       experimental_data, numExperiments,  num_cols, 
			       expDataFileAnnotated);

  // Get views of the data in 3 matrices for convenience

  size_t start_row, start_col;
  if (numExpConfigVars > 0) {
    start_row = 0;
    start_col = 0;
    RealMatrix x_obs_data(Teuchos::View, experimental_data,
			  numExperiments, numExpConfigVars,
			  start_row, start_col);
    xObsData.reshape(numExperiments, numExpConfigVars);
    for (int i=0; i<numExperiments; i++)
      for (int j=0; j<numExpConfigVars; j++)
        xObsData(i,j) = x_obs_data(i,j);
  }
 
  start_row = 0;
  start_col = numExpConfigVars;
  RealMatrix y_obs_data(Teuchos::View, experimental_data,
			numExperiments, numFunctions,
			start_row, start_col);
  yObsData.reshape(numExperiments, numFunctions);
  for (int i=0; i<numExperiments; i++)
    for (int j=0; j<numFunctions; j++)
      yObsData(i,j) = y_obs_data(i,j);

  // BMA TODO: The number of experimental functions may not match the
  // user functions, so can't assume numFunctions
  yStdData.reshape(numExperiments, numFunctions);
  if (numExpStdDeviationsRead > 0) {
    start_row = 0;
    start_col = numExpConfigVars + numFunctions;
    RealMatrix y_std_data(Teuchos::View, experimental_data,
			  numExperiments, numExpStdDeviationsRead,
			  start_row, start_col);
    // We allow 1 or numFunctions sigmas
    for (int i=0; i<numExperiments; i++)
      for (int j=0; j<numFunctions; j++) {
	if (numExpStdDeviationsRead == 1)
	  yStdData(i,j) = y_std_data(i,0);
	else
	  yStdData(i,j) = y_std_data(i,j);
      }
    // BMA: This is odd to me -- we overwrite the read values with
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
  }
  else if (calc_sigma_from_data) {
    // calculate sigma terms
    Real mean_est, var_est;
    for (int j=0; j<numFunctions; j++){
      mean_est = 0;
      for (int i=0; i<numExperiments; i++)
        mean_est += yObsData(i,j);
      mean_est = mean_est / ((Real)numExperiments);
      var_est = 0;
      for (int i=0; i<numExperiments; i++)
        var_est += (yObsData(i,j)-mean_est)*(yObsData(i,j)-mean_est); 
      // If only one data, point, use 1.0 in the likelihood (no weight)
      for (int i=0; i<numExperiments; i++)
        yStdData(i,j) = (numExperiments > 1) ? 
	  std::sqrt(var_est/(Real)(numExperiments-1)) : 1.0;
    }
  }
  else {
    // Default: use 1.0 in the likelihood (no weight)
    yStdData = 1.0;
  }

}

// Need to be careful not to get a view here...
SingleExperiment::
SingleExperiment(int num_repl, const RealVector& real_config_vars,
		 int sigma_type, const RealVector& sigma_scalar,
		 const RealVector& scalar_data)
  : numReplicates(num_repl), 
    realConfigVars(real_config_vars),
    sigmaType(sigma_type), 
    sigmaScalar(sigma_scalar),
    experimentScalarData(scalar_data)
{
  

}



ExpDataPerResponse::ExpDataPerResponse(int num_exp, int exp_type)
  :numExperiments(num_exp), experimentType(exp_type) 
{
  //dataThisResponse.resize(numExperiments);
}


void ExperimentData::
load_scalar(const std::string& expDataFileName,
	    const std::string& context_message,
	    size_t numExperiments,
	    size_t numExpConfigVars,
	    size_t numFunctions,
	    size_t numExpStdDeviationsRead,
	    bool expDataFileAnnotated,
	    bool calc_sigma_from_data,
	    short verbosity)
{
  RealMatrix xObsData, yObsData, yStdData;
  read_historical_data(expDataFileName, context_message,
		       numExperiments, 
		       numExpConfigVars, numFunctions, numExpStdDeviationsRead,
		       expDataFileAnnotated, calc_sigma_from_data,
		       xObsData, yObsData, yStdData);

  if (verbosity > NORMAL_OUTPUT) {
    Cout << "xobs_data" << xObsData << '\n';
    Cout << "yobs_data" << yObsData << '\n';
    Cout << "ystd_data" << yStdData << '\n';
  }



  // do in phases for now to avoid confusion: add a sized, but empty
  // data object for this response
  int exp_type = SCALAR_DATA;
  ExpDataPerResponse data1response(numExperiments, exp_type);
  allExperiments.assign(numFunctions, data1response);

  // parse each row of the read matrices (each experiment)
  for (size_t f_ind=0; f_ind<numFunctions; ++f_ind) {
    for (size_t exp_ind=0; exp_ind<numExperiments; ++exp_ind) {


      int num_replicates = 1;
      int sigma_type = SCALAR_SIGMA;
      
      // BMA TODO: write a copy_data function?

      RealVector real_config_vars(numExpConfigVars);
      for (int i=0; i<numExpConfigVars; ++i)
	real_config_vars[i] = xObsData(exp_ind, i);

      // scalar data for a single function over 1 replicates
      RealVector scalar_data(num_replicates);
      scalar_data[0] = yObsData(exp_ind, f_ind);

      // sigma data for a single function over 1 replicates
      RealVector sigma_scalar(num_replicates);
      sigma_scalar[0] = yStdData(exp_ind, f_ind);


      // SingleExperiment should be constructible from a single row of the
      // data, but we can't slice it that way easily
      SingleExperiment single_exp(num_replicates, real_config_vars, sigma_type, 
				  sigma_scalar, scalar_data);

      allExperiments[f_ind].dataThisResponse.push_back(single_exp);
    }
  }
}

const RealVector& ExperimentData::
config_vars(size_t response, size_t experiment)
{
  return(allExperiments[response].dataThisResponse[experiment].realConfigVars);
}



Real ExperimentData::
scalar_data(size_t response, size_t experiment, size_t replicate)
{
  if (allExperiments[response].experimentType != SCALAR_DATA) {
    Cerr << "Error (ExperimentData): invalid query of scalar data." << std::endl;
    abort_handler(-1);
  }
  return(allExperiments[response].dataThisResponse[experiment].experimentScalarData[replicate]);
}

Real ExperimentData::
scalar_sigma(size_t response, size_t experiment, size_t replicate)
{
  if (allExperiments[response].experimentType != SCALAR_DATA) {
    Cerr << "Error (ExperimentData): invalid query of scalar data." << std::endl;
    abort_handler(-1);
  }
  return(allExperiments[response].dataThisResponse[experiment].sigmaScalar[replicate]);
}


}  // namespace Dakota
