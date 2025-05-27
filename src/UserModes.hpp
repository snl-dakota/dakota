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

/// @brief  UserModes stores the user-requested run modes, file names, and file formats
class UserModes {

    public:

    /// is pre-run mode active?
    bool preRun{};
    /// is run mode active?
    bool run{};
    /// is post-run mode active?
    bool postRun{};

    /// did the user provide a pre-run, run, or post-run flag?
    bool requestedUserModes{};

    // The meanings of preRun, run, postRun, and userModes feel inconsistent.
    // In ProgramOptions:: parse() and validate_run_modes(), preRun, run, and postRun
    // are treated as MODES: If a user specified none of them, all three are set to true
    // because Dakota should do all three.
    // userModes, on the other hand, is only true when a user made a non-default selection.

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
    ///< tabular format for post_run input
    unsigned short postRunInputFormat{TABULAR_ANNOTATED};

    /// helper function for reading some class data from MPI buffer
    void read(MPIUnpackBuffer& s);

    /// helper function for writing some class data to MPI buffer
    void write(MPIPackBuffer& s) const;
};

/// MPIUnpackBuffer extraction operator
MPIUnpackBuffer& operator>>(MPIUnpackBuffer& s, UserModes& u);

/// MPIPackBuffer insertion operator
MPIPackBuffer& operator<<(MPIPackBuffer& s, const UserModes& u);

}