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
                              // WJB: wd_adjust has a MAJOR dependency on statics in filesystem_utils..C
#include <boost/array.hpp>
#include <sys/param.h>

#ifdef __SUNPRO_CC
#include <stdlib.h>  // for putenv
#endif


namespace Dakota {

char* WorkdirHelper::cwdBegin     = 0;
char* WorkdirHelper::envPathBegin = 0;

//std::pair<std::string, std::string>
//  WorkdirHelper::startupCwdEnvPathPair = WorkdirHelper::get_dakpath();


/** Portability adapter for getcwd
 */
std::string WorkdirHelper::get_cwd()
{
  // Boost.Filesystem V2/V3 "adapter layer" should be easily implemented here
  boost::array<char, MAXPATHLEN> cwd;

#ifdef WIN32
#define DAK_FAIL_FMT "GetCurrentDirectory() failed!\n"
  size_t len = GetCurrentDirectory(MAXPATHLEN, cwd.c_array());
  if (len <= 0 || len >= MAXPATHLEN)
#else
#define DAK_FAIL_FMT "getcwd() failed!\n"
  if (!getcwd(cwd.c_array(), MAXPATHLEN))
#endif
  {
    Cerr << "\nERROR: " << DAK_FAIL_FMT << std::endl;
    abort_handler(-1);
  }

  return std::string( cwd.c_array() );
}


/** Gets the CWD and the $PATH and stuffs them into a common buffer
    (WJB: when dakota was launched!?) */
std::pair<std::string, std::string> WorkdirHelper::get_dakpath()
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

  cwdBegin = new char [total_buf_size];

  // first, copy cwd into the buffer and terminate with NULL as a separator
  std::memcpy(cwdBegin, cwd.data(), cwd_len);
  cwdBegin[cwd_len] = 0;

  // second, copy PATH environment variable into the same buffer
  envPathBegin = cwdBegin + cwd_len + 1;
  std::memcpy(envPathBegin, DAK_PATH_ENV_NAME "=", 5);
  std::memcpy(envPathBegin+5, env_path, path_len);
  envPathBegin[path_len+5] = 0;

  return std::make_pair( cwd, std::string(env_path, path_len) );
}


/** Resets the working directory "state" to its initial state when DAKOTA 
    was launched */
void WorkdirHelper::reset()
{
  if (DAK_CHDIR(cwdBegin)) {
    Cerr << "\nError: chdir(" << cwdBegin
         << ") failed in workdir_reset()" << std::endl;
    abort_handler(-1);
  }

  if (putenv(envPathBegin)) {
    Cerr << "\nError: putenv(" << envPathBegin
         << ") failed in workdir_reset()" << std::endl;
    abort_handler(-1);
  }
}



/** Change directory to workdir and make necessary adjustments
    to $PATH (and drive for Windows platform) */
void WorkdirHelper::change_cwd(const std::string& workdir)
{
  workdir_adjust(workdir);
}

} // namespace Dakota
