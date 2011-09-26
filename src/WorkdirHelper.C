/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        WorkdirHelper
//- Description:  Class implementation
//- Owner:        Bill Bohnhoff

#include "WorkdirHelper.H"
#include "global_defs.h"
#include "filesystem_utils.h" // undesirable dependency for workdir_adjust
                              // WJB: wd_adjust has a MAJOR dependency on statics in filesystem_utils.C
#include <boost/array.hpp>
#include <boost/lexical_cast.hpp>
#include <cassert>

#if defined(__APPLE__)
#include <mach-o/dyld.h>
#endif


namespace Dakota {

/* WJB 0
std::string WorkdirHelper::startupPWD  = get_cwd();
std::string WorkdirHelper::startupPATH = init_startup_path();

std::string WorkdirHelper::dakPreferredEnvPath = set_preferred_env_path();
end WJB 0 */
char* WorkdirHelper::cwdBegin     = 0;
char* WorkdirHelper::envPathBegin = 0;

//WJB: gradually re-enable features and commit if BMA tests pass
#if 0
std::vector<char> WorkdirHelper::cwdAndEnvPathBuf =
  std::vector<char>(get_cwd().size(), DAK_PATH_SEP);


/** Overwrites $PATH with additional directories so that analysis driver
 *  detection is (hopefully) more robust
 */
std::string WorkdirHelper::set_preferred_env_path()
{
  std::string path_sep_string(1, DAK_PATH_SEP);
  std::string parent_of_dakexe(DAK_MAXPATHLEN, '*');

#if defined(__APPLE__)

  uint32_t exe_pathlen = DAK_MAXPATHLEN;
  if (_NSGetExecutablePath((char*)parent_of_dakexe.c_str(),
                           &exe_pathlen) == 0) {
    std::string::size_type split_pos = parent_of_dakexe.find_last_of(DAK_SLASH);
    parent_of_dakexe = parent_of_dakexe.substr(0, split_pos);
  }
  else {
    Cerr << "\nWarning: _NSGetExecutablePath() failed; continuing anyway.."
         << std::endl;
    parent_of_dakexe = std::string();
    //Cout << "buffer too small; need " << exe_pathlen << std::endl;
    //abort_handler(-1);
  }

#elif defined(__linux__)

  std::string path_for_readlink =
    "/proc/" + boost::lexical_cast<std::string>(getpid()) + "/exe";

  if (readlink(path_for_readlink.c_str(), (char*)parent_of_dakexe.c_str(),
               DAK_MAXPATHLEN) != -1) {
    //parent_of_dakexe[pos] = 0; // WJB: verify already NULL terminated
    std::string::size_type split_pos = parent_of_dakexe.find_last_of(DAK_SLASH);
    parent_of_dakexe = parent_of_dakexe.substr(0, split_pos);
  }
  else {
    Cerr << "\nWarning: readlink() failed; continuing anyway.." << std::endl;
    parent_of_dakexe = std::string();
  }

#elif defined(_WIN32) || defined(_WIN64)

  if (GetModuleFileName(NULL, (char*)parent_of_dakexe.c_str(),
                        DAK_MAXPATHLEN) != 0) {
    std::string::size_type split_pos = parent_of_dakexe.find_last_of(DAK_SLASH);
    parent_of_dakexe = parent_of_dakexe.substr(0, split_pos);
  }
  else {
    Cerr << "\nWarning: GetModuleFileName() failed; continuing anyway.."
         << std::endl;
    parent_of_dakexe = std::string();
  }

#elif defined(__SUNPRO_CC)

  char* exec_name = (char*)parent_of_dakexe.c_str();

  if (( exec_name=(char*)getexecname() ) != NULL) {
    parent_of_dakexe = exec_name;
    std::string::size_type split_pos = parent_of_dakexe.find_last_of(DAK_SLASH);
    parent_of_dakexe = parent_of_dakexe.substr(0, split_pos);
  }
  else {
    Cerr << "\nWarning: getexecname() failed; continuing anyway.."
         << std::endl;
    parent_of_dakexe = std::string();
  }

#else

  parent_of_dakexe = std::string(); // default case (empty string should be OK)

#endif

  //Cout << "Analysis Driver POTENTIALLY in " << parent_of_dakexe << std::endl;

  std::string preferred_env_path(DAK_PATH_ENV_NAME"=");

  // Go ahead and add '.' to the preferred $PATH here as well since it can't hurt

  preferred_env_path += "." + path_sep_string + parent_of_dakexe;

  preferred_env_path += path_sep_string + get_cwd() + path_sep_string;
  preferred_env_path += init_startup_path();

  putenv_impl(preferred_env_path);

  return preferred_env_path;
}


/** Overwrites $PATH with an additional directory prepended,
 *  typically for the purpose of ensuring templatedir is in the $PATH
 */
void WorkdirHelper::prepend_preferred_env_path(const std::string& extra_path)
{
  // Assume a relative extra_path arg is relative to dakota's startupPWD
  // WJB - NOTE: BoostFS would help here (WIN32 drive letter!)
  std::string abs_extra_path = (extra_path[0] != DAK_SLASH) ?
                               startupPWD + std::string(1,DAK_SLASH)+extra_path :
                               extra_path;

  std::string preferred_env_path(DAK_PATH_ENV_NAME"=");
  std::string old_preferred_path
    = dakPreferredEnvPath.substr(preferred_env_path.size());

  preferred_env_path += abs_extra_path + std::string(1, DAK_PATH_SEP)
                      + old_preferred_path;
  dakPreferredEnvPath = preferred_env_path;

  putenv_impl(preferred_env_path);
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


/** Gets the CWD and the $PATH and stuffs them into a common buffer
 */
void WorkdirHelper::get_dakpath()
{
  char* env_path = std::getenv(DAK_PATH_ENV_NAME);

  if (!env_path) {
    Cerr << "\nERROR: "
         << "getenv(\"" DAK_PATH_ENV_NAME "\") failed in get_dakpath().\n"
         << std::endl;
    abort_handler(-1);
  }

  const std::string& cwd = get_cwd();

  size_t path_len = std::strlen(env_path);
  size_t cwd_len  = cwd.size();

  // Allocate enough space for BOTH strings + "PATH=" + 2 NULL terminators
  size_t total_buf_size = cwd_len + path_len + 7;

  char* cwd_and_env_path_buf = new char [total_buf_size];
  cwdBegin = &cwd_and_env_path_buf[0];  // or cwdAndEnvPathBuf.data() if managed with class scoped std::vector

  // first, copy cwd into the buffer and terminate with NULL as a separator
  std::memcpy(cwdBegin, cwd.data(), cwd_len);
  cwdBegin[cwd_len] = 0;

  // second, copy PATH environment variable into the same buffer
  envPathBegin = cwdBegin + cwd_len + 1;
  std::memcpy(envPathBegin, DAK_PATH_ENV_NAME "=", 5);
  std::memcpy(envPathBegin+5, env_path, path_len);
  envPathBegin[path_len+5] = 0;
}
#endif  // end long block WJB: gradually re-enable features and commit if.. pass


/** Resets the working directory "state" to its initial state when DAKOTA 
    was launched */
void WorkdirHelper::reset()
{
  if (DAK_CHDIR( cwdBegin )) {
    Cerr << "\nError: chdir(" << std::string(cwdBegin)
         << ") failed in workdir_reset()" << std::endl;
    abort_handler(-1);
  }

  putenv_impl(envPathBegin);
}



/** Change directory to workdir and make necessary adjustments
    to $PATH (and drive for Windows platform) */
void WorkdirHelper::change_cwd(const std::string& workdir)
{
  // WJB:  use as an "adapter layer" to manager 3 different APIs ?
  //       1. DMG not_executable,  2. BoostFS V2, and  3. BoostFS V3
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
                        for(n = 0; n < n0; ++n) // WJB: tmp sting object going out of scope?? putenv_impl(std::string(al[n]));
                                putenv_impl((char*)(al[n]));
                        }
                al += n0;
        }

        return al;
}


// WJB - ToDo: totally rewrite in C++ (perhaps leverage STL)
//       should workdir arg be an actual BoostFS "path" object?
//       reVal:  StringArray?!
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

} // namespace Dakota
