/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#pragma once

#include "DakotaGlobalEnums.hpp"
#include "dakota_data_types.hpp"

#include <cstddef>

namespace Dakota {

/** \brief Output, restart, tabular-data, and results settings for Study.
 *
 * Default construction retains Dakota's normal library-mode output behavior.
 * \ingroup StudyAPI
 */
struct StudyOutputConfig
{
  ///< File receiving standard output; empty retains the active output stream.
  String outputFile{};
  ///< File receiving error output; empty retains the active error stream.
  String errorFile{};
  ///< Restart file read before execution; empty disables restart input.
  String readRestart{};
  ///< Maximum evaluation read from the restart file; zero is unlimited.
  size_t stopRestart{0};
  ///< Restart file written during execution; empty disables restart output.
  String writeRestart{};

  ///< Enable graphics when the selected build and execution mode support it.
  bool graphics{false};
  ///< Enable tabular graphics-data output.
  bool tabularGraphicsData{false};
  ///< Destination for tabular graphics data.
  String tabularGraphicsFile{"dakota_tabular.dat"};
  ///< Dakota tabular format enumeration; defaults to annotated output.
  unsigned short tabularFormat{TABULAR_ANNOTATED};

  ///< Numeric output precision; zero retains Dakota's default.
  int precision{0};

  ///< Enable results-database output.
  bool resultsOutput{false};
  ///< Base name for results-database output.
  String resultsOutputFile{"dakota_results"};
  ///< Dakota results-output format enumeration.
  unsigned short resultsOutputFormat{0};
  ///< Policy selecting model evaluations stored in results output.
  unsigned short modelEvalsSelection{MODEL_EVAL_STORE_TOP_METHOD};
  ///< Policy selecting interface evaluations stored in results output.
  unsigned short interfaceEvalsSelection{INTERF_EVAL_STORE_SIMULATION};
};

/** \brief Controls the pre-run, run, and post-run execution phases of Study.
 * \ingroup StudyAPI
 */
struct StudyRunConfig
{
  ///< Enable the pre-run phase.
  bool preRun{false};
  ///< Enable the main run phase.
  bool run{false};
  ///< Enable the post-run phase.
  bool postRun{false};

  ///< Input file for the pre-run phase.
  String preRunInput{};
  ///< Output file for the pre-run phase.
  String preRunOutput{};
  ///< Input file for the main run phase.
  String runInput{};
  ///< Output file for the main run phase.
  String runOutput{};
  ///< Input file for the post-run phase.
  String postRunInput{};
  ///< Output file for the post-run phase.
  String postRunOutput{};

  ///< Dakota format enumeration for pre-run output.
  unsigned short preRunOutputFormat{TABULAR_ANNOTATED};
  ///< Dakota format enumeration for post-run input.
  unsigned short postRunInputFormat{TABULAR_ANNOTATED};
};

/** \brief Top-level typed configuration for a DI/library-mode Dakota study.
 * \ingroup StudyAPI
 */
struct StudyConfig
{
  ///< Output, restart, tabular-data, and results settings.
  StudyOutputConfig output;
  ///< Pre-run, run, and post-run phase settings.
  StudyRunConfig run;
};

} // namespace Dakota
