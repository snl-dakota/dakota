/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        SIM::SerialDirectApplicInterface
//- Description:  Class implementation
//- Owner:        Mike Eldred

#include "DakotaResponse.H"
#include "ParamResponsePair.H"
#include "PluginSerialDirectApplicInterface.H"
#include "system_defs.h"
#include "ProblemDescDB.H"
#include "ParallelLibrary.H"


namespace SIM {

SerialDirectApplicInterface::
SerialDirectApplicInterface(const Dakota::ProblemDescDB& problem_db):
  Dakota::DirectApplicInterface(problem_db)
{ }


int SerialDirectApplicInterface::derived_map_ac(const Dakota::String& ac_name)
{
#ifdef MPI_DEBUG
    Cout << "analysis server " << analysisServerId << " invoking " << ac_name
         << " within SIM::SerialDirectApplicInterface." << std::endl;
#endif // MPI_DEBUG

  int fail_code = 0;
  if (ac_name == "plugin_rosenbrock") {

    if (multiProcAnalysisFlag) {
      Cerr << "Error: rosenbrock direct fn does not yet support multiprocessor "
	   << "analyses." << std::endl;
      Dakota::abort_handler(-1);
    }
    if ( numVars!=2 || numADIV || numADRV ||
	 ( ( gradFlag || hessFlag ) && numDerivVars != 2 ) ) {
      Cerr << "Error: Bad number of variables in rosenbrock direct fn."
	   << std::endl;
      Dakota::abort_handler(-1);
    }
    if (numFns > 1) { // 1 fn -> opt
      Cerr << "Error: Bad number of functions in rosenbrock direct fn."
	   << std::endl;
      Dakota::abort_handler(-1);
    }

    double f0 = xC[1]-xC[0]*xC[0];
    double f1 = 1.-xC[0];

    // **** f:
    if (directFnASV[0] & 1)
      fnVals[0] = 100.*f0*f0+f1*f1;

    // **** df/dx:
    if (directFnASV[0] & 2) {
      fnGrads[0][0] = -400.*f0*xC[0] - 2.*f1;
      fnGrads[0][1] =  200.*f0;
    }
    
    // **** d^2f/dx^2:
    if (directFnASV[0] & 4) {
      double fx = xC[1] - 3.*xC[0]*xC[0];
      fnHessians[0](0,0) = -400.*fx + 2.0;
      fnHessians[0](0,1) = fnHessians[0](1,0) = -400.*xC[0];
      fnHessians[0](1,1) =  200.;
    }
  }
  else {
    Cerr << ac_name << " is not available as an analysis within "
         << "SIM::SerialDirectApplicInterface." << std::endl;
    Dakota::abort_handler(-1);
  }

  // Failure capturing
  if (fail_code)
    throw fail_code;

  return 0;
}

} // namespace SIM
