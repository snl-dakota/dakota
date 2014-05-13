/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        MatlabInterface

//- Description:  Derived interface class for performing analysis via
//-               linked Matlab engine, permitting single startup/shutdown of 
//-               Matlab engine for the whole set of function evaluations.
//-               Initially created November 2005.
//- Owner:        Brian Adams
//- Version: $Id$

#ifndef MATLAB_INTERFACE_H
#define MATLAB_INTERFACE_H

#include "DirectApplicInterface.hpp"
#include "engine.h"

// Would prefer, but can't get forward declarations working
//struct engine;  //forward declaration for engine pointer
//struct mxArray; //forward declaration for mxArray argument

namespace Dakota {

/** Specialization of DirectApplicInterface to link to Matlab analysis 
    drivers.  Includes convenience functions to map data to/from Matlab */
class MatlabInterface: public DirectApplicInterface
{

public:

  /// Constructor: start Matlab engine
  MatlabInterface(const ProblemDescDB& problem_db);

  /// Destructor: close Matlab engine
  ~MatlabInterface();

protected:

  /// execute an analysis code portion of a direct evaluation invocation
  virtual int derived_map_ac(const String& ac_name);

  /// pointer to the MATLAB engine used for direct evaluations
  engine *matlabEngine;

  // TODO: Separate Matlab send, execute, retreive

  /// Helper function supporting derived_map_ac.  Sends data to
  /// Matlab, executes analysis, collects return data.
  int matlab_engine_run(const Dakota::String& ac_name);

  /// check that the dakota_matlab strucutre has the specified field_name and
  /// add if necessary; free structure memory in preparation for new alloc
  int matlab_field_prep(mxArray* dakota_matlab, const char* field_name);

}; // class MatlabInterface

} // namespace Dakota

#endif  // MATLAB_INTERFACE_H
