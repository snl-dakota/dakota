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
#include <cassert>


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
    driver_path_str = driver_name;
  }

  return driver_path_str;
}


/** Resets the working directory "state" to its initial state when DAKOTA 
    was launched */
void WorkdirHelper::reset()
{
  if (DAK_CHDIR( startupPWD.c_str() )) {
    Cerr << "\nError: chdir(" << startupPWD
         << ") failed in workdir_reset()" << std::endl;
    abort_handler(-1);
  }

  putenv_impl(dakPreferredEnvPath.c_str());
}


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
    for(i = 1; (t = av[i]); ++i)
      if (!std::strncmp(s, t, len) && t[len] == '/')
        av[i] = t + len + 1;
  }

  return av;
}


// BMA TODO: link first, then copy so we skip if not replacing...

/** Iterate source items (paths or wildcards), linking each of them
    from the destination.  If overwrite, remove and replace any
    existing destination target, otherwise, allow to persist */
void WorkdirHelper::link_items(const StringArray& source_items,
			       const bfs::path& destination_path,
			       bool overwrite) 
{
  // iterate the items and link each entry or expanded wildcard
  StringArray::const_iterator src_it  = source_items.begin();
  StringArray::const_iterator src_end = source_items.end();
  for( ; src_it != src_end; ++src_it) {

    // TODO: more efficient
    //    if ( contains_wildcard(*src_it) ) {
    if ( strcontains(*src_it, "*") || strcontains(*src_it, "?") ) {
      //   iterate paths matching the wildcard;
    }
    else {

      //   link the path;
      bfs::path src_path(*src_it);
      if (bfs::exists(src_path)) {
	bfs::path src_filename = src_path.filename();
	link(src_path, destination_path / src_filename, overwrite);
      }
      else {
	Cout << "Warning: path " << src_path 
	     << " specified to link to does not exist. Skipping." << std::endl;
      }

    }

  }

}

/** Both paths must be fully resolved. Assumes source file exists
    since it was iterated in the calling context. If overwrite, any
    existing path dest_link will be removed prior to creating the new
    link. */
void WorkdirHelper::link(const bfs::path& src_path, const bfs::path& dest_link,
			 bool overwrite)
{
  // symlink facilities require a qualifed source and destination
  // name, be absolute for now
  
  if (overwrite && bfs::exists(dest_link))
    bfs::remove_all(dest_link);

  // now, only make the link if the dest doesn't exist
  if (!bfs::exists(dest_link)) {
    if (bfs::is_directory(src_path))
      bfs::create_directory_symlink(src_path, dest_link);
    else
      bfs::create_symlink(src_path, dest_link);
  }

}


} // namespace Dakota
