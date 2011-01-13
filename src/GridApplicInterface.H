/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        GridApplicInterface
//- Description:  Derived class for the case when analysis code simulators are
//-               invoked using a Condor/Globus computational grid
//- Owner:        Mike Eldred
//- Version: $Id: GridApplicInterface.H 6492 2009-12-19 00:04:28Z briadam $

#ifndef GRID_APPLIC_INTERFACE_H
#define GRID_APPLIC_INTERFACE_H

#include "SysCallAnalysisCode.H"
#include "ApplicationInterface.H"


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

class GridApplicInterface: public ApplicationInterface
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
  void derived_synch(PRPQueue& prp_queue)
	{
  	while (completionSet.empty())
          derived_synch_kernel(prp_queue);
	}

  ///
  void derived_synch_nowait(PRPQueue& prp_queue)
	{
	derived_synch_kernel(prp_queue);
	}

  /** This code provides the derived function used by
   *  ApplicationInterface::serve_analyses_synch().
   */
  int derived_synchronous_local_analysis(const int& analysis_id)
	{
	///
	/// TODO - allow local analyses?????
	///
  	code.spawn_analysis(analysis_id, BLOCK);
  	return 0; // used for failure codes in DirectFn case
	}

  const StringArray& analysis_drivers() const;

  const AnalysisCode* analysis_code() const;

  /// Used to read/write parameter files and responses
  SysCallAnalysisCode code;

protected:

  //
  //- Heading: Methods
  //

  /// Convenience function for common code between derived_synch() &
  /// derived_synch_nowait()
  void derived_synch_kernel(PRPQueue& prp_queue);

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


inline const StringArray& GridApplicInterface::analysis_drivers() const
{ return code.program_names(); }


inline const AnalysisCode* GridApplicInterface::analysis_code() const
{ return &code; }

} // namespace Dakota

#endif
