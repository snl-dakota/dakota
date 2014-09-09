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
#include "dakota_filesystem_utils.hpp" // undesirable dependency for workdir_adjust
                              // WJB: wd_adjust has a MAJOR dependency on statics in dakota_filesystem_utils.cpp
#include <boost/array.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>
#include <boost/version.hpp>
#include <cassert>

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


/// Initialize defers calls to Boost filesystem utilities until runtime
void WorkdirHelper::initialize()
{
  startupPWD          = get_cwd();
  startupPATH         = init_startup_path();
  dakPreferredEnvPath = init_preferred_env_path();
}


void WorkdirHelper::change_directory(const bfs::path& new_dir)
{
  // TODO: check DAK_MAXPATHLEN? (advice varies on practicality)
  //  bfs::path::string_type new_dir_str = new_dir.native();
  // path::c_str() should return right width string...
  int retcode = -1;
#if defined(_WIN32) || defined(_WIN64)
  retcode = _wchdir(new_dir.c_str());  // path::value_type is wchar on Windows
#else
  retcode = chdir(new_dir.c_str());
#endif
  if (retcode) {
    Cerr << "\nError: failed to change directory to " << new_dir << std::endl;
    abort_handler(-1);
  }
}

/** Creates a "PATH=.:$PWD:PATH" string so that analysis driver
 *  detection is (hopefully) more robust
 */
std::string WorkdirHelper::init_preferred_env_path()
{
  std::string path_sep_string(1, DAK_PATH_SEP);
  std::string preferred_env_path(DAK_PATH_ENV_NAME"=");

  // Go ahead and prepend '.' to the preferred $PATH since it can't hurt

  preferred_env_path += "." + path_sep_string + get_cwd() + path_sep_string;
  preferred_env_path += init_startup_path();

  return preferred_env_path;
}


/** Overwrites $PATH with an additional directory prepended,
 *  typically for the purpose of ensuring templatedir is in the $PATH
 */
void WorkdirHelper::prepend_preferred_env_path(const std::string& extra_path)
{
  // Assume a relative extra_path arg is relative to dakota's startupPWD

  std::string abs_extra_path = !bfs::path(extra_path).is_absolute() ?
                               startupPWD+std::string(1,DAK_SLASH)+extra_path :
                               extra_path;

  std::string preferred_env_path(DAK_PATH_ENV_NAME"=");
  std::string old_preferred_path
    = dakPreferredEnvPath.substr(preferred_env_path.size());

  preferred_env_path += abs_extra_path + std::string(1, DAK_PATH_SEP)
                      + old_preferred_path;
  dakPreferredEnvPath = preferred_env_path;

  putenv_impl(dakPreferredEnvPath.c_str());
}


void WorkdirHelper::set_environment(const std::string& env_name, 
				    const std::string& env_val, 
				    bool overwrite)
{
  // revert to legacy implementation if needed
  //std::string putenv_str(env_name + '=' + env_val);
  //putenv_impl(putenv_str.c_str());
  int retcode = -1;
#if defined(_WIN32) || defined(_WIN64)
  retcode = SetEnvironmentVariable(env_name.c_str(), env_val.c_str()) ? 0 : -1;
#else
  retcode = setenv(env_name.c_str(), env_val.c_str(), overwrite);
#endif
  if (retcode)
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

  tokenizer dir_tokens( env_path.begin() + 5,
                        env_path.end(), sep ); // account for "PATH=" @beginning

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


/** Uses string representing $PATH to locate an analysis driver on the host
 *  computer.  Returns the path to the driver (as a string)
 *
 *  This version is a wrapper over the "plain old which" implementation,
 *  allowing an array of windows, 3-letter extensions to be checked.
 */
std::string WorkdirHelper::which(const std::string& driver_name)
{
  StringArray extensions = get_pathext(); // expect empty vector on posix

#if defined(_WIN32)
  extensions.push_back(".com"); extensions.push_back(".exe");
  
  std::string driver_path;

  BOOST_FOREACH(const std::string& e, extensions) {

    // check with extension as given (potentially mixed case), lower, and upper
    std::string driver_name_we = driver_name;
    driver_name_we += e;
    driver_path = po_which(driver_name_we);

    driver_name_we = driver_name;
    driver_name_we += boost::algorithm::to_lower_copy(e);
    driver_path = po_which(driver_name_we);

    driver_name_we = driver_name;
    driver_name_we += boost::algorithm::to_upper_copy(e);
    driver_path = po_which(driver_name_we);

    if( !driver_path.empty() ) {
#if defined(DEBUG)
      Cout << driver_path << " FOUND when " << e << " appended" << std::endl;
#endif
      break;
    }
  }

  return driver_path;

#else

  // "Plain ol" UNIX case was reliable, so just call it if NOT native Windows
  return po_which(driver_name);
#endif // _WIN32
}

/** Uses string representing $PATH to locate an analysis driver on the host
 *  computer.  Returns the path to the driver (as a string)
 *
 *  This is the "plain old which" impl that worked well, historically, on unix.
 */
std::string WorkdirHelper::po_which(const std::string& driver_name)
{
  std::string driver_path_str;
  bfs::path driver_path(driver_name);

  if( !driver_path.is_absolute() ) {
    //Cout << "RELATIVE path to driver case" << '\n';
    std::vector<std::string> search_dirs =
      tokenize_env_path(dakPreferredEnvPath);

    BOOST_FOREACH(const std::string& d, search_dirs) {
      boost::filesystem::path f;
      boost::filesystem::path d_path(d);

      if( contains(d_path, driver_name, f) ) {
        //Cout << driver_name << " FOUND in: " << d << std::endl;
        driver_path_str = f.string();
        break;
      }
    }

  }
  else {
    //Cout << "ABSOLUTE path to driver was specified" << std::endl;
    if (bfs::is_regular_file(driver_path))
      driver_path_str = driver_name;
  }

  return driver_path_str;
}


void WorkdirHelper::set_preferred_path()
{
  putenv_impl(dakPreferredEnvPath.c_str());
}


void WorkdirHelper::reset()
{
  if (DAK_CHDIR( startupPWD.c_str() )) {
    Cerr << "\nError: chdir(" << startupPWD
         << ") failed in workdir_reset()" << std::endl;
    abort_handler(-1);
  }
  set_preferred_path();
}


#ifdef DEBUG_LEGACY_WORKDIR

/** Change directory to workdir and make necessary adjustments
    to $PATH (and drive for Windows platform) */
void WorkdirHelper::change_cwd(const std::string& workdir)
{
  // Potential "adapter layer" to manager 3 different APIs
  //       1. DMG version,  2. BoostFS V2, and  3. BoostFS V3
  //
  // (although NOT convinced BoostFS provides the necessary features)

  workdir_adjust(workdir);
}


/* WJB - ToDo: debug later.. 
int WorkdirHelper::symlink(const char* from, const char* to)
{
  std::string prefix_from("/");
  prefix_from += from;
  std::string adjusted_from( (*from != '/') ?
                             WorkdirHelper::startupPWD + prefix_from : from );

  Cout << "Symlink-newFrom: " << adjusted_from << '\n' << std::endl;
  return symlink(adjusted_from.c_str(), to);
}
*/

/* The following routines assume ASCII or UTF-encoded Unicode. */

void find_env_token(const char *s0, const char **s1,
                    const char **s2, const char **s3)
{
  boost::array<char, DAK_MAXPATHLEN> ebuf;
  const char *t;
  const unsigned char *s;
  int q;
  size_t i, n;

top:
  for(s = (const unsigned char*)s0; *s <= ' '; ++s)
    if (!*s) {
      *s1 = *s2 = *s3 = (const char*)s;
      return;
    }

  *s1 = t = (const char*)s;
  if ((q = *s) == '"' || q == '\'') {
    for(*s1 = (const char*)++s; *s != q; ++s) {
      if (!*s) {
        *s2 = *s3 = (const char*)s;
        return;
      }
    }

    *s2 = (const char*)s;
    *s3 = (const char*)s + 1;
    return;
  }

  while((q = *++s) > ' ' && q != '"' && q != '\'' && q != '$');

  *s2 = *s3 = (const char*)s;
  if (*t == '$' && (n = *s2 - t) <= DAK_MAXPATHLEN) {
    --n;
    for(i = 0; i < n; ++i)
      ebuf[i] = *++t;

    ebuf[n] = 0;

    if ((t = std::getenv(ebuf.c_array()))) {
      *s1 = t;
      while(*t)
        ++t;
      *s2 = t;
    }
    else {
      s0 = *s3;
      goto top;
    }
  }
}


// WJB:  The majority of the code below I DO NOT WANT TO TOUCH!
//       (legacy "utilities" from the "not_executable" module)

/* adjust for quoted strings in arg_list[0] */
 const char**
arg_list_adjust(const char **arg_list, void **a0)
{
        const char **al, *s, *s1, *s2, *s3;
        char *t;
        int n, n0;
        size_t L;

        s = arg_list[0];
        find_env_token(s = arg_list[0], &s1, &s2, &s3);

        if (!*s3 && s == s1) {
                if (a0)
                        *a0 = 0;
                return arg_list;
        }
        n = 0;
        while(arg_list[n++]);

        n0 = n;
        L = s2 - s1 + 1;
        while(*s3) {
                find_env_token(s3, &s1, &s2, &s3);
                if (s1 == s3)
                        break;
                L += s2 - s1 + 1;
                if (*(unsigned char*)s3 <= ' ')
                        ++n;
        }

        // WJB:  NOTE boost::shared_array comment in the header file
        al = (const char**)std::malloc(n*sizeof(const char*) + L); /* freed implicitly by execvp */
        if (a0)
                *a0 = (void*)al;
        if (!al)
                return arg_list; /* should not happen */
        t = (char*)(al + n);
        find_env_token(s = arg_list[0], &s1, &s2, &s3);

        for(n = 0;;) {
                al[n++] = t;
                while(s1 < s2)
                        *t++ = *s1++;
                while(*(unsigned char*)s3 > ' ') {
                        find_env_token(s3, &s1, &s2, &s3);
                        while(s1 < s2)
                                *t++ = *s1++;
                        }
                *t++ = 0;
                if (!*s3)
                        break;
                find_env_token(s3, &s1, &s2, &s3);
                if (s1 == s3)
                        break;
        }

        n0 = 0;
        while((al[n] = arg_list[++n0]))
                ++n;
        for(n0 = 0; n0 < n && std::strchr(al[n0], '='); ++n0);
        if (n0 && n0 < n) {
                if (!a0) {
                        for(n = 0; n < n0; ++n)
                                putenv_impl((char*)(al[n]));
                        }
                al += n0;
        }

        return al;
}


// WJB - ToDo: totally rewrite in C++ (perhaps leverage STL)
//       should workdir arg be an actual BoostFS "path" object?
//       retVal:  std::vector<std::string>?!
const char**
WorkdirHelper::arg_adjust(bool cmd_line_args,
                          const std::vector<std::string>& args,
                          const char** av, const std::string& workdir)
{
  av[0] = args[0].c_str();

  int i = 1;
  if (cmd_line_args)
    for(; i < 3; ++i)
      av[i] = args[i].c_str();

  av[i] = 0;
  av = arg_list_adjust(av, 0);

  if ( !workdir.empty() ) {
    change_cwd(workdir);
    size_t len = workdir.size();
    const char* s = workdir.c_str();
    const char* t;
    // WJB: more effective STL/Boost.Regex utils for this loop?
    //      also, how bout a more portable (BstFS) way to manage '/' vs. '\'

    // BMA: I think this is stripping the leading workdir from any
    // entries in args
    for(i = 1; (t = av[i]); ++i)
      if (!std::strncmp(s, t, len) && t[len] == '/')
        av[i] = t + len + 1;
  }

  return av;
}


#endif  // DEBUG_LEGACY_WORKDIR


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


bfs::path WorkdirHelper::system_tmp_name(const std::string& prefix)
{
  // TODO: split into a separate filename and directory portion to
  // migrate sooner than later for param file name generation.
  // TODO: tmp files in . or /tmp?   Could offer option.
  // TODO: try/catch
  bfs::path temp_directory = bfs::temp_directory_path();
  // generate a 6 hex character unique name
  std::string temp_name_pattern(prefix + "_%%%%%%%%");
  bfs::path temp_name = bfs::unique_path(temp_name_pattern);
  return (temp_directory / temp_name);

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


/** mkdir_option is DIR_CLEAN (remove and recreate), DIR_PERSIST
    (leave existing), or DIR_ERROR (don't allow existing) returns
    whether a new directory was created. */
bool WorkdirHelper::create_directory(const bfs::path& dir_path,
				     short mkdir_option)
{
  bool dir_created = false;  // whether this function created a new directory

  // remove any existing directory if requested
  if (mkdir_option == DIR_CLEAN && bfs::exists(dir_path))
    recursive_remove(dir_path);

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
      Cerr << "\nError: Directory " << dir_path << " exists (permitted), but "
	   << "not writable." << std::endl;
      abort_handler(-1);
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
      Cerr << "\nError: could not create directory " << dir_path << ";\n" 
	   << e.what() << std::endl;
      abort_handler(IO_ERROR);
    }

  }

  return dir_created;
}


void WorkdirHelper::recursive_remove(const bfs::path& rm_path)
{
  try {
    if (bfs::exists(rm_path))
      bfs::remove_all(rm_path);
  }
  catch (const bfs::filesystem_error& e) {
    Cerr << "\nError: could not remove path " << rm_path << ";\n" 
        << e.what() << std::endl;
    abort_handler(IO_ERROR);
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

  return false;
}


/// note dest_dir is the containing folder for the src_path contents
/// to be placed in for consistency with other convenience functions
/// (may need to reconsider)
bool WorkdirHelper::recursive_copy(const bfs::path& src_path, 
				   const bfs::path& dest_dir, bool overwrite)
{
  // precondition: dest exists and is a dir
  if (!bfs::exists(dest_dir) || !bfs::is_directory(dest_dir))
    abort_handler(-1);

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
	// TODO: try/catch
	bfs::path src_item(dir_it->path());
	recursive_copy(src_item, dest_path, overwrite);
      }
    }
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
  if (bfs::is_directory(src_path))
    prepend_preferred_env_path(src_path.string());

  return false;
}


bool WorkdirHelper::check_equivalent(const bfs::path& src_path, 
				     const bfs::path& dest_dir, bool overwrite)
{
  if (bfs::equivalent(src_path, dest_dir)) {
    Cerr << "Error: specified link/copy_file " << src_path << "\n"
	 << "       is same as work_directory " << dest_dir << "." << std::endl;
    return true;
  }
  return false;
}

bool WorkdirHelper::find_file(const bfs::path& src_path, 
			      const bfs::path& search_file, bool overwrite)
{
  if ( bfs::is_regular_file(src_path) && 
       src_path.filename() == search_file.filename())
    return true;

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
