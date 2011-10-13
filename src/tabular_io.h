/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_TABULAR_IO_H
#define DAKOTA_TABULAR_IO_H

#include "data_types.h"

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
   Proposed user documentation:

   DAKOTA versions 5.1+ (October 2011) and newer use two formats for
   tabular data file input and output.  Tabular data refer to numeric
   data in text form related to, e.g., tabular graphics data, least
   squares and Bayesian calibration data, samples/points files for
   surrogates, pre-run output, and post-run input.  Both formats are
   written/read with C++ stream floating point ops, so most integer
   and floating point formats are acceptable for numeric data.  The
   formats are:

   1. Annotated Matrix (default for all I/O; specified via 'annotated'): 
   text file with 1 leading row of comments/labels and 1 leading
   column of evaluation/row IDs surrounding num_rows x num_cols
   whitespace-separated numeric data, (newlines separating rows not
   currently required, but may be in the future).  The numeric data in
   a row may correspond to variables, variables followed by responses,
   data point for calibration, etc., depending on context.

   2. Free-form (optional; previously default for samples files and
   least squares data; specified via 'freeform'): text file with no
   leading row and no leading column.  The num_rows x num_cols total
   numeric data entries may appear separated with any whitespace
   including arbitrary spaces, tabs, and newlines.  In this format,
   vectors may therefore appear as a single row or single column (or
   mixture, entries will populate the vector in order).  Matrices will
   be populated row-wise as data is read (row 1 all cols, then row 2
   all cols, etc.).

   For both fomats, a warning will be generated if a specific number
   of data are expected, but extra is found and an error generated
   when there is insufficient data.

   Note that some TPLs like Coliny and JEGA manage their own file I/O
   and only support the free-form option.
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
