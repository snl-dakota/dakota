/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        WorkdirHelper
//- Description:  Class implementation
//- Owner:        Bill Bohnhoff

#include "WorkdirHelper.hpp"
#include "dakota_data_util.hpp"  // for strcontains
#include "dakota_global_defs.hpp"
#include <boost/array.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>
#include <boost/version.hpp>
#include <cassert>

#if defined(_WIN32) || defined(_WIN64)

  #define NOMINMAX
  #include <io.h>
  #include "dakota_windows.h"
  #define DAK_PATH_ENV_NAME "Path"
  #define DAK_PATH_SEP ';'
  #define DAK_SLASH '\\'

#else
  #include <unistd.h>
  #include <sys/param.h>             // for MAXPATHLEN
  #define DAK_PATH_ENV_NAME "PATH"
  #define DAK_PATH_SEP ':'
  #define DAK_SLASH '/'

#endif // _WIN32 or _WIN64

// BMA TODO: Boost versions <= 1.49 do not include these mappings,
// perhaps due to conflicts with v2?
#if BOOST_VERSION / 100 % 1000 < 50
namespace boost {
  namespace filesystem {
    using filesystem3::copy;
    using filesystem3::copy_directory;
  }
}
#endif


namespace Dakota {

std::string WorkdirHelper::startupPWD          = ".";
std::string WorkdirHelper::startupPATH         = ".";
std::string WorkdirHelper::dakPreferredEnvPath = ".";


/* Portability adapter: return the cwd string in OS-native
   format.  TODO: change paths throughout code to use bfs::path where
   possible, since Windows (and Cygwin) use wchar_t instead of
   char_t. */
std::string get_cwd_str()
{
  // Get the native path and return as a string, using locale-specific
  // conversion from wchar to char if needed.
  bfs::path curr_path;
  try {
    curr_path = boost::filesystem::current_path();
  }
  catch (const bfs::filesystem_error& e) {
    Cerr << "\nError: could not get current directory path;\n       " 
	 << e.what() << std::endl;
    abort_handler(IO_ERROR);
  }
  return curr_path.string();
}


/** Initialize defers calls to Boost filesystem utilities until
    runtime (required on some operating systems. */
void WorkdirHelper::initialize()
{
  startupPWD          = get_cwd_str();
  startupPATH         = init_startup_path();
  dakPreferredEnvPath = init_preferred_env_path();
}


void WorkdirHelper::change_directory(const bfs::path& new_dir)
{
  // TODO: check DAK_MAXPATHLEN? (advice varies on practicality)
  //  bfs::path::string_type new_dir_str = new_dir.native();
  // path::c_str() should return right width string...
  int err_code = -1;
#if defined(_WIN32) || defined(_WIN64)
  err_code = _wchdir(new_dir.c_str());  // path::value_type is wchar on Windows
#else
  err_code = chdir(new_dir.c_str());
#endif
  if (err_code) {
    Cerr << "\nError: failed to change directory to " << new_dir << std::endl;
    abort_handler(-1);
  }
#if defined(DEBUG)
  else
    Cout << "\nEXPECTED: successful change directory to " << new_dir << std::endl;
#endif
}

/** Prepends '.' and the startupPWD to the initial startup $PATH
 *  string so that analysis driver detection is more robust
 */
std::string WorkdirHelper::init_preferred_env_path()
{
  std::string path_sep_string(1, DAK_PATH_SEP);

  std::string preferred_env_path;
  preferred_env_path += "." + path_sep_string + startupPWD + path_sep_string;
  preferred_env_path += startupPATH;

  return preferred_env_path;
}


/** Overwrites $PATH with an additional directory prepended, typically
 *  for the purpose of ensuring templatedir is in the $PATH; updates
 *  cached preferred PATH and environment PATH, so exercise caution
 *  with repeated calls.
 */
void WorkdirHelper::prepend_preferred_env_path(const std::string& extra_path)
{
  // Assume a relative extra_path arg is relative to dakota's startupPWD
  std::string abs_extra_path = bfs::path(extra_path).is_absolute() ? extra_path :
    startupPWD + std::string(1,DAK_SLASH) + extra_path;

  std::string path_sep_string(1, DAK_PATH_SEP);
  dakPreferredEnvPath = abs_extra_path + path_sep_string + dakPreferredEnvPath;

  set_environment(DAK_PATH_ENV_NAME, dakPreferredEnvPath, true);
}


void WorkdirHelper::set_environment(const std::string& env_name, 
				    const std::string& env_val, 
				    bool overwrite)
{
  // revert to legacy implementation if needed
  int err_code = -1;
#if defined(_WIN32) || defined(_WIN64)
  err_code = SetEnvironmentVariable(env_name.c_str(), env_val.c_str()) ? 0 : -1;
#else
  err_code = setenv(env_name.c_str(), env_val.c_str(), overwrite);
#endif
  if (err_code)
    Cout << "\nWarning: set_environment " << env_name << " = " << env_val 
         << "failed."<< std::endl;
}


/** Gets the $PATH (%PATH% on windows) and returns the std::string value
 */
std::string WorkdirHelper::init_startup_path()
{
  char* env_path = std::getenv(DAK_PATH_ENV_NAME);

  if (!env_path) {
    Cerr << "\nERROR: "
         << "getenv(\"" DAK_PATH_ENV_NAME "\") failed in init_startup_path().\n"
         << std::endl;
    abort_handler(-1);
  }

  return std::string(env_path);
}


/** Creates a a vector of directories (as an aid to search) by breaking up
 *  the $PATH environment variable (passed in as a string argument)
 */
std::vector<std::string>
WorkdirHelper::tokenize_env_path(const std::string& env_path)
{
  std::vector<std::string> dirs;

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep( std::string(1, DAK_PATH_SEP).c_str() );

  tokenizer dir_tokens(env_path.begin(), env_path.end(), sep);

  for(tokenizer::iterator tok_iter = dir_tokens.begin();
      tok_iter != dir_tokens.end(); ++tok_iter) {
    std::string dir_path = *tok_iter;
    dirs.push_back(dir_path);

#if defined(DEBUG)
    if( !bfs::is_directory(dir_path) )
      Cout << "Warning - DAKOTA analysis driver resolution detects issue with: "
           << dir_path << " on the environment path.\n\t"
           << "Directory may not exist" << std::endl;
#endif
  }

#if defined(DEBUG)
  Cout << "Search " << env_path << '\n';
  BOOST_FOREACH(const std::string& d, dirs)
    Cout << "\tDir = " << d << '\n';
  Cout << std::endl;
#endif

  return dirs;
}


/** Utility function for executable file search algorithms
 *
 */
std::vector<std::string> get_pathext()
{
  // Get the possible filename extensions from the system environment variable
  // (getenv is not case-sensitive on Windows)
  char* env_ext_str_list = std::getenv("PATHEXT");

  // Create a SringArray of viable extensions for use in executable
  // detection algorithms

  StringArray ext_list;

  if ( env_ext_str_list )
    boost::split( ext_list, env_ext_str_list, boost::is_any_of(";") );

  ext_list.push_back("");  // Backward compatibility:  Some users may
                           // already be explicit and specify filename.ext
  return ext_list;
}


/** Uses string representing $PATH to locate an analysis driver on the host
 *  computer.  Returns the path to the driver (as a string)
 *
 *  This version is a wrapper over the "plain ol' which" implementation,
 *  allowing an array of windows, 3-letter extensions to be checked.
 */
bfs::path WorkdirHelper::which(const std::string& driver_name)
{
  boost::filesystem::path driver_found;

#if defined(_WIN32)

  // TODO: consider skipping this if the user gave an explicit
  // extension (use bfs::path::extension())

  // get list of valid extensions for executables
  StringArray extensions = get_pathext();
  // always check these as a fallback
  extensions.push_back(".com"); extensions.push_back(".exe");

  BOOST_FOREACH(const std::string& e, extensions) {

    // check with extension as given (potentially mixed case), lower, and upper
    // don't bother checking all possible case variants
    std::string driver_name_we = driver_name;

    driver_name_we += e;
    driver_found = po_which(driver_name_we);
    if( !driver_found.empty() ) break;

    driver_name_we = driver_name;
    driver_name_we += boost::algorithm::to_lower_copy(e);
    driver_found = po_which(driver_name_we);
    if( !driver_found.empty() ) break;

    driver_name_we = driver_name;
    driver_name_we += boost::algorithm::to_upper_copy(e);
    driver_found = po_which(driver_name_we);
    if( !driver_found.empty() ) break;

  }
#if defined(DEBUG)
  if( !driver_found.empty() )
    Cout << driver_found << " FOUND when " << e << " appended" << std::endl;
#endif

#else  // _WIN32

  // "Plain ol" POSIX case was reliable, so just call it if NOT native Windows
  driver_found = po_which(driver_name);

#endif  // _WIN32

  return driver_found;
}


StringArray WorkdirHelper::tokenize_driver(const String& user_an_driver)
{
  using boost::escaped_list_separator;
  using boost::tokenizer;

  StringArray driver_and_args;  // to hold tokens of the driver

  // tokenize on whitespace, preserving quoted strings and escapes,
  // so the outermost quoted strings become single command-line args
  escaped_list_separator<char> els("\\", " \t", "\"'");
  tokenizer<escaped_list_separator<char> > tok(user_an_driver, els);
  std::copy(tok.begin(), tok.end(), std::back_inserter(driver_and_args));

  return driver_and_args;
}


bool WorkdirHelper::resolve_driver_path(String& an_driver)
{
  // tokenize to get first entry
  StringArray driver_and_args = WorkdirHelper::tokenize_driver(an_driver);
  if (driver_and_args.empty()) {
    Cerr << "\nError: unexpected empty analysis_driver in resolve_driver_path"
	 << std::endl;
    abort_handler(-1);
  }
  const String& driver0 = driver_and_args[0];

  // see if relative (with ., .., not bare filename)
  String dot_str(".");      
  dot_str += DAK_SLASH;
  String dotdot_str("..");  
  dotdot_str += DAK_SLASH;
  if ( strbegins(driver0, dot_str) || strbegins(driver0, dotdot_str) ) {
    // append startupPWD
    an_driver = rel_to_abs(driver0).string();
    // append the rest of the driver (arguments)
    // TODO: consider boost::algorithm::join to rebuild the string, or
    // just don't fully tokenize it.
    //    user_options = boost::algorithm::join(driver_and_args, " ");
    StringArray::const_iterator daa_it = ++driver_and_args.begin();
    StringArray::const_iterator daa_end = driver_and_args.end();
    for ( ; daa_it != daa_end; ++daa_it)
      an_driver += " " + *daa_it;
    return true;
  }
  // else no conversion to absolute necessary; leave source alone
  return false;
}



/** Utility function for "which"
 *  sets complete_filepath from dir_path/file_name combo
 */
inline bool contains(const bfs::path& dir_path, const std::string& file_name,
                     boost::filesystem::path& complete_filepath)
{
  try {
    complete_filepath = dir_path;
    complete_filepath /= file_name;
    return boost::filesystem::is_regular_file(complete_filepath);
  }
  catch(const bfs::filesystem_error&) {
#if defined(DEBUG)
    Cout << "\nWarning: unable to perform search for analysis driver"
         << " using a directory in $PATH:  " << dir_path << std::endl;
#endif
    return false;
  }
}

/** For absolute driver_name, validates that is regular file.  For
 *  relative, uses string representing $PATH (preferred path) to
 *  locate an analysis driver on the host computer.  Returns the path
 *  to the driver, or empty if not found.
 *
 *  This is the "plain ol' which" impl that worked well, historically, on POSIX.
 */
bfs::path WorkdirHelper::po_which(const std::string& driver_name)
{
  bfs::path driver_path_spec(driver_name);
  bfs::path driver_path_found;

  if( !driver_path_spec.is_absolute() ) {
    //Cout << "RELATIVE path to driver case" << '\n';
    std::vector<std::string> search_dirs =
      tokenize_env_path(dakPreferredEnvPath);

    BOOST_FOREACH(const std::string& d, search_dirs) {
      boost::filesystem::path complete_path_to_driver;
      boost::filesystem::path search_dir_path(d);

      if( contains(search_dir_path, driver_name, complete_path_to_driver) ) {
        //Cout << driver_name << " FOUND in: " << d << "; complete path is: " 
	//     << complete_path_to_driver << std::endl;
        driver_path_found = complete_path_to_driver;
        break;
      }
    }

  }
  else {
    //Cout << "ABSOLUTE path to driver was specified" << std::endl;
    if (bfs::is_regular_file(driver_path_spec))
      driver_path_found = driver_path_spec;
  }

  return driver_path_found;
}


void WorkdirHelper::set_preferred_path()
{
  set_environment(DAK_PATH_ENV_NAME, dakPreferredEnvPath, true);
}


/** If needed, convert the passed item to an absolute path (while
    could make sense to prepend a relative path, no current use cases)
    and prepend when setting environment. Does not update cached
    preferred path.  */
void WorkdirHelper::set_preferred_path(const boost::filesystem::path& extra_path)
{
  boost::filesystem::path abs_extra_path = extra_path.is_absolute() ? 
    extra_path : rel_to_abs(extra_path);

  std::string path_sep_string(1, DAK_PATH_SEP);
  std::string new_preferred_path = 
    abs_extra_path.string() + path_sep_string + dakPreferredEnvPath;

  set_environment(DAK_PATH_ENV_NAME, new_preferred_path, true);
}


void WorkdirHelper::reset()
{
  WorkdirHelper::change_directory(startupPWD);
  set_preferred_path();
}


/** Input: path_with_wc; Output: search_dir, wild_card */
void WorkdirHelper::split_wildcard(const std::string& path_with_wc, 
				   bfs::path& search_dir, 
				   bfs::path& wild_card)
{
  // could coerce string to path in passing args...
  bfs::path fq_search(path_with_wc);

  // TODO: better way to do this (trailing slash yields filename of .)
  // want to allow matching /tmp/foo/ as /tmp with entry foo so foo
  // gets copied or linked

  // BUT if fq_search is a symlink, perhaps want to resolve it...
  // In Posix, trailing slash on symlink means follow it

  if ( boost::equals( (--fq_search.end())->native(), ".") )
    fq_search.remove_filename();
  
  search_dir = fq_search.parent_path();

  // could instead consider the Dakota runtime PWD
  if (search_dir.empty())
    search_dir = ".";

  // might we need wstring on Windows?
  wild_card = fq_search.filename();

}


bfs::path WorkdirHelper::system_tmp_file(const std::string& prefix)
{
  bfs::path temp_filename;
  try {
    // generate an 8 hex character unique name
    std::string temp_name_pattern(prefix + "_%%%%%%%%");
    temp_filename = bfs::unique_path(temp_name_pattern);
  }
  catch (const bfs::filesystem_error& e) {
    Cerr << "\nError: could not generate temporary filename with prefix "
	 << prefix << ";\n       " << e.what() << std::endl;
    abort_handler(IO_ERROR);
  }
  return temp_filename;

// OLD RATIONALE:
//#ifdef LINUX
      /*
      // mkstemp generates a unique filename using tmp_str _and_ creates the
      // file.  This prevents potential issues with race conditions, but has
      // the undesirable feature of requiring additional file deletions in the
      // case of secondary tagging for multiple analysis drivers.
      char tmp_str[20] = "/tmp/dakvars_XXXXXX";
      mkstemp(tmp_str); // replaces XXXXXX with unique id
      paramsFileName = tmp_str;
      */
//#else
      // tmpnam generates a unique filename that does not exist at the time of
      // invocation, but does not create the file.  It is more dangerous than
      // mkstemp, since multiple applications could potentially generate the
      // same temp filename prior to creating the file.
      //paramsFileName = std::tmpnam(NULL);
//#endif
// END RATIONALE
}


bfs::path WorkdirHelper::system_tmp_path()
{
  // TODO: tmp files in . or /tmp?   Could offer option.
  bfs::path temp_directory;
  try {
    temp_directory = bfs::temp_directory_path();
  }
  catch (const bfs::filesystem_error& e) {
    Cerr << "\nError: could not determine temporary directory path;\n       " 
	 << e.what() << std::endl;
    abort_handler(IO_ERROR);
  }
  return temp_directory;
}


/** mkdir_option is DIR_CLEAN (remove and recreate), DIR_PERSIST
    (leave existing), or DIR_ERROR (don't allow existing) returns
    whether a new directory was created. */
bool WorkdirHelper::create_directory(const bfs::path& dir_path,
				     short mkdir_option)
{
  bool dir_created = false;  // whether this function created a new directory

  // remove any existing directory if requested, error since we test if exists
  if (mkdir_option == DIR_CLEAN && bfs::exists(dir_path))
    recursive_remove(dir_path, FILEOP_ERROR);

  try {
    // now conditionally create a new one
    if (bfs::exists(dir_path)) {

      if (mkdir_option == DIR_ERROR) {
	// DIR_ERROR or failure in removal
	Cerr << "\nError: Directory " << dir_path << " exists (disallowed).\n"
	     << std::endl;
	abort_handler(-1);
      }

      // DIR_PERSIST case
      if (!bfs::is_directory(dir_path)) {
	Cerr << "\nError: Directory " << dir_path << " exists (permitted), but "
	     << "is not a directory." << std::endl;
	abort_handler(-1);
      }

      // check permissions; syntax requires Boost 1.49 or newer
      bfs::perms dir_perms = bfs::status(dir_path).permissions();
      // TODO: verify owner_all on Windows?
      //      if ( !(dir_perms & owner_all) ) {
      if ( !(dir_perms & bfs::owner_write) ) {
	// BMA: Make this a warning until we get permission checking fixed
	Cout << "\nWarning: Directory " << dir_path << " exists (permitted), but "
	     << "not writable." << std::endl;
      }

    }
    else {

      try {
	// directory does not exist, create recursively (as if mkdir -p, we hope)
	bfs::create_directories(dir_path);
	dir_created = true;

	// Shouldn't need this as create_directory is probably already more
	// permissive than we want...

	// make sure new directory has at least rwx (might also have s)
	// TODO: make sure any intermediate paths have right permissions
	//bfs::permissions(dir_path, bfs::add_perms | bfs::owner_all);
      }
      catch (const bfs::filesystem_error& e) {
	Cerr << "\nError: could not create directory " << dir_path 
	     << ";\n       " << e.what() << std::endl;
	abort_handler(IO_ERROR);
      }
    }
  }
  catch  (const bfs::filesystem_error& e) {
    Cerr << "\nError: could not get status of directory " << dir_path 
	 << ";\n       " << e.what() << std::endl;
    abort_handler(IO_ERROR);
  }

  return dir_created;
}


void WorkdirHelper::recursive_remove(const bfs::path& rm_path, short fileop_opt)
{
  try {
    if (bfs::exists(rm_path))
      bfs::remove_all(rm_path);
    else {
      if (fileop_opt == FILEOP_WARN) {
	Cerr << "\nWarning: path " << rm_path << " to remove does not exist." 
	     << std::endl;
      }
      else if (fileop_opt == FILEOP_ERROR) {
	Cerr << "\nError: path " << rm_path << " to remove does not exist." 
	     << std::endl;
	abort_handler(IO_ERROR);
      }
    }
  }
  catch (const bfs::filesystem_error& e) {
    if (fileop_opt == FILEOP_WARN) {
      Cerr << "\nWarning: could not remove path " << rm_path << ";\n" 
	   << e.what() << std::endl;
    }
    else if (fileop_opt == FILEOP_ERROR) {
      Cerr << "\nError: could not remove path " << rm_path << ";\n" 
	   << e.what() << std::endl;
      abort_handler(IO_ERROR);
    }
  }
}


void WorkdirHelper::rename(const bfs::path& old_path, const bfs::path& new_path,
			   short fileop_opt)
{
  try {
    if (bfs::exists(old_path))
      bfs::rename(old_path, new_path);
    else {
      if (fileop_opt == FILEOP_WARN) {
	Cerr << "\nWarning: path " << old_path << " to rename does not exist." 
	     << std::endl;
      }
      else if (fileop_opt == FILEOP_ERROR) {
	Cerr << "\nError: path " << old_path << " to rename does not exist." 
	     << std::endl;
	abort_handler(IO_ERROR);
      }
    }
  }
  catch (const bfs::filesystem_error& e) {
    if (fileop_opt == FILEOP_WARN) {
      Cerr << "\nWarning: could not rename path " << old_path << " to "
	   << new_path << ";\n" << e.what() << std::endl;
    }
    else if (fileop_opt == FILEOP_ERROR) {
      Cerr << "\nError: could not rename path " << old_path << " to "
	   << new_path << ";\n" << e.what() << std::endl;
      abort_handler(IO_ERROR);
    }
  }
}


/** Iterator implementation for copy, link, etc file operation.
    Iterate source items (paths or wildcards), performing file_op on
    each w.r.t. destination.  If overwrite, remove and replace any
    existing destination target (at top-level), otherwise, allow to
    persist.  Return code true indicates abnormal behavior. */
bool WorkdirHelper::file_op_items(const file_op_function& file_op, 
				  const StringArray& source_items,
				  const bfs::path& dest_dir,
				  bool overwrite) 
{
  bool return_code = false;

  // iterate the items and link each entry or expanded wildcard
  StringArray::const_iterator src_it  = source_items.begin();
  StringArray::const_iterator src_end = source_items.end();
  for( ; src_it != src_end; ++src_it) {

    // TODO: more efficient
    //    if ( contains_wildcard(*src_it) ) {
    if ( strcontains(*src_it, "*") || strcontains(*src_it, "?") ) {
      //   iterate paths matching the wildcard;
      std::string glob_string = *src_it;
      bfs::path root_dir;
      bfs::path wild_card;
      WorkdirHelper::split_wildcard(glob_string, root_dir, wild_card);

      MatchesWC wc_predicate(wild_card);
      glob_iterator fit(wc_predicate, bfs::directory_iterator(root_dir));
      glob_iterator fitend(wc_predicate, bfs::directory_iterator());
      for ( ; fit != fitend; ++fit) {
        bfs::path src_path = fit->path();
        if (bfs::exists(src_path)) {
	  bfs::path src_filename = src_path.filename();
	  if (file_op(src_path, dest_dir, overwrite))
	    return_code = true;
        }
        else {
	  Cout << "Warning: path " << src_path << " specified to link/copy "
	       << "does not exist. Skipping." << std::endl;
        }
      }
    }
    else {
      // perform file_op directly on the path;
      bfs::path src_path(*src_it);
      if (bfs::exists(src_path)) {
	bfs::path src_filename = src_path.filename();
	if (file_op(src_path, dest_dir, overwrite))
	  return_code = true;
      }
      else {
	Cout << "Warning: path " << src_path << " specified to link/copy "
	     << "does not exist. Skipping." << std::endl;
      }
    }

  } // for each item

  return return_code;
}


/** Iterate source items (paths or wildcards), linking each of them
    from the destination.  If overwrite, remove and replace any
    existing destination target, otherwise, allow to persist */
void WorkdirHelper::link_items(const StringArray& source_items,
			       const bfs::path& dest_dir,
			       bool overwrite) 
{
  file_op_function file_op = &WorkdirHelper::link;
  file_op_items(file_op, source_items, dest_dir, false);
}


/** Iterate source items (paths or wildcards), copying each of them
    into the destination.  If overwrite, remove and replace any
    existing destination target, otherwise, allow to persist */
void WorkdirHelper::copy_items(const StringArray& source_items,
			       const bfs::path& dest_dir,
			       bool overwrite) 
{
  file_op_function file_op = &WorkdirHelper::recursive_copy;
  file_op_items(file_op, source_items, dest_dir, false);
}


// no longer being used...
void WorkdirHelper::prepend_path_items(const StringArray& source_items)
{
  file_op_function file_op = &WorkdirHelper::prepend_path_item;
  bfs::path dummy_path;
  file_op_items(file_op, source_items, dummy_path, false);
}


bool WorkdirHelper::check_equivalent_dest(const StringArray& source_items,
					  const bfs::path& dest_dir)
{
  file_op_function file_op = &WorkdirHelper::check_equivalent;
  return file_op_items(file_op, source_items, dest_dir, false);
}

bool WorkdirHelper::find_driver(const StringArray& source_items,
				const bfs::path& search_driver)
{
  // BMA TODO: file_op_items should short-circuit when found and not be verbose...
  file_op_function file_op = &WorkdirHelper::find_file;
  return file_op_items(file_op, source_items, search_driver, false);
}


/** Assumes source file exists since it was iterated in the calling
    context. If overwrite, any existing file in dest_dir will be
    removed prior to creating the new link. */
bool WorkdirHelper::link(const bfs::path& src_path, const bfs::path& dest_dir,
			 bool overwrite)
{
  // symlink facilities require a qualifed source and destination
  // name, be absolute for now
  bfs::path dest_link = dest_dir / src_path.filename();

  try {

    // when relative, assume relative to current path
    bfs::path fq_src_path(src_path);
    if (src_path.is_relative())
      fq_src_path = bfs::current_path() / src_path;

    if (overwrite && bfs::exists(dest_link))
      bfs::remove_all(dest_link);

    // now, only make the link if the dest doesn't exist
    if (!bfs::exists(dest_link)) {
      if (bfs::is_directory(fq_src_path))
	bfs::create_directory_symlink(fq_src_path, dest_link);
      else
	bfs::create_symlink(fq_src_path, dest_link);
    }

  }
  catch (const bfs::filesystem_error& e) {
    Cerr << "\nError: could not create symlink from " << dest_dir 
	 << " to " << src_path << ";\n       " << e.what() << std::endl;
    abort_handler(IO_ERROR);
  }

  return false;
}


/// note dest_dir is the containing folder for the src_path contents
/// to be placed in for consistency with other convenience functions
/// (may need to reconsider)
bool WorkdirHelper::recursive_copy(const bfs::path& src_path, 
				   const bfs::path& dest_dir, bool overwrite)
{
  try {
    // precondition: dest exists and is a dir
    if (!bfs::exists(dest_dir) || !bfs::is_directory(dest_dir)) {
      Cerr << "\nError: destination directory " << dest_dir 
	   << " must exist for recursive_copy." << std::endl;
      abort_handler(IO_ERROR);
    }

    bfs::path dest_path = dest_dir / src_path.filename();
  
    // TODO: gentler overwrite of contents, not top-level paths
    if (overwrite && bfs::exists(dest_path))
      bfs::remove_all(dest_path);

    if (!bfs::exists(dest_path)) {

      // non-recursive copy of file or directory or symlink into dest
      bfs::copy(src_path, dest_path);
      //bfs::create_directory(dest_path);
      //bfs::copy_directory(src_path, dest_path);

      if (bfs::is_directory(src_path)) {
	bfs::directory_iterator dir_it(src_path);
	bfs::directory_iterator dir_end;
	for ( ; dir_it != dir_end; ++dir_it) {
	  bfs::path src_item(dir_it->path());
	  recursive_copy(src_item, dest_path, overwrite);
	}
      }
    }

  }
  catch (const bfs::filesystem_error& e) {
    Cerr << "\nError: could not recursive copy " << src_path 
	 << " to " << dest_dir << ";\n       " << e.what() << std::endl;
    abort_handler(IO_ERROR);
  }
  return false;
}


/// prepend the env path with source path if it's a directory or
/// directory symlink
bool WorkdirHelper::prepend_path_item(const bfs::path& src_path, 
				      const bfs::path& dest_dir, bool overwrite)
{
  // BMA TODO: this should use wstring vs. string...
  // Change once we upgrade preferred path to a set of BFS paths
  try {
    if (bfs::is_directory(src_path))
      prepend_preferred_env_path(src_path.string());
  }
  catch (const bfs::filesystem_error& e) {
    Cerr << "\nError: could not prepend PATH with " << src_path 
	 << ";\n       " << e.what() << std::endl;
    abort_handler(IO_ERROR);
  }

  return false;
}


bool WorkdirHelper::check_equivalent(const bfs::path& src_path, 
				     const bfs::path& dest_dir, bool overwrite)
{
  try {
    if (bfs::equivalent(src_path, dest_dir)) {
      Cerr << "Error: specified link/copy_file " << src_path << "\n"
	   << "       is same as work_directory " << dest_dir << "." 
	   << std::endl;
      return true;
    }
  }
  catch (const bfs::filesystem_error& e) {
    Cerr << "\nError: could not determine equivalence of paths " << src_path 
	 << " and " << dest_dir << ";\n       " << e.what() << std::endl;
    abort_handler(IO_ERROR);
  }

  return false;
}

bool WorkdirHelper::find_file(const bfs::path& src_path, 
			      const bfs::path& search_file, bool overwrite)
{
  try {
    if ( bfs::is_regular_file(src_path) && 
	 src_path.filename() == search_file.filename())
      return true;
  }
  catch (const bfs::filesystem_error& e) {
    Cerr << "\nError: could not determine equivalence of files " << src_path 
	 << " and " << search_file << ";\n       " << e.what() << std::endl;
    abort_handler(IO_ERROR);
  }
  return false;
}

// TODO: Boost 1.50 and newer support concat (+=) on paths, remove
// this function when we allow that version
bfs::path WorkdirHelper::concat_path(const bfs::path& p_in, const String& tag)
{
  // explicit template argument to avoid conversion ambiguity
  bfs::path::string_type p_in_str = p_in.generic_string<bfs::path::string_type>();
  // may need to convert from string to wstring
  bfs::path::string_type tag_str(tag.begin(), tag.end());
  return (p_in_str + tag_str);
}


} // namespace Dakota
