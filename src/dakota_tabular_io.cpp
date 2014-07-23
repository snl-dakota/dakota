/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "dakota_data_io.hpp"
#include "dakota_tabular_io.hpp"
#include "DakotaResponse.hpp"
#include "DakotaVariables.hpp"

namespace Dakota {

// BMA TODO: Consider removing this namespace if not helpful in
// protecting certain components
namespace TabularIO {


//
//- Utilities for opening tabular files
//

void open_file(std::ifstream& data_file, const std::string& input_filename, 
	       const std::string& context_message) 
{
  // TODO: try/catch
  data_file.open(input_filename.c_str());
  if (!data_file.good()) {
    Cerr << "\nError (" << context_message << "): Could not open file " 
	 << input_filename << " for reading tabular data." << std::endl;
    abort_handler(-1);
  }
  // TODO (fix): can't except on failbit when trying to read to EOF
  //  data_file.exceptions(std::fstream::failbit | std::fstream::badbit);
  data_file.exceptions(std::fstream::badbit);
}


void open_file(std::ofstream& data_file, const std::string& output_filename, 
	       const std::string& context_message) 
{
  // TODO: try/catch
  data_file.open(output_filename.c_str());
  if (!data_file.good()) {
    Cerr << "\nError (" << context_message << "): Could not open file " 
	 << output_filename << " for writing tabular data." << std::endl;
    abort_handler(-1);
  }
  data_file.exceptions(std::fstream::failbit | std::fstream::badbit);
}


//
//- Utilities for tabular write
//

void write_header_tabular(std::ostream& tabular_ostream, 
  const Variables& vars, const Response& response,
  const std::string& counter_label, bool active_only, bool response_labels)
{
  tabular_ostream << '%' << counter_label << ' '; // matlab comment syntax
  if (active_only) {
    // output active variable labels
    Dakota::write_data_tabular(tabular_ostream, 
			       vars.continuous_variable_labels());
    Dakota::write_data_tabular(tabular_ostream, 
			       vars.discrete_int_variable_labels());
    Dakota::write_data_tabular(tabular_ostream, 
			       vars.discrete_real_variable_labels());
  }
  else {
    // output all variable labels
    Dakota::write_data_tabular(tabular_ostream, 
			       vars.all_continuous_variable_labels());
    Dakota::write_data_tabular(tabular_ostream, 
		       vars.all_discrete_int_variable_labels());
    Dakota::write_data_tabular(tabular_ostream, 
		       vars.all_discrete_real_variable_labels());
  }
  if (response_labels)
    Dakota::write_data_tabular(tabular_ostream, response.function_labels());
  tabular_ostream << std::endl;
}


// BMA: This writer needs to be updated to mirror other tabular write ordering
// possibly reviewed for removal...
void write_data_tabular(std::ostream& tabular_ostream, 
  const Variables& vars, const Response& response, size_t counter, 
  bool active_only, bool write_responses)
{
  if (active_only) {
    if (counter != _NPOS)
      tabular_ostream << std::setw(8) << counter << ' ';
    Dakota::write_data_tabular(tabular_ostream, vars.continuous_variables());
    Dakota::write_data_tabular(tabular_ostream, vars.discrete_int_variables());
    Dakota::write_data_tabular(tabular_ostream, vars.discrete_real_variables());
  }
  else {
    if (counter != _NPOS)
      tabular_ostream << std::setw(8) << counter << ' ';
    vars.write_tabular(tabular_ostream);
  }
  if (write_responses)
    response.write_tabular(tabular_ostream);
}


void write_data_tabular(const std::string& output_filename, 
			const std::string& context_message,
			const RealVectorArray& output_coeffs, 
			const UShort2DArray& output_indices)
{
  std::ofstream output_stream;
  open_file(output_stream, output_filename, context_message);

  size_t num_fns = output_coeffs.size();
  size_t num_ind_rows = output_indices.size();

  bool error_flag = false;
  if (num_fns == 0) {
    Cerr << "\nError (write_data_tabular): empty coefficient array." 
	 << std::endl;
    error_flag = true;
  }
  if (num_ind_rows == 0) {
    Cerr << "\nError (write_data_tabular): empty indices array." << std::endl;
    error_flag = true;
  }
  if (error_flag)  
    abort_handler(-1);

  size_t num_coeff_rows = output_coeffs[0].length();
  size_t num_vars = output_indices[0].size();
  if (num_coeff_rows != num_ind_rows) {
    Cerr << "\nError (write_data_tabular): mismatch in PCE coefficient and "
	 << "index lengths." << std::endl;
    error_flag = true;
  }
  if (num_vars == 0) {
    Cerr << "\nError (write_data_tabular): empty indices row." << std::endl;
    error_flag = true;
  }
  if (error_flag)
    abort_handler(-1);

  // TODO: consider removing TabularIO namespace
  for (size_t row = 0; row < num_coeff_rows; ++row) {
    for (size_t fn_ind = 0; fn_ind < num_fns; ++fn_ind)
      Dakota::
	write_data_tabular(output_stream, &output_coeffs[fn_ind][row], 1);
    Dakota::
      write_data_tabular(output_stream, &output_indices[row][0], num_vars);
    output_stream << std::endl;
  }
}


//
// Tabular read helpers
//

/** Discard header row from tabular file; alternate could read into a
    string array.  Requires header to be delimited by a newline. */
void read_header_tabular(std::istream& input_stream)
{
  String discard_labels;
  getline(input_stream, discard_labels);
}


bool exists_extra_data(std::istream& input_stream)
{
  // TODO: verify we need to check for non-whitespace
  //       shouldn't need both good and eof checks
  std::string extra_data;
  while (input_stream.good() && !input_stream.eof()) {
    try {
      input_stream >> extra_data;
    }
    catch (const std::ios_base::failure& failorbad_except) {
      return false;
    }
    std::string::const_iterator it = extra_data.begin();
    std::string::const_iterator it_end = extra_data.end();
    for( ; it != it_end; ++it)
      if (!std::isspace(*it))
	return true;
  }
  return false;
}


void read_data_tabular(const std::string& input_filename, 
		       const std::string& context_message,
		       RealVector& input_vector, size_t num_rows,
		       bool annotated)
{
  // TODO: handle both row and col vectors in the text?
  std::ifstream input_stream;
  open_file(input_stream, input_filename, context_message);

  if (annotated)
    read_header_tabular(input_stream);

  input_vector.resize(num_rows);
  try {
    if (annotated) {
      for (size_t row_ind = 0; row_ind < num_rows; ++row_ind) {
	// discard the row label (typically eval or data ID)
	size_t discard_row_label;
	input_stream >> discard_row_label;
	input_stream >> input_vector[row_ind];
      } 
    } else {
      // read raw whitespace separated data into vector
      read_data(input_stream, input_vector);
    }
  }
  catch (const std::ios_base::failure& failorbad_except) {
    Cout << "\nError (" << context_message << "): could not read file.";
    if (annotated) {
      Cout << "\nExpected header-annotated tabular file:"
	   << "\n  * header row with labels and " << num_rows << " data rows"
	   << "\n  * leading column with counter and 1 data column";
    }
    else {
      Cout << "\nExpected free-form tabular file: no leading row nor column; "
	   << num_rows << " whitespace-separated numeric data.";
    }
    Cout << std::endl;
    abort_handler(-1);
  }

  if (exists_extra_data(input_stream)) { 
    Cout << "\nWarning (" << context_message << "): "
	 << "found unexpected extra data in " 
	 << (annotated ? "header-annotated" : "free-form")
	 << "\nfile " << input_filename << "." << std::endl; 
  }

}



  // BMA TODO: use a helper to read each line.
void read_data_tabular(const std::string& input_filename, 
		       const std::string& context_message,
		       RealArray& input_vector, bool annotated,
		       size_t num_vars)
{
  std::ifstream input_stream;
  open_file(input_stream, input_filename, context_message);

  //  input_vector.resize(num_rows);
  try {

    if (annotated)
      read_header_tabular(input_stream);

    while (input_stream.good() && !input_stream.eof()) {
      if (annotated) {
	// discard the row label (typically eval or data ID)
	size_t discard_row_label;
	if (!(input_stream >> discard_row_label))
	  break;
      }
      for (size_t vi = 0; vi < num_vars; ++vi) {
	double read_value;
	if (!(input_stream >> read_value))
	  break;
	input_vector.push_back(read_value);
      }
    }
  }
  catch (const std::ios_base::failure& failorbad_except) {
    Cerr << "\nError (" << context_message << "): could not read file " 
	 << input_filename << ".";
    if (annotated) {
      Cout << "\nExpected header-annotated tabular file:"
	   << "\n  * header row with labels "
	   << "\n  * leading column with counter and " << num_vars << " data columns";
    }
    else {
      Cout << "\nExpected free-form tabular file: no leading row nor column; "
	   << num_vars << " columns of whitespace-separated numeric data.";
    }
    Cout << std::endl;
    abort_handler(-1);
  }
  catch(...) {
    Cerr << "\nError (" << context_message << "): could not read file " 
	 << input_filename << " (unknown error).";
    abort_handler(-1);
  }
}

/** Read possibly annotated data with unknown num_rows data into
    input_coeffs (num_fns x num_rows) and input_indices (num_rows x
    num_vars) */
void read_data_tabular(const std::string& input_filename, 
		       const std::string& context_message,
		       RealVectorArray& input_coeffs, 
		       UShort2DArray& input_indices, 
		       bool annotated, size_t num_vars, size_t num_fns)
{
  std::ifstream input_stream;
  open_file(input_stream, input_filename, context_message);
  
  // clear so we can append
  input_indices.clear();
  // need a temporary due to layout of returned data; this will be 
  // num_rows x num_fns
  Real2DArray coeffs_tmp;

  try {

    if (annotated)
      read_header_tabular(input_stream);

    while (input_stream.good() && !input_stream.eof()) {
      if (annotated) {
	// discard the row label (typically eval or data ID)
	size_t discard_row_label;
	if (!(input_stream >> discard_row_label))
	  break;
      }
      RealArray read_coeffs(num_fns);
      if (!(input_stream >> read_coeffs)) {
	if (annotated) {
	  // token required to exist
	  Cerr << "\nError (" << context_message << "): unexpected coeff read "
	       << "error in file " << input_filename << "." << std::endl;
	  abort_handler(-1);
	}
	else
	  // reached end of data (or TODO: other error)
	  break;
      }
      coeffs_tmp.push_back(read_coeffs);
      // read the (required) indices of length num_vars
      UShortArray index_set(num_vars, 0);
      // don't break as these are required data
      // use templated stream extraction from data_io
      if (!(input_stream >> index_set)) {
	Cerr << "\nError (" << context_message << "): unexpected indices read "
	     << "error in file " << input_filename << "." << std::endl;
	abort_handler(-1);
      }
      input_indices.push_back(index_set);
    }
  }
  catch (const std::ios_base::failure& failorbad_except) {
    Cerr << "\nError (" << context_message << "): could not read file " 
	 << input_filename << ".";
    if (annotated) {
      Cout << "\nExpected header-annotated tabular file:"
	   << "\n  * header row with labels "
	   << "\n  * leading column with counter and " << num_vars 
	   << " data columns";
    }
    else {
      Cout << "\nExpected free-form tabular file: no leading row nor column; "
	   << num_vars << " columns of whitespace-separated numeric data.";
    }
    Cout << std::endl;
    abort_handler(-1);
  }
  catch(...) {
    Cerr << "\nError (" << context_message << "): could not read file " 
	 << input_filename << " (unknown error).";
    abort_handler(-1);
  }

  size_t num_rows = coeffs_tmp.size();

  // transpose copy from coeffs_tmp to input_coeffs
  if (input_coeffs.size() != num_fns)
    input_coeffs.resize(num_fns);
  for (size_t fn_ind = 0; fn_ind < num_fns; ++fn_ind) {
    if (input_coeffs[fn_ind].length() != num_rows)
      input_coeffs[fn_ind].sizeUninitialized(num_rows);
    for (size_t row_ind = 0; row_ind < num_rows; ++row_ind)
      input_coeffs[fn_ind][row_ind] = coeffs_tmp[row_ind][fn_ind];
  }
}

void read_data_tabular(const std::string& input_filename, 
		       const std::string& context_message,
		       VariablesList& input_vars, ResponseList& input_resp,
		       const SharedVariablesData& svd,
		       size_t num_c_vars,
		       const ActiveSet& temp_set,
		       bool annotated,
		       bool verbose)
{
  std::ifstream data_file;
  open_file(data_file, input_filename, context_message);

  if (annotated)
    read_header_tabular(data_file);

  // shouldn't need both good and eof checks
  while (data_file.good() && !data_file.eof()) {
    // vars/responses must already be sized for istream read
    RealVector reuse_file_c_vars(num_c_vars);
    Response   reuse_file_responses(temp_set);
    try {
      if (annotated) {
	// discard the row label (typically eval or data ID)
	size_t discard_row_label;
	data_file >> discard_row_label;
      }
      // read_data() lacks eof checks
      Dakota::read_data_tabular(data_file, reuse_file_c_vars);
      reuse_file_responses.read_tabular(data_file);
    }
    // TODO: catch any
    catch (const std::ios_base::failure& failorbad_except) {
      break; // out of while loop
    }
    catch(String& err_msg) {
      //Cerr << "Warning: " << err_msg << std::endl;
      break; // out of while loop
    }
    Variables reuse_file_vars(svd); // instantiate-on-the-fly
    reuse_file_vars.continuous_variables(reuse_file_c_vars);
    if (verbose)
      Cout << "Variables and responses read:\n" 
	   << reuse_file_c_vars << reuse_file_responses;
    input_vars.push_back(reuse_file_vars);       // shallow copy
    input_resp.push_back(reuse_file_responses);  // shallow copy
  }

}


void read_data_tabular(const std::string& input_filename, 
		       const std::string& context_message,
		       RealMatrix& input_matrix, 
		       size_t num_rows,
		       size_t num_cols,
		       bool annotated,
		       bool verbose)
{
  std::ifstream input_stream;
  open_file(input_stream, input_filename, context_message);

  if (verbose) {
    Cout << "\nAttempting to read " << num_rows << " x " << num_cols << " = "
	 << num_rows*num_cols << " numeric data from " 
	 << (annotated ? "header-annotated" : "free-form")
	 << " file " << input_filename << "..." << std::endl;
  }

  if (annotated)
    read_header_tabular(input_stream);
  
  input_matrix.shapeUninitialized(num_rows, num_cols);	
  for (size_t row_ind = 0; row_ind < num_rows; ++row_ind) {
    try {
      if (annotated) {
	// discard the row label (typically eval or data ID)
	size_t discard_row_label;
	input_stream >> discard_row_label;
      }
      for (size_t col_ind = 0; col_ind < num_cols; ++col_ind)
	input_stream >> input_matrix(row_ind, col_ind);
    }
    catch (const std::ios_base::failure& failorbad_except) {
      Cout << "\nError (" << context_message << "): could not read file.";
      if (annotated) {
	Cout << "\nExpected header-annotated tabular file:"
	     << "\n  * header row with labels and " << num_rows << " data rows"
	     << "\n  * leading column with counter and " << num_cols 
	     << " data columns";
      }
      else {
	Cout << "\nExpected free-form tabular file: no leading row nor column; "
	     << num_rows*num_cols << " whitespace-separated numeric data.";
      }
      Cout << std::endl;
      abort_handler(-1);
    }
  }

  if (exists_extra_data(input_stream)) { 
    Cout << "\nWarning (" << context_message << "): "
	 << "found unexpected extra data in " 
	 << (annotated ? "header-annotated" : "free-form")
	 << "\nfile " << input_filename << "." << std::endl; 
  }
}


size_t read_vars_tabular(const std::string& input_filename, 
			 const std::string& context_message,
			 const SizetArray& vc_totals,
			 RealVectorArray& cva, IntVectorArray& diva, 
			 StringMulti2DArray& dsva, RealVectorArray& drva,
			 bool annotated)
{
  size_t num_evals = 0;

  // Consider passing this info in?
  //  size_t num_vars = numContinuousVars     + numDiscreteIntVars
  //  + numDiscreteStringVars + numDiscreteRealVars;
  size_t num_cv  = 
    vc_totals[TOTAL_CDV]   + vc_totals[TOTAL_CAUV] + 
    vc_totals[TOTAL_CEUV]  + vc_totals[TOTAL_CSV];
  size_t num_div = 
    vc_totals[TOTAL_DDIV]  + vc_totals[TOTAL_DAUIV] + 
    vc_totals[TOTAL_DEUIV] + vc_totals[TOTAL_DSIV];
  size_t num_dsv = 
    vc_totals[TOTAL_DDSV]  + vc_totals[TOTAL_DAUSV] + 
    vc_totals[TOTAL_DEUSV] + vc_totals[TOTAL_DSSV];
  size_t num_drv = 
    vc_totals[TOTAL_DDRV]  + vc_totals[TOTAL_DAURV] +
    vc_totals[TOTAL_DEURV] + vc_totals[TOTAL_DSRV];
  size_t num_vars = num_cv + num_div + num_dsv + num_drv;

  RealVector        cv(num_cv);
  IntVector        div(num_div);
  StringMultiArray dsv(boost::extents[num_dsv]);      
  RealVector       drv(num_drv);
  
  std::ifstream input_stream;
  open_file(input_stream, input_filename, context_message);

  try {


    if (annotated) {
      input_stream >> std::ws;
      read_header_tabular(input_stream);
    }

    input_stream >> std::ws;  // advance to next readable input
    while (input_stream.good() && !input_stream.eof()) {
      if (annotated) {
	// discard the row label (typically eval or data ID)
	size_t discard_row_label;
	if (!(input_stream >> discard_row_label))
	  break;
      }
      
      read_vars_tabular(input_stream, vc_totals, cv, div, dsv, drv);

      ++num_evals;

      cva.push_back(cv);
      diva.push_back(div);
      // opting for linear growth for now instead of a temporary...
      dsva.resize(boost::extents[num_evals][num_dsv]);
      dsva[num_evals-1] = dsv;
      drva.push_back(drv);

      input_stream >> std::ws;  // advance to next readable input
    }
  }
  catch (const std::ios_base::failure& failorbad_except) {
    Cerr << "\nError (" << context_message << "): could not read file " 
	 << input_filename << ".";
    if (annotated) {
      Cout << "\nExpected header-annotated tabular file:"
	   << "\n  * header row with labels "
	   << "\n  * leading column with counter and " << num_vars 
	   << " data columns";
    }
    else {
      Cout << "\nExpected free-form tabular file: no leading row nor column; "
	   << num_vars << " columns of whitespace-separated numeric data.";
    }
    Cout << std::endl;
    abort_handler(-1);
  }
  catch(const String& str_except) {
    Cerr << "\nError (" << context_message << "): could not read file " 
	 << input_filename << ";\n  " << str_except << "." << std::endl;
    abort_handler(-1);

  }
  catch(...) {
    Cerr << "\nError (" << context_message << "): could not read file " 
	 << input_filename << " (unknown error)." << std::endl;
    abort_handler(-1);
  }

  return num_evals;
}


/** Tabular reader that reads data in order design, aleatory,
    epistemic, state according to counts in vc_totals (extract in
    order: cdv/ddiv/ddrv, cauv/dauiv/daurv, ceuv/deuiv/deurv,
    csv/dsiv/dsrv, which might reflect active or all depending on
    context. Assumes container sized, since might be a view into a
    larger array.

    Used in MixedVariables:read_tabular and by above fn for ParamStudy
*/
void read_vars_tabular(std::istream& s, const SizetArray& vc_totals,
		       RealVector& cv, IntVector& div, 
		       StringMultiArray& dsv, RealVector& drv)
{

  size_t num_cdv = vc_totals[TOTAL_CDV], num_ddiv = vc_totals[TOTAL_DDIV],
    num_ddsv  = vc_totals[TOTAL_DDSV],  num_ddrv  = vc_totals[TOTAL_DDRV],
    num_cauv  = vc_totals[TOTAL_CAUV],  num_dauiv = vc_totals[TOTAL_DAUIV],
    num_dausv = vc_totals[TOTAL_DAUSV], num_daurv = vc_totals[TOTAL_DAURV],
    num_ceuv  = vc_totals[TOTAL_CEUV],  num_deuiv = vc_totals[TOTAL_DEUIV],
    num_deusv = vc_totals[TOTAL_DEUSV], num_deurv = vc_totals[TOTAL_DEURV],
    num_csv   = vc_totals[TOTAL_CSV],   num_dsiv  = vc_totals[TOTAL_DSIV],
    num_dssv  = vc_totals[TOTAL_DSSV],  num_dsrv  = vc_totals[TOTAL_DSRV],
    acv_offset = 0, adiv_offset = 0, adsv_offset = 0, adrv_offset = 0;

  // read design variables
  read_data_partial_tabular(s, acv_offset,  num_cdv,  cv);
  read_data_partial_tabular(s, adiv_offset, num_ddiv, div);
  read_data_partial_tabular(s, adsv_offset, num_ddsv, dsv);
  read_data_partial_tabular(s, adrv_offset, num_ddrv, drv);
  acv_offset  += num_cdv;  adiv_offset += num_ddiv;
  adsv_offset += num_ddsv; adrv_offset += num_ddrv;

  // read aleatory uncertain variables
  read_data_partial_tabular(s, acv_offset,  num_cauv,  cv); 
  read_data_partial_tabular(s, adiv_offset, num_dauiv, div);
  read_data_partial_tabular(s, adsv_offset, num_dausv, dsv);
  read_data_partial_tabular(s, adrv_offset, num_daurv, drv);
  acv_offset  += num_cauv;  adiv_offset += num_dauiv;
  adsv_offset += num_dausv; adrv_offset += num_daurv;

  // read epistemic uncertain variables
  read_data_partial_tabular(s, acv_offset,  num_ceuv,  cv); 
  read_data_partial_tabular(s, adiv_offset, num_deuiv, div);
  read_data_partial_tabular(s, adsv_offset, num_deusv, dsv);
  read_data_partial_tabular(s, adrv_offset, num_deurv, drv);
  acv_offset  += num_ceuv;  adiv_offset += num_deuiv;
  adsv_offset += num_deusv; adrv_offset += num_deurv;

  // read state variables
  read_data_partial_tabular(s, acv_offset,  num_csv,  cv); 
  read_data_partial_tabular(s, adiv_offset, num_dsiv, div);
  read_data_partial_tabular(s, adsv_offset, num_dssv, dsv);
  read_data_partial_tabular(s, adrv_offset, num_dsrv, drv);
  //acv_offset  += num_csv;  adiv_offset += num_dsiv;
  //adsv_offset += num_dssv; adrv_offset += num_dsrv;

}



} // namespace TabularIO

} // namespace Dakota
