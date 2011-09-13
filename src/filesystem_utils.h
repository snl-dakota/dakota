/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Description:  Filesystem_utils (to be replaced by Boost, eventually)
//-
//- Version: $Id$

#ifndef FILESYSTEM_UTILS_H
#define FILESYSTEM_UTILS_H

#include "system_defs.h"
#include <string>


namespace Dakota {

int not_executable(const char*, const char*);


int rec_cp(const char *from, const char *todir, int copy,
           int flatten, int replace);
int rec_rmdir(const char*);


/// Portability adapter for getcwd
std::string get_cwd();


// WJB:  MAJOR OVERHAUL required to make this function a member of 
//       WorkdirHelper (logically belongs there, but worth the effort?)
void workdir_adjust(const std::string& workdir);

} // namespace Dakota

#endif // FILESYSTEM_UTILS_H
