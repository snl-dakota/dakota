/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "ResultsFileReader.hpp"

#include "JSONResultsFileReader.hpp"
#include "StandardResultsFileReader.hpp"
#include "dakota_global_defs.hpp"

namespace Dakota {

std::unique_ptr<ResultsFileReader> ResultsFileReader::get_reader(
    unsigned short results_file_format, bool labeled) {
  switch (results_file_format) {
    case RESULTS_FILE_STANDARD:
      return std::make_unique<StandardResultsFileReader>(labeled);
      break;
    case RESULTS_FILE_JSON:
      return std::make_unique<JSONResultsFileReader>(labeled);
      break;
    default:
      return nullptr;
      break;
  }
}

ResultsFileReader::ResultsFileReader(bool labeled) : labeledFlag(labeled) {}
}  // namespace Dakota
