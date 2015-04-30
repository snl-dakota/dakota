/*  _________________________________________________________________________
 *
 *  Acro: A Common Repository for Optimizers
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README.txt file in the top Acro directory.
 *  _________________________________________________________________________
 */

//
// MultiStart.cpp
//

#if 0

#include <acro_config.h>
#include <scolib/MultiStart.h>
#include <colin/SolverMngr.h>
#include <colin/ProblemTraits.h>

namespace scolib {

typedef MultiStart<utilib::MixedIntVars, colin::UMINLP2_problem> ms_type;

REGISTER_COLIN_SOLVER_WITH_ALIAS(ms_type, "sco:MultiStart","sco:ms", "The SCO MultiStart optimizer")

} // namespace scolib

#endif
