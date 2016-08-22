/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_TABULAR_IO_H
#define DAKOTA_TABULAR_IO_H

#include "dakota_data_types.hpp"
#include "dakota_global_defs.hpp"

/** \file dakota_tabular_io.hpp
    \brief Utility functions for reading and writing tabular data files
    Emerging utilities for tabular file I/O.  For now, just extraction
    of capability from separate contexts to faciliate rework.  These
    augment (and leverage) those in data_util.h

    Design/capability goals:
    * Ability to read / write data with row/col headers or in free-form 
    * Detect premature end of file, report if extra data
    * More consistent and reliable checks for file open errors
    * Require right number of cols in header mode; only total data
      checking in free-form (likely)
    * Allow comment character for header rows or even in data?
    * variables vs. variables/responses for both read and write
    * Should we support CSV? delimiter = ','; other?
    * Verify treatment of trailing newline without reading a zero
    * Allow reading into the transpose of the data structure
*/

/* 
   Could consider class with attributes
   * filename
   * read/write
   * context message
   * reference to istream

   For design and review consider:
   * write Array[Variables]      (allVariables in DakotaAnalyzer)
   * write RealMatrix transposed (allSamples in DakotaAnalyzer)
   * allow specifying lsq data in input file!
   * truly separate functions that take a file name from those using a stream
   * Verify: pre-run for active vs. all case; pre-run precision and formatting
   * should readers or clients size data?  If sized could reduce args.

   GOALS:
    * avoid creating dangling variables with no refs
    * avoid temp vectors that aren't needed (copy overhead)
*/


namespace Dakota {

class SharedVariablesData;

namespace TabularIO {

//
//- Utilities for status messages
//

/// Translate tabular_format into a user-friendly name
String format_name(unsigned short tabular_format);

/// Describe the expected data file format based on passed parameters
void print_expected_format(std::ostream& s, unsigned short tabular_format, 
			   size_t num_rows, size_t num_cols);

/// Print a warning if there's extra data in the file
void print_unexpected_data(std::ostream& s, const String& filename,
			   const String& context_message, 
			   unsigned short tabular_format);

//
//- Utilities for opening and closing tabular files
//

/// open the file specified by name for reading, using passed input
/// stream, presenting context-specific error on failure
void open_file(std::ifstream& data_file, const std::string& input_filename, 
	       const std::string& context_message);

/// open the file specified by name for writing, using passed output
/// stream, presenting context-specific error on failure
void open_file(std::ofstream& data_file, const std::string& output_filename, 
	       const std::string& context_message);

/// close the file specified by name after reading, using passed input
/// stream, presenting context-specific error on failure
void close_file(std::ifstream& data_file, const std::string& input_filename, 
		const std::string& context_message);

/// close the file specified by name after writing, using passed output
/// stream, presenting context-specific error on failure
void close_file(std::ofstream& data_file, const std::string& output_filename, 
		const std::string& context_message);

//
//- Utilities for tabular write
//

/// Output the header row (labels) for a tabular data file for
/// variables and responses, with variables in input spec order.
/// Conditionally include interface ID.  Primary uses: environment
/// tabular data, pre-run output, surrogate approx evals
void write_header_tabular(std::ostream& tabular_ostream, const Variables& vars,
			  const Response& response,
			  const std::string& counter_label,
			  unsigned short tabular_format);
/// Output the header row (labels) for a tabular data file for
/// variables and additional labels not tied to a response.  Variables
/// are in input spec order.  Conditionally include interface ID.
/// Primary uses: MCMC chain export, including calibration sigmas.
void write_header_tabular(std::ostream& tabular_ostream, const Variables& vars,
			  const StringArray& addtnl_labels,
			  const std::string& counter_label,
			  unsigned short tabular_format);

/// Write the leading column with eval ID and conditionally, the interface ID
void write_leading_columns(std::ostream& tabular_ostream, size_t eval_id, 
			   const String& iface_id, 
			   unsigned short tabular_format);

/// Output a row of tabular data from a variables object.  All active/inactive
/// variables written in input spec order.  Conditionally include interface ID.
/// Primary uses: output of sampling sets.
void write_data_tabular(std::ostream& tabular_ostream, 
			const Variables& vars, const String& iface, 
			size_t counter, unsigned short tabular_format);

/// Output a row of tabular data from variables and response objects.
/// All active/inactive variables written in input spec order.
/// Conditionally include interface ID.  Primary uses: environment
/// tabular data, pre-run output, surrogate approx evals.
void write_data_tabular(std::ostream& tabular_ostream, 
			const Variables& vars, const String& iface, 
			const Response& response, size_t counter,
			unsigned short tabular_format);

/// PCE export: write freeform format file with whitespace-separated
/// data where each row has num_fns reals from coeffs, followed
/// by num_vars unsigned shorts from indices
void write_data_tabular(const std::string& output_filename, 
			const std::string& context_message,
			const RealVectorArray& output_coeffs, 
			const UShort2DArray& output_indices);


//
//- Utilities for tabular read
//

/// Check if an input stream contains unexpected additional data
bool exists_extra_data(std::istream& tabular_file);

/// read and discard header line from the stream
void read_header_tabular(std::istream& input_stream, 
			 unsigned short tabular_format);

/// read leading columns [ int eval_id [ String iface_id ] ]
int read_leading_columns(std::istream& input_stream,
			 unsigned short tabular_format);
/// read leading columns [ int eval_id [ String iface_id ] ]
void read_leading_columns(std::istream& input_stream,
			  unsigned short tabular_format,
			  int& eval_id, String& iface_id);

// TODO: The following need review, rework, and consolidation

//
// Uses: LeastSq data import (to be transitioned to Bayesian case
//       where each row will denote an experiment)
//
/// read possibly header-annotated whitespace-separated data into a
/// vector of length num_entries; if annotated then it's a column
/// vector for now
void read_data_tabular(const std::string& input_filename, 
		       const std::string& context_message,
		       RealVector& input_data, size_t num_entries,
		       unsigned short tabular_format);

/// Tabular read for ApproximationInterface challenge data: read
/// possibly header-annotated whitespace-separated data of possible mixed Variables,
/// followed by num_fns, each into RealMatrix with minimal error checking
void read_data_tabular(const std::string& input_filename, 
		       const std::string& context_message,
		       Variables vars, size_t num_fns,
		       RealMatrix& vars_matrix, RealMatrix& resp_matrix, 
                       unsigned short tabular_format, bool active_only);

/// Tabular read for PCE import: read possibly header-annotated
/// whitespace-separated data of unknown length where each row has
/// num_fns reals followed by num_vars unsigned shorts; append data to
/// arrays passed by reference
void read_data_tabular(const std::string& input_filename, 
		       const std::string& context_message,
		       RealVectorArray& input_coeffs, 
		       UShort2DArray& input_indices, 
		       unsigned short tabular_format,
		       size_t num_vars, size_t num_fns);

/// Tabular read for DataFitSurrModel (build points): read
/// whitespace-separated data with optional row and column headers
/// into lists of Variables and Responses until out of data
void read_data_tabular(const std::string& input_filename, 
		       const std::string& context_message,
		       Variables vars, Response resp, PRPList& input_prp,
		       unsigned short tabular_format,
		       bool verbose=false, bool active_only=false);

/// Tabular read for import_approx_points_file: read
/// whitespace-separated data with optional row and column headers
/// into a single matrix, with length of record as specified and
/// number of records to be determined by file content.  The matrix is
/// stored as record_len rows by num_records columns.
void read_data_tabular(const std::string& input_filename, 
		       const std::string& context_message,
		       RealMatrix& input_matrix, size_t record_len,
		       unsigned short tabular_format, bool verbose=false);

// BMA: Probably retire in favor of new data readers, rather than
// propagating to other Bayesian calibration and deterministic least
// squares
/// Tabular read for GPMSA data: read whitespace-separated data with
/// optional row and column headers into a single matrix, with size as
/// specified (one experiment per row)
void read_data_tabular(const std::string& input_filename, 
		       const std::string& context_message,
		       RealMatrix& input_matrix, 
		       size_t num_rows, size_t num_cols,
		       unsigned short tabular_format, bool verbose=false);

// special reader for list parameter studies: probably move back to ParamStudy
/// Tabular read for ParamStudy: read specified input data file into
/// arrays with sizes specified by the passed vc_totals array
size_t read_data_tabular(const std::string& input_filename, 
			 const std::string& context_message,
			 RealVectorArray& cva, IntVectorArray& diva, 
			 StringMulti2DArray& dsva, RealVectorArray& drva,
			 unsigned short tabular_format,
			 bool active_only, Variables vars);

} // namespace TabularIO

} // namespace Dakota

#endif // DAKOTA_TABULAR_IO_H
