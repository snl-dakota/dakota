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
			  const std::string& counter_label)
{
  // headers use Matlab comment syntax
  tabular_ostream << "%" << counter_label;
  if (TABULAR_IFACE_ID)
    tabular_ostream << " interface ";
  vars.write_tabular_labels(tabular_ostream);
  response.write_tabular_labels(tabular_ostream);
}


void write_leading_columns(std::ostream& tabular_ostream, size_t eval_id, 
			   const String& iface_id)
{
  tabular_ostream << std::setw(8) << eval_id << ' ';
  if (TABULAR_IFACE_ID) {
    // write the interface ID string, EMPTY for empty
    if (iface_id.empty())
      tabular_ostream << std::setw(9) << "EMPTY";
    else 
      tabular_ostream << std::setw(9) << iface_id << ' ';
  }
}


void write_data_tabular(std::ostream& tabular_ostream, 
			const Variables& vars, const String& iface_id, 
			const Response& response, size_t counter,
			bool annotated)
{
  // write evaluation ID and interface ID
  if (annotated)
    write_leading_columns(tabular_ostream, counter, iface_id);
  vars.write_tabular(tabular_ostream);
  response.write_tabular(tabular_ostream);
}


// PCE export 
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
  std::cout << num_coeff_rows << " <---> " << num_ind_rows << std::endl;
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
//- Utilities for tabular read
//


/** Discard header row from tabular file; alternate could read into a
    string array.  Requires header to be delimited by a newline. */
void read_header_tabular(std::istream& input_stream, bool annotated)
{
  if (annotated) {
    String discard_labels;
    getline(input_stream, discard_labels);
  }
}


/**  for now we discard the interface data; later will return for validation */
size_t read_leading_columns(std::istream& input_stream, bool annotated)
{
  size_t row_label = _NPOS;
  if (annotated) {
    input_stream >> row_label;
    if (TABULAR_IFACE_ID) {
      String iface_id;
      input_stream >> iface_id;
      if (iface_id == "EMPTY")
	iface_id.clear();
    }
  }
  // else no-op
  return row_label;
}


bool exists_extra_data(std::istream& input_stream)
{
  input_stream >> std::ws;
  while (input_stream.good() && !input_stream.eof()) {
    try {
      std::string extra_data;
      input_stream >> extra_data;
      if (!extra_data.empty())
	return true;
    }
    catch (const std::ios_base::failure& failorbad_except) {
      // TODO: report error in this branch
      return false;
    }
  }
  return false;
}


void read_data_tabular(const std::string& input_filename, 
		       const std::string& context_message,
		       RealVector& input_vector, size_t num_entries,
		       bool annotated)
{
  // TODO: handle both row and col vectors in the text?
  std::ifstream input_stream;
  open_file(input_stream, input_filename, context_message);

  if (annotated) {
    input_stream >> std::ws;
    read_header_tabular(input_stream, annotated);
  }

  input_vector.resize(num_entries);
  try {
    if (annotated) {
      for (size_t row_ind = 0; row_ind < num_entries; ++row_ind) {
	input_stream >> std::ws;
	// discard the leading cols (typically eval or data ID, iface ID)
	read_leading_columns(input_stream, annotated);
	input_stream >> input_vector[row_ind];
      } 
    } else {
      // read raw whitespace separated data into (sized) vector
      input_stream >> std::ws;
      read_data(input_stream, input_vector);
    }
  }
  catch (const std::ios_base::failure& failorbad_except) {
    Cout << "\nError (" << context_message << "): could not read file.";
    if (annotated) {
      Cout << "\nExpected header-annotated tabular file:"
	   << "\n  * header row with labels and " << num_entries << " data rows"
	   << "\n  * leading column with counter and 1 data column";
    }
    else {
      Cout << "\nExpected free-form tabular file: no leading row nor column; "
	   << num_entries << " whitespace-separated numeric data.";
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
  // BMA TODO: what to do about discrete vars, esp string?
void read_data_tabular(const std::string& input_filename, 
		       const std::string& context_message,
		       Variables vars, size_t num_fns,
		       RealArray& input_vector, bool annotated,
		       bool active_only)
{
  std::ifstream input_stream;
  open_file(input_stream, input_filename, context_message);

  size_t num_vars = active_only ? 
    vars.cv() + vars.div() + vars.dsv() + vars.drv() : vars.tv();

  //  input_vector.resize(num_rows);
  try {

    if (annotated) {
      input_stream >> std::ws;
      read_header_tabular(input_stream, annotated);
    }

    input_stream >> std::ws;
    while (input_stream.good() && !input_stream.eof()) {

      // discard any leading columns
      read_leading_columns(input_stream, annotated);

      // use a variables object because it knows how to read active vs. all
      vars.read_tabular(input_stream, active_only);

      // Extract the continuous variables
      // TODO: handle discrete variable types
      const RealVector& c_vars = active_only ? vars.continuous_variables() :
	vars.all_continuous_variables();
      std::copy(c_vars.values(), c_vars.values()+c_vars.length(), 
		std::back_inserter(input_vector));

      // read the raw function data
      for (size_t fi = 0; fi < num_fns; ++fi) {
	double read_value = std::numeric_limits<double>::quiet_NaN();
	if (input_stream >> read_value)
	  input_vector.push_back(read_value);
      }
      input_stream >> std::ws;
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

    if (annotated) {
      input_stream >> std::ws;
      read_header_tabular(input_stream, annotated);
    }

    input_stream >> std::ws;
    while (input_stream.good() && !input_stream.eof()) {

      // discard any leading columns; annotated is unlikely in this case
      read_leading_columns(input_stream, annotated);

      // read the (required) coefficients of length num_fns
      RealArray read_coeffs(num_fns, std::numeric_limits<double>::quiet_NaN());
      if (!(input_stream >> read_coeffs)) {
	Cout << "read: " << read_coeffs << std::endl;
	Cerr << "\nError (" << context_message << "): unexpected coeff read "
	     << "error in file " << input_filename << "." << std::endl;
	abort_handler(-1);
      }
      Cout << "read: " << read_coeffs << std::endl;
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
      Cout << "index set: " << index_set << std::endl;
      input_stream >> std::ws;
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


// Data fit surrogate import, which might only read active variables
void read_data_tabular(const std::string& input_filename, 
		       const std::string& context_message,
		       Variables vars, Response resp,
		       VariablesList& input_vars, ResponseList& input_resp,
		       bool annotated,
		       bool verbose, bool active_only
		       )
{
  std::ifstream data_file;
  open_file(data_file, input_filename, context_message);

  if (annotated) {
    data_file >> std::ws;
    read_header_tabular(data_file, annotated);
  }

  // shouldn't need both good and eof checks
  data_file >> std::ws;
  while (data_file.good() && !data_file.eof()) {
    try {
      // discard the leading columns 
      read_leading_columns(data_file, annotated);
      vars.read_tabular(data_file, active_only);
      resp.read_tabular(data_file);
    }
    // TODO: catch any; don't break on these errors!
    catch (const std::ios_base::failure& failorbad_except) {
      break; // out of while loop
    }
    catch(String& err_msg) {
      //Cerr << "Warning: " << err_msg << std::endl;
      break; // out of while loop
    }

    if (verbose)
      Cout << "Variables and responses read:\n" << vars << resp;
    input_vars.push_back(vars.copy());       // deep copy
    input_resp.push_back(resp.copy());  // deep copy

    // advance so EOF can detect properly
    data_file >> std::ws;
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

  if (annotated) {
    input_stream >> std::ws;
    read_header_tabular(input_stream, annotated);
  }

  input_matrix.shapeUninitialized(num_rows, num_cols);	
  for (size_t row_ind = 0; row_ind < num_rows; ++row_ind) {
    try {
      // experiment data would never have an interface ID
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


// special reader for list parameter studies: probably move back to ParamStudy
size_t read_data_tabular(const std::string& input_filename, 
			 const std::string& context_message,
			 RealVectorArray& cva, IntVectorArray& diva, 
			 StringMulti2DArray& dsva, RealVectorArray& drva,
			 bool annotated, bool active_only, Variables vars)
{
  size_t num_evals = 0, num_vars = vars.tv();
  // temporary dynamic container to read string variables
  std::vector<StringMultiArray> list_dsv_points;

  std::ifstream input_stream;
  open_file(input_stream, input_filename, context_message);
 
  try {

    if (annotated) {
      input_stream >> std::ws;
      read_header_tabular(input_stream, annotated);
    }

    input_stream >> std::ws;  // advance to next readable input
    while (input_stream.good() && !input_stream.eof()) {
      if (annotated) {
	// discard the row labels (typically eval and iface ID)
	read_leading_columns(input_stream, annotated);
      }

      // read all or active, but set only the active variables into the lists
      vars.read_tabular(input_stream, active_only);
      ++num_evals;

      cva.push_back(vars.continuous_variables());
      diva.push_back(vars.discrete_int_variables());
      list_dsv_points.push_back(vars.discrete_string_variables());
      drva.push_back(vars.discrete_real_variables());

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


  // copy into the string multiarray
  size_t num_dsv = vars.dsv();
  dsva.resize(boost::extents[num_evals][num_dsv]);
  for (size_t i=0; i<num_evals; ++i)
    dsva[i] = list_dsv_points[i];
  list_dsv_points.clear();

  return num_evals;
}


} // namespace TabularIO

} // namespace Dakota
