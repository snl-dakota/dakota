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

/// Output-related configuration for DI/library-mode studies.
struct StudyOutputConfig
{
  String outputFile{};
  String errorFile{};
  String readRestart{};
  size_t stopRestart{0};
  String writeRestart{};

  bool graphics{false};
  bool tabularGraphicsData{false};
  String tabularGraphicsFile{"dakota_tabular.dat"};
  unsigned short tabularFormat{TABULAR_ANNOTATED};

  int precision{0};

  bool resultsOutput{false};
  String resultsOutputFile{"dakota_results"};
  unsigned short resultsOutputFormat{0};
  unsigned short modelEvalsSelection{MODEL_EVAL_STORE_TOP_METHOD};
  unsigned short interfaceEvalsSelection{INTERF_EVAL_STORE_SIMULATION};
};

/// Pre/run/post phase configuration for DI/library-mode studies.
struct StudyRunConfig
{
  bool preRun{false};
  bool run{false};
  bool postRun{false};

  String preRunInput{};
  String preRunOutput{};
  String runInput{};
  String runOutput{};
  String postRunInput{};
  String postRunOutput{};

  unsigned short preRunOutputFormat{TABULAR_ANNOTATED};
  unsigned short postRunInputFormat{TABULAR_ANNOTATED};
};

/// Top-level typed configuration for a DI/library-mode Dakota study.
struct StudyConfig
{
  StudyOutputConfig output;
  StudyRunConfig run;
};

} // namespace Dakota
