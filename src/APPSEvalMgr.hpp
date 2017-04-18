/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale
    Applications Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota
    directory.
    _______________________________________________________________________ */

//- Class:       APPSEvalMgr
//- Description: Specialized evaluation manager class derived from
//-              APPSPACK's Executor class which redefines virtual
//-              evaluation functions with DAKOTA's response
//-              computation procedures
//- Owner:       Patty Hough
//- Checked by:
//- Version: $Id$

#ifndef APPSEvalMgr_H
#define APPSEvalMgr_H

#include "HOPSPACK_Executor.hpp"
#include "HOPSPACK_Vector.hpp"

#include "DakotaModel.hpp"
#include "dakota_data_types.hpp"

namespace Dakota {

/// Evaluation manager class for APPSPACK 

/** The APPSEvalMgr class is derived from APPSPACK's Executor class.
    It implements the methods of that class in such away that allows
    DAKOTA to manage the computation of responses instead of APPS.
    Iterate and response values are passed between Dakota and APPSPACK
    via this interface. */

class APPSEvalMgr : public HOPSPACK::Executor
{

public:
  
  //
  //- Heading: Constructor and destructor
  //

  /// constructor
  APPSEvalMgr(Model& model);

  /// destructor
  ~APPSEvalMgr() {};
    
  //
  //- Heading: Virtual function redefinitions
  //

  /// tells APPS whether or not there is a processor available to perform a function evaluation
  bool isReadyForWork() const;

  /// performs a function evaluation at APPS-provided x_in
  bool submit(const int apps_tag, const HOPSPACK::Vector& apps_xtrial,
	      const HOPSPACK::EvalRequestType apps_request);

  /// returns a function value to APPS
  int recv(int& apps_tag, HOPSPACK::Vector& apps_f,
	   HOPSPACK::Vector& apps_cEqs, HOPSPACK::Vector& apps_cIneqs,
	   string& apps_msg);

  /// return the type of the Dakota linked evaluator
  std::string getEvaluatorType(void) const;

  /// empty implementation of debug info needed to complete the interface
  void printDebugInfo(void) const {};
  /// empty implementation of timing info needed to complete the interface
  void printTimingInfo(void) const {};

  //
  //- Heading: Convenience functions
  //

  /// publishes whether or not to do asynchronous evaluations
  void set_asynch_flag(const bool dakotaAsynchFlag)
  {modelAsynchFlag = dakotaAsynchFlag;}

  /// publishes whether or not APPS is operating synchronously
  void set_blocking_synch(const bool blockingSynchFlag)
  {blockingSynch = blockingSynchFlag;}

  /// publishes the number of processors available for function evaluations
  void set_total_workers(const int numDakotaWorkers)
  {numWorkersTotal = numDakotaWorkers;}

//PDH: Shouldn't need this anymore.

  /// publishes constraint transformation
  void set_constraint_map(std::vector<int> constraintMapIndices,
			  std::vector<double> constraintMapMultipliers,
			  std::vector<double> constraintMapOffsets)
  {constrMapIndices = constraintMapIndices;
   constrMapMultipliers = constraintMapMultipliers;
   constrMapOffsets = constraintMapOffsets;}

private:

  //
  //- Heading: Private data
  //

  /// reference to the APPSOptimizer's model passed in the constructor
  Model& iteratedModel;

  /// flag for asynchronous function evaluations
  bool modelAsynchFlag;

  /// flag for APPS synchronous behavior
  bool blockingSynch;

  /// number of processors actively performing function evaluations
  int numWorkersUsed;

  /// total number of processors available for performing function evaluations
  int numWorkersTotal;

//PDH: Don't think we'll need these three constraint map data members
//anymore.

  /// map from Dakota constraint number to APPS constraint number
  std::vector<int> constrMapIndices;

  /// multipliers for constraint transformations
  std::vector<double> constrMapMultipliers;

  /// offsets for constraint transformations
  std::vector<double> constrMapOffsets;

  /// trial iterate
  RealVector xTrial;

  /// map of DAKOTA eval id to APPS eval id (for asynchronous evaluations)
  std::map<int,int> tagList;

  /// map of APPS eval id to responses (for synchronous evaluations)
  std::map<int,RealVector> functionList;

  /// map of DAKOTA responses returned by synchronize_nowait()
  IntResponseMap dakotaResponseMap;

};

} //namespace Dakota

#endif
