/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_SYSTEM_DEFS_H
#define DAKOTA_SYSTEM_DEFS_H

#if defined(HAVE_CONFIG_H) && !defined(DISABLE_DAKOTA_CONFIG_H)
// Unfortunately, STILL need to set HAVE_CONFIG_H since it is used in many
// TPL header files included while building Dakota/src (even in the CMake
// build!) so use a "disable config header" conditional to help manage the
// transition (so, far pecos/surfpack are DONE and ddace, opt++ are close).
#include "dakota_config.h"
#endif  // HAVE_CONFIG_H

// C++ headers

#include <fstream>
#include <iomanip>
#include <iostream>

// C++ compiler using new, h-less standard C headers
#include <cassert>
#include <cctype>
#include <cerrno>
#include <cfloat>
#include <climits>
#include <cmath>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#endif  // DAKOTA_SYSTEM_DEFS_H
