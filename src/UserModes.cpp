/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */
    
#include "RunOptions.hpp"
#include "dakota_data_io.hpp"

namespace Dakota {
    /// MPIUnpackBuffer extraction operator
MPIUnpackBuffer& operator>>(MPIUnpackBuffer& s, RunOptions& u)
{ 
  u.read(s);
  return s; 
}

/// MPIPackBuffer insertion operator
MPIPackBuffer& operator<<(MPIPackBuffer& s, const RunOptions& u)
{ 
  u.write(s);
  return s; 
}

void RunOptions::read(MPIUnpackBuffer& is)
{
  is >> preRun >> run >> postRun >> requestedUserModes
     >> preRunInput >> preRunOutput >> runInput >> runOutput
     >> postRunInput >> postRunOutput
     >> preRunOutputFormat >> postRunInputFormat;
}

void RunOptions::write(MPIPackBuffer& is) const
{
  is << preRun << run << postRun << requestedUserModes
     << preRunInput << preRunOutput << runInput << runOutput
     << postRunInput << postRunOutput
     << preRunOutputFormat << postRunInputFormat;
}

bool operator==(const RunOptions& lhs, const RunOptions& rhs)
{
  return lhs.preRun == rhs.preRun &&
         lhs.run == rhs.run &&
         lhs.postRun == rhs.postRun &&
         lhs.requestedUserModes == rhs.requestedUserModes &&
         lhs.preRunInput == rhs.preRunInput &&
         lhs.preRunOutput == rhs.preRunOutput &&
         lhs.runInput == rhs.runInput &&
         lhs.runOutput == rhs.runOutput &&
         lhs.postRunInput == rhs.postRunInput &&
         lhs.postRunOutput == rhs.postRunOutput &&
         lhs.preRunOutputFormat == rhs.preRunOutputFormat &&
         lhs.postRunInputFormat == rhs.postRunInputFormat;
}

bool operator!=(const RunOptions& lhs, const RunOptions& rhs)
{
  return !(lhs == rhs);
}

}