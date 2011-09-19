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
#include "global_defs.h"
#include <boost/algorithm/string.hpp> 
#include <string>
#include <vector>

#ifdef __SUNPRO_CC
#include <stdlib.h>  // for putenv (eventually, setenv)
#endif


namespace Dakota {


int rec_cp(const char *from, const char *todir, int copy,
           int flatten, int replace);
int rec_rmdir(const char*);


/// Portability adapter for getcwd
std::string get_cwd();


// WJB:  MAJOR OVERHAUL required to make this function a member of 
//       WorkdirHelper (logically belongs there, but worth the effort?)
void workdir_adjust(const std::string& workdir);


/// Utility function from boost/test, not available in the DAKOTA snapshot
inline void putenv_impl(const std::string& name, const std::string& value)
{
  setenv(name.c_str(), value.c_str(), 1);
}

inline void putenv_impl(const std::string& name_and_value)
{
  if ( putenv( const_cast<char*>( name_and_value.c_str() ) ) ) {
    Cerr << "\nError: putenv(" << name_and_value
         << ") failed in putenv_impl()" << std::endl;
    abort_handler(-1);
  }

/* WJB: alternate impl IF I believe what I read at following site:
  // http://stackoverflow.com/questions/5873029/questions-about-putenv-and-setenv
  std::vector<std::string> var_name_and_val_tokens;

  boost::split( var_name_and_val_tokens,
                name_and_value, boost::is_any_of("=") );

  if ( setenv(var_name_and_val_tokens[0].c_str(),
              var_name_and_val_tokens[1].c_str(), true) ) {
    Cerr << "\nError: setenv(" << name_and_value
         << ") failed in putenv_impl()" << std::endl;
  }
*/
}

} // namespace Dakota

#endif // FILESYSTEM_UTILS_H
