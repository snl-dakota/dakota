/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#pragma once

#include <string>
#include "dakota_global_defs.hpp"

namespace Dakota {

class MPIUnpackBuffer;
class MPIPackBuffer;

/// RunOptions stores iterator pre/run/post phase policy and phase I/O settings.
class RunOptions {
public:
  /// is pre-run mode active?
  bool preRun{};
  /// is run mode active?
  bool run{};
  /// is post-run mode active?
  bool postRun{};

  /// did the user provide a pre-run, run, or post-run flag?
  bool requestedUserModes{};

  /// filename for pre-run input
  std::string preRunInput{};
  /// filename for pre-run output
  std::string preRunOutput{};
  /// filename for run input
  std::string runInput{};
  /// filename for run output
  std::string runOutput{};
  /// filename for post-run input
  std::string postRunInput{};
  /// filename for post-run output
  std::string postRunOutput{};

  /// tabular format for pre_run output
  unsigned short preRunOutputFormat{TABULAR_ANNOTATED};
  /// tabular format for post_run input
  unsigned short postRunInputFormat{TABULAR_ANNOTATED};

  /// helper function for reading class data from MPI buffer
  void read(MPIUnpackBuffer& s);

  /// helper function for writing class data to MPI buffer
  void write(MPIPackBuffer& s) const;
};

bool operator==(const RunOptions& lhs, const RunOptions& rhs);
bool operator!=(const RunOptions& lhs, const RunOptions& rhs);

/// MPIUnpackBuffer extraction operator
MPIUnpackBuffer& operator>>(MPIUnpackBuffer& s, RunOptions& u);

/// MPIPackBuffer insertion operator
MPIPackBuffer& operator<<(MPIPackBuffer& s, const RunOptions& u);

} // namespace Dakota
