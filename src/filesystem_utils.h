/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Description:  Filesystem_utils (to be replaced by Boost, eventually)
//-
//- Version: $Id$

#ifndef FILESYSTEM_UTILS_H
#define FILESYSTEM_UTILS_H

#include "system_defs.h"

#if defined(_WIN32) 
#include <io.h>
#define DAK_MKDIR(a,b) mkdir(a)
#elif defined(HAVE_UNISTD_H) 
#include <unistd.h>
#define DAK_MKDIR(a,b) mkdir(a,b)
#endif

#include <string>


namespace Dakota {

int not_executable(const char*, const char*);

void workdir_adjust(const char* workdir);
void workdir_reset();


// WJB - ToDo, ASAP: void workdir_adjust(const std::string& workdir);
#if 0
// WJB- ToDo: consider a WorkdirManager class with a C++ implementation
//            with members funcs properly accessing dakdir/dakpath private data?
inline void wd_reset()
{
  if (chdir(dakdir)) {
    // Cerr ...
    abort_handler(-1);
  }
  if (putenv(dakpath)) {
    // Cerr ...
    abort_handler(-1);
  }
}
#endif // WJB 0

} // namespace Dakota

#endif // FILESYSTEM_UTILS_H
