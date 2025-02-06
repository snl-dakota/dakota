/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Description:  Derived interface class for performing analysis via
//-               linked Scilab API, permitting single startup/shutdown of 
//-               Scilab for the whole set of function evaluations.
//-               Original authors: Yann Collette and STUDENT TBD
//- Owner:        Brian Adams
//- Version: $Id$

#ifndef SCILAB_INTERFACE_H
#define SCILAB_INTERFACE_H

#include "DirectApplicInterface.hpp"

namespace Dakota {

/** Specialization of DirectApplicInterface to link to Scilab analysis 
    drivers.  Includes convenience functions to map data to/from Scilab */
class ScilabInterface: public DirectApplicInterface
{
public:

  /// Constructor: start Scilab engine
  ScilabInterface(const ProblemDescDB& problem_db);

  /// Destructor: close Scilab engine
  ~ScilabInterface();

protected:

  /// execute an analysis code portion of a direct evaluation invocation
  virtual int derived_map_ac(const String& ac_name);

  /// identifier for the running Scilab enginer
  int scilabEngine;

  // TODO: Scilab send, execute, retreive
  /// principal Scilab execute function
  int scilab_engine_run(const String& ac_name);

}; // class ScilabInterface

} // namespace Dakota

#endif // SCILAB_INTERFACE_H
