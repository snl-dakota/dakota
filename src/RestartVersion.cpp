/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */


#include "dakota_global_defs.hpp"
#include "DakotaBuildInfo.hpp"
#include "RestartVersion.hpp"
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/export.hpp>

namespace Dakota {

/** This creates its own iarchive for the restart file as if the file
    doesn't contain full restart information, need to rewind and
    create a new iarchive anyway. */
RestartVersion
RestartVersion::check_restart_version(const std::string& rst_filename)
{
  std::ifstream ifs(rst_filename, std::ios::binary);
  if (!ifs.good()) {
    Cerr << "\nError: could not open restart file '"
	 << rst_filename << "' for reading."<< std::endl;
    abort_handler(IO_ERROR);
  }

  boost::archive::binary_iarchive inarch(ifs);
  RestartVersion rst_ver;
  try {
    inarch & rst_ver;
    if (rst_ver.restartVersion < RestartVersion::restartFirstVersionNumber) {
      Cout << "Warning: Restart file '" << rst_filename
	   << "' predates restart versioning.\n  Will attempt to read as pre-"
	   << RestartVersion::firstSupportedDakotaVersion()
	   << " restart file; use with caution." << std::endl;
      // return default-constructed RestartVersion so has version 0 / unknown
      return RestartVersion();
    }
    // else rstFirstVer# <= read version <= curr ver
    else if (RestartVersion::latestRestartVersion < rst_ver.restartVersion) {
      Cerr << "\nError: cannot read restart file '" << rst_filename
	   << "'created with newer version of Dakota.\nRunning Dakota version is "
	   << DakotaBuildInfo::get_release_num()
	   << "\n  which has restart version "
	   << RestartVersion::latestRestartVersionDelta
	   << ".\nWhile read restart file Dakota version is "
	   << rst_ver.dakotaRelease
	   << "\n  with restart version " << rst_ver.restartVersion << "."
	   << std::endl;
      // abort_handler(IO_ERROR);
    }
    else {
      Cout << "Reading restart file '" << rst_filename << "' containing: " << rst_ver;
    }
  }
  catch (const std::exception& e) {
    Cout << "Warning: Unknown error reading version info from restart file '"
	 << rst_filename << "'\n  Will attempt to read as pre-"
	 << RestartVersion::firstSupportedDakotaVersion()
	 << " restart file; use with caution." << std::endl;
         // return default-constructed so has version 0 / unknown
         return RestartVersion();
  }
  return rst_ver;
}

}

BOOST_CLASS_EXPORT(Dakota::RestartVersion)
