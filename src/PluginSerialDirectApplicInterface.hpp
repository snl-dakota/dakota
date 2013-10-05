/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        SerialDirectApplicInterface
//- Description:  Derived class for the case when analysis code simulators are
//-               linked into the code and may be invoked directly
//- Owner:        Mike Eldred
//- Version: $Id: PluginSerialDirectApplicInterface.hpp 6492 2009-12-19 00:04:28Z briadam $

#ifndef PLUGIN_SERIAL_DIRECT_APPLIC_INTERFACE_H
#define PLUGIN_SERIAL_DIRECT_APPLIC_INTERFACE_H

#include "DirectApplicInterface.hpp"


/// A sample namespace for derived classes that use assign_rep() to 
/// plug facilities into DAKOTA.

/** A typical use of plug-ins with assign_rep() is to publish a 
    simulation interface for use in library mode  See \ref DakLibrary
    for more information. */

namespace SIM {

/// Sample derived interface class for testing serial simulator
/// plug-ins using assign_rep().

/** The plug-in SerialDirectApplicInterface resides in namespace SIM
    and uses a copy of rosenbrock() to perform serial parameter to
    response mappings. It may be activated by specifying the
    --with-plugin configure option, which activates the DAKOTA_PLUGIN
    macro in dakota_config.h used by main.cpp (which activates the
    plug-in code block within that file) and activates the PLUGIN_S
    declaration defined in Makefile.include and used in
    Makefile.source (which add this class to the build).  Test input
    files should then use an analysis_driver of "plugin_rosenbrock". */

class SerialDirectApplicInterface: public Dakota::DirectApplicInterface
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// constructor
  SerialDirectApplicInterface(const Dakota::ProblemDescDB& problem_db);
  /// destructor
  ~SerialDirectApplicInterface();

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

  /// no-op hides base error; job batching occurs within
  /// wait_local_evaluations()
  void derived_map_asynch(const Dakota::ParamResponsePair& pair);

  /// evaluate the batch of jobs contained in prp_queue
  void wait_local_evaluations(Dakota::PRPQueue& prp_queue);
  /// invokes wait_local_evaluations() (no special nowait support)
  void test_local_evaluations(Dakota::PRPQueue& prp_queue);

  /// no-op hides default run-time error checks at DirectApplicInterface level
  void set_communicators_checks(int max_iterator_concurrency);

private:

  //
  //- Heading: Convenience functions
  //

  /// Rosenbrock plug-in test function
  int rosenbrock(const Dakota::RealVector& c_vars, short asv,
		 Dakota::Real& fn_val, Dakota::RealVector& fn_grad,
		 Dakota::RealSymMatrix& fn_hess);

  //
  //- Heading: Data
  //

};


inline SerialDirectApplicInterface::
SerialDirectApplicInterface(const Dakota::ProblemDescDB& problem_db):
  Dakota::DirectApplicInterface(problem_db)
{ }


inline SerialDirectApplicInterface::~SerialDirectApplicInterface()
{ /* Virtual destructor handles referenceCount at Interface level. */ }


inline void SerialDirectApplicInterface::
derived_map_asynch(const Dakota::ParamResponsePair& pair)
{
  // no-op (just hides base class error throw). Jobs are run exclusively within
  // wait_local_evaluations(), prior to there existing true batch processing
  // facilities.
}


/** For use by ApplicationInterface::serve_evaluations_asynch(), which can
    provide a batch processing capability within message passing schedulers
    (called using chain Strategy::run_iterator() --> Model::serve() -->
    ApplicationInterface::serve_evaluations() -->
    ApplicationInterface::serve_evaluations_asynch()). */
inline void SerialDirectApplicInterface::
test_local_evaluations(Dakota::PRPQueue& prp_queue)
{ wait_local_evaluations(prp_queue); }


// Hide default run-time error checks at DirectApplicInterface level
inline void SerialDirectApplicInterface::
set_communicators_checks(int max_iterator_concurrency)
{ }

} // namespace Dakota

#endif
