/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Utility:      Dakota Windows headers
//- Description:  Include file to compartmentalize Windows includes and
//-               definitions for consistency across build
//- Owner:        Brian Adams
//- Version: $Id$

#ifndef DAKOTA_WINDOWS_H
#define DAKOTA_WINDOWS_H

// eventually use only _WIN32 here?
#if defined(_WIN32) || defined(_MSC_VER) || defined(__MINGW32__)
// avoid needless Windows API headers
#define WIN32_LEAN_AND_MEAN
// Avoid redefinition of MIN and MAX macros by MSVC
#define NOMINMAX
#include <windows.h>
#endif

#endif  // DAKOTA_WINDOWS_H
