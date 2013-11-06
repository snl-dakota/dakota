/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Description: Monostate storage class for Subversion revision,
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

  // Queries
  static std::string get_release_num();
  static std::string get_rev_number();
  static std::string get_build_date();
  static std::string get_build_time();

private:

  // Cached data - updated by the build system
  static std::string releaseNum;
  static std::string rev;
};


inline std::string DakotaBuildInfo::get_release_num() { return releaseNum; }
inline std::string DakotaBuildInfo::get_rev_number()  { return rev; }
inline std::string DakotaBuildInfo::get_build_date()  { return __DATE__; }
inline std::string DakotaBuildInfo::get_build_time()  { return __TIME__; }

} // namespace Dakota

#endif  // DAKOTA_BUILD_INFO_H
