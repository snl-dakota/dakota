/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "dakota_data_io.hpp"
#include "dakota_tabular_io.hpp"
#include "DakotaVariables.hpp"
#include "DakotaResponse.hpp"
#include "ParamResponsePair.hpp"

namespace Dakota {

// BMA TODO: Consider removing this namespace if not helpful in
// protecting certain components
namespace TabularIO {

String format_name(unsigned short tabular_format)
{
  String file_format("annotated");
  if (tabular_format == TABULAR_NONE)
    file_format = "freeform";
  else if (tabular_format < TABULAR_ANNOTATED)
    file_format = "custom_annotated";

  return file_format;
}

void print_expected_format(std::ostream& s, unsigned short tabular_format, 
			   size_t num_rows, size_t num_cols)
{
  s << "\nExpected " << format_name(tabular_format) << " tabular file:";
  if (tabular_format > TABULAR_NONE) {
    if (tabular_format & TABULAR_HEADER) 
      s << "\n  * header row with labels";
    if (tabular_format & TABULAR_EVAL_ID)
      s << "\n  * leading column with counter";
    if (tabular_format & TABULAR_IFACE_ID)
      s << "\n  * leading column with interface ID";
    s << "\nsurrounding ";
  }
  else 
    s << '\n';
  s << "whitespace-separated data";
  if (num_rows)
    s << "; " << num_rows << " rows";
  if (num_cols)
    s << "; " << num_cols << " columns";
  s << std::endl;
}

void print_unexpected_data(std::ostream& s, const String& filename,
			   const String& context_message, 
			   unsigned short tabular_format)
{
  s << "\nWarning (" << context_message << "): found unexpected extra data in "
    << format_name(tabular_format) << "\nfile " << filename << "." << std::endl;
}


//
//- Utilities for opening tabular files
//

void open_file(std::ifstream& data_stream, const std::string& input_filename, 
	       const std::string& context_message) 
{
  // TODO: try/catch
  data_stream.open(input_filename.c_str());
  if (!data_stream.good()) {
    Cerr << "\nError (" << context_message << "): Could not open file " 
	 << input_filename << " for reading tabular data." << std::endl;
    abort_handler(-1);
  }
  // TODO (fix): can't except on failbit when trying to read to EOF
  //  data_stream.exceptions(std::fstream::failbit | std::fstream::badbit);
  data_stream.exceptions(std::fstream::badbit);
}


void open_file(std::ofstream& data_stream, const std::string& output_filename, 
	       const std::string& context_message) 
{
  // TODO: try/catch
  data_stream.open(output_filename.c_str());
  if (!data_stream.good()) {
    Cerr << "\nError (" << context_message << "): Could not open file " 
	 << output_filename << " for writing tabular data." << std::endl;
    abort_handler(-1);
  }
  data_stream.exceptions(std::fstream::failbit | std::fstream::badbit);
}


//
//- Utilities for closing tabular files
//
// Note: an fstream destructor can manage the different states and close the
// stream properly.  However, for the case of a class-member stream, we should
// close it such that any subsequent re-opening works properly.

void close_file(std::ifstream& data_stream, const std::string& input_filename, 
		const std::string& context_message) 
{
  // TODO: try/catch

  // ifstream's have 4 states: good, eof, fail and bad.  Testing this state
  // prior to close() is likely overkill in the current context...
  if (data_stream.good() || data_stream.eof())
    data_stream.close();
  else {
    Cerr << "\nError (" << context_message << "): Could not close file " 
	 << input_filename << " used for reading tabular data." << std::endl;
    abort_handler(-1);
  }
}


void close_file(std::ofstream& data_stream, const std::string& output_filename, 
		const std::string& context_message) 
{
  // TODO: try/catch

  // ofstream's have 4 states: good, eof, fail and bad.  Testing this state
  // prior to close() is likely overkill in the current context...
  if (data_stream.good() || data_stream.eof())
    data_stream.close();
  else {
    Cerr << "\nError (" << context_message << "): Could not close file " 
	 << output_filename << " used for writing tabular data." << std::endl;
    abort_handler(-1);
  }
}


//
//- Utilities for tabular write
//


void write_header_tabular(std::ostream& tabular_ostream, 
			  const std::string& eval_label,
			  const std::string& iface_label,
			  unsigned short tabular_format)
{
  if ( !(tabular_format & TABULAR_HEADER) ) return;

  tabular_ostream << "%"; // headers use Matlab comment syntax
  if (tabular_format & TABULAR_EVAL_ID)
    tabular_ostream << std::setw(7) << std::left <<  eval_label << ' ';
  if (tabular_format & TABULAR_IFACE_ID)
    tabular_ostream << std::setw(8) << std::left << iface_label << ' ';
}


void write_header_tabular(std::ostream& tabular_ostream, 
			  const std::string& eval_label,
			  const StringArray& iface_labels,
			  unsigned short tabular_format)
{
  if ( !(tabular_format & TABULAR_HEADER) ) return;

  tabular_ostream << "%"; // headers use Matlab comment syntax
  if (tabular_format & TABULAR_EVAL_ID)
    tabular_ostream << std::setw(7) << std::left << eval_label << ' ';
  if (tabular_format & TABULAR_IFACE_ID) {
    size_t i, num_labels = iface_labels.size();
    for (i=0; i<num_labels; ++i)
      tabular_ostream << std::setw(10) << std::left << iface_labels[i] << ' ';
  }
}


void write_header_tabular(std::ostream& tabular_ostream, 
			  const Variables& vars, const Response& response,
			  const std::string& eval_label,
			  const std::string& interface_label,
			  unsigned short tabular_format)
{
  write_header_tabular(tabular_ostream, eval_label, interface_label,
		       tabular_format);
  append_header_tabular(tabular_ostream, vars,     tabular_format);
  append_header_tabular(tabular_ostream, response, tabular_format);
}


void write_header_tabular(std::ostream& tabular_ostream, const Variables& vars, 
			  const StringArray& addtnl_labels,
			  const std::string& eval_label,
			  const std::string& interface_label,
			  unsigned short tabular_format)
{
  write_header_tabular(tabular_ostream, eval_label, interface_label,
		       tabular_format);
  append_header_tabular(tabular_ostream, vars, tabular_format);
  Dakota::write_data_tabular(tabular_ostream, addtnl_labels);
  tabular_ostream << std::endl; // table row completed
}


void append_header_tabular(std::ostream& tabular_ostream, 
			   const Variables& vars, unsigned short tabular_format)
{
  if ( !(tabular_format & TABULAR_HEADER) ) return;
  vars.write_tabular_labels(tabular_ostream);
}


void append_header_tabular(std::ostream& tabular_ostream, 
			   const Variables& vars, size_t start_index,
			   size_t num_items, unsigned short tabular_format)
{
  if ( !(tabular_format & TABULAR_HEADER) ) return;
  vars.write_tabular_partial_labels(tabular_ostream, start_index, num_items);
}


void append_header_tabular(std::ostream& tabular_ostream, 
			   const StringArray& labels,
			   unsigned short tabular_format)
{
  if ( !(tabular_format & TABULAR_HEADER) ) return;
  Dakota::write_data_tabular(tabular_ostream, labels);
}


void append_header_tabular(std::ostream& tabular_ostream, 
			  const Response& response,
			   unsigned short tabular_format, bool eol)
{
  if ( !(tabular_format & TABULAR_HEADER) ) return;
  response.write_tabular_labels(tabular_ostream, eol);
}


void write_leading_columns(std::ostream& tabular_ostream, size_t eval_id)
{
  // align left to make eval_id consistent w/ whitespace-delimited header row
  std::ios_base::fmtflags before_left_align = tabular_ostream.flags();
  tabular_ostream << std::setw(8) << std::left << eval_id << ' ';
  tabular_ostream.flags(before_left_align);
}


void write_leading_columns(std::ostream& tabular_ostream,
			   const String& iface_id)
{
  // write the interface ID string, NO_ID for empty
  // (Dakota 6.1 used EMPTY for missing ID)
  if (iface_id.empty())
    tabular_ostream << std::setw(10) << std::left << "NO_ID"  << ' ';
  else
    tabular_ostream << std::setw(10) << std::left << iface_id << ' ';
}


void write_leading_columns(std::ostream& tabular_ostream, size_t eval_id, 
			   const String& iface_id,
			   unsigned short tabular_format)
{
  // conditionally write evaluation ID and/or interface ID
  if (tabular_format & TABULAR_EVAL_ID)
    write_leading_columns(tabular_ostream, eval_id);
  if (tabular_format & TABULAR_IFACE_ID)
    write_leading_columns(tabular_ostream, iface_id);
}


void write_leading_columns(std::ostream& tabular_ostream, size_t eval_id, 
			   const StringArray& iface_ids,
			   unsigned short tabular_format)
{
  if (tabular_format & TABULAR_EVAL_ID)
    write_leading_columns(tabular_ostream, eval_id);
  if (tabular_format & TABULAR_IFACE_ID) {
    size_t i, num_ids = iface_ids.size();
    for (i=0; i<num_ids; ++i)
      write_leading_columns(tabular_ostream, iface_ids[i]);
  }
}


void write_data_tabular(std::ostream& tabular_ostream, const Variables& vars)
{ vars.write_tabular(tabular_ostream); } // no EOL


void write_data_tabular(std::ostream& tabular_ostream, const Variables& vars,
			size_t start_index, size_t num_items)
{ vars.write_tabular_partial(tabular_ostream, start_index, num_items); }


void write_data_tabular(std::ostream& tabular_ostream,
			const Response& response, bool eol)
{ response.write_tabular(tabular_ostream, eol); }


void write_data_tabular(std::ostream& tabular_ostream, const Response& response,
			size_t start_index, size_t num_items)
{ response.write_tabular_partial(tabular_ostream, start_index, num_items); }


void write_data_tabular(std::ostream& tabular_ostream, 
			const Variables& vars, const String& iface_id, 
			const Response& response, size_t counter,
			unsigned short tabular_format)
{
  write_leading_columns(tabular_ostream, counter, iface_id, tabular_format);
  vars.write_tabular(tabular_ostream);
  response.write_tabular(tabular_ostream); // includes EOL
}


void write_data_tabular(std::ostream& tabular_ostream, 
			const Variables& vars, const String& iface_id, 
			size_t counter, unsigned short tabular_format)
{
  write_leading_columns(tabular_ostream, counter, iface_id, tabular_format);
  vars.write_tabular(tabular_ostream); // no EOL
  write_eol(tabular_ostream);
}


void write_eol(std::ostream& tabular_ostream)
{ tabular_ostream << std::endl; }


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

  close_file(output_stream, output_filename, context_message);
}


//
//- Utilities for tabular read
//

/** Discard header row from tabular file; alternate could read into a
    string array.  Requires header to be delimited by a newline. */
StringArray read_header_tabular(std::istream& input_stream,
				unsigned short tabular_format)
{
  StringArray header_fields;
  if (tabular_format & TABULAR_HEADER) {
    input_stream >> std::ws;
    String header;
    getline(input_stream, header);
    return strsplit(header);
  }
  return StringArray();
}


/** reads eval and interface ids; if no eval ID to read due to format,
    increment the passed eval ID */
void read_leading_columns(std::istream& input_stream,
			  unsigned short tabular_format,
			  int& eval_id, String& iface_id)
{
  if (tabular_format & TABULAR_EVAL_ID)
    input_stream >> eval_id;
  else
    ++eval_id;

  if (tabular_format & TABULAR_IFACE_ID) {
    input_stream >> iface_id;
    // (Dakota 6.1 used EMPTY for missing ID)
    if (iface_id == "EMPTY")
      iface_id = "NO_ID";
  }
  else
    iface_id = "NO_ID";
}


/** Discards the (eval ID and) interface data, which should be used
    for validation */
void read_leading_columns(std::istream& input_stream,
			    unsigned short tabular_format)
{
  int     eval_id; // discarded
  String iface_id; // discarded
  read_leading_columns(input_stream, tabular_format, eval_id, iface_id);
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


/// return indices (relative to first variable position) into the read
/// var labels that yield input spec ordered vars
std::vector<size_t>
find_vars_map(const StringArray::const_iterator& read_vars_begin,
	      const StringArray& expected_vars)
{
  // Pre-condition: read var labels are a permutation of expected
  size_t num_vars = expected_vars.size();
  std::vector<size_t> var_inds(num_vars);
  for(size_t i=0; i<num_vars; ++i) {
    auto lab_it = std::find(read_vars_begin, read_vars_begin + num_vars,
			    expected_vars[i]);
    var_inds[i] = std::distance(read_vars_begin, lab_it);
  }
  return var_inds;
}


/// Given a row of a tabular file, reorder the variables, leaving
/// leading cols and responses as-is. var_inds are zero-based indices
/// into the variables only in the read row
std::string reorder_row(const std::string& read_str,
			std::vector<size_t> var_inds, size_t num_lead)
{
  if (var_inds.empty()) return read_str; // no reordering needed

  StringArray row_vals = strsplit(read_str);

  // create a new string with reordered vars
  std::ostringstream ordered_str;
  std::ostream_iterator<String> os_it(ordered_str, " ");
  auto num_vars = var_inds.size();
  std::copy(row_vals.begin(), row_vals.begin() + num_lead, os_it);
  for (const auto v_index : var_inds)
    ordered_str << row_vals[num_lead + v_index] + " ";
  std::copy(row_vals.begin() + num_lead + num_vars, row_vals.end(), os_it);

  return ordered_str.str();
}

void print_expected_labels(bool active_only,
			   const StringArray& expected_vars,
			   const StringArray::const_iterator& read_vars_begin,
			   const StringArray::const_iterator& header_fields_end)
{
  std::ostream_iterator<String> out_it (Cout, " ");
  Cout << "\nExpected labels (for "
       << ((active_only) ? "active" : "all")
       << " variables):\n  ";
  std::copy(expected_vars.begin(), expected_vars.end(), out_it);
  Cout << std::endl << "Instead found these in header (including "
       << "variable and response labels):\n  ";
  std::copy(read_vars_begin, header_fields_end, out_it);
  Cout << '\n' << std::endl;
}


// NOTE: Passing all these args around begs for a class to
// encapsulate, BMA TODO: refactor procedural code
std::vector<size_t>
validate_header(std::ifstream& data_stream,
		const std::string& input_filename,
		const std::string& context_message,
		const Variables& vars,
		unsigned short tabular_format, bool verbose,
		bool use_var_labels, bool active_only)
{
  // TODO: Validate response labels
  // TODO: Side-by-side diff of labels
  // TODO: Can we guide the user further when data appear active vs. all?

  size_t num_lead = 0;
  if (tabular_format & TABULAR_EVAL_ID) ++num_lead;
  if (tabular_format & TABULAR_IFACE_ID) ++num_lead;
  size_t num_vars = active_only ? vars.total_active() : vars.tv();

  StringArray expected_vars =
    vars.ordered_labels(active_only ? ACTIVE_VARS : ALL_VARS);
  StringArray header_fields = read_header_tabular(data_stream, tabular_format);
  size_t read_fields = header_fields.size();

  std::vector<size_t> var_inds;  // only populated if reordering

  // iterator to start of read vars, skipping any leading columns;
  // take care to not advance beyond end()
  auto read_vars_begin = (num_lead < read_fields) ?
    header_fields.begin() + num_lead : header_fields.end();

  bool vars_equal = (num_lead + num_vars > read_fields) ? false :
    std::equal(expected_vars.begin(), expected_vars.end(), read_vars_begin);

  bool vars_permuted = (num_lead + num_vars > read_fields) ? false :
    std::is_permutation(expected_vars.begin(), expected_vars.end(),
			read_vars_begin);

  if (use_var_labels) {
    // Input spec restricts to TABULAR_HEADER case; require equal or permutation
    if (vars_equal) {
      ; // no map needed (no-op to simplify logic)
    }
    else if (vars_permuted ) {
      Cout << "\nInfo (" << context_message << "):\n"
	   << "Reordering variables imported from tabular file '"
	   << input_filename << "'\nbased on labels in header.\n" << std::endl;
      var_inds = find_vars_map(read_vars_begin, expected_vars);
    }
    else {
      Cerr << "\nError (" << context_message << "):\n"
	   << "Cannot reorder variables imported from tabular file '"
	   << input_filename << "'\nas requested by use_variable_labels. First "
	   << num_vars << " variable labels in tabular\nfile header are not a "
	   << "permutation of expected variable labels." << std::endl;
      if (verbose)
	print_expected_labels(active_only, expected_vars, read_vars_begin,
			      header_fields.end());
      abort_handler(IO_ERROR);
    }

  }
  else if (tabular_format & TABULAR_HEADER) {
    if (!vars_equal) {
      if (vars_permuted) {
	Cout << "\nWarning (" << context_message << "):\n"
	     << "Variable labels in header of tabular file '" << input_filename
	     << "' are a\npermutation of expected variable labels;"
	     << " consider use_variable_labels keyword." << std::endl;
	if (verbose)
	  print_expected_labels(active_only, expected_vars, read_vars_begin,
				header_fields.end());
      }
      else {
	Cout << "\nWarning (" << context_message << "):\n"
	     << "Variable labels in header of tabular file '" << input_filename
	     << "'\ndo not match " << num_vars << " variables being"
	     <<" imported to." << std::endl;
	if (verbose)
	  print_expected_labels(active_only, expected_vars, read_vars_begin,
				header_fields.end());
      }
    }
    // Can't guide further as unable to reconcile vars vs. responses,
    // but we decided not a hard error for now
  }

  return var_inds;
}


void read_data_tabular(const std::string& input_filename, 
		       const std::string& context_message,
		       RealVector& input_vector, size_t num_entries,
		       unsigned short tabular_format)
{
  // TODO: handle both row and col vectors in the text?
  std::ifstream input_stream;
  open_file(input_stream, input_filename, context_message);
  
  read_header_tabular(input_stream, tabular_format);

  input_vector.resize(num_entries);
  try {
    if (tabular_format & TABULAR_EVAL_ID || tabular_format & TABULAR_IFACE_ID) {
      for (size_t row_ind = 0; row_ind < num_entries; ++row_ind) {
	input_stream >> std::ws;
	// discard the leading cols (typically eval or data ID, iface ID)
	read_leading_columns(input_stream, tabular_format);
	input_stream >> input_vector[row_ind];
      } 
    } else {
      // read raw whitespace separated data into (sized) vector
      input_stream >> std::ws;
      read_data(input_stream, input_vector);
    }
  }
  catch (const std::ios_base::failure& failorbad_except) {
    Cerr << "\nError (" << context_message << "): could not read file.";
    print_expected_format(Cerr, tabular_format, num_entries, 1);
    abort_handler(-1);
  }

  if (exists_extra_data(input_stream))
    print_unexpected_data(Cout, input_filename, context_message,tabular_format);

  close_file(input_stream, input_filename, context_message);
}


// New prototype to support mixed variable reads
void read_data_tabular(const std::string& input_filename, 
		       const std::string& context_message,
		       Variables vars, size_t num_fns,
		       RealMatrix& vars_matrix, RealMatrix& resp_matrix,
                       unsigned short tabular_format, bool verbose,
		       bool use_var_labels, bool active_only)
{
  // Disallow string variables for now - RWH
  if( (active_only && vars.dsv()) || (!active_only && vars.adsv()) ) {
    Cerr << "\nError (" << context_message
	 << "): String variables are not currently supported.\n";
    abort_handler(-1);
  }

  std::ifstream input_stream;
  open_file(input_stream, input_filename, context_message);

  size_t num_vars = active_only ? 
    vars.cv() + vars.div() + vars.dsv() + vars.drv() : vars.tv();

  RealVectorArray work_vars_va;
  RealVectorArray work_resp_va;
  RealVector work_vars_vec(num_vars);
  RealVector work_resp_vec(num_fns);

  //  Need to delay sizing of input_matrix 
  try {

    // only populated if reordering
    std::vector<size_t> var_inds =
      validate_header(input_stream, input_filename, context_message, vars,
		      tabular_format, verbose, use_var_labels, active_only);

    size_t line = (tabular_format & TABULAR_HEADER) ? 1 : 0;

    size_t num_lead = 0;
    if (tabular_format & TABULAR_EVAL_ID) ++num_lead;
    if (tabular_format & TABULAR_IFACE_ID) ++num_lead;
    size_t num_vars = active_only ? vars.total_active() : vars.tv();
    size_t num_cols = num_lead + num_vars + num_fns;;

    input_stream >> std::ws;
    while (input_stream.good() && !input_stream.eof()) {

      // Read a line, then use existing vars/resp read functions
      input_stream >> std::ws;
      String row_str;
      getline(input_stream, row_str);
      ++line;

      size_t num_read = strsplit(row_str).size(); // TODO: count without storing
      if (num_read != num_cols) {
	// TODO: more detailed message about column contents
	Cerr << "\nError (" << context_message
	     << "): wrong number of columns on line " << line << "\nof file '"
	     << input_filename << "'; expected " << num_cols << ", found "
	     << num_read << ".\n";
	print_expected_format(Cerr, tabular_format, 0, num_cols);
	abort_handler(IO_ERROR);
      }

      std::istringstream row_iss(var_inds.empty() ? row_str :
				 reorder_row(row_str, var_inds, num_lead));

      // discard any leading columns
      read_leading_columns(row_iss, tabular_format);

      // use a variables object because it knows how to read active vs. all
      vars.read_tabular(row_iss, (active_only ? ACTIVE_VARS : ALL_VARS) );

      // Extract the variables
      const RealVector& c_vars  = active_only ? vars.continuous_variables()
	: vars.all_continuous_variables();
      const IntVector&  di_vars = active_only ? vars.discrete_int_variables()
	: vars.all_discrete_int_variables();
      const RealVector& dr_vars = active_only ? vars.discrete_real_variables()
	: vars.all_discrete_real_variables();
      copy_data_partial(c_vars, work_vars_vec, 0);
      merge_data_partial(di_vars, work_vars_vec, c_vars.length());
      copy_data_partial(dr_vars, work_vars_vec,
			c_vars.length()+di_vars.length());
      //varsMatrix(row,:) = [vars.continuous_variables(),
      //  vars.discrete_int_variables(), vars.discrete_real_variables() ]
      work_vars_va.push_back(work_vars_vec);
      //Cout << "Working Variables vector contents: \n" << work_vars_vec
      //     << std::endl;

      // read the raw function data
      for (size_t fi = 0; fi < num_fns; ++fi) {
        double read_value = std::numeric_limits<double>::quiet_NaN();
        if (row_iss >> read_value)
          work_resp_vec(fi) = read_value;
      }
      work_resp_va.push_back(work_resp_vec);
      //Cout << "Working Response vector contents: \n" << work_resp_vec
      //     << std::endl;

      input_stream >> std::ws;
    }
  }
  catch (const std::ios_base::failure& failorbad_except) {
    Cerr << "\nError (" << context_message << "): could not read file " 
	 << input_filename << ".";
    print_expected_format(Cerr, tabular_format, 0, num_vars);
    abort_handler(-1);
  }
  catch (const TabularDataTruncated& tdtrunc) {
    // this will be thrown if Variables was truncated
    Cerr << "\nError (" << context_message
	 << "): could not read variables from file " << input_filename
	 << ";\n  " << tdtrunc.what() << std::endl;
    abort_handler(-1);
  }
  catch(...) {
    Cerr << "\nError (" << context_message << "): could not read file " 
	 << input_filename << " (unknown error).";
    abort_handler(-1);
  }

  copy_data(work_vars_va, vars_matrix);
  copy_data(work_resp_va, resp_matrix);

  close_file(input_stream, input_filename, context_message);
}

/** Read possibly annotated data with unknown num_rows data into input_coeffs
    (num_fns x num_rows) and input_indices (num_rows x num_vars) */
void read_data_tabular(const std::string& input_filename, 
		       const std::string& context_message,
		       RealVectorArray& input_coeffs, 
		       UShort2DArray& input_indices, 
		       unsigned short tabular_format,
		       size_t num_vars, size_t num_fns)
{
  std::ifstream input_stream;
  open_file(input_stream, input_filename, context_message);
  
  // clear so we can append
  input_indices.clear();
  // need a temporary due to layout of returned data; this will be 
  // num_rows x num_fns
  Real2DArray coeffs_tmp;

  try {

    read_header_tabular(input_stream, tabular_format);

    input_stream >> std::ws;
    while (input_stream.good() && !input_stream.eof()) {

      // discard any leading columns; annotated is unlikely in this case
      read_leading_columns(input_stream, tabular_format);

      // read the (required) coefficients of length num_fns
      RealArray read_coeffs(num_fns, std::numeric_limits<double>::quiet_NaN());
      if (input_stream >> read_coeffs) {
	Cout << "read:\n" << read_coeffs;
	coeffs_tmp.push_back(read_coeffs);
      }
      else {
	Cerr << "\nError (" << context_message << "): unexpected coeff read "
	     << "error in file " << input_filename << " for read:\n"
	     << read_coeffs << std::endl;
	abort_handler(-1);
      }

      // read the (required) indices of length num_vars
      UShortArray index_set(num_vars, 0);
      // don't break as these are required data
      // use templated stream extraction from data_io
      if (input_stream >> index_set) {
	Cout << "index set:\n" << index_set << std::endl;
	input_indices.push_back(index_set);
      }
      else {
	Cerr << "\nError (" << context_message << "): unexpected indices read "
	     << "error in file " << input_filename << " for read:\n"
	     << index_set << std::endl;
	abort_handler(-1);
      }
      input_stream >> std::ws;
    }
  }
  catch (const std::ios_base::failure& failorbad_except) {
    Cerr << "\nError (" << context_message << "): could not read file " 
	 << input_filename << ".";
    print_expected_format(Cerr, tabular_format, 0, num_vars);
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

  close_file(input_stream, input_filename, context_message);
}


void read_data_tabular(const std::string& input_filename, 
		       const std::string& context_message,
		       Variables vars, Response resp, PRPList& input_prp,
		       unsigned short tabular_format, bool verbose,
		       bool use_var_labels, bool active_only)
{
  std::ifstream data_stream;
  open_file(data_stream, input_filename, context_message);

  // only populated if reordering
  std::vector<size_t> var_inds =
    validate_header(data_stream, input_filename, context_message, vars,
		    tabular_format, verbose, use_var_labels, active_only);

  int eval_id = 0;  // number the evals starting from 1 if not contained in file
  String iface_id;
  size_t line = (tabular_format & TABULAR_HEADER) ? 1 : 0;

  size_t num_lead = 0;
  if (tabular_format & TABULAR_EVAL_ID) ++num_lead;
  if (tabular_format & TABULAR_IFACE_ID) ++num_lead;
  size_t num_vars = active_only ? vars.total_active() : vars.tv();
  size_t num_cols = num_lead + num_vars + resp.num_functions();;
  // shouldn't need both good and eof checks
  data_stream >> std::ws;
  while (data_stream.good() && !data_stream.eof()) {
    try {

      // Read a line, then use existing vars/resp read functions
      data_stream >> std::ws;
      String row_str;
      getline(data_stream, row_str);
      ++line;

      size_t num_read = strsplit(row_str).size(); // TODO: count without storing
      if (num_read != num_cols) {
	// TODO: more detailed message about column contents
	Cerr << "\nError (" << context_message
	     << "): wrong number of columns on line " << line << "\nof file '"
	     << input_filename << "'; expected " << num_cols << ", found "
	     << num_read << ".\n";
	print_expected_format(Cerr, tabular_format, 0, num_cols);
	abort_handler(IO_ERROR);
      }

      std::istringstream row_iss(var_inds.empty() ? row_str :
				 reorder_row(row_str, var_inds, num_lead));

      read_leading_columns(row_iss, tabular_format, eval_id, iface_id);
      vars.read_tabular(row_iss, (active_only ? ACTIVE_VARS : ALL_VARS) );
      resp.read_tabular(row_iss);
    }
    catch (const TabularDataTruncated& tdtrunc) {
      // this will be thrown if either Variables or Response was truncated
      Cerr << "\nError (" << context_message
	   << "): could not read variables or responses from file "
	   << input_filename << ";\n  "  << tdtrunc.what() << std::endl;
      abort_handler(IO_ERROR);
    }
    catch(...) {
      Cerr << "\nError (" << context_message << "): could not read file " 
	   << input_filename << " (unknown error).";
      abort_handler(IO_ERROR);
    }
    if (verbose) {
      Cout << "Variables read:\n" << vars;
      if (!iface_id.empty())
	Cout << "\nInterface identifier = " << iface_id << '\n';
      Cout << "\nResponse read:\n" << resp;
    }

    // append deep copy of vars,resp as PRP
    input_prp.push_back(ParamResponsePair(vars, iface_id, resp, eval_id));

    // advance so EOF can detect properly
    data_stream >> std::ws;
  }

  close_file(data_stream, input_filename, context_message);
}


void read_data_tabular(const std::string& input_filename, 
		       const std::string& context_message,
		       RealMatrix& input_matrix, 
		       size_t num_rows, size_t num_cols,
		       unsigned short tabular_format, bool verbose)
{
  std::ifstream input_stream;
  open_file(input_stream, input_filename, context_message);

  if (verbose) {
    Cout << "\nAttempting to read " << num_rows << " x " << num_cols << " = "
	 << num_rows*num_cols << " numeric data from " 
	 << (tabular_format > TABULAR_NONE ? "header-annotated" : "free-form")
	 << " file " << input_filename << "..." << std::endl;
  }

  read_header_tabular(input_stream, tabular_format);

  input_matrix.shapeUninitialized(num_rows, num_cols);	
  for (size_t row_ind = 0; row_ind < num_rows; ++row_ind) {
    try {
      // experiment data would never have an interface ID
      if (tabular_format & TABULAR_EVAL_ID) {
	// discard the row label (typically eval or data ID)
	size_t discard_row_label;
	input_stream >> discard_row_label;
      }
      for (size_t col_ind = 0; col_ind < num_cols; ++col_ind)
	input_stream >> input_matrix(row_ind, col_ind);
    }
    catch (const std::ios_base::failure& failorbad_except) {
      Cerr << "\nError (" << context_message << "): could not read file.";
      print_expected_format(Cerr, tabular_format, num_rows, num_cols);
      abort_handler(-1);
    }
  }

  if (exists_extra_data(input_stream))
    print_unexpected_data(Cout, input_filename, context_message, tabular_format);

  close_file(input_stream, input_filename, context_message);
}


void read_data_tabular(const std::string& input_filename,
		       const std::string& context_message,
		       RealMatrix& input_matrix, size_t record_len,
		       unsigned short tabular_format, bool verbose)
{
  std::ifstream input_stream;
  open_file(input_stream, input_filename, context_message);

  RealVectorArray rva;
  RealVector read_rv(record_len);
  try {

    read_header_tabular(input_stream, tabular_format);

    input_stream >> std::ws;
    while (input_stream.good() && !input_stream.eof()) {

      // discard any leading columns; annotated is unlikely in this case
      read_leading_columns(input_stream, tabular_format);

      // read the (required) coefficients of length num_fns
      read_rv = std::numeric_limits<Real>::quiet_NaN();
      if (input_stream >> read_rv) {
	if (verbose) Cout << "read:\n" << read_rv;
	rva.push_back(read_rv);
      }
      else {
	Cerr << "\nError (" << context_message << "): unexpected row read "
	     << "error in file " << input_filename << ".\nread:\n" << read_rv;
	abort_handler(-1);
      }
      input_stream >> std::ws; // advance to next input for EOF detection
    }
  }
  catch (const std::ios_base::failure& failorbad_except) {
    Cerr << "\nError (" << context_message << "): could not read file " 
	 << input_filename << ".";
    print_expected_format(Cerr, tabular_format, 0, record_len);
    abort_handler(-1);
  }
  catch(...) {
    Cerr << "\nError (" << context_message << "): could not read file " 
	 << input_filename << " (unknown error).";
    abort_handler(-1);
  }

  // this transposes the rva tabular layout (num_records X record_len) into the
  // rm layout (record_len X num_records), since the natural place to store the
  // ith vector rva[i] is as rm[i], a Teuchos column vector.
  copy_data_transpose(rva, input_matrix);

  close_file(input_stream, input_filename, context_message);
}


size_t read_data_tabular(const std::string& input_filename, 
			 const std::string& context_message,
			 RealVectorArray& cva, IntVectorArray& diva, 
			 StringMulti2DArray& dsva, RealVectorArray& drva,
			 unsigned short tabular_format, 
			 bool active_only, Variables vars)
{
  size_t num_evals = 0, num_vars = vars.tv();
  // temporary dynamic container to read string variables
  std::vector<StringMultiArray> list_dsv_points;

  std::ifstream input_stream;
  open_file(input_stream, input_filename, context_message);
 
  try {

    read_header_tabular(input_stream, tabular_format);

    input_stream >> std::ws;  // advance to next readable input
    while (input_stream.good() && !input_stream.eof()) {
      // discard the row labels (typically eval and iface ID)
      read_leading_columns(input_stream, tabular_format);

      // read all or active, but set only the active variables into the lists
      vars.read_tabular(input_stream, (active_only ? ACTIVE_VARS : ALL_VARS) );
      ++num_evals;

      // the Variables object vars passed in is a deep copy, but these
      // accessors return views; force a deep copy of each vector for
      // storage in array
      RealVector c_vars(Teuchos::Copy, vars.continuous_variables().values(), 
			vars.continuous_variables().length());
      cva.push_back(c_vars);
      IntVector di_vars(Teuchos::Copy, vars.discrete_int_variables().values(),
			vars.discrete_int_variables().length());
      diva.push_back(di_vars);
      list_dsv_points.push_back(vars.discrete_string_variables());
      RealVector dr_vars(Teuchos::Copy, vars.discrete_real_variables().values(),
			 vars.discrete_real_variables().length());
      drva.push_back(dr_vars);

      input_stream >> std::ws;  // advance to next readable input
    }
  }
  catch (const std::ios_base::failure& failorbad_except) {
    Cerr << "\nError (" << context_message << "): could not read file " 
	 << input_filename << ".";
    print_expected_format(Cerr, tabular_format, 0, num_vars);
    abort_handler(-1);
  }
  catch (const TabularDataTruncated& tdtrunc) {
    // this will be thrown if Variables was truncated
    Cerr << "\nError (" << context_message << "): could not read variables from "
	 << "file " << input_filename << ";\n  " << tdtrunc.what() << std::endl;
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

  close_file(input_stream, input_filename, context_message);
 
  return num_evals;
}


/** Read configuration variables from tabular file into active entries
    in the passed array, up to max_configs. Return number read and
    whether data remains in file. */
std::pair<size_t, bool> read_data_tabular(const std::string& input_filename, 
					  const std::string& context_message,
					  size_t max_configs,
					  VariablesArray& config_array,
					  unsigned short tabular_format)
{
  assert(max_configs <= config_array.size());
  size_t configs_read = 0;
  std::ifstream input_stream;
  open_file(input_stream, input_filename, context_message);
 
  try {

    read_header_tabular(input_stream, tabular_format);

    input_stream >> std::ws;  // advance to next readable input
    while (configs_read < max_configs &&
	   input_stream.good() && !input_stream.eof()) {
      // discard the row labels (typically eval and iface ID)
      read_leading_columns(input_stream, tabular_format);

      config_array[configs_read].read_tabular(input_stream, ACTIVE_VARS);
      ++configs_read;
      
      input_stream >> std::ws;  // advance to next readable input
    }
  }
  catch (const std::ios_base::failure& failorbad_except) {
    Cerr << "\nError (" << context_message << "): could not read file " 
	 << input_filename << ".";
    print_expected_format(Cerr, tabular_format, 0,
			  config_array[0].total_active());
    abort_handler(-1);
  }
  catch (const TabularDataTruncated& tdtrunc) {
    // this will be thrown if Variables was truncated
    Cerr << "\nError (" << context_message << "): could not read variables from "
	 << "file " << input_filename << ";\n  " << tdtrunc.what() << std::endl;
    abort_handler(-1);
  }
  catch(...) {
    Cerr << "\nError (" << context_message << "): could not read file " 
	 << input_filename << " (unknown error)." << std::endl;
    abort_handler(-1);
  }

  bool more_data = exists_extra_data(input_stream);
  close_file(input_stream, input_filename, context_message);

  return std::make_pair(configs_read, more_data);
}


} // namespace TabularIO

} // namespace Dakota
