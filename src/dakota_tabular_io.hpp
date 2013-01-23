/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_TABULAR_IO_H
#define DAKOTA_TABULAR_IO_H

#include "dakota_data_types.hpp"

/// Utility functions for reading and writing tabular data files
/**
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
//- Utilities for opening tabular files
//

/// open the file specified by name for reading, using passed input
/// stream, presenting context-specific error on failure
void open_file(std::ifstream& data_file, const std::string& input_filename, 
	       const std::string& context_message);

/// open the file specified by name for writing, using passed output
/// stream, presenting context-specific error on failure
void open_file(std::ofstream& data_file, const std::string& output_filename, 
	       const std::string& context_message);


//
//- Utilities for tabular write
//

// TODO: consider separate prototype without Response for false case
// and annotated vs. non-annotated case?

//
// Uses: tabular graphics data and pre-run output
//
/// output the header row (labels) for a tabular data file
/// used by Analyzer and Graphics
void write_header_tabular(std::ostream& tabular_ostream, 
  const std::string& counter_label, const Variables& vars, 
  const Response& response, bool active_only, bool response_labels);


// TODO: counter doesn't make sense when not annotated!
// (would like to avoid default parameters if possible)

//
// Uses: tabular graphics data and pre-run output
//
/// output a row of tabular data from variables and response object
/// used by graphics to append to tabular file during iteration
void write_data_tabular(std::ostream& tabular_ostream, 
  size_t counter, const Variables& vars, 
  const Response& response, bool active_only, bool annotated,
  bool write_responses);


//
//- Utilities for tabular read
//

/// Check if an input stream contains unexpected additional data
bool exists_extra_data(std::istream& tabular_file);

/// read and discard header line from the stream
void read_header_tabular(std::istream& input_stream);


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
		       bool annotated);

//
// Uses: DataFitSurrModel (highly specialized)
//
/// read whitespace-separated data with optional row and column headers into 
/// lists of Variables (using provided SVD) and Responses until out of data;
/// continuous variables only
void read_data_tabular(const std::string& input_filename, 
		       const std::string& context_message,
		       VariablesList& input_vars, ResponseList& input_resp,
		       const SharedVariablesData& svd,
		       size_t num_c_vars,
		       const ActiveSet& temp_set,
		       bool annotated,
		       bool verbose=false);


//
// Uses: Bayesian calibration
//       Eventually regular least squares where we'll have one experiment/row
//
/// read whitespace-separated data with optional row and column
/// headers into a single matrix
void read_data_tabular(const std::string& input_filename, 
		       const std::string& context_message,
		       RealMatrix& input_matrix, 
		       size_t num_rows,
		       size_t num_cols,
		       bool annotated,
		       bool verbose=false
		       );

} // namespace TabularIO

} // namespace Dakota

#endif // DAKOTA_TABULAR_IO_H
