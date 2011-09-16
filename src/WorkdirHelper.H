/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Description:  WorkdirHelper (may be completely replaced by Boost.Filesystem)
//-
//- Version: $Id$

#ifndef WORKDIR_HELPER_H
#define WORKDIR_HELPER_H

#include "system_defs.h"
//#include <boost/shared_array.hpp>   WJB - ToDo: look into improved mem mgmt
//                                    e.g. use of malloc w/o free is undesirable

#ifdef DAKOTA_HAVE_BOOST_FS
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
namespace bfs = boost::filesystem;
#endif

#if defined(WIN32) 

  #include <io.h>
  #define DAK_CHDIR(s) (SetCurrentDirectory(s) == 0)
  #define DAK_MKDIR(a,b) mkdir(a)
  #define DAK_PATH_ENV_NAME "Path"
  #define DAK_PATH_SEP ';'
  #define DAK_SLASH '\\'

#elif defined(HAVE_UNISTD_H) 

  #include <unistd.h>
  #define DAK_CHDIR chdir
  #define DAK_MKDIR(a,b) mkdir(a,b)
  #define DAK_PATH_ENV_NAME "PATH"
  #define DAK_PATH_SEP ':'
  #define DAK_SLASH '/'

#endif // WIN32

#include <string>
#include <utility>
#include <vector>


namespace Dakota {

class WorkdirHelper
{
  //
  //- Heading: Friends
  //

  friend class CommandShell;                          // calls change_cwd, reset
  friend class ForkAnalysisCode;                      // calls arg_adjust

  /// Treat get_npath as a legacy, "blackbox" utility
  friend void get_npath(int, char**);                 // calls get_dakpath

private:

  /// Initializes class member, startupPATH
  static std::string init_startup_path();

  /// "Factory" for memory buffer needed by get_npath
  static char* get_dakpath();

  static void change_cwd(const std::string& wd_str);  // old nm: workdir_adjust
  static void reset();                                // old nm: workdir_reset

  static const char** arg_adjust(bool cmd_line_args,
                                 const std::vector<std::string>& args,
                                 const char **av, const std::string& workdir);
  //
  //- Heading: Data
  //

  /// Value of $PWD var upon entry to dakota main()
  static std::string startupPWD; // "old name": dakdir (ptr to the start of cwd)

  /// Value of $PATH (%PATH% on windows) var upon entry to dakota main()
  static std::string startupPATH; // "old name": dakpath (ptr just past the end of cwd)


  static std::vector<char> cwdAndEnvPathBuf; // improve mem mgmt of key buffer


#ifdef DAKOTA_HAVE_BOOST_FS
  static bfs::path dakLaunchDir;    // WJB: should be "similar" to DMG dakdir
#endif

  //
  //- Heading:  DISABLED constructors, destructor, assignment operator
  //

  /// default constructor
  WorkdirHelper();
  /// copy constructor
  WorkdirHelper(const WorkdirHelper&);

  /// destructor
  ~WorkdirHelper();

  /// assignment operator
  WorkdirHelper& operator=(const WorkdirHelper&);
};


/// Utility function from legacy, "not_executable" module -- DO NOT TOUCH!
const char** arg_list_adjust(const char **, void **);


} // namespace Dakota

#endif // WORKDIR_HELPER_H
