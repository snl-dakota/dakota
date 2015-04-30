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
// EAminlp.cpp
//

#include <acro_config.h>
#include <scolib/EAminlp.h>
#include <colin/SolverMngr.h>

namespace scolib {

REGISTER_COLIN_SOLVER_WITH_ALIAS( EAminlp,"sco:EAminlp","sco:ea", 
                                  "The SCO EAminlp optimizer" )

} // namespace scolib
