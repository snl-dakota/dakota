/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        SIM::ParallelDirectApplicInterface
//- Description:  Class implementation
//- Owner:        Mike Eldred

#include "DakotaResponse.H"
#include "ParamResponsePair.H"
#include "PluginParallelDirectApplicInterface.H"
#include "system_defs.h"
#include "ProblemDescDB.H"
#include "ParallelLibrary.H"
#include <algorithm>


namespace SIM {

#define POW_VAL 1.0 // text_book: 1.0 is nominal, 1.4 used for B&B testing


ParallelDirectApplicInterface::
ParallelDirectApplicInterface(const Dakota::ProblemDescDB& problem_db,
			      const MPI_Comm& analysis_comm):
  Dakota::DirectApplicInterface(problem_db)//, analysisComm(analysis_comm)
{
  // it is not necessary to update analysis communicator size/rank, since
  // these are inherited for use at run time.  The only purpose for passing
  // analysis_comm is for use initializing a parallel app within the ctor.

#ifdef MPI_DEBUG
  // For testing purposes, output size/rank of the incoming analysis_comm
  int rank, size;
  MPI_Comm_rank(analysis_comm, &rank);
  MPI_Comm_size(analysis_comm, &size);
  Cout << "In SIM::ParallelDirectApplicInterface ctor, rank = " << rank
       << " size = " << size << std::endl;
#endif // MPI_DEBUG
}


int ParallelDirectApplicInterface::derived_map_ac(const Dakota::String& ac_name)
{
#ifdef MPI_DEBUG
    Cout << "analysis server " << analysisServerId << " invoking " << ac_name
         << " within SIM::ParallelDirectApplicInterface." << std::endl;
#endif // MPI_DEBUG

  int fail_code = 0;
  if (ac_name != "plugin_text_book") {
    Cerr << ac_name << " is not available as an analysis within "
         << "SIM::ParallelDirectApplicInterface." << std::endl;
    Dakota::abort_handler(-1);
  }
  if (numFns > 3) {
    Cerr << "Error: Bad number of functions in plug-in parallel direct "
	 << "interface." << std::endl;
    Dakota::abort_handler(-1);
  }
  // The presence of discrete variables can cause offsets in directFnDVV which
  // the text_book derivative logic does not currently account for.
  if ( (gradFlag || hessFlag) && (numADIV || numADRV) ) {
    Cerr << "Error: plug-in parallel direct interface assumes no discrete "
	 << "variables in derivative mode." << std::endl;
    Dakota::abort_handler(-1);
  }

  // **********************************
  // **** f: sum (x[i] - POWVAL)^4 ****
  // **********************************
  size_t i;
  if (directFnASV[0] & 1) {
    double local_val = 0.0;
    for (i=analysisCommRank; i<numVars; i+=analysisCommSize) {
      // orders all continuous vars followed by all discrete vars.  This is 
      // fine in the direct case so long as everything is self-consistent.
      double x_i;
      if (i<numACV)
	x_i = xC[i];
      else if (i<numACV+numADIV)
	x_i = (double)xDI[i-numACV];
      else
	x_i = xDR[i-numACV-numADIV];
      local_val += std::pow(x_i-POW_VAL, 4);
    }
    if (multiProcAnalysisFlag) {
      double global_val = 0.0;
      parallelLib.reduce_sum_a(&local_val, &global_val, 1);
      // only analysisCommRank 0 has the correct sum.  This is OK (MPI_Allreduce
      // not needed) since only analysisCommRank 0 updates response for 
      // evalCommRank 0 in overlay_response.  evalCommRank 0 then returns the 
      // results to the iterator in ApplicationInterface::serve_evaluations().
      if (analysisCommRank == 0)
	fnVals[0] = global_val;
    }
    else
      fnVals[0] = local_val;
  }

  // ****************
  // **** df/dx: ****
  // ****************
  if (directFnASV[0] & 2) {
    //for (i=0; i<numDerivVars; i++)
    //  fnGrads[0][i] = 4.*std::pow(xC[i]-POW_VAL,3);
    //fnGrads[0] = 0.;
    std::fill(fnGrads[0], fnGrads[0] + numDerivVars, 0.);
    for (i=analysisCommRank; i<numDerivVars; i+=analysisCommSize) {
      size_t var_index = directFnDVV[i] - 1;
      double x_i = xC[var_index]; // assumes no discrete vars
      fnGrads[0][i] = 4.*std::pow(x_i-POW_VAL,3);
    }
    if (multiProcAnalysisFlag) {
      double* sum_fns = (analysisCommRank) ? NULL : new double [numDerivVars];
      parallelLib.reduce_sum_a(fnGrads[0], sum_fns, 
			       numDerivVars);
      if (analysisCommRank == 0) {
	//wjbVERIFY: Dakota::copy_data(sum_fns, static_cast<int>(numDerivVars), fnGrads[0]);
	Teuchos::setCol(
	  Teuchos::SerialDenseVector<int,double>(Teuchos::View, sum_fns,
                                                 numDerivVars), 0, fnGrads );
	delete [] sum_fns;
      }
    }
  }

  // ********************
  // **** d^2f/dx^2: ****
  // ********************
  if (directFnASV[0] & 4) {
    fnHessians[0] = 0.;
    //for (i=0; i<numDerivVars; i++)
    //  fnHessians[0][i][i] = 12.*std::pow(xC[i]-POW_VAL,2);
    for (i=analysisCommRank; i<numDerivVars; i+=analysisCommSize) {
      size_t var_index = directFnDVV[i] - 1;
      double x_i = xC[var_index]; // assumes no discrete vars
      fnHessians[0](i,i) = 12.*std::pow(x_i-POW_VAL,2);
    }
    if (multiProcAnalysisFlag) {
      int num_doubles = numDerivVars * numDerivVars;
      double* local_fns = new double [num_doubles];
      std::copy(fnHessians[0].values(), fnHessians[0].values() + num_doubles,
                local_fns);
      double* sum_fns = (analysisCommRank) ? NULL : new double [num_doubles];
      parallelLib.reduce_sum_a(local_fns, sum_fns, num_doubles);
      delete [] local_fns;
      if (analysisCommRank == 0) {
        std::copy(sum_fns, sum_fns + num_doubles, fnHessians[0].values());
	delete [] sum_fns;
      }
    }
  }

  // **********************************
  // **** c1: x[0]*x[0] - 0.5*x[1] ****
  // **********************************
  if (numFns > 1 && (directFnASV[1] & 1)) {
    double local_val = 0.0;
    // Definitely not the most efficient way to do this, but the point is to
    // demonstrate Comm communication.
    for (i=analysisCommRank; i<numVars; i+=analysisCommSize) {
      // orders all continuous vars followed by all discrete vars.  This is 
      // fine in the direct case so long as everything is self-consistent.
      double x_i;
      if (i<numACV)
	x_i = xC[i];
      else if (i<numACV+numADIV)
	x_i = (double)xDI[i-numACV];
      else
	x_i = xDR[i-numACV-numADIV];
      if (i==0) // could be changed to i % 2 == 0 to get even vars.
        local_val += x_i*x_i;
      else if (i==1) // could be changed to i % 2 == 1 to get odd vars
        local_val -= 0.5*x_i;
    }
    if (multiProcAnalysisFlag) {
      double global_val = 0.0;
      parallelLib.reduce_sum_a(&local_val, &global_val, 1);
      // only analysisCommRank 0 has the correct sum.  This is OK (MPI_Allreduce
      // not needed) since only analysisCommRank 0 updates response for 
      // evalCommRank 0 in overlay_response.  evalCommRank 0 then returns the 
      // results to the iterator in ApplicationInterface::serve_evaluations().
      if (analysisCommRank == 0)
	fnVals[1] = global_val;
    }
    else
      fnVals[1] = local_val;
  }

  // *****************
  // **** dc1/dx: ****
  // *****************
  if (numFns > 1 && (directFnASV[1] & 2)) {
    //fnGrads[1] = 0.;
    std::fill(fnGrads[1], fnGrads[1] + numDerivVars, 0.);
    //fnGrads[1][0] = 2.*xC[0];
    //fnGrads[1][1] = -0.5;
    for (i=analysisCommRank; i<numDerivVars; i+=analysisCommSize) {
      int var_index = directFnDVV[i] - 1; // assumes no discrete vars
      if (var_index == 0)
        fnGrads[1][i] = 2.*xC[0];
      else if (var_index == 1)
        fnGrads[1][i] = -0.5;
    }
    if (multiProcAnalysisFlag) {
      double* sum_fns = (analysisCommRank) ? NULL : new double [numDerivVars];
      parallelLib.reduce_sum_a(fnGrads[1], sum_fns,
			       numDerivVars);
      if (analysisCommRank == 0) {
	Teuchos::setCol(
	  Teuchos::SerialDenseVector<int,double>(Teuchos::View, sum_fns,
                                                 numDerivVars), 1, fnGrads );
	delete [] sum_fns;
      }
    }
  }

  // *********************
  // **** d^2c1/dx^2: ****
  // *********************
  if (numFns > 1 && (directFnASV[1] & 4)) {
    fnHessians[1] = 0.;
    //fnHessians[1][0][0] = 2.;
    for (i=analysisCommRank; i<numDerivVars; i+=analysisCommSize) {
      int var_index = directFnDVV[i] - 1; // assumes no discrete vars
      if (var_index == 0)
	fnHessians[1](i,i) = 2.;
    }
    if (multiProcAnalysisFlag) {
      int num_doubles = numDerivVars * numDerivVars;
      double* local_fns = new double [num_doubles];
      std::copy(fnHessians[1].values(), fnHessians[1].values() + num_doubles,
                local_fns);
      double* sum_fns = (analysisCommRank) ? NULL : new double [num_doubles];
      parallelLib.reduce_sum_a(local_fns, sum_fns, num_doubles);
      delete [] local_fns;
      if (analysisCommRank == 0) {
        std::copy(sum_fns, sum_fns + num_doubles, fnHessians[1].values());
	delete [] sum_fns;
      }
    }
  }

  // **********************************
  // **** c2: x[1]*x[1] - 0.5*x[0] ****
  // **********************************
  if (numFns > 2 && (directFnASV[2] & 1)) {
    double local_val = 0.0;
    // Definitely not the most efficient way to do this, but the point is to
    // demonstrate Comm communication.
    for (i=analysisCommRank; i<numVars; i+=analysisCommSize) {
      // orders all continuous vars followed by all discrete vars.  This is 
      // fine in the direct case so long as everything is self-consistent.
      double x_i;
      if (i<numACV)
	x_i = xC[i];
      else if (i<numACV+numADIV)
	x_i = (double)xDI[i-numACV];
      else
	x_i = xDR[i-numACV-numADIV];
      if (i==0) // could be changed to i % 2 == 0 to get even vars.
        local_val -= 0.5*x_i;
      else if (i==1) // could be changed to i % 2 == 1 to get odd vars
        local_val += x_i*x_i;
    }
    if (multiProcAnalysisFlag) {
      double global_val = 0.0;
      parallelLib.reduce_sum_a(&local_val, &global_val, 1);
      // only analysisCommRank 0 has the correct sum.  This is OK (MPI_Allreduce
      // not needed) since only analysisCommRank 0 updates response for 
      // evalCommRank 0 in overlay_response.  evalCommRank 0 then returns the 
      // results to the iterator in ApplicationInterface::serve_evaluations().
      if (analysisCommRank == 0)
	fnVals[2] = global_val;
    }
    else
      fnVals[2] = local_val;
  }

  // *****************
  // **** dc2/dx: ****
  // *****************
  if (numFns > 2 && (directFnASV[2] & 2)) {
    //fnGrads[2] = 0.;
    std::fill(fnGrads[2], fnGrads[2] + numDerivVars, 0.);
    //fnGrads[2][0] = -0.5;
    //fnGrads[2][1] = 2.*xC[1];
    for (i=analysisCommRank; i<numDerivVars; i+=analysisCommSize) {
      int var_index = directFnDVV[i] - 1; // assumes no discrete vars
      if (var_index == 0)
        fnGrads[2][i] = -0.5;
      else if (var_index == 1)
        fnGrads[2][i] = 2.*xC[1];
    }
    if (multiProcAnalysisFlag) {
      double* sum_fns = (analysisCommRank) ? NULL : new double [numDerivVars];
      parallelLib.reduce_sum_a(fnGrads[2], sum_fns,
			       numDerivVars);
      if (analysisCommRank == 0) {
	Teuchos::setCol(
	  Teuchos::SerialDenseVector<int,double>(Teuchos::View, sum_fns,
                                                 numDerivVars), 2, fnGrads );
	delete [] sum_fns;
      }
    }
  }

  // *********************
  // **** d^2c2/dx^2: ****
  // *********************
  if (numFns > 2 && (directFnASV[2] & 4)) {
    fnHessians[2] = 0.;
    //fnHessians[2][1][1] = 2.;
    for (i=analysisCommRank; i<numDerivVars; i+=analysisCommSize) {
      int var_index = directFnDVV[i] - 1; // assumes no discrete vars
      if (var_index == 1)
	fnHessians[2](i,i) = 2.;
    }
    if (multiProcAnalysisFlag) {
      int num_doubles = numDerivVars * numDerivVars;
      double* local_fns = new double [num_doubles];
      std::copy(fnHessians[2].values(), fnHessians[2].values() + num_doubles,
                local_fns);
      double* sum_fns = (analysisCommRank) ? NULL : new double [num_doubles];
      parallelLib.reduce_sum_a(local_fns, sum_fns, num_doubles);
      delete [] local_fns;
      if (analysisCommRank == 0) {
        std::copy(sum_fns, sum_fns + num_doubles, fnHessians[2].values());
	delete [] sum_fns;
      }
    }
  }

  // Failure capturing
  if (fail_code)
    throw fail_code;

  return 0;
}

} // namespace SIM
