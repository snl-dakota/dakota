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

#include <boost/function.hpp>

/// BEGIN: Section to be cleaned up and hopefully moved to .cpp file
/// after dakota_filesystem_utils is retired
#if defined(_WIN32) || defined(_WIN64)

  #define NOMINMAX
  #include <io.h>
  #include "dakota_windows.h"
  #include <direct.h>               // remove when _mkdir no longer needed
  // consider using _chdir if appropriate, since has same API?
  #define DAK_CHDIR(s) (SetCurrentDirectory(s) == 0)
//  #define DAK_MKDIR(a,b) _mkdir(a)  // to be removed
  #define DAK_MAXPATHLEN MAX_PATH
  #define DAK_PATH_ENV_NAME "Path"
  #define DAK_PATH_SEP ';'
  #define DAK_SLASH '\\'            // likely not needed with BFS

#elif defined(HAVE_UNISTD_H) 
  // probably not necessary to tie conditional compilation to build 
  //#if defined(HAVE_UNISTD_H) 
  #include <unistd.h>
  //#endif
  #include <sys/param.h>             // for MAXPATHLEN?
  #define DAK_CHDIR chdir
//  #define DAK_MKDIR(a,b) mkdir(a,b)  // to be removed
  #define DAK_MAXPATHLEN MAXPATHLEN
  #define DAK_PATH_ENV_NAME "PATH"
  #define DAK_PATH_SEP ':'
  #define DAK_SLASH '/'              // likely not needed with BFS

#endif // _WIN32 or _WIN64

#include <sys/stat.h>
//#include <string>
//#include <utility>
//#include <vector>

/// END: Section to be cleaned up and hopefully moved to .cpp file


namespace Dakota {

/// define directory creation options
enum { DIR_CLEAN, DIR_PERSIST, DIR_ERROR };


// define a function type that maps src to dest with option to overwrite
typedef 
boost::function<bool (const bfs::path& src_path, 
		      const bfs::path& dest_path, bool overwrite)> 
file_op_function;


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

  /// Treat get_npath as a legacy, "blackbox" utility
#ifdef DEBUG_GET_NPATH
  /** get_npath "shuffles" the string representing the current $PATH variable
      definition so that '.' is first in the $PATH.  It then returns the new
      string as the result (last arg in the call). */
  friend void get_npath(int,std::string*);         // dakPreferredEnvPath access
#endif

#ifdef DEBUG_LEGACY_WORKDIR
  /// Treat my_cp as a legacy, "blackbox" utility
  /** my_cp is a wrapper around 'cp -r'.  The extra layer allows for symlink
      to be used instead of file copy. */
  friend int my_cp(const char*, const struct stat*,
                   int, int, void*);               // calls symlink
#endif

public:

  /// initialize paths in the workdir helper at runtime
  static void initialize();

  /// Query for dakota's startup $PWD
  static const std::string& startup_pwd() { return startupPWD; }

  /// change current directory 
  static void change_directory(const bfs::path& new_dir);

  /// Prepend $PATH environment variable with an extra_path
  static void prepend_preferred_env_path(const std::string& extra_path);

  /// Set an environment variable
  static void set_environment(const std::string& env_name, 
			      const std::string& env_val, 
			      bool overwrite_flag = true);

  /// Returns the string representing the path to the analysis driver,
  //  including typical windows extensions
  static std::string which(const std::string& driver_name);

  /// given a string with an optional path and a wildcard, e.g.,
  /// /tmp/D*.?pp, parse it into the search path /tmp (default .) and
  /// the wildcard D*.?pp.  Return wild_card as path to reduce wstring
  /// conversions
  static void split_wildcard(const std::string& path_with_wc, 
			     bfs::path& search_dir, bfs::path& wild_card);


  /// concatenate a string onto the end of a path
  static bfs::path concat_path(const bfs::path& p_in, const String& tag);


  /// get a valid path to a temporary file/directory in the system tmp
  /// path whose name starts with the passed prefix
  static bfs::path system_tmp_name(const std::string& prefix);

  /// Create a directory, with options for remove or error
  static bool create_directory(const bfs::path& dir_path,
			       short mkdir_option);

  /// Remove a path (file, directory, or symlink) without regard to its
  /// type.  Only error if existed and there's an error in the remove.
  static void recursive_remove(const bfs::path& rm_path);


  // Convenience functions which invoke file_op_items() with various kernels

  /// top-level link a list of source_paths (files, directories,
  /// symlinks), potentially including wildcards, from
  /// destination_dir, which must exist
  static void link_items(const StringArray& source_itemss,
			 const bfs::path& dest_dir,
			 bool overwrite);

  /// copy a list of source_paths (files, directories, symlinks),
  /// potentially including wildcards into destination_dir, which must
  /// exist
  static void copy_items(const StringArray& source_items,
			 const bfs::path& dest_dir,
			 bool overwrite);

  /// prepend any directories (including wildcards) found in
  /// source_items to the preferred environment path
  static void prepend_path_items(const StringArray& source_items);

  /// check whether any of the passed source items are filesystem
  /// equivalent to the destination path, return true if any one is
  /// equivalent to dest
  static bool check_equivalent_dest(const StringArray& source_items,
				    const bfs::path& dest_dir);


  /// check whether the any of the passed source items (possibly
  /// including wildcards to be expanded) matches the passed search driver
  static bool find_driver(const StringArray& source_items,
			  const bfs::path& search_driver);

  // Kernels for file operations to apply when iterating directory
  // entries or wildcards.  All must be of type file_op_function.

  /// create link from dest_dir/src_path.filename() to a single path
  /// (file, dir, link) in source directory
  static bool link(const bfs::path& src_path,
		   const bfs::path& dest_dir, bool overwrite);
 
  /// Recrusive copy of src_path into dest_dir, with optional
  /// top-level overwrite (remove/recreate) of
  /// dest_dir/src_path.filename()
  static bool recursive_copy(const bfs::path& src_path, 
			     const bfs::path& dest_dir, bool overwrite);
 
  /// prepend the env path with source path if it's a directory
  static bool prepend_path_item(const bfs::path& src_path, 
				const bfs::path& dest_dir, bool overwrite);

  /// return true if the src and dest are filesystem equivalent
  static bool check_equivalent(const bfs::path& src_path, 
			       const bfs::path& dest_dir, bool overwrite);

  /// return true if the src_path is a regular file and has same
  /// filename as search_file
  static bool find_file(const bfs::path& src_path, 
			const bfs::path& search_file, bool overwrite);

  /// recursively perform file_op (copy, path adjust, etc.) on a list
  /// of source_paths (files, directories, symlinks), which
  /// potentially include wildcards, w.r.t. destination_dir
  static bool file_op_items(const file_op_function& file_op,
			    const StringArray& source_paths,
			    const bfs::path& dest_dir,
			    bool overwrite);
  ///  Resets the working directory "state" to its initial state when
  ///  DAKOTA  was launched
  static void reset();

private:

  /// Returns the string representing the path to the analysis driver
  static std::string po_which(const std::string& driver_name);

  /// Initializes class member, startupPATH
  static std::string init_startup_path();

  /// Initializes class member, dakPreferredEnvPath
  static std::string init_preferred_env_path();

  /// Tokenizes $PATH environment variable into a "list" of directories
  static std::vector<std::string> tokenize_env_path(const std::string& path);

#ifdef DEBUG_LEGACY_WORKDIR
  /// "Helper"/wrapper for system symlink when "from" needs path adjustment
  static int symlink(const char* from, const char* to);

  static void change_cwd(const std::string& wd_str);  // old nm: workdir_adjust

  static const char** arg_adjust(bool cmd_line_args,
                                 const std::vector<std::string>& args,
                                 const char **av, const std::string& workdir);
#endif

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


#ifdef DEBUG_LEGACY_WORKDIR
/// Utility function from legacy, "not_executable" module -- DO NOT TOUCH!
const char** arg_list_adjust(const char **, void **);
#endif

/// Helper for "which" - sets complete_filepath from dir_path/file_name combo
inline bool contains(const bfs::path& dir_path, const std::string& file_name,
                     boost::filesystem::path& complete_filepath)
{
  complete_filepath = dir_path; complete_filepath /= file_name;
  return boost::filesystem::is_regular_file(complete_filepath);
}

} // namespace Dakota

#endif // WORKDIR_HELPER_H

