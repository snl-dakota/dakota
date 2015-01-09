/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        MatlabInterface
//- Description:  Class implementation
//- Owner:        Brian Adams

// TODO: 
// * Index fields by name, not number to reduce errors
// * Error check here and everywhere there's a Matlab call
// * support input and output filters in Matlab

#include "MatlabInterface.hpp"
#include "DataMethod.hpp"
#include "engine.h"

namespace Dakota {

#ifndef MWSIZE_MAX
// Older Matlab versions used int for sizes.  Newer versions
// (7.3/R2006b and later) typedef mwSize as int or size_t in
// tmwtypes.h.  Provide definition for mwSize if necessary:
#define mwSize int
#endif

/** fields to pass to Matlab in Dakota structure */
const char *FIELD_NAMES[] = { "numFns", "numVars", "numACV", "numADIV",  // 0
			      "numADRV", "numDerivVars", "xC", "xDI",    // 4
			      "xDR", "xCLabels", "xDILabels",            // 8
			      "xDRLabels", "directFnASV", "directFnDVV", // 11
			      "fnFlag", "gradFlag", "hessFlag",          // 14
			      "fnVals",  "fnGrads",  "fnHessians",       // 17
			      "fnLabels", "failure", "currEvalId" };     // 20
/** number of fields in above structure */
const int NUMBER_OF_FIELDS = 23;
//const int NUMBER_OF_FIELDS = sizeof(FIELD_NAMES)/sizeof(char);


MatlabInterface::MatlabInterface(const ProblemDescDB& problem_db)
  : DirectApplicInterface(problem_db) 
{
  // TODO: this code should be unreachable and can be removed; test.
  if (analysisDrivers.empty()) {
    Cerr << "\nError: MATLAB interface requires analysis_drivers "
	 << "specification.\n" << std::endl;
    abort_handler(-1);
  }

  std::string engine_cmd;
  const char* env_engine_cmd = getenv("DAKOTA_MATLAB_ENGINE_CMD");
  // eventually just use _WIN32 and __CYGWIN__ here
#if defined(__CYGWIN__) || defined(_WIN32) || defined(__MINGW32__) || defined(_MSC_VER)
  if (env_engine_cmd)
    Cerr << "\nWarning: DAKOTA_MATLAB_ENGINE_CMD ignored on Windows."
	 << std::endl;
  engine_cmd = "\0";
#else
  // consider adding -nojvm here
  engine_cmd = env_engine_cmd ? env_engine_cmd :
    "matlab -nodesktop -nosplash";
#endif
  matlabEngine = engOpen(engine_cmd.c_str());
  if (matlabEngine) {
    if (outputLevel >= NORMAL_OUTPUT)
      Cout << "Matlab engine initialized for direct function evaluation."
	   << std::endl;
    const char * env_matlab_startup = getenv("DAKOTA_MATLAB_STARTUP");
    if (env_matlab_startup)
      engEvalString(matlabEngine, env_matlab_startup);      
  }
  else {
    Cerr << "Error (MatlabInterface): Could not initialize Matlab engine "
	 << "for direct fn. eval." << std::endl;
    abort_handler(-1);
  }

  // create the Matlab Dakota structure which will persist across evals
  mxArray *dakota_matlab = NULL;  // MATLAB Dakota structure
  /* fields to pass to Matlab in Dakota structure */
  const mwSize ndim = 2;
  const mwSize dims[2] = {1, 1};
  // push Dakota variable structure into the engine workspace
  dakota_matlab = mxCreateStructArray(ndim, dims, NUMBER_OF_FIELDS,
				      FIELD_NAMES);
  engPutVariable(matlabEngine, "Dakota", dakota_matlab);
}

MatlabInterface::~MatlabInterface()
{
  if (matlabEngine) {
    if ( engClose(matlabEngine) ) {
      Cerr << "Error (MatlabInterface): Couldn't terminate Matlab engine post "
	   << "direct fn. eval." << std::endl;
      abort_handler(-1);
    } 
    else if (outputLevel >= NORMAL_OUTPUT)
      Cout << "Matlab engine terminated." << std::endl;
  }
}


/// Matlab specialization of dervied analysis components
int MatlabInterface::derived_map_ac(const String& ac_name)
{
#ifdef MPI_DEBUG
    Cout << "analysis server " << analysisServerId << " invoking " << ac_name
         << " within MatlabInterface." << std::endl;
#endif // MPI_DEBUG

  int fail_code = matlab_engine_run(ac_name);

  // Failure capturing
  if (fail_code) {
    std::string err_msg("Error evaluating Matlab analysis_driver ");
    err_msg += ac_name;
    throw FunctionEvalFailure(err_msg);
  }
  
  return 0;
}


/** Direct interface to Matlab through Mathworks external API.  m-file
    executed is specified through analysis_drivers, extra strings
    through analysis_components.  (Original BMA 11/28/2005)

    Special thanks to Lee Peterson for substantial enhancements 12/15/2007:
    * Added output buffer for the MATLAB command response and error messages
    * Made the Dakota variable persistent in the MATLAB engine workspace
    * Added robustness to the user deleting required Dakota fields */
int MatlabInterface::matlab_engine_run(const String& ac_name)
{

  const int MATLAB_FAIL = 1;
  const int RESPONSE_TYPES[] = {1, 2, 4};
  int i, j, k, iF;

  /* variables for Matlab data type objects */
  mxArray *dakota_matlab = NULL;
  mxArray *mx_tmp[NUMBER_OF_FIELDS];
  mxArray *mx_string;

  /* temporary variables */
  mwSize dims[3];
  const mwSize *dim_ptr;
  double *ptr;
  std::string analysis_command;
  const int BUFSIZE = 32768;
  char buffer_char[BUFSIZE+1] = {'\0'};
  std::string buffer_string;
  
  int fail_code;
  bool fn_flag;
  
  // make sure the Matlab user did not corrupt the workspace variable Dakota
  if ( (dakota_matlab = engGetVariable(matlabEngine, "Dakota")) == NULL ) {
    Cerr << "Error (MatlabInterface): Failed to get variable Dakota from "
         << "Matlab." << std::endl;
    return(MATLAB_FAIL);
  }
  if ( mxIsEmpty(dakota_matlab) ) {
    Cerr << "Error (MatlabInterface): Dakota variable from Matlab "
         << "is empty. " << std::endl;
    return(MATLAB_FAIL);
  }
  if ( !mxIsStruct(dakota_matlab) ) {
    Cerr << "Error (MatlabInterface): Dakota variable from Matlab "
         << "is not a structure array. " << std::endl;
    return(MATLAB_FAIL);
  }
  if ( (mxGetM(dakota_matlab) != 1) | (mxGetN(dakota_matlab) != 1) ) {
    Cerr << "Error (MatlabInterface): Dakota variable from Matlab "
         << "is not a scalar. " << std::endl;
    return(MATLAB_FAIL);
  }

  /* Set scalar valued data */
  if ( (iF = matlab_field_prep(dakota_matlab, FIELD_NAMES[0])) >= 0)
    mxSetFieldByNumber(dakota_matlab,0,iF,
		       mxCreateDoubleScalar((double) numFns));
  else
    return(MATLAB_FAIL);
  
  if ( (iF = matlab_field_prep(dakota_matlab, FIELD_NAMES[1])) >= 0)
    mxSetFieldByNumber(dakota_matlab,0,iF,
		       mxCreateDoubleScalar((double) numVars));
  else
    return(MATLAB_FAIL);
  
  if ( (iF = matlab_field_prep(dakota_matlab, FIELD_NAMES[2])) >= 0)
    mxSetFieldByNumber(dakota_matlab,0,iF,
		       mxCreateDoubleScalar((double) numACV));
  else
    return(MATLAB_FAIL);
    
  if ( (iF = matlab_field_prep(dakota_matlab, FIELD_NAMES[3])) >= 0)
    mxSetFieldByNumber(dakota_matlab,0,iF,
		       mxCreateDoubleScalar((double) numADIV));
  else
    return(MATLAB_FAIL);

  if ( (iF = matlab_field_prep(dakota_matlab, FIELD_NAMES[4])) >= 0)
    mxSetFieldByNumber(dakota_matlab,0,iF,
		       mxCreateDoubleScalar((double) numADRV));
  else
    return(MATLAB_FAIL);
  
  if ( (iF = matlab_field_prep(dakota_matlab, FIELD_NAMES[5])) >= 0)
    mxSetFieldByNumber(dakota_matlab,0,iF, 
		       mxCreateDoubleScalar((double) numDerivVars));
  else
    return(MATLAB_FAIL);
  
  /* Create Matlab matrix, get pointer to it, populate, put in structure 
     NOTE that mxSetFieldByNumber needs to retain allocated memory -- it
     just points to the created mxArray mx_tmp[k], so don't deallocate mx_tmp. 
     MAY be able to use mxSetPr, depending on what types 
     RealVector, IntArray, StringArray are
  */

  /* continuous variables */
  if ( (iF = matlab_field_prep(dakota_matlab, FIELD_NAMES[6])) >= 0) {
    mx_tmp[6] = mxCreateDoubleMatrix(1,numACV,mxREAL);
    ptr = mxGetPr(mx_tmp[6]);
    for( i=0; i<numACV; ++i)  
      *(ptr+i) = xC[i];  
    mxSetFieldByNumber(dakota_matlab, 0, iF, mx_tmp[6]);
  }
  else
    return(MATLAB_FAIL);

  /* discrete integer variables */
  if ( (iF = matlab_field_prep(dakota_matlab, FIELD_NAMES[7])) >= 0) {
    mx_tmp[7] = mxCreateDoubleMatrix(1,numADIV,mxREAL);
    ptr = mxGetPr(mx_tmp[7]);
    for( i=0; i<numADIV; ++i)  
      *(ptr+i) = (double) xDI[i];
    mxSetFieldByNumber(dakota_matlab, 0, iF, mx_tmp[7]);
  }
  else
    return(MATLAB_FAIL);

  if ( (iF = matlab_field_prep(dakota_matlab, FIELD_NAMES[8])) >= 0) {
    mx_tmp[8] = mxCreateDoubleMatrix(1,numADRV,mxREAL);
    ptr = mxGetPr(mx_tmp[8]);
    for( i=0; i<numADRV; ++i)  
      *(ptr+i) = xDR[i];
    mxSetFieldByNumber(dakota_matlab, 0, iF, mx_tmp[8]);
  }
  else
    return(MATLAB_FAIL);

  /* continuous labels */
  if ( (iF = matlab_field_prep(dakota_matlab, FIELD_NAMES[9])) >= 0) {
    dims[0] = numACV;
    dims[1] = 1;
    mx_tmp[9] = mxCreateCellArray(2,dims);
    for( i=0; i<numACV; ++i)  
      mxSetCell(mx_tmp[9], i, mxCreateString( xCLabels[i].c_str() ));
    mxSetFieldByNumber(dakota_matlab, 0, iF, mx_tmp[9]);
  }
  else
    return(MATLAB_FAIL);

  /* discrete integer labels */
  if ( (iF = matlab_field_prep(dakota_matlab, FIELD_NAMES[10])) >= 0) {
    dims[0] = numADIV;
    dims[1] = 1;
    mx_tmp[10] = mxCreateCellArray(2,dims);
    for( i=0; i<numADIV; ++i)
      mxSetCell(mx_tmp[10], i, mxCreateString( xDILabels[i].c_str() ));
    mxSetFieldByNumber(dakota_matlab, 0, iF, mx_tmp[10]);
  }
  else
    return(MATLAB_FAIL);

  if ( (iF = matlab_field_prep(dakota_matlab, FIELD_NAMES[11])) >= 0) {
    dims[0] = numADRV;
    dims[1] = 1;
    mx_tmp[11] = mxCreateCellArray(2,dims);
    for( i=0; i<numADRV; ++i)
      mxSetCell(mx_tmp[11], i, mxCreateString( xDRLabels[i].c_str() ));
    mxSetFieldByNumber(dakota_matlab, 0, iF, mx_tmp[11]);
  }
  else
    return(MATLAB_FAIL);

  /* active set vector (ASV) */
  if ( (iF = matlab_field_prep(dakota_matlab, FIELD_NAMES[12])) >= 0) {
    mx_tmp[12] = mxCreateDoubleMatrix(1,numFns,mxREAL);
    for( i=0; i<numFns; ++i)
      *(mxGetPr(mx_tmp[12])+i) = directFnASV[i];  
    mxSetFieldByNumber(dakota_matlab, 0, iF, mx_tmp[12]);
  }
  else
    return(MATLAB_FAIL);
  
  /* derivative variables vector (DVV) */
  if ( (iF = matlab_field_prep(dakota_matlab, FIELD_NAMES[13])) >= 0) {
    mx_tmp[13] = mxCreateDoubleMatrix(1,numDerivVars,mxREAL);
    for( i=0; i<numDerivVars; ++i)
      *(mxGetPr(mx_tmp[13])+i) = directFnDVV[i];  
    mxSetFieldByNumber(dakota_matlab, 0, iF, mx_tmp[13]);
  }
  else
    return(MATLAB_FAIL);

  /* fn, grad, hess Flags; as needed, allocate & initialize matrices to zero */
  fn_flag = false;
  for ( j=0; j<numFns; ++j)
    if (directFnASV[j] & 1) {
      fn_flag = true;
      if ( (iF = matlab_field_prep(dakota_matlab, FIELD_NAMES[17])) >= 0) {
	mx_tmp[17] = mxCreateDoubleMatrix(1,numFns,mxREAL);
	mxSetFieldByNumber(dakota_matlab, 0, iF, mx_tmp[17]);
      }
      else 
	return(MATLAB_FAIL);
      break;
    }

  if (gradFlag) {
    if ( (iF = matlab_field_prep(dakota_matlab, FIELD_NAMES[18])) >= 0) {
      mx_tmp[18] = mxCreateDoubleMatrix(numFns,numDerivVars,mxREAL);
      mxSetFieldByNumber(dakota_matlab, 0, iF, mx_tmp[18]);
    }
    else	
      return(MATLAB_FAIL);
  }

  if (hessFlag) {
    if ( (iF = matlab_field_prep(dakota_matlab, FIELD_NAMES[19])) >= 0) {
      dims[0] = numFns;
      dims[1] = numDerivVars;
      dims[2] = numDerivVars;
      mx_tmp[19] = mxCreateNumericArray(3,dims,mxDOUBLE_CLASS,mxREAL);
      mxSetFieldByNumber(dakota_matlab, 0, iF, mx_tmp[19]);
    }
    else 
      return(MATLAB_FAIL);
  }
 
  if ( (iF = matlab_field_prep(dakota_matlab, FIELD_NAMES[14])) >= 0) {
    mxSetFieldByNumber(dakota_matlab,0,iF,
		       mxCreateDoubleScalar((double) fn_flag));
  }
  else	
    return(MATLAB_FAIL);
  
  if ( (iF = matlab_field_prep(dakota_matlab, FIELD_NAMES[15])) >= 0) {
    mxSetFieldByNumber(dakota_matlab,0,iF,
		       mxCreateDoubleScalar((double) gradFlag));
  }
  else	
    return(MATLAB_FAIL);

  if ( (iF = matlab_field_prep(dakota_matlab, FIELD_NAMES[16])) >= 0) {
    mxSetFieldByNumber(dakota_matlab,0,iF,
		       mxCreateDoubleScalar((double) hessFlag));
  }
  else	
    return(MATLAB_FAIL);

  if ( (iF = matlab_field_prep(dakota_matlab, FIELD_NAMES[22])) >= 0) {
    mxSetFieldByNumber(dakota_matlab,0,iF,
		       mxCreateDoubleScalar((double) currEvalId));
  }
  else	
    return(MATLAB_FAIL);
 
  // Put the structure into the Matlab workspace,
  engPutVariable(matlabEngine, "Dakota", dakota_matlab);
  buffer_char[BUFSIZE] = '\0';
  engOutputBuffer(matlabEngine, buffer_char, BUFSIZE);

  // The active analysis_driver is passed in ac_name.  We pass any
  // analysis components as string arguments to the m-function.
  // omit any trailing .m
  size_t pos = ac_name.find(".m", ac_name.size() - 2);
  std::string function_name = ac_name.substr(0,pos);
  if (function_name.size() == 0) {
    Cerr << "\nError: invalid Matlab function '" << ac_name
	 << "' specified in analysis_drivers." << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  // build up an analysis command to execute in Matlab:
  // Dakota = function_name(Dakota[, analysisComponents]);
  analysis_command = "Dakota = " + function_name + "(Dakota";
  // append any optional analysis component data as quoted strings
  if (analysisComponents.size() > 0)
    for (StringArray::const_iterator ac_it = 
	   analysisComponents[analysisDriverIndex].begin();
	 ac_it != analysisComponents[analysisDriverIndex].end(); ++ac_it)
      analysis_command += ", '" + *ac_it + "'";
  analysis_command += ");";
  
  engEvalString(matlabEngine, analysis_command.c_str());

  buffer_string = buffer_char;
  if (outputLevel > SILENT_OUTPUT && buffer_string.length() > 3)
    Cout << "Warning (MatlabInterface): Matlab function output:\n" 
	 << buffer_char+3 << std::endl;  
  
  /* retrieve and parse the response */
  dakota_matlab = NULL;
  if ( (dakota_matlab = engGetVariable(matlabEngine, "Dakota")) == NULL ) {
    Cerr << "Error (MatlabInterface): Failed to get variable Dakota from " 
         << "Matlab." << std::endl;
    fail_code = MATLAB_FAIL;
  } else if ( (mx_tmp[21] = mxGetField(dakota_matlab, 0, "failure")) != NULL &&
              mxGetClassID(mx_tmp[21]) == mxDOUBLE_CLASS && 
              *(mxGetPr(mx_tmp[21])) != 0 ) {

    /* Matlab user indicated failure, don't process */
    fail_code = (int) *(mxGetPr(mx_tmp[21]));

  } else {
    
    fail_code = 0;
    /* get fields by name in case the user somehow reordered, or decided to
       only return some of the fields to us... update all of
         fns:   1 x numFns
         grads: numFns * numDerivVars
         hess:  numFns * numDerivVars * numDerivVars
       if any of these come back wrong, set fail_code
    */    
     
    if (fn_flag)
      if ( (mx_tmp[17]=mxGetField(dakota_matlab, 0, "fnVals")) == NULL ) {
      
        Cerr << "Error (MatlabInterface): Failed to get field fnVals from " 
             << "Dakota structure." << std::endl;
        fail_code = MATLAB_FAIL;
      
      } else if ( mxGetM(mx_tmp[17]) != 1 | mxGetN(mx_tmp[17]) != numFns ) {
        
        Cerr << "Error (MatlabInterface): Dakota.fnVals must be [1 x numFns]." 
             << std::endl;    
        fail_code = MATLAB_FAIL;
      
      } else {
        
        ptr=mxGetPr(mx_tmp[17]);
        for (i=0; i<numFns; ++i)
          fnVals[i] = *(ptr + i);
      }   
  
    if (gradFlag) 
      if ( (mx_tmp[18]=mxGetField(dakota_matlab, 0, "fnGrads")) == NULL ) {
      
        Cerr << "Error (MatlabInterface): Failed to get field fnGrads from " 
             << "Dakota structure." << std::endl;
        fail_code = MATLAB_FAIL;     
      
      } else if ( mxGetM(mx_tmp[18]) != numFns |
		  mxGetN(mx_tmp[18]) != numDerivVars ) {
      
        Cerr << "Error (MatlabInterface): Dakota.fnVals must be "
             << "[numFns x numDerivVars]." << std::endl;    
        fail_code = MATLAB_FAIL;
      
      } else  {

	// Matlab stores column major
        ptr=mxGetPr(mx_tmp[18]);
        for (i=0; i<numFns; ++i)
          for (j=0; j<numDerivVars; ++j)
            fnGrads(j,i) = *(ptr + j*numFns + i);
      
      }
  
    if (hessFlag) 
      if ( (mx_tmp[19]=mxGetField(dakota_matlab, 0, "fnHessians")) == NULL ) {
        
        Cerr << "Error (MatlabInterface): Failed to get field fnHessians from " 
             << "Dakota structure." << std::endl;
        fail_code = MATLAB_FAIL;
      
      } else if ( mxGetNumberOfDimensions(mx_tmp[19]) != 3 ) { 
        
        Cerr << "Error (MatlabInterface): Dakota.fnHessians must be "
             << "3 dimensional." << std::endl;    
        fail_code = MATLAB_FAIL;
        
      } else {
        
        dim_ptr = mxGetDimensions(mx_tmp[19]);
        if ( dim_ptr[0] != numFns | dim_ptr[1] != numDerivVars | 
             dim_ptr[2] != numDerivVars ) {
          
          Cerr << "Error (MatlabInterface): Dakota.fnHessians must be "
               << "[numFns x numDerivVars x numDerivVars]." << std::endl;     
          fail_code = MATLAB_FAIL;
          
        } else {
          
	  // Matlab stores column major
          ptr=mxGetPr(mx_tmp[19]);
          for (i=0; i<numFns; ++i)
            for (j=0; j<numDerivVars; ++j)
              for (k=0; k<=j; ++k)
                fnHessians[i](j,k) = 
		  *(ptr + k*numFns*numDerivVars + j*numFns + i);
        }
      
      }
    
    /* get fnLabels -- optional return value */
    if ( (mx_tmp[20]=mxGetField(dakota_matlab, 0, "fnLabels")) != NULL )
      if ( mxGetClassID(mx_tmp[20]) == mxCELL_CLASS &&
           mxGetNumberOfElements(mx_tmp[20]) == numFns )
        for (i=0; i<numFns; ++i) {
        
          mx_string = mxGetCell(mx_tmp[20], i);  
          if ( mxGetClassID(mx_string) != mxCHAR_CLASS ) 
            Cout << "Warning (MatlabInterface): Dakota.fnLabels{" << i
                 << "} is not a string.  Not returning fnLabel " << i 
                 << "." << std::endl;     
          else {
            
            mxGetString(mx_string, buffer_char, 256);
            fnLabels[i] = buffer_char;
        
          }
        }
   
  } /* end else parse response */

  return(fail_code);    

}

int MatlabInterface::matlab_field_prep(mxArray* dakota_matlab,
					       const char* field_name)
{
  int iF;
  if ( (iF=mxGetFieldNumber(dakota_matlab, field_name)) == -1 )
    if ( (iF=mxAddField(dakota_matlab, field_name)) == -1 ) {
      Cerr << "Error (MatlabInterface): Cannot init field Dakota." << field_name
	   << "."<< std::endl;
      return(iF);
    }
  mxDestroyArray(mxGetFieldByNumber(dakota_matlab,0,iF));
  return(iF);
}


} // namespace Dakota
