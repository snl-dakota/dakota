/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Description:  WorkdirHelper (may be completely replaced by Boost.Filesystem)
//-
//- Version: $Id$

#ifndef WORKDIR_HELPER_H
#define WORKDIR_HELPER_H

#include "dakota_system_defs.hpp"
#include "dakota_data_types.hpp"
//#include <boost/shared_array.hpp>   WJB - ToDo: look into improved mem mgmt
//                                    e.g. use of malloc w/o free is undesirable

#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <boost/iterator/filter_iterator.hpp>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
namespace bfs = boost::filesystem;

#if defined(_WIN32) || defined(_WIN64)

  #define NOMINMAX
  #include <io.h>
  #include "dakota_windows.h"
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

/// define directory creation options
enum { DIR_CLEAN, DIR_PERSIST, DIR_ERROR };


// Notes on wildcard matching:

// decent example: http://www.cplusplus.com/forum/general/88115/

// see   http://www.tldp.org/LDP/GNU-Linux-Tools-Summary/html/x11655.htm
//       http://en.wikipedia.org/wiki/Glob_%28programming%29
//       "man 7 glob" 
// for more globbing we could support
// later: {} [] ! 

// supporting [] could be tricky due to order precedence

// TODO: escape various things:
// http://stackoverflow.com/questions/3300419/file-name-matching-with-wildcard


/// Predicate that returns true when the passed path matches the
/// wild_card with which it was configured.  
/// Currently supports * and ?.
struct MatchesWC {

  // path value_type on Windows is wchar, otherwise char; try to
  // manage generically in this struct by converting where needed
 
  /// ctor that builds and stores the regular expression
  MatchesWC(const bfs::path& wild_card)
  { 

    bfs::path::string_type file_regex(wild_card.native());

    // map * and ? wildcards to regular expression syntax
    boost::replace_all(file_regex, ".", "\\.") ;
    boost::replace_all(file_regex, "*", ".*") ;
    boost::replace_all(file_regex, "?", "(.{1,1})") ;

    boost::regex::flag_type regex_opts = boost::regex::normal;
#ifdef _WIN32
    // allow case insensitive match on native Windows
    regex_opts |= boost::regex::icase;
#endif

    wildCardRegEx.assign(file_regex, regex_opts);
  }

  /// return true is dir_entry matches wildCardRegEx
  bool operator()(const bfs::path& dir_entry) 
  {  
    // generic vs. native shouldn't matter for the filename part;
    // using native in case we extend the regex to directories later
    return boost::regex_match(dir_entry.filename().native(), wildCardRegEx);
  }

  /// archived RegEx; wchar-based on Windows
  boost::basic_regex<bfs::path::value_type> wildCardRegEx;
};

/// a glob_iterator filters a directory_iterator based on a wildcard predicate
typedef boost::filter_iterator<MatchesWC, bfs::directory_iterator> glob_iterator;



class WorkdirHelper
{
  //
  //- Heading: Friends
  //

  friend class CommandShell;                       // calls change_cwd, reset
#if defined(HAVE_SYS_WAIT_H) && defined(HAVE_UNISTD_H)
  friend class ForkApplicInterface;                // calls arg_adjust
#elif defined(_WIN32) // or _MSC_VER (native MSVS compilers)
  friend class SpawnApplicInterface;               // calls arg_adjust
#endif // HAVE_SYS_WAIT_H, HAVE_UNISTD_H

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

  /// initialize paths in the workdir helper at runtime
  static void initialize();

  /// Query for dakota's startup $PWD
  static const std::string& startup_pwd() { return startupPWD; }

  /// Prepend $PATH environment variable with an extra_path
  static void prepend_preferred_env_path(const std::string& extra_path);

  /// Returns the string representing the path to the analysis driver,
  //  including typical windows extensions
  static std::string which(const std::string& driver_name);

  /// given a string with an optional path and a wildcard, e.g.,
  /// /tmp/D*.?pp, parse it into the search path /tmp (default .) and
  /// the wildcard D*.?pp.  Return wild_card as path to reduce wstring
  /// conversions
  static void split_wildcard(const std::string& path_with_wc, 
			     bfs::path& search_dir, bfs::path& wild_card);

  /// get a valid path to a temporary file/directory in the system tmp
  /// path whose name starts with the passed prefix
  static bfs::path system_tmp_name(const std::string& prefix);

  /// Create a directory, with options for remove or error
  static bool create_directory(const bfs::path& dir_path,
			       short mkdir_option);

  /// Remove a path (file, directory, or symlink) without regard to its
  /// type.  Only error if existed and there's an error in the remove.
  static void recursive_remove(const bfs::path& rm_path);

  /// recursively copy a list of source_paths (files, directories, symlinks),
  /// potentially including wildcards, to destination_path
  static void copy_items(const StringArray& source_paths,
			 const bfs::path& destination_path,
			 bool overwrite) 
  { };

  /// top-level link a list of source_paths (files, directories, symlinks),
  /// potentially including wildcards, from destination_path
  static void link_items(const StringArray& source_paths,
			 const bfs::path& destination_path,
			 bool overwrite);

  /// link from path dest_link to a single path (file, dir, link) in
  /// source directory
  static void link(const bfs::path& src_path, const bfs::path& dest_link,
		   bool overwrite);


private:

  /// Returns the string representing the path to the analysis driver
  static std::string po_which(const std::string& driver_name);

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

/// Helper for "which" - sets complete_filepath from dir_path/file_name combo
inline bool contains(const bfs::path& dir_path, const std::string& file_name,
                     boost::filesystem::path& complete_filepath)
{
  complete_filepath = dir_path; complete_filepath /= file_name;
  return boost::filesystem::is_regular_file(complete_filepath);
}

} // namespace Dakota

#endif // WORKDIR_HELPER_H

