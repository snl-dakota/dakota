/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        ParallelDirectApplicInterface
//- Description:  Derived class for the case when analysis code simulators are
//-               linked into the code and may be invoked directly
//- Owner:        Mike Eldred
//- Version: $Id: PluginParallelDirectApplicInterface.H 6492 2009-12-19 00:04:28Z briadam $

#ifndef PLUGIN_PARALLEL_DIRECT_APPLIC_INTERFACE_H
#define PLUGIN_PARALLEL_DIRECT_APPLIC_INTERFACE_H

#include "DirectApplicInterface.H"
#ifdef DAKOTA_HAVE_MPI
#include <mpi.h>
#else
typedef int MPI_Comm;
#endif // DAKOTA_HAVE_MPI


namespace SIM {

/// Sample derived interface class for testing parallel simulator
/// plug-ins using assign_rep().

/** The plug-in ParallelDirectApplicInterface resides in namespace SIM
    and uses a copy of textbook() to perform parallel parameter to
    response mappings. It may be activated by specifying the
    --with-plugin configure option, which activates the DAKOTA_PLUGIN
    macro in dakota_config.h used by main.C (which activates the
    plug-in code block within that file) and activates the PLUGIN_S
    declaration defined in Makefile.include and used in
    Makefile.source (which add this class to the build).  Test input
    files should then use an analysis_driver of "plugin_textbook". */

class ParallelDirectApplicInterface: public Dakota::DirectApplicInterface
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// constructor
  ParallelDirectApplicInterface(const Dakota::ProblemDescDB& problem_db,
				const MPI_Comm& analysis_comm);
  /// destructor
  ~ParallelDirectApplicInterface();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  // execute the input filter portion of a direct evaluation invocation
  //int derived_map_if(const Dakota::String& if_name);

  /// execute an analysis code portion of a direct evaluation invocation
  int derived_map_ac(const Dakota::String& ac_name);

  // execute the output filter portion of a direct evaluation invocation
  //int derived_map_of(const Dakota::String& of_name);

  /// no-op hides base error; job batching occurs within derived_synch()
  void derived_map_asynch(const Dakota::ParamResponsePair& pair);

  /// evaluate the batch of jobs contained in prp_queue
  void derived_synch(Dakota::PRPQueue& prp_queue);
  /// invokes derived_synch() (no special nowait support)
  void derived_synch_nowait(Dakota::PRPQueue& prp_queue);

private:

  //
  //- Heading: Data
  //

  // intracommunicator for a multiprocessor analysis server
  //MPI_Comm analysisComm;

  /// demo evaluator function for parallel plug-ins
  int text_book(const Dakota::RealVector& c_vars, const Dakota::ShortArray& asv,
		Dakota::RealVector& fn_vals, Dakota::RealMatrix& fn_grads,
		Dakota::RealSymMatrixArray& fn_hessians);
};


inline ParallelDirectApplicInterface::~ParallelDirectApplicInterface()
{ /* Virtual destructor handles referenceCount at Interface level. */ }

} // namespace Dakota

#endif
