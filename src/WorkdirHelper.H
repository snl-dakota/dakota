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

// WJB:  eventually (before Dak rel 5.2) #define BOOST_FILESYSTEM_NO_DEPRECATED

#ifdef DAKOTA_HAVE_BOOST_FS
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
namespace bfs = boost::filesystem;
#endif

#if defined(_WIN32) || defined(_WIN64)

  #include <io.h>
  #include <windows.h>
  #include <direct.h>
  #define DAK_CHDIR(s) (SetCurrentDirectory(s) == 0)
  #define DAK_MKDIR(a,b) _mkdir(a)
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

#endif // _WIN32 or _WIN64

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

  friend class CommandShell;                       // calls change_cwd, reset
  friend class ForkAnalysisCode;                   // calls arg_adjust

  /// Treat get_npath as a legacy, "blackbox" utility
#ifdef DEBUG_GET_NPATH
  /** get_npath "shuffles" the string representing the current $PATH variable
      definition so that '.' is first in the $PATH.  It then returns the new
      string as the result (last arg in the call). */
  friend void get_npath(int,std::string*);         // dakPreferredEnvPath access
#endif

  /// Treat my_cp as a legacy, "blackbox" utility
  /** my_cp is a wrapper around 'cp -r'.  The extra layer allows for symlink
      to be used instead of file copy. */
  friend int my_cp(const char*, const struct stat*,
                   int, int, void*);               // calls symlink

public:

  /// Query for dakota's startup $PWD
  static const std::string& startup_pwd() { return startupPWD; }

  /// Prepend $PATH environment variable with an extra_path
  static void prepend_preferred_env_path(const std::string& extra_path);

  /// Returns the string representing the path to the analysis driver
  static std::string which(const std::string& driver_name);


private:

  /// Initializes class member, startupPATH
  static std::string init_startup_path();

  /// Initializes class member, dakPreferredEnvPath
  static std::string init_preferred_env_path();

  /// Tokenizes $PATH environment variable into a "list" of directories
  static std::vector<std::string> tokenize_env_path(const std::string& path);

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

/// Same as path.is_absolute() in latest boost::filesystem
inline bool path_is_absolute(const bfs::path& p)
{
#if BOOST_FILESYSTEM_VERSION < 3
  #if defined(_WIN32) || defined(_WIN64)
    return ( p.has_root_name() && p.has_root_directory() );
  #else
    return p.has_root_directory();
  #endif // _WIN 32 or 64
#else
  return p.is_absolute();
#endif // BOOST_FILESYSTEM_VERSION
}

/// Same as file_path.filename() in latest boost::filesystem
inline std::string filename(const bfs::path& file_path)
{
#if BOOST_FILESYSTEM_VERSION < 3
  return file_path.empty() ? std::string() : *--file_path.end();
#else
  return file_path.empty() ? std::string() : (*--file_path.end()).string();
#endif
}

/// Should be same as fp.parent_path() in latest boost::filesystem
inline bfs::path parent_path(const bfs::path& fp)
{
  std::string fn = filename(fp);
  std::string fpath_str = fp.string();

  return fpath_str.substr(0, fpath_str.length() - fn.length() - 1);
}


/// Helper for "which" - sets complete_filepath from dir_path/file_name combo
inline bool contains(const bfs::path& dir_path, const std::string& file_name,
                     boost::filesystem::path& complete_filepath)
{
  complete_filepath = dir_path; complete_filepath /= file_name;
  return boost::filesystem::is_regular_file(complete_filepath);
}
#endif // DAKOTA_HAVE_BOOST_FS

} // namespace Dakota

#endif // WORKDIR_HELPER_H
