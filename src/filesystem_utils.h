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

#if defined(_WIN32) 
#include <io.h>
#define DAK_MKDIR(a,b) mkdir(a)
#elif defined(HAVE_UNISTD_H) 
#include <unistd.h>
#define DAK_MKDIR(a,b) mkdir(a,b)
#endif

#ifdef DAKOTA_HAVE_BOOST_FS
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
namespace bfs = boost::filesystem;
#endif

//#include <boost/shared_array.hpp>  // WJB - ToDo: look into improved mem mgmt
#include <string>
#include <utility>


namespace Dakota {

int not_executable(const char*, const char*);


int rec_cp(const char *from, const char *todir, int copy,
           int flatten, int replace);
int rec_rmdir(const char*);


typedef
struct WorkdirHelper
{
  static std::pair<std::string, std::string> get_dakpath();

  static void change_cwd(const std::string& wd_str);  // old nm: workdir_adjust
  static void reset();                                // old nm: workdir_reset

  static char* cwdBegin;      // old name: dakdir
  static char* envPathBegin;  // old name: dakpath
  //static std::pair<std::string, std::string> startupCwdEnvPathPair;

#ifdef DAKOTA_HAVE_BOOST_FS
  static bfs::path dakLaunchDir;  // WJB: should be "similar" to DMG dakdir
#endif
} Filesys_buf;


} // namespace Dakota

#endif // FILESYSTEM_UTILS_H
