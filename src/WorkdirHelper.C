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
#include <sys/param.h>
#include <cassert>


namespace Dakota {

std::string WorkdirHelper::startupPWD  = get_cwd();
std::string WorkdirHelper::startupPATH = init_startup_path();

std::vector<char> WorkdirHelper::cwdAndEnvPathBuf =
  std::vector<char>(get_cwd().size(), DAK_PATH_SEP);


/** Gets the $PWD and the $PATH and stuffs them into a common buffer
 */
char* WorkdirHelper::get_dakpath()
{
  size_t cwd_len  = startupPWD.size();
  size_t path_len = startupPATH.size();

  // Desired $PATH includes space for $PWD (cwd_len + 1 colon char)
  //
  // Allocate enough space for BOTH strings + "PATH=" + 2 NULL terminators
  //                                          (and now an xtra colon for $PWD)
  size_t total_buf_size = 2*cwd_len + path_len + 8;

  cwdAndEnvPathBuf.resize(total_buf_size, DAK_PATH_SEP);
  char* cwd = cwdAndEnvPathBuf.data();  // aka &cwdAndEnvPathBuf[0]

  // first, copy cwd into the buffer and terminate with NULL as a separator
  std::copy(startupPWD.data(), startupPWD.data() + cwd_len, cwd);
  cwdAndEnvPathBuf[cwd_len] = 0;

  // second, copy PATH environment variable into the same buffer
  char* env_path = cwd + cwd_len + 1;

  // WJB:  dataBuf is now a vector, so consider std::copy instead of memcpy
  std::memcpy(env_path, DAK_PATH_ENV_NAME "=", 5);

  std::memcpy(env_path+5, startupPWD.data(), cwd_len);
  std::memcpy(env_path+6+cwd_len, startupPATH.data(), path_len);
  env_path[path_len+cwd_len+6] = 0;

  putenv_impl(env_path);
  return env_path;
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


/** Resets the working directory "state" to its initial state when DAKOTA 
    was launched */
void WorkdirHelper::reset()
{
  if (DAK_CHDIR( startupPWD.c_str() )) {
    Cerr << "\nError: chdir(" << startupPWD.c_str()
         << ") failed in workdir_reset()" << std::endl;
    abort_handler(-1);
  }

  putenv_impl( &cwdAndEnvPathBuf[startupPWD.size()+1] );
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



/* The following routines assume ASCII or UTF-encoded Unicode. */

void find_env_token(const char *s0, const char **s1,
                    const char **s2, const char **s3)
{
  boost::array<char, FILENAME_MAX> ebuf;
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
  if (*t == '$' && (n = *s2 - t) <= FILENAME_MAX) { // 512 vs FNM_MAX=ebuf.size() ?
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
                                putenv_impl(std::string(al[n]));
                        }
                al += n0;
        }

        return al;
}


// WJB - ToDo: totally rewrite in C++ (perhaps leverage STL)
//       should workdir arg be an actual BstFS "path" object??
//       static member function of WorkdirHelper!  reVal:  StringArray?!
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
