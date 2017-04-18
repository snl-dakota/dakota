/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "ExperimentData.hpp"
#include "DataMethod.hpp"
#include "ProblemDescDB.hpp"

namespace Dakota {

ExperimentData::ExperimentData():
  calibrationDataFlag(false), numExperiments(0), numConfigVars(0), 
  covarianceDeterminant(1.0), logCovarianceDeterminant(0.0),
  scalarDataFormat(TABULAR_EXPER_ANNOT), scalarSigmaPerRow(0),
  readSimFieldCoords(false), interpolateFlag(false), outputLevel(NORMAL_OUTPUT)
{  /* empty ctor */  }                                


ExperimentData::
ExperimentData(const ProblemDescDB& pddb, 
               const SharedResponseData& srd, short output_level):
  calibrationDataFlag(pddb.get_bool("responses.calibration_data")),
  numExperiments(pddb.get_sizet("responses.num_experiments")), 
  numConfigVars(pddb.get_sizet("responses.num_config_vars")),
  covarianceDeterminant(1.0), logCovarianceDeterminant(0.0),
  scalarDataFilename(pddb.get_string("responses.scalar_data_filename")),
  scalarDataFormat(pddb.get_ushort("responses.scalar_data_format")),
  scalarSigmaPerRow(0), 
  readSimFieldCoords(pddb.get_bool("responses.read_field_coordinates")), 
  interpolateFlag(pddb.get_bool("responses.interpolate")),
  outputLevel(output_level)
{ 
  initialize(pddb.get_sa("responses.variance_type"), srd);
}                                


ExperimentData::
ExperimentData(size_t num_experiments, size_t num_config_vars, 
               const boost::filesystem::path& data_prefix,
               const SharedResponseData& srd,
               const StringArray& variance_types,
               short output_level,
               std::string scalar_data_filename):
  calibrationDataFlag(true), numExperiments(num_experiments),
  numConfigVars(num_config_vars),
  covarianceDeterminant(1.0), logCovarianceDeterminant(0.0),
  dataPathPrefix(data_prefix), scalarDataFilename(scalar_data_filename),
  scalarDataFormat(TABULAR_EXPER_ANNOT), scalarSigmaPerRow(0),
  readSimFieldCoords(false), interpolateFlag(false), outputLevel(output_level)
{
  initialize(variance_types, srd);
}

ExperimentData::
ExperimentData(size_t num_experiments, const SharedResponseData& srd,
               const RealMatrix& config_vars,
               const IntResponseMap& all_responses, short output_level):
  calibrationDataFlag(false), numExperiments(num_experiments),
  numConfigVars(config_vars.numRows()),
  covarianceDeterminant(1.0), logCovarianceDeterminant(0.0),
  scalarDataFormat(TABULAR_EXPER_ANNOT), scalarSigmaPerRow(0),
  readSimFieldCoords(false), interpolateFlag(false), outputLevel(output_level)
{
  simulationSRD = srd.copy();
  allConfigVars.resize(numExperiments);
  for (size_t i=0; i<numExperiments; ++i) {
    allConfigVars[i] =
      Teuchos::getCol(Teuchos::Copy, const_cast<RealMatrix&>(config_vars),
                      (int) i);
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << " allConfigVars i " << allConfigVars[i] << '\n';
  }
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Number of config vars " << numConfigVars << '\n';

  // BMA TODO: This doesn't make an object of type ExperimentResponse!
  IntRespMCIter resp_it = all_responses.begin();
  IntRespMCIter resp_end = all_responses.end();
  for ( ; resp_it != resp_end; ++resp_it)
     allExperiments.push_back(resp_it->second);
}


void ExperimentData::initialize(const StringArray& variance_types, 
				const SharedResponseData& srd)
{
  // only initialize data if needed; TODO: consider always initializing
  if (calibrationDataFlag || !scalarDataFilename.empty()) {

    if (outputLevel > NORMAL_OUTPUT) {
      Cout << "Constructing ExperimentData with " << numExperiments 
	   << " experiment(s).";
      if (!scalarDataFilename.empty())
	Cout << "\n  Scalar data file name: '" << scalarDataFilename << "'";
      Cout << std::endl;
    }

    if (interpolateFlag) {
      // can't interpolate if there are no simulation coordinates
      if (!readSimFieldCoords) {
	Cerr << "\nError: calibration data 'interpolate' option not available " 
	     << "if simulation coordinates are not read in also. " 
             << "Please specify simulation coordinates with read_field_coordinates.\n";
	abort_handler(-1);
      }
         
      // can't use normInf as the vector is a 1 x num_fields matrix
      bool multiple_coords = false;
      const IntVector coords_per_field = srd.num_coords_per_field();
      for (size_t f_ind = 0; f_ind < coords_per_field.length(); ++f_ind)
	if (coords_per_field[f_ind] > 1) {
	  multiple_coords = true;
	  break;
	}
      if (multiple_coords) {
	Cerr << "\nError: calibration data 'interpolate' option not available " 
	     << "for fields with\n       more than 1 independent coordinate.\n";
	abort_handler(-1);
      }
    }

    // for now, copy in case any recasting between construct and read;
    // don't want to share a rep, or do we?
    simulationSRD = srd.copy();

    parse_sigma_types(variance_types);
  }
  else
    {
      experimentLengths.sizeUninitialized(1);
      experimentLengths[0] = srd.num_functions();
      expOffsets.size(1); // init to 0
    }
}

/** Validate user-provided sigma specifcation. User can specify 0, 1,
    or num_response_groups sigmas.  If specified, sigma types must be
    the same for all scalar responses. */
void ExperimentData::parse_sigma_types(const StringArray& sigma_types)
{
  // leave array empty if not needed (could have many responses and no sigmas)
  if (sigma_types.size() == 0)
    return;

  // valid options for sigma_type, and mapping to enum
  std::map<String, unsigned short> sigma_map;
  sigma_map["none"] = NO_SIGMA;
  sigma_map["scalar"] = SCALAR_SIGMA;
  sigma_map["diagonal"] = DIAGONAL_SIGMA;
  sigma_map["matrix"] = MATRIX_SIGMA;

  // expand sigma if 0 or 1 given, without validation
  size_t num_resp_groups = simulationSRD.num_response_groups();
  size_t num_scalar = simulationSRD.num_scalar_responses();
  varianceTypes.resize(num_resp_groups, NO_SIGMA);
  if (sigma_types.size() == 1) {
    // assign all sigmas to the specified one
    if (sigma_map.find(sigma_types[0]) != sigma_map.end())
      varianceTypes.assign(num_resp_groups, sigma_map[sigma_types[0]]);
    else {
      Cerr << "\nError: invalid sigma_type '" << sigma_types[0] 
	   << "' specified." << std::endl;
      abort_handler(PARSE_ERROR);
    }
  }
  else if (sigma_types.size() == num_resp_groups) {
    // initialize one sigma type per 
    for (size_t resp_ind = 0; resp_ind < num_resp_groups; ++resp_ind) {
      if (sigma_map.find(sigma_types[resp_ind]) != sigma_map.end())
	varianceTypes[resp_ind] = sigma_map[sigma_types[resp_ind]];
      else {
	Cerr << "\nError: invalid sigma_type '" << sigma_types[resp_ind] 
	     << "' specified." << std::endl;
	abort_handler(PARSE_ERROR);
      }
    }
  }
  else  {
    Cerr << "\nError: sigma_types must have length 1 or number of "
	 << "calibration_terms." << std::endl;
    abort_handler(PARSE_ERROR);
  }

  // when using simple scalar data format, must validate that all
  // scalar are the same and valid (when using separate files, can
  // differ)
  
  // scalar sigma must be 0 or scalar
  bool scalar_data_file = !scalarDataFilename.empty(); 
  for (size_t scalar_ind = 0; scalar_ind < num_scalar; ++scalar_ind) {
    if (varianceTypes[scalar_ind] != NO_SIGMA && 
	varianceTypes[scalar_ind] != SCALAR_SIGMA) {
      Cerr << "\nError: sigma_type must be 'none' or 'scalar' for scalar "
	   << "responses." << std::endl;
      abort_handler(PARSE_ERROR);
    }
    if (scalar_data_file) {
      if (varianceTypes[scalar_ind] != varianceTypes[0]) {
	Cerr << "\nError: sigma_type must be the same for all scalar responses "
	     << "when using scalar data file." 
	     << std::endl;
	abort_handler(PARSE_ERROR);
      }
    }
  }
  // number of sigma to read from simple data file(0 or num_scalar)
  if (scalar_data_file && varianceTypes.size() > 0 && 
      varianceTypes[0] == SCALAR_SIGMA)
    scalarSigmaPerRow = num_scalar;

}

void ExperimentData::
add_data(const RealVector& one_configvars, const Response& one_response)
{
  // BMA TODO: This doesn't make an object of type ExperimentResponse!
  numExperiments += 1;
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "numExperiments in add_data " << numExperiments << '\n';

  allConfigVars.push_back(one_configvars);
  allExperiments.push_back(one_response);
}


void ExperimentData::load_data(const std::string& context_message)
{
  // TODO: complete scalar and field cases

  bool scalar_data_file = !scalarDataFilename.empty(); 
  if (!calibrationDataFlag && !scalar_data_file) {
    Cerr << "\nError: load_data attempted for empty experiment spec."
	 << std::endl;
    abort_handler(-1);
  }

  // Get a copy of the simulation SRD to use in contructing the
  // experiment response; won't be able to share the core data since
  // different experiments may have different sizes...
  SharedResponseData exp_srd = simulationSRD.copy();

  // change the type of response
  // TODO: new ctor for experiment response
  exp_srd.response_type(EXPERIMENT_RESPONSE);
  Response exp_resp(exp_srd);
  if (outputLevel >= DEBUG_OUTPUT) {
    Cout << "Constructing experiment response; initial response is:" 
	 << std::endl;
    exp_resp.write(Cout);
  }

  if (numConfigVars > 0)
    allConfigVars.resize(numExperiments);

  size_t num_scalars = simulationSRD.num_scalar_responses();

  // Count how many fields have each sigma type (for sizing). For
  // field data, if "scalar" or "none" is specified, need to convert
  // to a diagonal the length of the actual data

  // only fields can have matrix or diagonal
  size_t num_field_sigma_matrices = 
    std::count(varianceTypes.begin(), varianceTypes.end(), MATRIX_SIGMA);
  size_t num_field_sigma_diagonals = 
    std::count(varianceTypes.begin(), varianceTypes.end(), DIAGONAL_SIGMA);
  // this counts how many fields have scalar or no sigma, as given by user
  // varianceTypes is either empty or number response groups
  size_t num_field_sigma_scalars = 0;
  size_t num_field_sigma_none = 0;
  if( !varianceTypes.empty() ) {
    num_field_sigma_scalars = 
      std::count(varianceTypes.begin()+num_scalars, varianceTypes.end(), 
		 SCALAR_SIGMA);
    num_field_sigma_none = 
      std::count(varianceTypes.begin()+num_scalars, varianceTypes.end(),
		 NO_SIGMA);
  }

  // TODO: temporary duplication until necessary covariance APIs are updated

  // setup for reading historical format, one experiment per line,
  // each consisting of [ [config_vars] fn values [ fn variance ] ]
  std::ifstream scalar_data_stream;
  if (scalar_data_file) {
    if (outputLevel >= NORMAL_OUTPUT) {
      Cout << "\nReading scalar experimental data from file " 
	   << scalarDataFilename << ":";
      Cout << "\n  " << numExperiments << " experiment(s) for";
      Cout << "\n  " << num_scalars << " scalar responses" << std::endl;
    }
    TabularIO::open_file(scalar_data_stream, scalarDataFilename, 
			 context_message);
    TabularIO::read_header_tabular(scalar_data_stream, scalarDataFormat);
  }

  if (!scalar_data_file && numConfigVars > 0) { 
    // read all experiment config vars from new field data format files at once
    // TODO: have the user give a name for this file, since should be
    // the same for all responses.  Read from foo.<exp_num>.config. 
    //    String config_vars_basename("experiment");
    boost::filesystem::path config_vars_basepath = dataPathPrefix / "experiment";
    try {
      read_config_vars_multifile(config_vars_basepath.string(), numExperiments, 
          numConfigVars, allConfigVars);
    }
    catch(std::runtime_error & e)
    {
      if( numConfigVars > 0 )
        throw
          std::runtime_error("Expected to read " +
                             convert_to_string(numConfigVars) +
                             " experiment config variables, but required file(s) \"" +
                             config_vars_basepath.string() +
                             ".*.config\" not found.");
    }
  }

  bool found_error = false;
  for (size_t exp_index = 0; exp_index < numExperiments; ++exp_index) {

    // conditionally read leading exp_id column
    // TODO: error handling
    if (scalar_data_file)
      TabularIO::read_leading_columns(scalar_data_stream, scalarDataFormat);

    // -----
    // Read and set the configuration variables
    // -----

    // Need to decide what to do if both scalar_data_file and "experiment.#" files exist - RWH
    if ( (numConfigVars > 0) && scalar_data_file ) {
      allConfigVars[exp_index].sizeUninitialized(numConfigVars);
      // TODO: try/catch
      scalar_data_stream >> std::ws;
      if ( scalar_data_stream.eof() ) {
        Cerr << "\nError: End of file '" << scalarDataFilename
          << "' reached before reading " 
          << numExperiments << " sets of values."
          << std::endl;
        abort_handler(-1);
      }
      scalar_data_stream >> allConfigVars[exp_index];
    }
    // TODO: else validate scalar vs. field configs?

    // read one file per field response, resize, and populate the
    // experiment (values, sigma, coords)
    load_experiment(exp_index, scalar_data_stream, num_field_sigma_matrices, 
		    num_field_sigma_diagonals, num_field_sigma_scalars, 
		    num_field_sigma_none, exp_resp);

    if (simulationSRD.field_lengths() != exp_resp.field_lengths() &&
	!interpolateFlag) {
      Cerr << "\nError: field lengths of experiment " << exp_index+1 
	   << " data:\n       " << exp_resp.field_lengths()
	   << "\n       differ from simulation field lengths:"  
	   << simulationSRD.field_lengths() << "and 'interpolate' not enabled."
	   << std::endl;
	found_error = true;
    }

    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "Values for experiment " << exp_index + 1 << ": \n" 
	   << exp_resp.function_values() << std::endl;

    allExperiments.push_back(exp_resp.copy());
  }
  if (found_error)
    abort_handler(-1);


  // verify that the two experiments have different data
  if (outputLevel >= DEBUG_OUTPUT) {
    Cout << "Experiment data summary:\n\n";
    for (size_t i=0; i<numExperiments; ++i) {
      if (numConfigVars > 0)
	Cout << "  Experiment " << i+1 << " configuration variables:"<< "\n"
	     << allConfigVars[i];
      Cout << "  Experiment " << i+1 << " data values:"<< "\n";
      write_data(Cout, allExperiments[i].function_values());
      Cout << '\n';
    }
  }

  // store the lengths (number of functions) of each experiment
  per_exp_length(experimentLengths);
  size_t i, num_exp = allExperiments.size();
  expOffsets.sizeUninitialized(num_exp);
  expOffsets(0) = 0;
  for (i=1; i<num_exp; i++) 
    expOffsets(i) = experimentLengths(i-1) + expOffsets(i-1);

  // Precompute and cache experiment determinants
  covarianceDeterminant = 1.0;
  logCovarianceDeterminant = 0.0;
  for (size_t exp_ind=0; exp_ind < numExperiments; ++exp_ind) {
    covarianceDeterminant *= allExperiments[exp_ind].covariance_determinant();
    logCovarianceDeterminant += 
      allExperiments[exp_ind].log_covariance_determinant();
  }

  // Check and warn if extra data exists in scalar_data_stream
  if (scalar_data_file) {
    scalar_data_stream >> std::ws;
    if ( !scalar_data_stream.eof() )
      Cout << "\nWarning: Data file '" << scalarDataFilename
        << "' contains extra data."
        << std::endl;
  }
}


/** Load an experiment from a mixture of legacy format data and field
    data format files */
void ExperimentData::
load_experiment(size_t exp_index, std::ifstream& scalar_data_stream, 
		size_t num_field_sigma_matrices, 
		size_t num_field_sigma_diagonals,
		size_t num_field_sigma_scalars, 
		size_t num_field_sigma_none, Response& exp_resp)
{
  bool scalar_data_file = !scalarDataFilename.empty();
  size_t num_scalars = simulationSRD.num_scalar_responses();
  size_t num_fields = simulationSRD.num_field_response_groups();
  size_t num_resp = num_scalars + num_fields;


  // -----
  // Data to populate from files for a single experiment
  // -----

  // Since the field lengths aren't known until all reads are
  // complete, use temporary storage to read them all, then resize the
  // Response object.  
  //
  // TODO: May want to have field lengths managed in Response instead
  // of SharedResponse and generate labels on the fly when needed.
  
  // scalar or field values; the RealVectors for scalars will have
  // length 1; the length of RealVectors for fields will be determined
  // by the file read
  RealVectorArray exp_values(num_scalars + num_fields);

  // field lengths may differ for each experiment
  IntVector field_lengths(num_fields, 0);

  // coordinates for fields only
  RealMatrix exp_coords;

  // Non-field response sigmas; field response sigma scalars later get expanded into diagonals
  RealVector sigma_scalars;
  IntVector scalar_map_indices;

  // -----
  // Read the data
  // -----

  // populate scalar data function and sigma values; translate data
  // from historical to new format (later collapse and eliminate
  // copies) TODO: advance a row of exp data in outer context and pass
  // in here to simplify these two cases
  sigma_scalars.resize(num_scalars);
  scalar_map_indices.resize(num_scalars);
  if (scalar_data_file) {
    // Non-field response sigmas; field response sigma scalars later get expanded into diagonals
    for (size_t fn_index = 0; fn_index < num_scalars; ++fn_index) {
      exp_values[fn_index].resize(1);
      scalar_data_stream >> std::ws;
      if ( scalar_data_stream.eof() ) {
        Cerr << "\nError: End of file '" << scalarDataFilename
          << "' reached before reading " 
          << numExperiments << " sets of values."
          << std::endl;
        abort_handler(-1);
      }
      scalar_data_stream >> exp_values[fn_index];
    }
    if (scalarSigmaPerRow > 0)
      read_scalar_sigma(scalar_data_stream, sigma_scalars, scalar_map_indices);
    else {
      sigma_scalars = 1.0;  // historically these defaulted to 1.0
      for (size_t i = 0; i<num_scalars; ++i) {
        scalar_map_indices[i] = i;
      }
      // BMA: in a mixed case we might want these populated with 1.0
      // even if data missing
    }
  }
  else
  {
    RealMatrix working_cov_values;
    const StringArray& scalar_labels = exp_resp.function_labels();
    for( size_t i=0; i<num_scalars; ++i ) {
      //std::cout << "Scalar function " << i << ": veriance_type = " << varianceTypes[i] << ", label = \"" << scalar_labels[i] << "\"" << std::endl;
      // Read data from file named: scalar_labels.expt_num.dat
      boost::filesystem::path field_base = dataPathPrefix / scalar_labels[i];
      read_field_values(field_base.string(), exp_index+1, exp_values[i]);

      // Optionally allow covariance data
      if (!varianceTypes.empty())
	if( varianceTypes[i] ) {
	  read_covariance(field_base.string(), exp_index+1, working_cov_values);
	  sigma_scalars[i] = working_cov_values(0,0);
	  scalar_map_indices[i] = i;
	}
	else {
	  sigma_scalars[i] = 1.0;
	  scalar_map_indices[i] = i;
	}
    }

  }

  // Data for sigma - Note: all fields have matrix, diagonal or none covariance (sigma_ data)
  //                        and so we dimension accordingly.

  // For sanity checking
  size_t count_no_sigmas       = 0;
  size_t count_sigma_scalars   = 0;
  size_t count_sigma_diagonals = 0;
  size_t count_sigma_matrices  = 0;

  std::vector<RealMatrix> sigma_matrices(num_field_sigma_matrices);
  // for fields we make diagonal covariance for these three types
  std::vector<RealVector> sigma_diagonals(num_field_sigma_diagonals + 
					  num_field_sigma_scalars + 
					  num_field_sigma_none);
  // indices for the entries in the above data structures
  IntVector matrix_map_indices(num_field_sigma_matrices), 
            diagonal_map_indices(num_field_sigma_diagonals + 
				 num_field_sigma_scalars + 
				 num_field_sigma_none);


  // populate field data, sigma, and coordinates from separate files
  const StringArray& field_labels = exp_resp.field_group_labels();
  for (size_t field_index = 0; field_index < num_fields; ++field_index) {
    size_t fn_index = num_scalars + field_index;
    const String& field_name = field_labels[field_index];

    // read a column vector of field values for this field from
    // fn_name.exp_num.dat
    boost::filesystem::path field_base = dataPathPrefix / field_name;
    read_field_values(field_base.string(), exp_index+1, exp_values[fn_index]);
    field_lengths[field_index] = exp_values[fn_index].length();

    // For fields with corresponding coords file, read coordinates
    // from field_name.exp_num.coords and validate number of rows is
    // field_lengths[field_index]
    std::string filename = field_name + "." + Dakota::convert_to_string(exp_index+1) + ".coords";
    boost::filesystem::path coord_path_and_file = dataPathPrefix / filename;
    if ( boost::filesystem::is_regular_file(coord_path_and_file) )
    {
      boost::filesystem::path coord_base = dataPathPrefix / field_name;
      read_coord_values(coord_base.string(), exp_index+1, exp_coords);
      // Sanity check length
      if( field_lengths[field_index] != exp_coords.numRows() )
        throw std::runtime_error("Inconsistent lengths of field data and coordinates.");
      exp_resp.field_coords(exp_coords, field_index);
    }
        
    if (!varianceTypes.empty()) {
       // read sigma 1, N (field_lengths[field_index]), or N^2 values
      RealMatrix working_cov_values;
      switch(varianceTypes[fn_index])
	{
	case NO_SIGMA:
	  // expand to a diagonal of 1.0 of appropriate length = field_length and add
	  // field_index to diagonals map
	  sigma_diagonals[count_sigma_diagonals].sizeUninitialized(field_lengths[field_index]);
	  for( int i=0; i<field_lengths[field_index]; ++i )
	    sigma_diagonals[count_sigma_diagonals](i) = 1.0;
	  diagonal_map_indices[count_sigma_diagonals++] = fn_index; // or should it be field_index? - RWH 
	  count_no_sigmas++;
	  break;

	case SCALAR_SIGMA:
	  // read single value, expand to a diagonal of appropriate length = field_length and add
	  // field_index to diagonals map
	  Cout << "Reading scalar cov from " << field_base.string() << std::endl;
	  read_covariance(field_base.string(), exp_index+1, working_cov_values);
	  sigma_diagonals[count_sigma_diagonals].sizeUninitialized(field_lengths[field_index]);
	  for( int i=0; i<field_lengths[field_index]; ++i )
	    sigma_diagonals[count_sigma_diagonals](i) = working_cov_values(0,0);
	  diagonal_map_indices[count_sigma_diagonals++] = fn_index; // or should it be field_index? - RWH 
	  count_sigma_scalars++;
	  break;

	case DIAGONAL_SIGMA:
	  // read N values, add to sigma_diagonals and add num_scalars +
	  // field_index to diagonals map
	  Cout << "Reading diagonal cov from " << field_base.string() << std::endl;
	  read_covariance(field_base.string(), exp_index+1, Dakota::CovarianceMatrix::VECTOR,
			  field_lengths[field_index], working_cov_values);
	  sigma_diagonals[count_sigma_diagonals].sizeUninitialized(field_lengths[field_index]);
	  for( int i=0; i<field_lengths[field_index]; ++i )
	    sigma_diagonals[count_sigma_diagonals](i) = working_cov_values[0][i];
	  diagonal_map_indices[count_sigma_diagonals++] = fn_index; // or should it be field_index? - RWH 
	  //sigma_diagonals[count_sigma_diagonals-1].print(Cout);
	  break;

	case MATRIX_SIGMA:
	  // read N^2 values, add to sigma_matrices and add num_scalars +
	  // field_index to matrices map
	  read_covariance(field_base.string(), exp_index+1, Dakota::CovarianceMatrix::MATRIX,
			  field_lengths[field_index], working_cov_values);
          // Check for symmetry
          if( !is_matrix_symmetric(working_cov_values) )
            throw std::runtime_error("Covariance matrix from \""+field_base.string()+"\" is not symmetric.");
	  sigma_matrices[count_sigma_matrices] = working_cov_values;
	  matrix_map_indices[count_sigma_matrices++] = fn_index; // or should it be field_index? - RWH 
	  //sigma_matrices[count_sigma_matrices-1].print(Cout);
	  break;
	}
    }
  }
  // Sanity check consistency
  if( count_no_sigmas != num_field_sigma_none )
    throw std::runtime_error("Mismatch between specified and actual fields with no sigma provided.");
  if( count_sigma_scalars != num_field_sigma_scalars )
    throw std::runtime_error("Mismatch between specified and actual sigma scalars.");
  if( count_sigma_diagonals != (num_field_sigma_diagonals + 
				num_field_sigma_scalars + num_field_sigma_none) )
    throw std::runtime_error("Mismatch between specified and actual sigma diagonals.");
  if( count_sigma_matrices != num_field_sigma_matrices )
    throw std::runtime_error("Mismatch between specified and actual sigma matrices.");


  // -----
  // Reshape and map in the data
  // -----

  // Reshape the experiment, now that we know total size
  exp_resp.field_lengths(field_lengths);

  for (size_t fn_index = 0; fn_index < num_scalars; ++fn_index)
    exp_resp.function_value(exp_values[fn_index][0], fn_index);

  for (size_t field_ind = 0; field_ind < num_fields; ++field_ind)
    exp_resp.field_values(exp_values[num_scalars + field_ind], field_ind);
 
  //Cout << "Sigma scalars " << sigma_scalars << "\n";
  //Cout << "Scalar map indices" << scalar_map_indices << "\n";

  exp_resp.set_full_covariance(sigma_matrices, sigma_diagonals, sigma_scalars,
        		       matrix_map_indices, diagonal_map_indices, 
        		       scalar_map_indices);

}



void ExperimentData::read_scalar_sigma(std::ifstream& scalar_data_stream,
				       RealVector& sigma_scalars,
				       IntVector& scalar_map_indices)
{
  // currently no longer allow 1 sigma to apply to all scalar responses
  // always read 0, or N
  RealVector sigma_row(scalarSigmaPerRow);
  scalar_data_stream >> sigma_row;
  for (size_t i = 0; i<scalarSigmaPerRow; ++i) {
    sigma_scalars[i] = sigma_row[i];
    scalar_map_indices[i] = i;
  }
}

size_t ExperimentData::
num_scalars() const
{
  if( simulationSRD.is_null() )
    throw std::runtime_error("ExperimentData is incorrectly (or not) initialized.");

  return simulationSRD.num_scalar_responses();
}

size_t ExperimentData::
num_fields() const
{
  if( simulationSRD.is_null() )
    throw std::runtime_error("ExperimentData is incorrectly (or not) initialized.");

  return  simulationSRD.num_field_response_groups();
}


size_t ExperimentData::num_config_vars() const
{
  return numConfigVars; 
}


const std::vector<RealVector>& ExperimentData::config_vars() const
{
  return allConfigVars;
}


void ExperimentData::per_exp_length(IntVector& per_length) const
{
  per_length.resize(allExperiments.size());
  //Cout << "num experiments " << num_experiments();

  for (size_t i=0; i<num_experiments(); i++) 
    per_length(i)= allExperiments[i].function_values().length();
  //Cout << "per length " << per_length;
}


const IntVector& ExperimentData::field_lengths(size_t experiment) const
{
  return allExperiments[experiment].field_lengths();
}

const RealVector& ExperimentData::all_data(size_t experiment)
{
  if (experiment >= allExperiments.size()) {
    Cerr << "\nError: invalid experiment index " << experiment << std::endl;
    abort_handler(-1);
  }
  return allExperiments[experiment].function_values();
}

const Response& ExperimentData::response(size_t experiment) 
{
  if (experiment >= allExperiments.size()) {
    Cerr << "\nError: invalid experiment index " << experiment << std::endl;
    abort_handler(-1);
  }
  return allExperiments[experiment];
}

size_t ExperimentData::num_total_exppoints() const
{
  size_t res_size = 0;
  for (size_t i=0; i<num_experiments(); i++) 
    res_size += allExperiments[i].function_values().length();
  return res_size;
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

RealVector ExperimentData::
field_data_view(size_t response, size_t experiment) const
{
  return(allExperiments[experiment].field_values_view(response));
}

RealMatrix ExperimentData::
field_coords_view(size_t response, size_t experiment) const
{
  return(allExperiments[experiment].field_coords_view(response));
}

bool ExperimentData::variance_type_active(short variance_type) const
{
  UShortArray::const_iterator vt_it = 
    std::find(varianceTypes.begin(), varianceTypes.end(), variance_type);
  return vt_it != varianceTypes.end();
}

bool ExperimentData::variance_active() const
{
  return (variance_type_active(SCALAR_SIGMA) || 
	  variance_type_active(DIAGONAL_SIGMA) || 
	  variance_type_active(MATRIX_SIGMA));
}

bool ExperimentData::interpolate_flag() const
{
  return interpolateFlag;
}

RealVector ExperimentData::
residuals_view(const RealVector& residuals, size_t experiment) const 
{
  int exp_offset = expOffsets[experiment];
  RealVector exp_resid(Teuchos::View, residuals.values()+exp_offset,
		       experimentLengths[experiment]);
  return exp_resid;
}
  
/// Return a view (to allowing updaing in place) of the gradients associated
/// with a given experiment, from a matrix contaning gradients from
/// all experiments
RealMatrix ExperimentData::
gradients_view(const RealMatrix &gradients, size_t experiment) const 
{
  int exp_offset = expOffsets[experiment];
  RealMatrix exp_grads(Teuchos::View, gradients, gradients.numRows(),
		       experimentLengths[experiment], 0, exp_offset );
  return exp_grads;
}
  
/// Return a view (to allowing updaing in place) of the hessians associated
/// with a given experiment, from an array contaning the hessians from 
/// all experiments
RealSymMatrixArray ExperimentData::
hessians_view(const RealSymMatrixArray &hessians, size_t experiment) const {
  int num_hess = experimentLengths[experiment],
    exp_offset = expOffsets[experiment];
  RealSymMatrixArray exp_hessians( num_hess );
  if ( !hessians.empty() ){
    for (size_t i=0; i<num_hess; ++i,++exp_offset)
      if (hessians[exp_offset].numRows()) // else leave exp_hessians[i] empty
	exp_hessians[i] = RealSymMatrix(Teuchos::View, hessians[exp_offset],
					hessians[exp_offset].numRows());
  }
  return exp_hessians;
}

Real ExperimentData::
apply_covariance(const RealVector& residuals, size_t experiment) const
{
  RealVector exp_resid = residuals_view( residuals, experiment );
  if ( variance_active() )
    return(allExperiments[experiment].apply_covariance(exp_resid));
  else 
    return exp_resid.dot( exp_resid );
}

// BMA TODO: These functions don't get called when covariance is
// inactive, but if they did, could undesireably resize the outbound
// object.
void ExperimentData::
apply_covariance_inv_sqrt(const RealVector& residuals, size_t experiment, 
			  RealVector& weighted_residuals) const
{
  RealVector exp_resid = residuals_view( residuals, experiment );
  if ( variance_active() ) 
    allExperiments[experiment].apply_covariance_inv_sqrt(exp_resid, 
							 weighted_residuals);
  else{
    // Return a deep copy
    weighted_residuals.sizeUninitialized( exp_resid.length() );
    weighted_residuals.assign( exp_resid );
  }
}

// BMA TODO: These functions don't get called when covariance is
// inactive, but if they did, could undesireably resize the outbound
// object.
void ExperimentData::
apply_covariance_inv_sqrt(const RealMatrix& gradients, size_t experiment, 
			  RealMatrix& weighted_gradients) const
{
  RealMatrix exp_grads = gradients_view( gradients, experiment );
  if ( variance_active() ) 
    allExperiments[experiment].apply_covariance_inv_sqrt(exp_grads, 
							 weighted_gradients);
  else{
    // Return a deep copy
    weighted_gradients.shapeUninitialized( exp_grads.numRows(),
					   exp_grads.numCols() );
    weighted_gradients.assign( exp_grads );
  }
}

// BMA TODO: These functions don't get called when covariance is
// inactive, but if they did, could undesireably resize the outbound
// object.
void ExperimentData::
apply_covariance_inv_sqrt(const RealSymMatrixArray& hessians, size_t experiment,
			  RealSymMatrixArray& weighted_hessians) const
{
  RealSymMatrixArray exp_hessians = hessians_view( hessians, experiment );
  if ( variance_active() ) 
    allExperiments[experiment].apply_covariance_inv_sqrt(exp_hessians, 
							 weighted_hessians);
  else{
    // Return a deep copy
    size_t num_hess = exp_hessians.size();
    weighted_hessians.resize( num_hess );
    if ( !exp_hessians.empty() ){
      for (size_t i=0; i<num_hess; ++i)
	if (exp_hessians[i].numRows()) // else leave exp_hessians[i] empty
	  weighted_hessians[i] = RealSymMatrix(Teuchos::Copy, 
					       exp_hessians[i],
					       exp_hessians[i].numRows());
    }
  }
  
}


void ExperimentData::
get_main_diagonal(RealVector &diagonal, size_t experiment ) const
{
  if ( !variance_active() )
    throw( std::runtime_error("ExperimentData::get_main_diagonal - covariance matrix is empty") );
    allExperiments[experiment].get_covariance_diagonal( diagonal );
}

void ExperimentData::cov_std_deviation(RealVectorArray& std_deviations) const
{
  std_deviations.resize(numExperiments);
  for (size_t exp_ind=0; exp_ind<numExperiments; ++exp_ind) {
    RealVector& sd_vec = std_deviations[exp_ind];
    allExperiments[exp_ind].experiment_covariance().get_main_diagonal(sd_vec);
    for (size_t i=0; i<sd_vec.length(); ++i)
      sd_vec[i] = std::sqrt(sd_vec[i]);
  }
}

void ExperimentData::cov_as_correlation(RealSymMatrixArray& corr_matrices) const
{
  corr_matrices.resize(numExperiments);
  for (size_t exp_ind=0; exp_ind<numExperiments; ++exp_ind) {
    const ExperimentCovariance& exp_cov = 
      allExperiments[exp_ind].experiment_covariance();
    exp_cov.as_correlation(corr_matrices[exp_ind]);
  }
}

void ExperimentData::covariance(int exp_ind, RealSymMatrix& cov_mat) const
{
  const ExperimentCovariance& exp_cov = 
    allExperiments[exp_ind].experiment_covariance();
  exp_cov.dense_covariance(cov_mat);
}


/** This assumes the souce gradient/Hessian are size less or equal to
    the destination response, and that the leading part is to be populated. */
void ExperimentData::
form_residuals(const Response& sim_resp, Response& residual_resp) const
{
  // BMA: perhaps a better name would be per_exp_asv?
  // BMA TODO: Make this call robust to zero and single experiment cases
  ShortArray total_asv = determine_active_request(residual_resp);

  IntVector experiment_lengths;
  per_exp_length(experiment_lengths);
  size_t residual_resp_offset = 0;
  for (size_t exp_ind = 0; exp_ind < numExperiments; ++exp_ind){
    size_t num_fns_exp = experiment_lengths[exp_ind]; // total length this exper
    form_residuals( sim_resp, exp_ind, total_asv, residual_resp_offset,
		    residual_resp );
    residual_resp_offset += num_fns_exp;
  }
}


void ExperimentData::
form_residuals(const Response& sim_resp, const size_t curr_exp,
	       Response& residual_resp) const
{
  // BMA: perhaps a better name would be per_exp_asv?
  // BMA TODO: Make this call robust to zero and single experiment cases
  ShortArray total_asv = determine_active_request(residual_resp);

  IntVector experiment_lengths;
  per_exp_length(experiment_lengths);
  size_t residual_resp_offset = 0;
  for (size_t exp_ind = 0; exp_ind < curr_exp; ++exp_ind){
    size_t num_fns_exp = experiment_lengths[exp_ind]; // total length this exper
    residual_resp_offset += num_fns_exp;
  }
  form_residuals(sim_resp, curr_exp, total_asv, residual_resp_offset,
		 residual_resp);
}


/** This assumes the souce gradient/Hessian are size less or equal to
    the destination response, and that the leading part is to be populated. */
void ExperimentData::
form_residuals(const Response& sim_resp, size_t exp_ind, 
	       const ShortArray &total_asv, size_t exp_offset, 
	       Response &residual_resp ) const
{
  // size of the residuals for this one experiment
  size_t exp_resid_size = allExperiments[exp_ind].function_values().length();
  // the functions from the simulation
  RealVector sim_fns = sim_resp.function_values();
  RealMatrix sim_grads = sim_resp.function_gradients_view();
  RealSymMatrixArray sim_hessians = sim_resp.function_hessians_view();

  size_t i, j;
  const IntVector& sim_field_lens = sim_resp.field_lengths();

  short asv = total_asv[exp_ind];
  RealVector all_residuals = residual_resp.function_values_view();
  RealVector exp_residuals(Teuchos::View, all_residuals.values()+exp_offset,
			   exp_resid_size);

  if (!interpolateFlag) {

    for (i=0; i<exp_resid_size; i++){
      exp_residuals[i] =
	sim_fns[i] - allExperiments[exp_ind].function_value(i);

      // populate only the part of the gradients/Hessians for this
      // experiment, for the active submodel derivative variables
      if ( asv & 2 ){
	size_t num_sm_cv = sim_grads.numRows();
	RealMatrix resid_grads
	  (gradients_view(residual_resp.function_gradients(), exp_ind));
	resid_grads = 0.0;
	for (size_t r_ind = 0; r_ind<exp_resid_size; ++r_ind)
	  for (size_t i=0; i<num_sm_cv; ++i)
	    resid_grads(i, r_ind) = sim_grads(i, r_ind);
      }

      if ( asv & 4 ) {
	size_t num_sm_cv = sim_grads.numRows();
	RealSymMatrixArray resid_hess
	  (hessians_view(residual_resp.function_hessians(), exp_ind));
	for (size_t r_ind = 0; r_ind<exp_resid_size; ++r_ind) {
	  resid_hess[r_ind] = 0.0;
	  for (size_t i=0; i<num_sm_cv; ++i)
	    for (size_t j=0; j<num_sm_cv; ++j)
	      resid_hess[r_ind](i,j) = sim_hessians[r_ind](i,j);
	}
      }
    }

  } else {   

    for (i=0; i<num_scalars(); i++) {
      exp_residuals[i] = sim_fns[i] - allExperiments[exp_ind].function_value(i);
      // BMA: Looked like gradients and Hessians of the scalars weren't
      // populated, so added:
      if (asv & 2) {
	size_t num_sm_cv = sim_grads.numRows();
	RealVector resid_grad = 
	  residual_resp.function_gradient_view(exp_offset+i);
	resid_grad = 0.0;
	for (size_t j=0; j<num_sm_cv; ++j)
	  resid_grad[j] = sim_grads(j, i);
      }
      if (asv & 4) {
	size_t num_sm_cv = sim_hessians[i].numRows();
	RealSymMatrix resid_hess = 
	  residual_resp.function_hessian_view(exp_offset+i);
	resid_hess = 0.0;
	for (size_t j=0; j<num_sm_cv; ++j)
	  for (size_t k=0; k<num_sm_cv; ++k)
	    resid_hess(j,k) = sim_hessians[i](j,k);

      }
    }

    // interpolate the simulation data onto the coordinates of the experiment
    // data, populating functions, gradients, Hessians

    // I think residuals are stored in continguous order, 
    // [exp1(scalars,fields),...,exp_n(scalars,fields)
    // if so need to pass exp_offset to interpolate function above
    // and inside that function set offset =  exp_offset and 
    // then increment in usual way
    interpolate_simulation_data(sim_resp, exp_ind, total_asv, exp_offset,
				residual_resp);

    if (outputLevel >= DEBUG_OUTPUT) 
      Cout << "interp values" << exp_residuals << '\n';

    if (asv & 1) {
      // compute the residuals, i.e. subtract the experiment data values
      // from the (interpolated) simulation values.
      size_t cntr = num_scalars();
      for (i=0; i<num_fields(); i++){
	size_t num_field_fns = field_data_view(i,exp_ind).length();
	for (j=0; j<num_field_fns; j++, cntr++)
	  exp_residuals[cntr] -= field_data_view(i,exp_ind)[j];
      }
      if (outputLevel >= DEBUG_OUTPUT) 
	Cout << "residuals in exp space" << exp_residuals << '\n';
    }
  }
}

void ExperimentData::
interpolate_simulation_data(const Response &sim_resp, size_t exp_ind,
			    const ShortArray &total_asv, size_t exp_offset,
			    Response &interp_resp ) const
{
  size_t offset = exp_offset + num_scalars();
  IntVector field_lens = field_lengths(exp_ind);
  for (size_t field_num=0; field_num<num_fields(); field_num++){ 
    RealMatrix exp_coords = field_coords_view(field_num,exp_ind);
    interpolate_simulation_field_data( sim_resp, exp_coords, field_num, 
				       total_asv[exp_ind],
				       offset, interp_resp );
    offset += field_lens[field_num]; 
  }
}


// BMA TODO: Make this call robust to zero and single experiment cases
ShortArray ExperimentData::
determine_active_request(const Response& resid_resp) const 
{
  ShortArray total_asv( numExperiments );

  // can't apply matrix-valued errors due to possibly incomplete
  // dataset when active set vector is in use (missing residuals)
  bool interogate_field_data = 
    variance_type_active(MATRIX_SIGMA) || interpolate_flag();

  IntVector experiment_lengths;
  per_exp_length(experiment_lengths);


  size_t calib_term_ind = 0; // index into the total set of calibration terms
  for (size_t exp_ind = 0; exp_ind < numExperiments; ++exp_ind){
    // total length this exper
    size_t num_fns_exp = experiment_lengths[exp_ind]; 
     
    // Within a field group, cannot have matrix (off-diagonal)
    // covariance and non-uniform ASV
    //
    // TODO: This is overly conservative; instead check whether
    // matrix covariance is active on per-field group basis
    const ShortArray& asv = resid_resp.active_set_request_vector();
    total_asv[exp_ind] = 0;
    if (interogate_field_data) {

      size_t num_scalar = num_scalars();
      for (size_t sc_ind = 0; sc_ind < num_scalar; ++sc_ind)
	total_asv[exp_ind] |= asv[calib_term_ind + sc_ind];

      const IntVector& exp_field_lens = field_lengths(exp_ind);
      size_t num_field_groups = num_fields();
      size_t field_start = calib_term_ind + num_scalar;
      for (size_t fg_ind = 0; fg_ind < num_field_groups; ++fg_ind) {

	// determine presence and consistency of active set vector
	// requests within this field
	size_t asv_1 = 0, asv_2 = 0, asv_4 = 0;
	size_t num_fns_field = exp_field_lens[fg_ind];
	for (size_t fn_ind = 0; fn_ind < num_fns_field; ++fn_ind) {
	  if (asv[field_start + fn_ind] & 1) ++asv_1;
	  if (asv[field_start + fn_ind] & 2) ++asv_2;
	  if (asv[field_start + fn_ind] & 4) ++asv_4;
	}
	  
	// with matrix covariance, each of fn, grad, Hess must have all
	// same asv (either none or all) (within a field response group)
	if ( (asv_1 != 0 && asv_1 != num_fns_field) ||
	     (asv_2 != 0 && asv_2 != num_fns_field) ||
	     (asv_4 != 0 && asv_4 != num_fns_field)) {  
	  Cerr << "\nError: matrix form of data error covariance cannot be "
	       << "used with non-uniform\n       active set vector; consider "
	       << "disabling active set vector or specifying no\n      , "
	       << "scalar, or diagonal covariance" << std::endl;
	  abort_handler(-1);
	}
	if (asv_1 > 0) total_asv[exp_ind] |= 1;
	if (asv_2 > 0) total_asv[exp_ind] |= 2;
	if (asv_4 > 0) total_asv[exp_ind] |= 4;
      }
    }
    else {
      // compute aggregate ASV over scalars and field data
      for (size_t fn_ind = 0; fn_ind < num_fns_exp; ++fn_ind)
	total_asv[exp_ind] |= asv[calib_term_ind + fn_ind];
    }

    calib_term_ind += num_fns_exp;
  }  // for each experiment
  return(total_asv);
}

void ExperimentData::
scale_residuals(const Response& residual_response, 
		RealVector& weighted_resid) const {

  ShortArray total_asv = determine_active_request(residual_response);

  for (size_t exp_ind = 0; exp_ind < numExperiments; ++exp_ind) {

    // apply noise covariance to the residuals for this experiment 
    // and store in correct place in weighted_residual
    if (outputLevel >= DEBUG_OUTPUT && total_asv[exp_ind] > 0)
      Cout << "Calibration: weighting residuals for experiment " 
	   << exp_ind + 1 << " with inverse of specified\nerror covariance." 
	   << std::endl;
       
    // apply cov_inv_sqrt to the residual vector
    if (total_asv[exp_ind] & 1) {
      // BMA: not sure why View didn't work (was disconnected):
      //RealVector exp_weighted_resid(residuals_view(weighted_resid, exp_ind));
      RealVector exp_weighted_resid;
      // takes full list of source residuals, but per-experiment output vector
      apply_covariance_inv_sqrt(residual_response.function_values(),
                                exp_ind, exp_weighted_resid);
      size_t calib_term_ind = expOffsets[exp_ind];
      copy_data_partial(exp_weighted_resid, weighted_resid, calib_term_ind);
    }

  }
}


void ExperimentData::scale_residuals(Response& residual_response) const 
{
  ShortArray total_asv = determine_active_request(residual_response);

  IntVector experiment_lengths;
  per_exp_length(experiment_lengths);

  size_t calib_term_ind = 0; // index into the total set of calibration terms
  for (size_t exp_ind = 0; exp_ind < numExperiments; ++exp_ind){
    // total residuals in this exper
    size_t num_fns_exp = experiment_lengths[exp_ind]; 
    
    // apply noise covariance to the residuals for this experiment 
    // and store in correct place in residual_response
    if (outputLevel >= DEBUG_OUTPUT && total_asv[exp_ind] > 0)
      Cout << "Calibration: weighting residuals for experiment " 
	   << exp_ind + 1 << " with inverse of\n specified error covariance." 
	   << std::endl;
       
    // apply cov_inv_sqrt to the residual vector
    RealVector weighted_resid;
    if (total_asv[exp_ind] & 1)
      apply_covariance_inv_sqrt(residual_response.function_values(),
				exp_ind, weighted_resid);
    else
      weighted_resid = 
	residuals_view(residual_response.function_values(), exp_ind);

    // apply cov_inv_sqrt to each row of gradient matrix
    RealMatrix weighted_grad;
    if (total_asv[exp_ind] & 2) {
      apply_covariance_inv_sqrt(residual_response.function_gradients(),
				exp_ind, weighted_grad);
    }
    else
      weighted_grad = 
	gradients_view(residual_response.function_gradients(), exp_ind);

    // apply cov_inv_sqrt to non-contiguous Hessian matrices
    RealSymMatrixArray weighted_hess;
    if (total_asv[exp_ind] & 4)
      apply_covariance_inv_sqrt(residual_response.function_hessians(), 
				exp_ind, weighted_hess);
    else
      weighted_hess = hessians_view(residual_response.function_hessians(), 
				    exp_ind);

    copy_field_data(weighted_resid, weighted_grad, weighted_hess, 
		    calib_term_ind, num_fns_exp, residual_response);

    calib_term_ind += num_fns_exp;
  }
}


/** Add the data back to the residual to recover the model, for use in
    surrogated-based LSQ where DB lookup will fail (need approx eval
    DB).  best_fns contains primary and secondary responses */
void ExperimentData::
recover_model(size_t num_pri_fns, RealVector& best_fns) const
{
  if (interpolateFlag) {
    Cerr << "Error: cannot recover model from residuals when interpolating.\n";
    abort_handler(-1);
  }
  const Response& experiment0 = allExperiments[0];
  if (num_pri_fns != experiment0.num_functions()) {
    Cerr << "Error: incompatible sizes in recover_model()\n";
    abort_handler(-1);
  }
  for (size_t i=0; i<num_pri_fns; ++i)
    best_fns[i] += experiment0.function_value(i);
}

void ExperimentData::
build_gradient_of_sum_square_residuals( const Response& resp, 
					const ShortArray& asrv,
					RealVector &ssr_gradient )
{
  // initialize ssr_gradient to zero, prior to summing over set of experiments
  size_t exp_ind, num_v = resp.active_set().derivative_vector().size();
  if (ssr_gradient.length() != num_v) ssr_gradient.size(num_v); // init to 0.
  else                                ssr_gradient = 0.;
  //size_t residual_resp_offset = 0;
  for (exp_ind = 0; exp_ind < numExperiments; ++exp_ind)
    // adds to ssr_gradient for each experiment
    build_gradient_of_sum_square_residuals_from_response( resp, asrv, exp_ind, 
							  ssr_gradient );
}

void ExperimentData::
build_gradient_of_sum_square_residuals_from_response( const Response& resp, 
						      const ShortArray& asrv,
						      int exp_ind,
						      RealVector &ssr_gradient)
{
  RealVector scaled_residuals = residuals_view(resp.function_values(), exp_ind);
  RealMatrix scaled_gradients = 
    gradients_view(resp.function_gradients(), exp_ind);

  /*scaled_residuals.print(std::cout);
  scaled_gradients.print(std::cout);*/

  build_gradient_of_sum_square_residuals_from_function_data( scaled_gradients, 
							     scaled_residuals,
							     ssr_gradient,
							     asrv);
}

void ExperimentData::
build_gradient_of_sum_square_residuals_from_function_data(
		 const RealMatrix &func_gradients,
                 const RealVector &residuals,
		 RealVector &ssr_gradient, const ShortArray& asrv )
{
  // This function assumes that residuals are r = ( approx - data )
  // NOT r = ( data - approx )

  // func_gradients is the transpose of the Jacobian of the functions
  int v, r, num_v = func_gradients.numRows(),
    num_residuals = residuals.length();
  for ( r=0; r<num_residuals; ++r )
    if ( (asrv[r] & 3) == 3 ) {
      Real res = residuals[r]; const Real* func_grad = func_gradients[r];
      for ( v=0; v<num_v; ++v )
	ssr_gradient[v] += res * func_grad[v];
      // we compute gradient of sum square residuals divided by 2 (i.e. r'r/2),
      // where r has been scaled by sqrt(inv Gamma_d)
    }
}

void ExperimentData::
build_hessian_of_sum_square_residuals( const Response& resp, 
				       const ShortArray& asrv,
				       RealSymMatrix &ssr_hessian )
{
  // initialize ssr_hessian to zero, prior to summing over set of experiments
  size_t exp_ind, num_v = resp.active_set().derivative_vector().size();
  if (ssr_hessian.numRows() != num_v) ssr_hessian.shape(num_v); // init to 0.
  else                                ssr_hessian = 0.;
  //size_t residual_resp_offset = 0;
  for (exp_ind = 0; exp_ind < numExperiments; ++exp_ind)
    // adds to ssr_hessian for each experiment
    build_hessian_of_sum_square_residuals_from_response( resp, asrv, exp_ind, 
							 ssr_hessian );
}

void ExperimentData::
build_hessian_of_sum_square_residuals_from_response( const Response& resp, 
						     const ShortArray& asrv,
						     int exp_ind,
						     RealSymMatrix &ssr_hessian)
{
  RealVector scaled_residuals = residuals_view(resp.function_values(), exp_ind );
  RealMatrix scaled_gradients = 
    gradients_view(resp.function_gradients(), exp_ind );
  RealSymMatrixArray scaled_hessians = 
    hessians_view(resp.function_hessians(), exp_ind );

  /*scaled_residuals.print(std::cout);
  scaled_gradients.print(std::cout);
  Cout << "\nSize:" << scaled_hessians.size();
  for (size_t j=0;j<scaled_hessians.size();j++)
  scaled_hessians[j].print(std::cout);*/

  build_hessian_of_sum_square_residuals_from_function_data( scaled_hessians, 
							    scaled_gradients, 
							    scaled_residuals,
							    ssr_hessian,
							    asrv);
}

void ExperimentData::
build_hessian_of_sum_square_residuals_from_function_data(
		 const RealSymMatrixArray &func_hessians, 
		 const RealMatrix &func_gradients,
                 const RealVector &residuals,
		 RealSymMatrix &ssr_hessian,
		 const ShortArray& asrv ){
  // This function assumes that residuals are r = ( approx - data )
  // NOT r = ( data - approx )

  // func_gradients is the transpose of the Jacobian of the functions
  int num_v = ssr_hessian.numRows(), num_residuals = residuals.length();
  for ( int k=0; k<num_v; k++ ) {
    for ( int j=0; j<=k; j++ ) {
      Real &hess_jk = ssr_hessian(j,k);
      for ( int i=0; i<num_residuals; i++ ) {
	short asrv_i = asrv[i];
	if (asrv_i & 2) hess_jk += func_gradients(j,i)*func_gradients(k,i);
	if ( (asrv_i & 5) == 5 ) hess_jk += residuals[i]*func_hessians[i](j,k);
      }
      // we adopt convention and compute hessian of sum square residuals
      // multiplied by 1/2 e.g. r'r/2. Thus we do not need following 
      // multiplication
      //hess_jk *= 2.;
    }
  }
}


/** In-place scaling of residual response by hyper-parameter multipliers */
void ExperimentData::
scale_residuals(const RealVector& multipliers, unsigned short multiplier_mode,
                size_t num_calib_params, Response& residual_response) const
{

  // NOTE: In most general case the index into multipliers is exp_ind
  // * num_response_groups + resp_ind (never per function ind, e.g.,
  // in a field)
  size_t num_hyper = multipliers.length();
  size_t total_resid = num_total_exppoints();
  const ShortArray& asv = residual_response.active_set_request_vector();

  switch (multiplier_mode) {

  case CALIBRATE_NONE:
    // no-op
    break;

  case CALIBRATE_ONE: {
    assert(multipliers.length() == 1);
    Real fn_scale = 1.0/sqrt(multipliers[0]);
    Real grad_scale = -0.5/multipliers[0];
    Real hess_scale = 0.75*std::pow(multipliers[0], -2.0);
 
    for (size_t i=0; i<total_resid; ++i) {

      if (asv[i] & 1) {
	Real& fn_value = residual_response.function_value_view(i);
	fn_value *= fn_scale;
      }

      if (asv[i] & 2) {
	const Real& fn_value = residual_response.function_value(i);
	RealVector fn_grad = residual_response.function_gradient_view(i);
	// scale all of the gradient (including zeroed entries)
	fn_grad *= fn_scale;
	// then augment with gradient entry for the hyper-parameter
	fn_grad[num_calib_params] = grad_scale * fn_value;
      }

      if (asv[i] & 4) {
	const Real& fn_value = residual_response.function_value(i);
	const RealVector fn_grad = residual_response.function_gradient_view(i);
	RealSymMatrix fn_hess = residual_response.function_hessian_view(i);
	// scale all of the Hessian (including zeroed entries)
	fn_hess *= fn_scale;
	// then augment with Hessian entries for the hyper-parameter
	for (size_t j=0; j<num_calib_params; ++j)  {
	  fn_hess(j, num_calib_params) = grad_scale * fn_grad[j];
	  fn_hess(num_calib_params, j) = grad_scale * fn_grad[j];
	}
	fn_hess(num_calib_params, num_calib_params) = hess_scale * fn_value;
      }

    }
    break;
  }

  case CALIBRATE_PER_EXPER: case CALIBRATE_PER_RESP: case CALIBRATE_BOTH: {
    IntVector resid2mult_indices;
    resid2mult_map(multiplier_mode, resid2mult_indices);

    for (size_t i=0; i<total_resid; ++i) {
      // index of multiplier for this residual
      int mult_ind = resid2mult_indices[i];
      Real fn_scale = 1.0/sqrt(multipliers[mult_ind]);
      Real grad_scale = -0.5/multipliers[mult_ind];
      Real hess_scale = 0.75*std::pow(multipliers[mult_ind], -2.0);

      if (asv[i] & 1) {
	Real& fn_value = residual_response.function_value_view(i);
	fn_value *= fn_scale;
      }

      if (asv[i] & 2) {
	const Real& fn_value = residual_response.function_value(i);
	RealVector fn_grad = residual_response.function_gradient_view(i);
	// scale all of the gradient (including zeroed entries)
	fn_grad *= fn_scale;
	// then augment with gradient entries for the hyper-parameters
	// only 1 multiplier can affect a given residual
	fn_grad[num_calib_params + mult_ind] = grad_scale * fn_value;
      }
      
      if (asv[i] & 4) {
	const Real& fn_value = residual_response.function_value(i);
	const RealVector fn_grad = residual_response.function_gradient_view(i);
	RealSymMatrix fn_hess = residual_response.function_hessian_view(i);
	// scale all of the Hessian (including zeroed entries)
	fn_hess *= fn_scale;
	// augment with Hessian entries for the hyper-parameters
	for (size_t j=0; j<num_calib_params; ++j)  {
	  fn_hess(j, num_calib_params + mult_ind) = grad_scale * fn_grad[j];
	  fn_hess(num_calib_params + mult_ind, j) = grad_scale * fn_grad[j];
	}
	fn_hess(num_calib_params + mult_ind, num_calib_params + mult_ind) = 
	  hess_scale * fn_value;
      }
    }
    break;
  }

  default:
    // unknown mode
    Cerr << "\nError: unknown multiplier mode in scale_residuals().\n";
    abort_handler(-1);
    break;
  }
}

// BMA Reviewed
/** Determinant of the total covariance used in inference, which has
    blocks mult_i * I * Cov_i. */
Real ExperimentData::
cov_determinant(const RealVector& multipliers, 
                unsigned short multiplier_mode) const
{
  // initialize with product of experiment covariance determinants
  Real det = covarianceDeterminant; 
  size_t total_resid = num_total_exppoints();

  switch (multiplier_mode) {
    
  case CALIBRATE_NONE:
    // no-op: multiplier is 1.0
    break;

  case CALIBRATE_ONE:
    assert(multipliers.length() == 1);
    det *= std::pow(multipliers[0], (double)total_resid);
    break;
    
  case CALIBRATE_PER_EXPER: case CALIBRATE_PER_RESP: case CALIBRATE_BOTH: {
    RealVector expand_mults;
    generate_multipliers(multipliers, multiplier_mode, expand_mults);
    // for each experiment, add contribution from mult: det(mult_i*I*Cov_i) 
    // Don't need to exponentiate to block size as we're multiplying det by 
    // each multiplier individiually
    for (size_t resid_ind = 0; resid_ind < total_resid; ++resid_ind)
      det *= expand_mults[resid_ind];
    break;
  }

  default:
    // unknown mode
    Cerr << "\nError: unknown multiplier mode in cov_determinant().\n";
    abort_handler(-1);
    break;

  } // switch

  return det;
}

/** Determinant of half the log of total covariance used in inference,
    which has blocks mult_i * I * Cov_i. */
Real ExperimentData::
half_log_cov_determinant(const RealVector& multipliers, 
			 unsigned short multiplier_mode) const
{
  // initialize with sum of experiment covariance log determinants
  Real log_det = logCovarianceDeterminant; 
  size_t total_resid = num_total_exppoints();

  switch (multiplier_mode) {
    
  case CALIBRATE_NONE:
    // no-op: multiplier is 1.0
    break;

  case CALIBRATE_ONE:
    assert(multipliers.length() == 1);
    log_det += std::log(multipliers[0]) * (double)total_resid;
    break;
    
  case CALIBRATE_PER_EXPER: case CALIBRATE_PER_RESP: case CALIBRATE_BOTH: {
    RealVector expand_mults;
    generate_multipliers(multipliers, multiplier_mode, expand_mults);
    // for each experiment, add contribution from mult: det(mult_i*I*Cov_i)
    for (size_t resid_ind = 0; resid_ind < total_resid; ++resid_ind)
      log_det += std::log(expand_mults[resid_ind]);
    break;
  }

  default:
    // unknown mode
    Cerr << "\nError: unknown multiplier mode in log_cov_determinant().\n";
    abort_handler(-1);
    break;

  } // switch

  return log_det / 2.0;
}


// BMA Reviewed
/** Compute the gradient of scalar f(m) 0.5*log(det(mult*Cov))
    w.r.t. mults.  Since this is the only use case, we include the 0.5
    factor and perform an update in-place. */
void ExperimentData::
half_log_cov_det_gradient(const RealVector& multipliers, 
			  unsigned short multiplier_mode, size_t hyper_offset,
			  RealVector& gradient) const
{
  switch (multiplier_mode) {

  case CALIBRATE_NONE:
    // no hyper-parameters being calibrated
    break;

  case CALIBRATE_ONE: {
    // This multiplier affects all functions
    assert(multipliers.length() == 1);
    Real total_resid = (Real) num_total_exppoints();
    gradient[hyper_offset] += 
      total_resid / multipliers[0] / 2.0;   
    break;
  }

  case CALIBRATE_PER_EXPER: case CALIBRATE_PER_RESP: case CALIBRATE_BOTH: {
    SizetArray resid_per_mult = residuals_per_multiplier(multiplier_mode);
    assert(multipliers.length() == resid_per_mult.size());
    for (size_t i=0; i<multipliers.length(); ++i)
      gradient[hyper_offset + i] += 
	((Real) resid_per_mult[i]) / multipliers[i] / 2.0; 
    break;
  }
 
  }

}

// BMA Reviewed
/** Compute the gradient of scalar f(m) log(det(mult*Cov)) w.r.t. mults */
void ExperimentData::
half_log_cov_det_hessian(const RealVector& multipliers, 
			 unsigned short multiplier_mode, size_t hyper_offset,
			 RealSymMatrix& hessian) const 
{
  switch (multiplier_mode) {

  case CALIBRATE_NONE:
    // no hyper-parameters being calibrated
    break;

  case CALIBRATE_ONE: {
    // This multiplier affects all functions
    assert(multipliers.length() == 1);
    Real total_resid = (Real) num_total_exppoints();
    hessian(hyper_offset, hyper_offset) += 
      -total_resid / multipliers[0] / multipliers[0] / 2.0;   
    break;
  }

  case CALIBRATE_PER_EXPER: case CALIBRATE_PER_RESP: case CALIBRATE_BOTH: {
    SizetArray resid_per_mult = residuals_per_multiplier(multiplier_mode);
    assert(multipliers.length() == resid_per_mult.size());
    for (size_t i =0; i<multipliers.length(); ++i)
      hessian(hyper_offset, hyper_offset) += 
	-((Real) resid_per_mult[i]) / multipliers[i] / multipliers[i] / 2.0;   
    break;
  }
 
  }
}


// BMA Reviewed
/** Calculate how many residuals each multiplier affects */
SizetArray ExperimentData::
residuals_per_multiplier(unsigned short multiplier_mode) const
{
  SizetArray resid_per_mult;
  switch (multiplier_mode) {
    
  case CALIBRATE_PER_EXPER: {
    resid_per_mult.resize(numExperiments, 0);
    for (size_t exp_ind=0; exp_ind < numExperiments; ++exp_ind) {
      size_t fns_this_exp = allExperiments[exp_ind].num_functions();
      resid_per_mult[exp_ind] = fns_this_exp;
    }
    break;
  }

  case CALIBRATE_PER_RESP: {
    size_t num_scalar = simulationSRD.num_scalar_responses();
    size_t num_fields = simulationSRD.num_field_response_groups();
    resid_per_mult.resize(num_scalar + num_fields, 0);
    // iterate scalar responses, then fields
    for (size_t s_ind  = 0; s_ind < num_scalar; ++s_ind)
      resid_per_mult[s_ind] += numExperiments;
    
    // each experiment can have different field lengths
    for (size_t exp_ind=0; exp_ind < numExperiments; ++exp_ind) {
      const IntVector& field_lens = allExperiments[exp_ind].field_lengths();
      for (size_t f_ind  = 0; f_ind < num_fields; ++f_ind)
	resid_per_mult[num_scalar + f_ind] += field_lens[f_ind];
    }
    break;
  }

  case CALIBRATE_BOTH: {
    size_t num_scalar = simulationSRD.num_scalar_responses();
    size_t num_fields = simulationSRD.num_field_response_groups();
    size_t multiplier_offset = 0;
    resid_per_mult.resize(numExperiments*simulationSRD.num_response_groups(), 0);
    for (size_t exp_ind=0; exp_ind < numExperiments; ++exp_ind) {
    
      // iterate scalar responses, then fields
      for (size_t s_ind  = 0; s_ind < num_scalar; ++s_ind)
        resid_per_mult[multiplier_offset++] = 1;
      
      // each experiment can have different field lengths
      const IntVector& field_lens = allExperiments[exp_ind].field_lengths();
      for (size_t f_ind  = 0; f_ind < num_fields; ++f_ind) {
	resid_per_mult[multiplier_offset++] = field_lens[f_ind];
      }
    }	       
    break;
  }

  }

  return resid_per_mult;
}


// BMA Reviewed
void ExperimentData::generate_multipliers(const RealVector& multipliers,
                                          unsigned short multiplier_mode,
                                          RealVector& expanded_multipliers) const
{
  // in most cases, we won't call this function for NONE or ONE cases
  expanded_multipliers.resize(num_total_exppoints());
  
  switch (multiplier_mode) {
    
  case CALIBRATE_NONE:
    expanded_multipliers = 1.0;
    break;

  case CALIBRATE_ONE:
    assert(multipliers.length() == 1);
    expanded_multipliers = multipliers[0];
    break;

  case CALIBRATE_PER_EXPER: {
    assert(multipliers.length() == numExperiments);
    size_t resid_offset = 0;
    for (size_t exp_ind=0; exp_ind < numExperiments; ++exp_ind) {
      size_t fns_this_exp = allExperiments[exp_ind].num_functions();
      for (size_t fn_ind = 0; fn_ind < fns_this_exp; ++fn_ind)
        expanded_multipliers[resid_offset++] = multipliers[exp_ind];
    }
    break;
  }

  case CALIBRATE_PER_RESP: {
    assert(multipliers.length() == simulationSRD.num_response_groups());
    size_t num_scalar = simulationSRD.num_scalar_responses();
    size_t num_fields = simulationSRD.num_field_response_groups();
    size_t resid_offset = 0;
    for (size_t exp_ind=0; exp_ind < numExperiments; ++exp_ind) {
      
      // each response has a different multiplier, but they don't
      // differ across experiments
      size_t multiplier_offset = 0;
      
      // iterate scalar responses, then fields
      for (size_t s_ind  = 0; s_ind < num_scalar; ++s_ind)
        expanded_multipliers[resid_offset++] = multipliers[multiplier_offset++];

      // each experiment can have different field lengths
      const IntVector& field_lens = allExperiments[exp_ind].field_lengths();
      for (size_t f_ind  = 0; f_ind < num_fields; ++f_ind) {
        for (size_t i=0; i<field_lens[f_ind]; ++i)
          expanded_multipliers[resid_offset++] = multipliers[multiplier_offset];
        // only increment per-top-level response (each field has different mult)
        ++multiplier_offset;
      }
    }
    break;
  }

  case CALIBRATE_BOTH: {
    assert(multipliers.length() == 
           numExperiments*simulationSRD.num_response_groups());
    size_t num_scalar = simulationSRD.num_scalar_responses();
    size_t num_fields = simulationSRD.num_field_response_groups();
    size_t resid_offset = 0, multiplier_offset = 0;
    for (size_t exp_ind=0; exp_ind < numExperiments; ++exp_ind) {
    
      // don't reset the multiplier_offset; each exp, each resp has its own

      // iterate scalar responses, then fields
      for (size_t s_ind  = 0; s_ind < num_scalar; ++s_ind)
        expanded_multipliers[resid_offset++] = multipliers[multiplier_offset++];
      
      // each experiment can have different field lengths
      const IntVector& field_lens = allExperiments[exp_ind].field_lengths();
      for (size_t f_ind  = 0; f_ind < num_fields; ++f_ind) {
        for (size_t i=0; i<field_lens[f_ind]; ++i)
          expanded_multipliers[resid_offset++] = multipliers[multiplier_offset];
        // only increment per-top-level response (each field has different mult)
        ++multiplier_offset;
      }
    }	       
    break;
  }
    
  default:
    // unknown mode
    Cerr << "\nError: unknown multiplier mode in generate_multipliers().\n";
    abort_handler(-1);
    break;
      
  } // switch

}


/// return the index of the multiplier that affects each residual
void ExperimentData::resid2mult_map(unsigned short multiplier_mode,
				    IntVector& resid2mult_indices) const
{
  // in most cases, we won't call this function for NONE or ONE cases
  resid2mult_indices.resize(num_total_exppoints());
  
  switch (multiplier_mode) {
    
  case CALIBRATE_NONE:
    Cerr << "\nError: cannot generate map for zero multipliers.\n";
    abort_handler(-1);
    break;

  case CALIBRATE_ONE:
    resid2mult_indices = 0;
    break;

  case CALIBRATE_PER_EXPER: {
    size_t resid_offset = 0;
    for (size_t exp_ind=0; exp_ind < numExperiments; ++exp_ind) {
      size_t fns_this_exp = allExperiments[exp_ind].num_functions();
      for (size_t fn_ind = 0; fn_ind < fns_this_exp; ++fn_ind)
        resid2mult_indices[resid_offset++] = exp_ind;
    }
    break;
  }

  case CALIBRATE_PER_RESP: {
    size_t num_scalar = simulationSRD.num_scalar_responses();
    size_t num_fields = simulationSRD.num_field_response_groups();
    size_t resid_offset = 0;
    for (size_t exp_ind=0; exp_ind < numExperiments; ++exp_ind) {
      
      // each response has a different multiplier, but they don't
      // differ across experiments
      size_t multiplier_offset = 0;
      
      // iterate scalar responses, then fields
      for (size_t s_ind  = 0; s_ind < num_scalar; ++s_ind)
        resid2mult_indices[resid_offset++] = multiplier_offset++;

      // each experiment can have different field lengths
      const IntVector& field_lens = allExperiments[exp_ind].field_lengths();
      for (size_t f_ind  = 0; f_ind < num_fields; ++f_ind) {
        for (size_t i=0; i<field_lens[f_ind]; ++i)
          resid2mult_indices[resid_offset++] = multiplier_offset;
        // only increment per-top-level response (each field has different mult)
        ++multiplier_offset;
      }
    }
    break;
  }

  case CALIBRATE_BOTH: {
    size_t num_scalar = simulationSRD.num_scalar_responses();
    size_t num_fields = simulationSRD.num_field_response_groups();
    size_t resid_offset = 0, multiplier_offset = 0;
    for (size_t exp_ind=0; exp_ind < numExperiments; ++exp_ind) {
    
      // don't reset the multiplier_offset; each exp, each resp has its own

      // iterate scalar responses, then fields
      for (size_t s_ind  = 0; s_ind < num_scalar; ++s_ind)
        resid2mult_indices[resid_offset++] = multiplier_offset++;
      
      // each experiment can have different field lengths
      const IntVector& field_lens = allExperiments[exp_ind].field_lengths();
      for (size_t f_ind  = 0; f_ind < num_fields; ++f_ind) {
        for (size_t i=0; i<field_lens[f_ind]; ++i)
          resid2mult_indices[resid_offset++] = multiplier_offset;
        // only increment per-top-level response (each field has different mult)
        ++multiplier_offset;
      }
    }	       
    break;
  }
    
  default:
    // unknown mode
    Cerr << "\nError: unknown multiplier mode in generate_multipliers().\n";
    abort_handler(-1);
    break;
      
  } // switch

}


StringArray ExperimentData::
hyperparam_labels(unsigned short multiplier_mode) const
{
  String cm_prefix("CovMult");
  StringArray hp_labels;

  switch(multiplier_mode) {

  case CALIBRATE_NONE:
    break;
    
  case CALIBRATE_ONE:
    hp_labels.push_back(cm_prefix);
    break;

  case CALIBRATE_PER_EXPER:
    for (size_t exp_ind=0; exp_ind < numExperiments; ++exp_ind) 
      hp_labels.
	push_back(cm_prefix + "Exp" + boost::lexical_cast<String>(exp_ind+1));
    break;
	
    // BMA TODO: Could use response labels here...
  case CALIBRATE_PER_RESP: {
    size_t num_resp = simulationSRD.num_scalar_responses() + 
      simulationSRD.num_field_response_groups();
    for (size_t resp_ind=0; resp_ind < num_resp; ++resp_ind)
      hp_labels.
	push_back(cm_prefix + "Resp" + boost::lexical_cast<String>(resp_ind+1));
    break;
  }

  case CALIBRATE_BOTH: {
    size_t num_resp = simulationSRD.num_scalar_responses() + 
      simulationSRD.num_field_response_groups();
    for (size_t exp_ind=0; exp_ind < numExperiments; ++exp_ind)
      for (size_t resp_ind=0; resp_ind < num_resp; ++resp_ind)
	hp_labels.
	  push_back(cm_prefix + "Exp" + boost::lexical_cast<String>(exp_ind+1) +
		    "Resp" + boost::lexical_cast<String>(resp_ind+1));
    break;
  }

  default:
    Cerr << "\nError: unkown multiplier mode in hyperparam_labels().\n";
    abort_handler(-1);

  }  

  return hp_labels;

}


}  // namespace Dakota
