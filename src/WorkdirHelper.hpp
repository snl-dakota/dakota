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

#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <boost/iterator/filter_iterator.hpp>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
namespace bfs = boost::filesystem;

#include <boost/function.hpp>


namespace Dakota {

/// define directory creation options
enum { DIR_CLEAN, DIR_PERSIST, DIR_ERROR };

/// enum indicating action on failed file operation
enum {FILEOP_SILENT, FILEOP_WARN, FILEOP_ERROR};

/// define a function type that operates from src to dest, with option
/// to overwrite
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


/* RATIONALE on path management: only update PATH when needed, for
   example, adjust working directory, PATH, and other environment
   variables when starting an external process, reset when done. */

/** Utility class for cross-platform management of environment and
    paths.  Including directory and file operations.  On
    initialization, this class does not manipulate the present working
    directory, nor the PATH environment variable, but stores context
    to manipulate them later. */
class WorkdirHelper
{
public:

  /// initialize (at runtime) cached values for paths and environment
  static void initialize();

  /// Query for dakota's startup $PWD
  static const std::string& startup_pwd() { return startupPWD; }

  /// change current directory 
  static void change_directory(const bfs::path& new_dir);

  /// Prepend cached preferredEnvPath with extra_path and update $PATH
  /// environment variable
  static void prepend_preferred_env_path(const std::string& extra_path);

  /// Set an environment variable
  static void set_environment(const std::string& env_name, 
			      const std::string& env_val, 
			      bool overwrite_flag = true);

  /// Returns the bfs::path for the analysis driver, supporting
  /// typical windows filename extensions, or empty if not found
  static bfs::path which(const std::string& driver_name);

  /// get a valid absolute bfs::path to a subdirectory relative to rundir
  static bfs::path rel_to_abs(const bfs::path& subdir_path)
  { return ( startupPWD / subdir_path ); }

  /// tokenize a white-space separated analysis driver, respecting
  /// escapes and nested quotes
  static StringArray tokenize_driver(const String& user_an_driver);

  /// parse off the first whitespace-separated entry in the user's
  /// analysis_driver, and convert it to an absolute path if it begins
  /// with ./ or ../, replacing the passed string if needed.  Returns
  /// true if the first token was modified.
  static bool resolve_driver_path(String& an_driver);

  /// given a string with an optional path and a wildcard, e.g.,
  /// /tmp/D*.?pp, parse it into the search path /tmp (default .) and
  /// the wildcard D*.?pp.  Return wild_card as path to reduce wstring
  /// conversions
  static void split_wildcard(const std::string& path_with_wc, 
			     bfs::path& search_dir, bfs::path& wild_card);


  /// concatenate a string onto the end of a path
  static bfs::path concat_path(const bfs::path& p_in, const String& tag);

  /// generate a valid temporary file name <prefix>_%%%%%%%% 
  static bfs::path system_tmp_file(const std::string& prefix);

  /// get the system tmp path, e.g., /tmp or C:\temp
  static bfs::path system_tmp_path();

  /// Create a directory, with options for remove or error
  static bool create_directory(const bfs::path& dir_path,
			       short mkdir_option);

  /// Remove a path (file, directory, or symlink) without regard to its
  /// type.  Only error if existed and there's an error in the remove.
  static void recursive_remove(const bfs::path& rm_path, short fileop_option);

  /// Rename a file, catching any errors and optionally warning/erroring.
  static void rename(const bfs::path& old_path, const bfs::path& new_path,
		     short fileop_option);

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
  /// source_items to the preferred environment path; this will update
  /// cached preferred path and PATH
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
 
  /// prepend the preferred env path with source path if it's a
  /// directory; this will update cached preferred path and manipulate
  /// PATH
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

  /// set/reset PATH to dakPreferredEnvPath
  static void set_preferred_path();

  /// set PATH to absolute(extra_path):dakPreferredEnvPath, without
  /// changing cached preferred PATH
  static void set_preferred_path(const boost::filesystem::path& extra_path);

  ///  Resets the working directory "state" to its initial state when
  ///  DAKOTA  was launched
  static void reset();

private:

  /// Returns the bfs::path for the analysis driver - POSIX-style
  /// implementation, returns empty if not found
  static bfs::path po_which(const std::string& driver_name);

  /// Initializes class member, startupPATH
  static std::string init_startup_path();

  /// Initializes class member, dakPreferredEnvPath
  static std::string init_preferred_env_path();

  /// Tokenizes $PATH environment variable into a "list" of directories
  static std::vector<std::string> tokenize_env_path(const std::string& path);

  //
  //- Heading: Data
  //

  /// Value of $PWD var upon entry to dakota main()
  static std::string startupPWD;

  /// Value of $PATH (%PATH% on windows) var upon entry to dakota
  /// main(), omitting any leading PATH= or Path=
  static std::string startupPATH;

  /// Dakota preferred search PATH/Path = ".:startupPWD:startupPATH",
  /// omitting any leading PATH= or Path=
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

} // namespace Dakota

#endif // WORKDIR_HELPER_H

