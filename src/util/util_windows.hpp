/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_UTIL_WINDOWS_HPP
#define DAKOTA_UTIL_WINDOWS_HPP

// eventually use only _WIN32 here?
#if defined(_WIN32) || defined(_MSC_VER) || defined(__MINGW32__)
// Avoid needless Windows API headers
#define WIN32_LEAN_AND_MEAN
// Use Math Constants that are not in Standard C++
#define _USE_MATH_DEFINES
// Avoid redefinition of MIN and MAX macros by MSVC
#define NOMINMAX
#include <windows.h>
#endif

#endif  // DAKOTA_UTIL_WINDOWS_HPP
