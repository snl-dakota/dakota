/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Description: Singleton storage class for Subversion revision, build
//-              time and date; referenced via getInstance() from clients.
//- 
//- Owner:       Shannon Brown
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
  static std::string getRev();
  static std::string getDate();
  static std::string getTime();

private:

  // Cached data - updated by the build system
  static std::string rev;
};


inline std::string DakotaBuildInfo::getRev()  { return rev;      }
inline std::string DakotaBuildInfo::getDate() { return __DATE__; }
inline std::string DakotaBuildInfo::getTime() { return __TIME__; }

} // namespace Dakota

#endif  // DAKOTA_BUILD_INFO_H
