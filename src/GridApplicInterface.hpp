/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        GridApplicInterface
//- Description:  Derived class for the case when analysis code simulators are
//-               invoked using a Condor/Globus computational grid
//- Owner:        Mike Eldred
//- Version: $Id: GridApplicInterface.hpp 6492 2009-12-19 00:04:28Z briadam $

#ifndef GRID_APPLIC_INTERFACE_H
#define GRID_APPLIC_INTERFACE_H

#include "SysCallApplicationInterface.hpp"


namespace Dakota {

extern "C" {
/// definition of start grid computing type (function pointer)
typedef int  (*start_grid_computing_t)(char *analysis_driver_script,
				       char *params_file, char *results_file);
/// definition of perform analysis type (function pointer)
typedef int  (*perform_analysis_t)(char *iteration_num);
/// definition of get completed jobs type (function pointer)
typedef int* (*get_jobs_completed_t)();
/// definition of stop grid computing type (function pointer)
typedef int  (*stop_grid_computing_t)();
}


/// Derived application interface class which spawns simulation codes
/// using grid services such as Condor or Globus.

/** This class is currently a modified copy of SysCallApplicInterface
    adapted for use with an external grid dervices library which was
    dynamically linked using dlopen() services. */

class GridApplicInterface: public SysCallApplicInterface
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// constructor
  GridApplicInterface(const ProblemDescDB& problem_db);

  /// destructor
  ~GridApplicInterface();

  //
  //- Heading: Virtual function redefinitions
  //

  ///
  void derived_map(const Variables& vars, const ActiveSet& set,
		   Response& response, int fn_eval_id);

  ///
  void derived_map_asynch(const ParamResponsePair& pair);

  ///
  void wait_local_evaluations(PRPQueue& prp_queue)
	{
  	while (completionSet.empty())
          test_local_evaluations(prp_queue);
	}

  ///
  void test_local_evaluations(PRPQueue& prp_queue);

  /** This code provides the derived function used by
   *  ApplicationInterface::serve_analyses_synch().
   */
  int synchronous_local_analysis(int analysis_id)
	{
	///
	/// TODO - allow local analyses?????
	///
  	spawn_analysis_to_shell(analysis_id, BLOCK);
  	return 0; // used for failure codes in DirectFn case
	}

protected:

  //
  //- Heading: Methods
  //

  /// test file(s) for existence based on root_file name 
  bool grid_file_test(const String& root_file);

  //
  //- Heading: Data
  //

  /// Set of function evaluation id's for active asynchronous
  /// system call evaluations
  IntSet idSet;

  /// map linking function evaluation id's to number of response read failures
  IntShortMap failCountMap; 

  /// handle to dynamically linked start_grid_computing function
  start_grid_computing_t start_grid_computing;
  /// handle to dynamically linked perform_analysis grid function
  perform_analysis_t     perform_analysis;
  /// handle to dynamically linked get_jobs_completed grid function
  get_jobs_completed_t   get_jobs_completed;
  /// handle to dynamically linked stop_grid_computing function
  stop_grid_computing_t  stop_grid_computing;
};

} // namespace Dakota

#endif
