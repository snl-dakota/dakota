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
// PIDOMS.cpp
//

#include <acro_config.h>
#if defined(ACRO_USING_PEBBL) 
// && defined(ACRO_USING_PLGO)

#include <scolib/PIDOMS.h>
#include <colin/SolverMngr.h>

namespace scolib {

// Defines scolib::StaticInitializers::PIDOMS_bool
REGISTER_COLIN_SOLVER_WITH_ALIAS(PIDOMS, "sco:PIDOMS", "sco:pidoms", "The SCO PIDOMS optimizer")

} // namespace scolib

#else

namespace scolib {
namespace StaticInitializers {
extern const volatile bool PIDOMS_bool = false;
} // namespace scolib::StaticInitializers
} // namespace scolib

#endif // defined(ACRO_USING_PLGO) && defined(ACRO_USING_PEBBL)
