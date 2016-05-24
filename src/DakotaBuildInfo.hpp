/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Description: Monostate storage class for repository revision,
//-              build time and date.
//- 
//- Owner:       Bill Bohnhoff
//- Checked by:
//- Version: $Id$

#ifndef DAKOTA_BUILD_INFO_H
#define DAKOTA_BUILD_INFO_H

#include <string>

namespace Dakota {

class DakotaBuildInfo
{
public:

  /// Release number such as 5.4, 5.4+, or 5.3.1
  static std::string get_release_num();
  /// Repository revision number
  static std::string get_rev_number();
  /// Release date, as set by CMake, or fallback on build date
  static std::string get_release_date();
  /// Compilation date
  static std::string get_build_date();
  /// Compilation time
  static std::string get_build_time();

private:

  // Cached data - updated by the build system

  /// Release number such as 5.4, 5.4+, or 5.3.1
  static std::string releaseNum;
  /// Release data; if empty, the build date is used as release date
  /// to support stable
  static std::string releaseDate;
  /// Repository revision of Dakota core
  static std::string revision;
};


inline std::string DakotaBuildInfo::get_release_num() { return releaseNum; }
inline std::string DakotaBuildInfo::get_rev_number()  { return revision; }
inline std::string DakotaBuildInfo::get_release_date()  
{ 
  if (releaseDate.empty())
    return __DATE__; 
  return releaseDate;
}
inline std::string DakotaBuildInfo::get_build_date()  { return __DATE__; }
inline std::string DakotaBuildInfo::get_build_time()  { return __TIME__; }

} // namespace Dakota

#endif  // DAKOTA_BUILD_INFO_H
