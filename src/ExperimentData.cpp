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
  scalarDataFormat(TABULAR_EXPER_ANNOT), interpolateFlag(false), 
  outputLevel(NORMAL_OUTPUT)
{  /* empty ctor */  }                                


ExperimentData::
ExperimentData(const ProblemDescDB& pddb, 
	       const SharedResponseData& srd, short output_level):
  calibrationDataFlag(pddb.get_bool("responses.calibration_data")),
  numExperiments(pddb.get_sizet("responses.num_experiments")), 
  numConfigVars(pddb.get_sizet("responses.num_config_vars")),
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
  calibrationDataFlag(true), 
  numExperiments(num_experiments), numConfigVars(num_config_vars),
  dataPathPrefix(data_prefix), scalarDataFilename(scalar_data_filename),
  scalarDataFormat(TABULAR_EXPER_ANNOT), scalarSigmaPerRow(0),
  readSimFieldCoords(false), interpolateFlag(false), outputLevel(output_level)
{
  initialize(variance_types, srd);
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
    Cout << "Constructing experiment response" << std::endl;
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

  if (!scalar_data_file) { 
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
        throw std::runtime_error("Expected "+convert_to_string(numConfigVars)+" experiment "
            + "config variables but the required file \""+config_vars_basepath.string()
            +"\" does not exist.");
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
    Cout << "Experiment data summary:";
    if (numConfigVars > 0)
      Cout << "Values of experiment configuration variables:\n" 
	   << allConfigVars << "\n";
    for (size_t i=0; i<numExperiments; ++i) {
      Cout << "\n  Data values, experiment " << i << "\n";
      allExperiments[i].write(Cout);
    }
  }

  // store the lengths (number of functions) of each experiment
  per_exp_length(experimentLengths);
  size_t i, num_exp = allExperiments.size();
  expOffsets.sizeUninitialized(num_exp);
  expOffsets(0) = 0;
  for (i=1; i<num_exp; i++) 
    expOffsets(i) = experimentLengths(i-1) + expOffsets(i-1);

  // TODO: exists extra data in scalar_data_stream

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

const RealVector& ExperimentData::
config_vars(size_t experiment)
{
  return(allConfigVars[experiment]);
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
field_data_view(size_t response, size_t experiment)
{
  return(allExperiments[experiment].field_values_view(response));
}

RealMatrix ExperimentData::
field_coords_view(size_t response, size_t experiment)
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

bool ExperimentData::interpolate_flag()
{
  return interpolateFlag;
}

RealVector ExperimentData::
residuals_view( const RealVector& residuals, size_t experiment ){
  int exp_offset = expOffsets[experiment];
  RealVector exp_resid(Teuchos::View, residuals.values()+exp_offset,
		       experimentLengths[experiment]);
  return exp_resid;
}
  
/// Return a view (to allowing updaing in place) of the gradients associated
/// with a given experiment, from a matrix contaning gradients from
/// all experiments
RealMatrix ExperimentData::
gradients_view( const RealMatrix &gradients, size_t experiment){
  int exp_offset = expOffsets[experiment];
  RealMatrix exp_grads(Teuchos::View, gradients, gradients.numRows(),
		       experimentLengths[experiment], 0, exp_offset );
  return exp_grads;
}
  
/// Return a view (to allowing updaing in place) of the hessians associated
/// with a given experiment, from an array contaning the hessians from 
/// all experiments
RealSymMatrixArray ExperimentData::
hessians_view( const RealSymMatrixArray &hessians, 
	       size_t experiment ){
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
apply_covariance(const RealVector& residuals, size_t experiment)
{
  RealVector exp_resid = residuals_view( residuals, experiment );
  if ( variance_active() )
    return(allExperiments[experiment].apply_covariance(exp_resid));
  else 
    return exp_resid.dot( exp_resid );
}

void ExperimentData::
apply_covariance_inv_sqrt(const RealVector& residuals, size_t experiment, 
			  RealVector& weighted_residuals)
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

void ExperimentData::
apply_covariance_inv_sqrt(const RealMatrix& gradients, size_t experiment, 
			  RealMatrix& weighted_gradients)
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

void ExperimentData::
apply_covariance_inv_sqrt(const RealSymMatrixArray& hessians, size_t experiment,
			  RealSymMatrixArray& weighted_hessians)
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

void ExperimentData::get_main_diagonal( RealVector &diagonal, 
					size_t experiment ) const{
  if ( !variance_active() )
    throw( std::runtime_error("ExperimentData::get_main_diagonal - covariance matrix is empty") );
    allExperiments[experiment].get_covariance_diagonal( diagonal );
}

void ExperimentData::
form_residuals(const Response& sim_resp, const ShortArray &total_asv, 
	       Response &residual_resp )
{
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
form_residuals(const Response& sim_resp, size_t exp_ind, 
	       const ShortArray &total_asv, size_t exp_offset, 
	       Response &residual_resp )
{
  size_t res_size = allExperiments[exp_ind].function_values().length();

  RealVector resid_fns = sim_resp.function_values();
  size_t i,j;
  const IntVector simLengths = sim_resp.field_lengths();
  int numfields = num_fields();

  RealVector all_residuals = residual_resp.function_values_view();
  RealVector residuals(Teuchos::View, all_residuals.values()+exp_offset,
		       res_size);

  if (outputLevel >= DEBUG_OUTPUT) 
    Cout << "interpolate " << interpolateFlag << '\n';
  if (!interpolateFlag) {

    short asv = total_asv[exp_ind];
    RealMatrix sim_grads = sim_resp.function_gradients_view();
    RealSymMatrixArray sim_hessians = sim_resp.function_hessians_view();

    //resid_fns -= allExperiments[exp_ind].function_values();
    // residuals = resid_fns;
    for (i=0; i<resid_fns.length(); i++){
      residuals[i]=
	resid_fns[i]-allExperiments[exp_ind].function_value(i);

      if ( asv & 2 ){
	RealVector sim_grad_i = 
	  Teuchos::getCol(Teuchos::View, sim_grads, (int)i);
	residual_resp.function_gradient(sim_grad_i, exp_offset+i);
      }

      if ( asv & 4 )
	residual_resp.function_hessian(sim_hessians[i], exp_offset+i);
    }
  }else{   
    if (num_scalars() > 0) {
      for (i=0; i<num_scalars(); i++) 
        residuals[i]= resid_fns[i]-allExperiments[exp_ind].function_value(i);
    }

    // interpolate the simulation data onto the coordinates of the experiment
    // data

    // I think resisuals are stored in continguous order, 
    // [exp1(scalars,fields),...,exp_n(scalars,fields)
    // if so need to pass exp_offset to interpolate function above
    // and inside that function set offset =  exp_offset and 
    // then increment in usual way
    interpolate_simulation_data(sim_resp, exp_ind, total_asv, exp_offset,
				residual_resp);

    if (outputLevel >= DEBUG_OUTPUT) 
      Cout << "interp values" << residuals << '\n';

    if (total_asv[exp_ind] & 1) {
      // compute the residuals, i.e. subtract the experiment data values
      // from the (interpolated) simulation values.
      size_t cntr = num_scalars();
      for (i=0; i<num_fields(); i++){
	size_t num_field_fns = field_data_view(i,exp_ind).length();
	for (j=0; j<num_field_fns; j++, cntr++)
	  residuals[cntr] -= field_data_view(i,exp_ind)[j];
      }
      if (outputLevel >= DEBUG_OUTPUT) 
	Cout << "residuals in exp space" << residuals << '\n';
    }
  }
}

void ExperimentData::
interpolate_simulation_data( const Response &sim_resp, size_t exp_ind,
			     const ShortArray &total_asv, size_t exp_offset,
			     Response &interp_resp ){
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

ShortArray ExperimentData::
determine_active_request( const Response& resid_resp,
			  bool interogate_field_data ){
  IntVector experiment_lengths;
  per_exp_length(experiment_lengths);
  ShortArray total_asv( numExperiments );

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

    if (outputLevel >= DEBUG_OUTPUT && total_asv[exp_ind] > 0)
      Cout << "\nLeast squares: weighting least squares terms with inverse of "
	   << "specified error\n               covariance." << std::endl;
           
    calib_term_ind += num_fns_exp;
  }  // for each experiment
  return(total_asv);
}

void ExperimentData::
scale_residuals(const Response& residual_response, ShortArray &total_asv,
                RealVector& weighted_resid) {

  for (size_t exp_ind = 0; exp_ind < numExperiments; ++exp_ind) {

    // apply noise covariance to the residuals for this experiment 
    // and store in correct place in weighted_residual
    if (outputLevel >= DEBUG_OUTPUT && total_asv[exp_ind] > 0)
      Cout << "\nCalibration: weighting residuals with inverse of specified "
           << "error covariance." << std::endl;
       
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


void ExperimentData::
scale_residuals( Response& residual_response, ShortArray &total_asv ){
  IntVector experiment_lengths;
  per_exp_length(experiment_lengths);

  size_t calib_term_ind = 0; // index into the total set of calibration terms
  for (size_t exp_ind = 0; exp_ind < numExperiments; ++exp_ind){
    // total length this exper
    size_t num_fns_exp = experiment_lengths[exp_ind]; 
    
    // apply noise covariance to the residuals for this experiment 
    // and store in correct place in residual_response
    if (outputLevel >= DEBUG_OUTPUT && total_asv[exp_ind] > 0)
      Cout << "\nCalibration: weighting residuals with inverse of specified "
           << "error covariance." << std::endl;
       
    // BMA @JDJ: The copies in the else clauses lmay have a size
    // mismatch, with residual_response having total calibration terms
    // being assigned to a left side that should be sized for one
    // experiment

    // apply cov_inv_sqrt to the residual vector
    RealVector weighted_resid;
    if (total_asv[exp_ind] & 1)
      apply_covariance_inv_sqrt(residual_response.function_values(),
				exp_ind, weighted_resid);
    else
      weighted_resid = residual_response.function_values();

    // apply cov_inv_sqrt to each row of gradient matrix
    RealMatrix weighted_grad;
    if (total_asv[exp_ind] & 2) {
      apply_covariance_inv_sqrt(residual_response.function_gradients(),
				exp_ind, weighted_grad);
    }
    else
      weighted_grad = residual_response.function_gradients();

    // apply cov_inv_sqrt to non-contiguous Hessian matrices
    RealSymMatrixArray weighted_hess;
    if (total_asv[exp_ind] & 4)
      apply_covariance_inv_sqrt(residual_response.function_hessians(), 
				exp_ind, weighted_hess);
    // JDJ it seems like the following will fail if (total_asv[exp_ind] & 4)
    // is false as a copy or view of weighted hess is not made, it is instead
    // just empty. Confirm with BMA

    //Todo: BMA/JDJ Only perform copy of resid, grads or hess if covariance
    //matrix was applied to that data
    copy_field_data(weighted_resid, weighted_grad, weighted_hess, 
		    calib_term_ind, num_fns_exp, residual_response);

    calib_term_ind += num_fns_exp;
  } // for each experiment
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
  bool apply_covariance = ( ( variance_type_active(MATRIX_SIGMA) ) ||
			    ( variance_type_active(SCALAR_SIGMA) ) || 
			    ( variance_type_active(DIAGONAL_SIGMA) ) );

  RealVector scaled_residuals;
  RealMatrix scaled_gradients;
  if ( apply_covariance ){
    apply_covariance_inv_sqrt(resp.function_values(), exp_ind,
			      scaled_residuals);
    apply_covariance_inv_sqrt(resp.function_gradients(), exp_ind,
			      scaled_gradients);
  }else{
    scaled_residuals = residuals_view( resp.function_values(), exp_ind );
    scaled_gradients = gradients_view( resp.function_gradients(), exp_ind );
  }

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
  const RealSymMatrixArray &func_hessians = resp.function_hessians();
  const RealMatrix &func_gradients = resp.function_gradients();
  const RealVector &residuals = resp.function_values();
  
  bool apply_covariance = ( ( variance_type_active(MATRIX_SIGMA) ) ||
			    ( variance_type_active(SCALAR_SIGMA) ) || 
			    ( variance_type_active(DIAGONAL_SIGMA) ) );

  RealVector scaled_residuals;
  RealMatrix scaled_gradients;
  RealSymMatrixArray scaled_hessians;
  if ( apply_covariance ){
    apply_covariance_inv_sqrt(residuals, exp_ind, scaled_residuals);
    apply_covariance_inv_sqrt(func_gradients, exp_ind, scaled_gradients);
    apply_covariance_inv_sqrt(func_hessians, exp_ind, scaled_hessians);
  }else{
    scaled_residuals = residuals_view( residuals, exp_ind );
    scaled_gradients = gradients_view( func_gradients, exp_ind );
    scaled_hessians = hessians_view( func_hessians, exp_ind );
  }

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


/** In-place scaling of residuals by observation error multipliers
 */
void ExperimentData::
scale_residuals(const RealVector& multipliers, short multiplier_mode,
                RealVector& residuals) 
{
  // TODO: consistent handling of ASV across non-experiment cases
  // NOTE: In most general case the index into multipliers is exp_ind
  // * num_response_groups + resp_ind (never per function ind, e.g.,
  // in a field)
  
  switch (multiplier_mode) {

  case CALIBRATE_NONE:
    // no-op
    break;

  case CALIBRATE_ONE:
    assert(multipliers.length() == 1);
    residuals *= (1.0/sqrt(multipliers[0]));
    break;

  case CALIBRATE_PER_EXPER: {
    assert(multipliers.length() == numExperiments);
    size_t residual_offset = 0;
    for (size_t exp_ind = 0; exp_ind < numExperiments; ++exp_ind) {
      // BMA TODO: try using view; hasn't worked as expected!
      //      RealVector exp_resid(residuals_view(residuals, exp_ind));
      //      exp_resid *= (1.0/sqrt(multipliers[exp_ind]));
      for (size_t fn_ind = 0; fn_ind < experimentLengths[exp_ind]; ++fn_ind)
	residuals[residual_offset + fn_ind] /= sqrt(multipliers[exp_ind]);
      residual_offset += experimentLengths[exp_ind];
    }
    break;
  }

  case CALIBRATE_PER_RESP: {
    assert(multipliers.length() == simulationSRD.num_response_groups());

    size_t residual_offset = 0;
    size_t num_scalar = simulationSRD.num_scalar_responses();
    size_t num_fields = simulationSRD.num_field_response_groups();

    for (size_t exp_ind = 0; exp_ind < numExperiments; ++exp_ind) {

      // each response has a different multiplier, but they don't
      // differ across experiments
      size_t multiplier_offset = 0;

      // iterate scalar responses, then fields
      // BMA TODO: encapsulate in ExperimentResponse?
      for (size_t s_ind  = 0; s_ind < num_scalar; ++s_ind) {
        residuals[residual_offset] /= sqrt(multipliers[multiplier_offset]);
	++residual_offset;
	++multiplier_offset;
      } 

      // each experiment can have different field lengths
      const IntVector& field_lens = allExperiments[exp_ind].field_lengths();
      for (size_t f_ind  = 0; f_ind < num_fields; ++f_ind) {
        Real sqrt_mult = sqrt(multipliers[multiplier_offset]);
	for (size_t fc_ind = 0; fc_ind < field_lens[f_ind]; ++fc_ind) {
	  residuals[residual_offset] /= sqrt_mult;
	  ++residual_offset;
	}
	++multiplier_offset;
      }
    }
    break;
  }

  case CALIBRATE_BOTH: {

    assert(multipliers.length() == 
	   numExperiments*simulationSRD.num_response_groups());

    size_t residual_offset = 0, multiplier_offset = 0;
    size_t num_scalar = simulationSRD.num_scalar_responses();
    size_t num_fields = simulationSRD.num_field_response_groups();
      
    for (size_t exp_ind = 0; exp_ind < numExperiments; ++exp_ind) {

      // iterate scalar responses, then fields
      // TODO: encapsulate in ExperimentResponse?
      for (size_t s_ind  = 0; s_ind < num_scalar; ++s_ind) {
	residuals[residual_offset] /= sqrt(multipliers[multiplier_offset]);
	++residual_offset;
	++multiplier_offset;
      } 

      // each experiment can have different field lengths
      const IntVector& field_lens = allExperiments[exp_ind].field_lengths();
      for (size_t f_ind  = 0; f_ind < num_fields; ++f_ind) {
	Real mult = sqrt(multipliers[multiplier_offset]);
	for (size_t fc_ind = 0; fc_ind < field_lens[f_ind]; ++fc_ind) {
	  residuals[residual_offset] /= mult;
	  ++residual_offset;
	}
	++multiplier_offset;
      }

    }	       
    break;
  }

  default:
    // unknown mode
    Cerr << "\nError: scale_residuals() called with unknown multiplier mode.\n";
    abort_handler(-1);
    break;
  }
}


/** Determinant of the total covariance used in inference, which has
    blocks mult_i * I * Cov_i. */
Real ExperimentData::scaled_cov_determinant(const RealVector& multipliers, 
					    short multiplier_mode)
{
  Real det = 1.0;
  if (!variance_active())  // short-circuit if no data covariance
    return det;

  size_t multiplier_offset = 0;
  for (size_t exp_ind=0; exp_ind < numExperiments; ++exp_ind) {

    // for each experiment, accumulate det(mult*I*Cov)
    det *= allExperiments[exp_ind].covariance_determinant();

    switch (multiplier_mode) {

    case CALIBRATE_NONE:
      // no-op
      break;

    case CALIBRATE_ONE:
      assert(multipliers.length() == 1);
      det *= std::pow(multipliers[0], allExperiments[exp_ind].num_functions());
      break;

    case CALIBRATE_PER_EXPER:
      assert(multipliers.length() == numExperiments);
      det *= std::pow(multipliers[exp_ind], 
		      allExperiments[exp_ind].num_functions());
      break;

    case CALIBRATE_PER_RESP: {
      assert(multipliers.length() == simulationSRD.num_response_groups());
      size_t num_scalar = simulationSRD.num_scalar_responses();
      size_t num_fields = simulationSRD.num_field_response_groups();
      
      // each response has a different multiplier, but they don't
      // differ across experiments
      multiplier_offset = 0;
      
      // iterate scalar responses, then fields
      for (size_t s_ind  = 0; s_ind < num_scalar; ++s_ind) {
	det *= multipliers[multiplier_offset];
	++multiplier_offset;
      } 
      // each experiment can have different field lengths
      const IntVector& field_lens = allExperiments[exp_ind].field_lengths();
      for (size_t f_ind  = 0; f_ind < num_fields; ++f_ind) {
	det *= std::pow(multipliers[multiplier_offset], field_lens[f_ind]);
	++multiplier_offset;
      }
      break;
    }

    case CALIBRATE_BOTH: {

      assert(multipliers.length() == 
	     numExperiments*simulationSRD.num_response_groups());
      
      size_t num_scalar = simulationSRD.num_scalar_responses();
      size_t num_fields = simulationSRD.num_field_response_groups();
      
      // don't reset the multiplier_offset; each exp, each resp has its own

      // iterate scalar responses, then fields
      for (size_t s_ind  = 0; s_ind < num_scalar; ++s_ind) {
	det *= multipliers[multiplier_offset];
	++multiplier_offset;
      } 

      // each experiment can have different field lengths
      const IntVector& field_lens = allExperiments[exp_ind].field_lengths();
      for (size_t f_ind  = 0; f_ind < num_fields; ++f_ind) {
	det *= std::pow(multipliers[multiplier_offset], field_lens[f_ind]);
	++multiplier_offset;
      }
      break;
    }	       
      
    default:
      // unknown mode
      Cerr << "\nError: scale_residuals() called with unknown multiplier mode.\n";
      abort_handler(-1);
      break;

    } // switch

  }  // for each experiment

  return det;
}


}  // namespace Dakota
