/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "ExperimentData.hpp"
#include "DataMethod.hpp"

namespace Dakota {

ExperimentData::ExperimentData(short output_level): 
  outputLevel(output_level)
{ /* empty ctor */ }                                

void ExperimentData::shared_data(const SharedResponseData& srd)
{ simulationSRD = srd.copy(); }

void ExperimentData::num_experiments(size_t num_experiments_in)
{ numExperiments = num_experiments_in; }

void ExperimentData::num_config_vars(size_t num_config_vars_in)
{ numConfigVars = num_config_vars_in; }

void ExperimentData::num_sigma(size_t num_sigma_in)
{ numSigma = num_sigma_in; }

void ExperimentData::sigma_type(const StringArray& sigma_type_in)
{ sigmaType = sigma_type_in; }


/* For the historical case:
     One experiment replicate per line, containing all responses in one line
     can't allow different sigma for each response and the 
     config vars are repeated per experiment. */
void ExperimentData::read_historical_data(const std::string& expDataFileName,
					  const std::string& context_message,
					  bool expDataFileAnnotated,
					  bool calc_sigma_from_data,
					  RealMatrix& xObsData,
					  RealMatrix& yObsData, 
					  RealMatrix& yStdData)
{
  //using boost::multi_array;
  //using boost::extents;
  size_t i,j,k, total_num_rows = 0;
  size_t num_functions = simulationSRD.num_scalar_responses();
  //for now, numExperiments are the same for all functions
  total_num_rows=numExperiments;
  Cout << "\nReading Experimental Data." << '\n';
  Cout << "Total number of rows " << total_num_rows << '\n';
  Cout << "Number of Response Functions " << num_functions << '\n';
  //yObsFull(extents[num_functions][numExperiments][max_replicates]);

  // Read from a matrix with numExperiments rows and a number of cols
  // columns:  numConfigVars X, num_functions Y, [num_functions Sigma]
  RealMatrix experimental_data;

  size_t num_cols = numConfigVars + num_functions + numSigma;

  TabularIO::read_data_tabular(expDataFileName, context_message, 
			       experimental_data, total_num_rows, num_cols, 
			       expDataFileAnnotated);

  // Get views of the data in 3 matrices for convenience

  size_t start_row, start_col;
  if (numConfigVars > 0) {
    start_row = 0;
    start_col = 0;
    RealMatrix x_obs_data(Teuchos::View, experimental_data,
			  total_num_rows, numConfigVars,
			  start_row, start_col);
    xObsData.reshape(total_num_rows, numConfigVars);
    for (i=0; i<total_num_rows; i++)
      for (j=0; j<numConfigVars; j++)
        xObsData(i,j) = x_obs_data(i,j);
  }
 
  start_row = 0;
  start_col = numConfigVars;
  RealMatrix y_obs_data(Teuchos::View, experimental_data,
			total_num_rows, num_functions,
			start_row, start_col);
  yObsData.reshape(numExperiments,num_functions); 
  for (i=0; i<numExperiments; i++){
    for (j=0; j<num_functions; j++){
        yObsData(i,j) = y_obs_data(i,j);
        Cout << yObsData(i,j) << "\n";
    }
  }
  // BMA TODO: The number of experimental functions may not match the
  // user functions, so can't assume num_functions
  yStdData.reshape(numExperiments,num_functions);
  if (numSigma > 0) {
    start_row = 0;
    start_col = numConfigVars + num_functions;
    RealMatrix y_std_data(Teuchos::View, experimental_data,
			  total_num_rows, numSigma,
			  start_row, start_col);
    // We allow 1 or num_functions sigmas
    for (i=0; i<numExperiments; i++){
      for (j=0; j<num_functions; j++){
	  if (numSigma == 1)
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
    //     for (int j=0; j<num_functions; j++)
    //       yStdData(i,j) = expStdDeviations(0);
    // }
    // else if (expStdDeviations.length()==num_functions) {
    //   for (int i=0; i<numExperiments; i++)
    //     for (int j=0; j<num_functions; j++)
    //       yStdData(i,j) = expStdDeviations(j);
    // }
  else if (calc_sigma_from_data) {
    // calculate sigma terms
    Real mean_est, var_est;
    for (j=0; j<num_functions; j++){
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
      for (j=0; j<num_functions; j++){
        yStdData(i,j) = 1.0;
        Cout << yStdData(i,j) << "\n";
      }
  }
}


void ExperimentData:: 
load_data(const std::string& expDataFileName,
	  const std::string& context_message,
	  bool expDataFileAnnotated,
	  bool calc_sigma_from_data)
{
  
  // TODO: Change the argument list to load_data, account for reading
  // scalar data and field data; get all needed data from the problem
  // DB, likely at construct time, delaying read.
  bool scalar_data_file = true;   // hard-wire to read scalar data
  bool read_field_coords = false;


  // For now, read historical data in the old-style format.
  // TODO: Read one row of scalar data at a time in the experiment loop below
  RealMatrix xObsData, yObsData, yStdData;
  if (scalar_data_file) {
    read_historical_data(expDataFileName, context_message,
			 expDataFileAnnotated, calc_sigma_from_data,
			 xObsData, yObsData, yStdData);
    // sigmaScalarValues will be replaced by scalar entries in ExperimentCovariance
    sigmaScalarValues = yStdData;
    if (outputLevel > VERBOSE_OUTPUT) {
      Cout << "xobs_data" << xObsData << '\n';
      Cout << "yobs_data" << yObsData << '\n';
      Cout << "ystd_data" << yStdData << '\n';
      Cout << "ytemp_sigma" << sigmaScalarValues << '\n';
    }
  }

  // Get a copy of the simulation SRD to use in contructing the
  // experiment response; won't be able to share the core data since
  // different experiments may have different sizes...
  SharedResponseData exp_srd = simulationSRD.copy();

  // change the type of response
  exp_srd.response_type(EXPERIMENT_RESPONSE);
  std::cout << "Construct experiment response" << std::endl;
  Response exp_resp(exp_srd);
  exp_resp.write(std::cout);

  if (numConfigVars > 0)
    allConfigVars.resize(numExperiments);

  // Count number of each sigma type for sizing
  //
  // TODO: If "none" is specified, map to appropriate type.  For field
  // data, if "scalar" or "none" is specified, need to convert to a
  // diagonal the length of the actual data
  size_t num_sigma_matrices = 0, num_sigma_diagonals = 0, num_sigma_scalars = 0;
  StringArray::const_iterator st_it = sigmaType.begin();
  StringArray::const_iterator st_end = sigmaType.end();
  for ( ; st_it != st_end; ++st_it) {
    if (*st_it == "matrix")
      ++num_sigma_matrices;
    else if (*st_it == "diagonal")
      ++num_sigma_diagonals;
    else if (*st_it == "scalar")
      ++num_sigma_scalars;
  }

  for (size_t exp_index = 0; exp_index < numExperiments; ++exp_index) {

    // TODO: Read one row of scalar data at a time, instead of reading all above

    // -----
    // Read and set the configuration variables
    // -----

    if (numConfigVars > 0) {
      allConfigVars[exp_index].sizeUninitialized(numConfigVars);
      if (scalar_data_file) {
	for (int var_i=0; var_i<numConfigVars; ++var_i)
	  allConfigVars[exp_index][var_i] = xObsData(exp_index, var_i);
      }
      else {
	// load configuration variables from field data formatted files
	// response_label.exp_num.config (ideally do both if present and
	// validate against scalar if discrepancies)
	// TODO: read_config_vars_multifile(); ?!?
      }
    }

    // read one file per field response, resize, and populate the
    // experiment (values, sigma, coords)
    load_experiment(exp_index, yObsData, yStdData, 
		    num_sigma_matrices, num_sigma_diagonals, num_sigma_scalars,
		    exp_resp);

    Cout << "CurrExp " << exp_resp.function_values() << '\n';
    allExperiments.push_back(exp_resp.copy());
  }
  // verify that the two experiments have different data
  for (size_t i=0; i<numExperiments; ++i) {
     std::cout << "vec_exp #" << i << std::endl;
     allExperiments[i].write(std::cout);
  }
 
 
}

// The field length is given by the data read. This is awkward as the
// full response size won't be known until the whole read is
// complete....  (field length is given by Response, not Shared,
// probably)
/** Load an experiment from a mixture of legacy format data passed in,
    and field data format files read in during this function call */
void ExperimentData::
load_experiment(size_t exp_index, const RealMatrix& yobs_data, 
		const RealMatrix& ystd_data, size_t num_sigma_matrices, 
		size_t num_sigma_diagonals, size_t num_sigma_scalars,
		Response& exp_resp)
{
  bool scalar_data_file = true;

  size_t num_scalars = simulationSRD.num_scalar_responses();
  size_t num_fields = simulationSRD.num_field_response_groups();
  size_t num_resp = num_scalars + num_fields;

  // TODO: populate from DB
  UShortArray sigma_type(num_resp, NO_SIGMA);

  const StringArray& fn_labels = exp_resp.function_labels();

  // -----
  // Data to populate from files for a single experiment
  // -----
  
  // scalar or field values; the RealVectors for scalars will have
  // length 1; the length of RealVectors for fields will be determined
  // by the file read
  RealVectorArray exp_values(num_scalars + num_fields);

  // field lengths may differ for each experiment
  IntVector field_lengths(num_fields, 0);

  // coordinates for fields only
  RealMatrixArray exp_coords(num_fields);

  // Data for sigma 
  std::vector<RealMatrix> sigma_matrices(num_sigma_matrices);
  std::vector<RealVector> sigma_diagonals(num_sigma_diagonals);
  RealVector sigma_scalars(num_sigma_scalars);
  // indices for the entries in the above data structures
  IntVector matrix_map_indices(num_sigma_matrices), 
    diagonal_map_indices(num_sigma_diagonals),
    scalar_map_indices(num_sigma_scalars);


  // -----
  // Read the data
  // -----

  // populate scalar data function and sigma values; translate data
  // from historical to new format (later collapse and eliminate
  // copies) TODO: advance a row of exp data in outer context and pass
  // in here to simplify these two cases
  if (scalar_data_file) {
    for (size_t fn_index = 0; fn_index < num_scalars; ++fn_index) {
      exp_values[fn_index].resize(1);
      exp_values[fn_index][0] = yobs_data(exp_index, fn_index);
      sigma_scalars[fn_index] = ystd_data(exp_index, fn_index);
    }
  }
  else {
    for (size_t fn_index = 0; fn_index < num_scalars; ++fn_index) {
      const String& fn_name = fn_labels[fn_index];
      // TODO: read data directly into values array from
      // fn_name.exp_num.dat
      read_field_values(fn_name, exp_index+1, exp_values[fn_index]);
      // TODO: error if length read != 1
      
      if (sigma_type[fn_index] == SCALAR_SIGMA) {
	Real sigma_val; // = TODO: read sigma data (single value) from fn_name.exp_num.sigma
	sigma_scalars[fn_index] = sigma_val; 
	scalar_map_indices[fn_index] = fn_index;
      }
      else {
	// TODO: if none, default population
      }
    }
  }

  // populate field data, sigma, and coordinates from separate files
  for (size_t field_index = 0; field_index < num_fields; ++field_index) {
    size_t fn_index = num_scalars + field_index;
    const String& fn_name = fn_labels[fn_index];

    // read a column vector of field values for this field from
    // fn_name.exp_num.dat
    read_field_values(fn_name, exp_index+1, exp_values[fn_index]);
    field_lengths[field_index] = exp_values[fn_index].length();

    // read coordinates from field_name.exp_num.coords and validate
    // number of rows is field_lengths[field_index]
    read_coord_values(fn_name, exp_index+1, exp_coords[field_index]);
    // TODO: check length
         
    // read sigma 1, N (field_lengths[field_index]), or N^2 values
    switch(sigma_type[fn_index]) {
    case SCALAR_SIGMA:
      // read single value, add to sigma_scalars and add num_scalars +
      // field_index to scalar map
      break;

      //    case VECTOR_SIGMA:
      // read N values, add to sigma_diagonals and add num_scalars +
      // field_index to diagonals map
      //      break;

    case COVARIANCE_MATRIX:
      // read N^2 values, add to sigma_matrices and add num_scalars +
      // field_index to matrices map
      break;

    }

  }

  // -----
  // Reshape the experiment
  // -----

  // now that we've read, we know the total length and can resize and populate
  // TODO: reshape that accepts just function resizing, or better,
  // just field resizing... There's a chicken and egg problem because
  // reshape() for SharedResponseData needs field lengths, but also
  // uses its current value.  Probably add a reshape that take the lengths.
  // size_t total_field_length = 
  //   std::accumulate(field_lengths.begin(), field_lengths.end());
  // exp_resp.reshape(num_scalars + total_field_length);

  
  // -----
  // Map in the data
  // -----

  for (size_t fn_index = 0; fn_index < num_scalars; ++fn_index) {
    exp_resp.function_value(exp_values[fn_index][0], fn_index);
  }

  for (size_t field_ind = 0; field_ind < num_fields; ++field_ind) {
    exp_resp.field_values(exp_values[num_scalars + field_ind], num_scalars + field_ind);
  }

  exp_resp.set_full_covariance(sigma_matrices, sigma_diagonals, sigma_scalars,
			       matrix_map_indices, diagonal_map_indices, scalar_map_indices);

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
