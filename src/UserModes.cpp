/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "UserModes.hpp"

#include "dakota_data_io.hpp"

namespace Dakota {
/// MPIUnpackBuffer extraction operator
MPIUnpackBuffer& operator>>(MPIUnpackBuffer& s, UserModes& u) {
  u.read(s);
  return s;
}

/// MPIPackBuffer insertion operator
MPIPackBuffer& operator<<(MPIPackBuffer& s, const UserModes& u) {
  u.write(s);
  return s;
}

void UserModes::read(MPIUnpackBuffer& is)

{
  is >> preRun >> run >> postRun >> preRunInput >> preRunOutput >> runInput >>
      runOutput >> postRunInput >> postRunOutput >> preRunOutputFormat >>
      postRunInputFormat;
}

void UserModes::write(MPIPackBuffer& is) const {
  is << preRun << run << postRun;
  is << preRunInput << preRunOutput << runInput << runOutput << postRunInput
     << postRunOutput;
  is << preRunOutputFormat << postRunInputFormat;
}

}  // namespace Dakota