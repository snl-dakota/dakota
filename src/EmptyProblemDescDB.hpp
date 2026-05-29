/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef EMPTY_PROBLEM_DESC_DB_H
#define EMPTY_PROBLEM_DESC_DB_H

#include "ProblemDescDB.hpp"

namespace Dakota {
 
class EmptyProblemDescDB: public ProblemDescDB
{
public:

  /// constructor
  EmptyProblemDescDB(int world_size, int world_rank);
  /// destructor
  ~EmptyProblemDescDB();
};


} // namespace Dakota

#endif
