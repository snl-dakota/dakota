/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
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


} // namespace TabularIO

} // namespace Dakota
