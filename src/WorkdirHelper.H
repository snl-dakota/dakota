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

#if defined(_WIN32) || defined(_WIN64)

  #include <io.h>
  #define DAK_CHDIR(s) (SetCurrentDirectory(s) == 0)
  #define DAK_MKDIR(a,b) mkdir(a)
  #define DAK_MAXPATHLEN MAX_PATH
  #define DAK_PATH_ENV_NAME "Path"
  #define DAK_PATH_SEP ';'
  #define DAK_SLASH '\\'

#elif defined(HAVE_UNISTD_H) 

  #include <unistd.h>
  #include <sys/param.h>
  #define DAK_CHDIR chdir
  #define DAK_MKDIR(a,b) mkdir(a,b)
  #define DAK_MAXPATHLEN MAXPATHLEN
  #define DAK_PATH_ENV_NAME "PATH"
  #define DAK_PATH_SEP ':'
  #define DAK_SLASH '/'

#endif // WIN32

#include <sys/stat.h>
#include <string>
#include <utility>
#include <vector>


namespace Dakota {

class WorkdirHelper
{
  //
  //- Heading: Friends
  //

  friend class CommandShell;                        // calls change_cwd, reset
  friend class ForkAnalysisCode;                    // calls arg_adjust

  /// Treat get_npath as a legacy, "blackbox" utility
  friend void get_npath(int, char**);               // dakPreferredEnvPath access

  /// Treat my_cp as a legacy, "blackbox" utility
  friend int my_cp(const char*, const struct stat*,
                   int, int, void*);                // calls symlink

public:

  /// Query for dakota's startup $PWD
  static const std::string& startup_pwd() { return startupPWD; }

  /// Prepend $PATH environment variable with an extra_path
  static void prepend_preferred_env_path(const std::string& extra_path);


private:

  /// Initializes class member, startupPATH
  static std::string init_startup_path();

  /// Initializes class member, dakPreferredEnvPath
  static std::string init_preferred_env_path();

  /// "Helper"/wrapper for system symlink when "from" needs path adjustment
  static int symlink(const char* from, const char* to);

  static void change_cwd(const std::string& wd_str);  // old nm: workdir_adjust
  static void reset();                                // old nm: workdir_reset

  static const char** arg_adjust(bool cmd_line_args,
                                 const std::vector<std::string>& args,
                                 const char **av, const std::string& workdir);
  //
  //- Heading: Data
  //

  /// Value of $PWD var upon entry to dakota main()
  static std::string startupPWD;

  /// Value of $PATH (%PATH% on windows) var upon entry to dakota main()
  static std::string startupPATH;

  /// PATH=".:startupPWD:startupPATH"
  static std::string dakPreferredEnvPath;


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


#ifdef DAKOTA_HAVE_BOOST_FS
/// Utilities to manage API differences in BFS_VERSION_2 vs BFS_VERSION_3

/// Same as file_path.filename() in latest boost::filesystem
inline static std::string filename(const bfs::path& file_path)
{
#if BOOST_FILESYSTEM_VERSION < 3
  return file_path.empty() ? std::string() : *--file_path.end();
#else
  return file_path.empty() ? std::string() : (*--file_path.end()).string();
#endif
}

/// Should be same as fp.parent_path() in latest boost::filesystem
// This is not entirely according to bfs::path::parent_path() in 1.39.0
inline static bfs::path parent_path(const bfs::path& fp)
{
  std::string fn = filename(fp);
  std::string fpath_str = fp.string();

  return fpath_str.substr(0, fpath_str.length() - fn.length() - 1);
}


inline static bool contains(const bfs::path& dir_path,
                            const std::string& file_name,
                            bool recurse, bfs::path& path_found)
{
  bool success_flag(false);
  if( exists(dir_path) && is_directory(dir_path) ) {
    bfs::directory_iterator iter(dir_path), dir_end;

    for(; iter != dir_end; ++iter) {
      /* WJB: consider possibility of recursion into subdirs
      if( bfs::is_directory(*iter) && recurse ) {
        if( contains(*iter, file_name, recurse, path_found) ) {
          success_flag = true;
          break;
        }
      } */
      //else if( filename(*iter) == file_name ) {
#if BOOST_FILESYSTEM_VERSION < 3
      if( iter->leaf() == file_name ) {
#else
      if( filename(*iter) == file_name ) {
#endif
        path_found = *iter;
        success_flag = true;
        break;
      }
    }
    return success_flag;
  }
}
#endif // DAKOTA_HAVE_BOOST_FS

} // namespace Dakota

#endif // WORKDIR_HELPER_H
