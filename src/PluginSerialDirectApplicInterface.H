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
//- Version: $Id: PluginSerialDirectApplicInterface.H 6492 2009-12-19 00:04:28Z briadam $

#ifndef PLUGIN_SERIAL_DIRECT_APPLIC_INTERFACE_H
#define PLUGIN_SERIAL_DIRECT_APPLIC_INTERFACE_H

#include "DirectApplicInterface.H"


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
    macro in dakota_config.h used by main.C (which activates the
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

  /// no-op hides base error; job batching occurs within derived_synch()
  void derived_map_asynch(const Dakota::ParamResponsePair& pair);

  /// evaluate the batch of jobs contained in prp_queue
  void derived_synch(Dakota::PRPQueue& prp_queue);

private:

  //
  //- Heading: Convenience functions
  //

  /// Rosenbrock plug-in test function
  void rosenbrock(const Dakota::RealVector& c_vars, short asv, double& fn_val, 
		  Dakota::RealVector& fn_grad, Dakota::RealSymMatrix& fn_hess);

  //
  //- Heading: Data
  //

};


inline SerialDirectApplicInterface::~SerialDirectApplicInterface()
{ /* Virtual destructor handles referenceCount at Interface level. */ }

} // namespace Dakota

#endif
