/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        ScilabInterface
//- Description:  Class implementation
//- Owner:        Brian Adams

#include "ScilabInterface.hpp"
#include "DataMethod.hpp"

#include <api_scilab.h>
#include <call_scilab.h>
#include <string.h>

namespace Dakota {

/** fields to pass to Scilab in Dakota structure */
const char *SCI_FIELD_NAMES[] = { "dakota_type", "numFns", "numVars", "numACV", "numADIV",  // 0
				  "numADRV", "numDerivVars", "xC", "xDI",    // 5
				  "xDR", "xCLabels", "xDILabels",            // 9
				  "xDRLabels", "directFnASV", "directFnASM", // 12
				  "directFnDVV", "directFnDVV_bool",         // 15
				  "fnFlag", "gradFlag", "hessFlag",          // 17
				  "fnVals",  "fnGrads",  "fnHessians",       // 20
				  "fnLabels", "failure", "currEvalId" };       // 23
/** number of fields in above structure */
const int SCI_NUMBER_OF_FIELDS = 26;


ScilabInterface::ScilabInterface(const ProblemDescDB& problem_db)
  : DirectApplicInterface(problem_db)
{
  // Scilab Initialisation
#ifdef _MSC_VER
  scilabEngine = Call_ScilabOpen(NULL,TRUE,NULL,NULL);
#else
  scilabEngine = Call_ScilabOpen(getenv("SCI"),TRUE,NULL,NULL);
#endif
  if ( scilabEngine !=0 ) {
    Cerr << "Error (ScilabInterface): Could not initialize Scilab engine "
	 << "for direct fn. eval." << std::endl;
    abort_handler(-1);
  }
  else {
    if (outputLevel >= NORMAL_OUTPUT)
      Cout << "Scilab engine initialized for direct function evaluation."
	   << std::endl;
    char * env_scilab_startup = getenv("DAKOTA_SCILAB_STARTUP");
    if (env_scilab_startup) SendScilabJob(env_scilab_startup);      
  }
}


ScilabInterface::~ScilabInterface()
{
  if (scilabEngine == 0)
    if (TerminateScilab(NULL) == FALSE) {
      Cerr << "Error (ScilabInterface): Couldn't terminate Scilab engine post "
	   << "direct fn. eval." << std::endl;
      abort_handler(-1);
    } 
    else if (outputLevel >= NORMAL_OUTPUT) {
      Cout << "Scilab engine terminated." << std::endl;
    }
}

int ScilabInterface::derived_map_ac(const String& ac_name)
{
#ifdef MPI_DEBUG
    Cout << "analysis server " << analysisServerId << " invoking " << ac_name
         << " within MatlabInterface." << std::endl;
#endif // MPI_DEBUG

  int fail_code = scilab_engine_run(ac_name);

  if (fail_code) {
    std::string err_msg("Error evaluating Scilab analysis_driver ");
    err_msg += ac_name;
    throw FunctionEvalFailure(err_msg);
  }

  return 0;
}


int ScilabInterface::scilab_engine_run(const String& ac_name)
{
  // direct interface to Scilab 

  const int SCILAB_FAIL = 1;
  const int RESPONSE_TYPES[] = {1, 2, 4};
  int i, j, k;

  /* temporary variables */
  double * x_tmp = NULL, dbl_tmp;
  std::string analysis_command;
  
  int fail_code;
  bool fn_flag;
  
  SciErr sci_err;
  int * piMListAddr = NULL, * piMListAddrHess = NULL, * piLength = NULL;
  int piRows, piCols, piNbItem;
  double * pdblReal = NULL;
  char ** pstStrings = NULL;

  // Create Scilab mlist to store data from DAKOTA
  sci_err = createNamedMList(pvApiCtx, "Dakota", SCI_NUMBER_OF_FIELDS, 
			     &piMListAddr);
  if (sci_err.iErr) {
    Cerr << "Error (ScilabInterface): Failed creating mlist Dakota."<< std::endl;
    printError(&sci_err, 0);
    return (SCILAB_FAIL);
  }

  // Insert the list of field names
  sci_err = 
    createMatrixOfStringInNamedList(pvApiCtx, "Dakota", piMListAddr, 1, 
				    1, SCI_NUMBER_OF_FIELDS, SCI_FIELD_NAMES);
 if (sci_err.iErr) {
    Cerr << "Error (ScilabInterface): Failed inserting field names" << std::endl;
    printError(&sci_err, 0);
    return (SCILAB_FAIL);
  }
  
  // Insert numFns
  dbl_tmp = numFns;
  sci_err = 
    createMatrixOfDoubleInNamedList(pvApiCtx, "Dakota", piMListAddr, 2, 
				    1, 1, &dbl_tmp);
  if (sci_err.iErr) {
    Cerr << "Error (ScilabInterface): Failed inserting numFns" << std::endl;
    printError(&sci_err, 0);
    return (SCILAB_FAIL);
  }
	  
  // numVars
  dbl_tmp = numVars;
  sci_err = 
    createMatrixOfDoubleInNamedList(pvApiCtx, "Dakota", piMListAddr, 3,
				    1, 1, &dbl_tmp);
  if (sci_err.iErr) {
    Cerr << "Error (ScilabInterface): Failed inserting numVars" << std::endl;
    printError(&sci_err, 0);
    return (SCILAB_FAIL);
  }
	  
  // numACV
  dbl_tmp = numACV;
  sci_err = 
    createMatrixOfDoubleInNamedList(pvApiCtx, "Dakota", piMListAddr, 4,
				    1, 1, &dbl_tmp);
  if (sci_err.iErr) {
    Cerr << "Error (ScilabInterface): Failed inserting numACV" << std::endl;
    printError(&sci_err, 0);
    return (SCILAB_FAIL);
  }
	  
  // numADIV
  dbl_tmp = numADIV;
  sci_err = 
    createMatrixOfDoubleInNamedList(pvApiCtx, "Dakota", piMListAddr, 5,
				    1, 1, &dbl_tmp);
  if (sci_err.iErr) {
    Cerr << "Error (ScilabInterface): Failed inserting numADIV" << std::endl;
    printError(&sci_err, 0);
    return (SCILAB_FAIL);
  }

  // numADRV
  dbl_tmp = numADRV;
  sci_err = 
    createMatrixOfDoubleInNamedList(pvApiCtx, "Dakota", piMListAddr, 6,
				    1, 1, &dbl_tmp);
  if (sci_err.iErr) {
    Cerr << "Error (ScilabInterface): Failed inserting numADRV" << std::endl;
    printError(&sci_err, 0);
    return (SCILAB_FAIL);
  }
	  
  // numDerivVars
  dbl_tmp = numDerivVars;
  sci_err = 
    createMatrixOfDoubleInNamedList(pvApiCtx,"Dakota", piMListAddr, 7,
				    1, 1, &dbl_tmp);
  if (sci_err.iErr) {
    Cerr << "Error (ScilabInterface): Failed inserting numDerivVars" << std::endl;
    printError(&sci_err, 0);
    return (SCILAB_FAIL);
  }
	  
  // xC (continuous variables)
  sci_err = 
    createMatrixOfDoubleInNamedList(pvApiCtx, "Dakota", piMListAddr, 8,
				    1, numACV, xC.values());
  if (sci_err.iErr) {
    Cerr << "Error (ScilabInterface): Failed inserting xC" << std::endl;
    printError(&sci_err, 0);
    return (SCILAB_FAIL);
  }

  // xDI (discrete integer variables)
  // create corresponding RealVector from IntVector (could eliminate)
  RealVector xdi_tmp(numADIV);
  std::copy(xDI.values(), xDI.values() + numADIV, xdi_tmp.values());
  sci_err = 
    createMatrixOfDoubleInNamedList(pvApiCtx, "Dakota", piMListAddr, 9, 
				    1, numADIV, xdi_tmp.values());
  if (sci_err.iErr) {
    Cerr << "Error (ScilabInterface): Failed inserting xDI" << std::endl;
    printError(&sci_err, 0);
    return (SCILAB_FAIL);
  }

  // xDR (discrete real variables)
  sci_err = createMatrixOfDoubleInNamedList(pvApiCtx, "Dakota", piMListAddr, 10,
					    1, numADRV, xDR.values());
  if (sci_err.iErr) {
    Cerr << "Error (ScilabInterface): Failed inserting xDR" << std::endl;
    printError(&sci_err, 0);
    return (SCILAB_FAIL);
  }

  // xCLabels (continuous labels)
  char ** pstStringsTmp = NULL;
  pstStringsTmp = (char **) malloc (sizeof(char*) * xCLabels.size());
  for(i = 0; i < xCLabels.size(); i++) {
    pstStringsTmp[i] = (char *)xCLabels[i].c_str();
  }
  sci_err = 
    createMatrixOfStringInNamedList(pvApiCtx, "Dakota", piMListAddr, 11,
				    1, xCLabels.size(), pstStringsTmp);
  free(pstStringsTmp);
  if (sci_err.iErr) {
    Cerr << "Error (ScilabInterface): Failed inserting xCLabels" << std::endl;
    printError(&sci_err, 0);
    return (SCILAB_FAIL);
  }
 
  // xDILabels (discrete integer labels)
  pstStringsTmp = (char **) malloc (sizeof(char*) * xDILabels.size());
  for(i = 0 ; i < xDILabels.size() ; i++) {
    pstStringsTmp[i] = (char *)xDILabels[i].c_str();
  }
  sci_err = 
    createMatrixOfStringInNamedList(pvApiCtx, "Dakota", piMListAddr, 12,
				    1, xDILabels.size(), pstStringsTmp);
  free(pstStringsTmp);
  if (sci_err.iErr) {
    Cerr << "Error (ScilabInterface): Failed inserting xDILabels" << std::endl;
    printError(&sci_err, 0);
    return (SCILAB_FAIL);
  }

  // xDRLabels
  pstStringsTmp = (char **) malloc (sizeof(char) * xDRLabels.size());
  for(i = 0 ; i < xDRLabels.size() ; i++) {
    pstStringsTmp[i] = (char *)xDRLabels[i].c_str();
  }
  sci_err = 
    createMatrixOfStringInNamedList(pvApiCtx, "Dakota", piMListAddr, 13,
				    1, xDRLabels.size(), pstStringsTmp);
  free(pstStringsTmp);
  if (sci_err.iErr) {
    Cerr << "Error (ScilabInterface): Failed inserting xDRLabels" << std::endl;
    printError(&sci_err, 0);
    return (SCILAB_FAIL);
  }

  // directFnASV (active set vector)
  RealVector asv_tmp(directFnASV.size());
  std::copy(directFnASV.begin(), directFnASV.end(), asv_tmp.values());
  sci_err = 
    createMatrixOfDoubleInNamedList(pvApiCtx, "Dakota", piMListAddr, 14,
				    1, numFns, asv_tmp.values());
  if (sci_err.iErr) {
    Cerr << "Error (ScilabInterface): Failed inserting directFnASV" << std::endl;
    printError(&sci_err, 0);
    return (SCILAB_FAIL);
  }
	  
  /* Create boolean version of ASV.  Rows are fnval, grad, hess; col per fn
     CAREFUL -- Matlab stores by column */
  // directFnASM
  x_tmp = (double *)malloc(sizeof(double) * 3 * numFns); // YC
  // x_tmp = new double[3 * numFns];
  for( i=0; i<3; i++) {
    for ( j=0; j<numFns; j++) {
      // FIX THIS MESS!
      (directFnASV[j] & RESPONSE_TYPES[i])
	? (x_tmp[3*j+i] = 1)
	: (x_tmp[3*j+i] = 0);
    }
  }
  sci_err = 
    createMatrixOfDoubleInNamedList(pvApiCtx, "Dakota", piMListAddr, 15,
				    3, numFns, x_tmp);
  free(x_tmp);
  //delete [] x_tmp;
  if (sci_err.iErr) {
    Cerr << "Error (ScilabInterface): Failed inserting directFnASM" << std::endl;
    printError(&sci_err, 0);
    return (SCILAB_FAIL);
  }
	  
  // directFnDVV (derivative variables vector)
  RealVector dvv_tmp(directFnDVV.size());
  std::copy(directFnDVV.begin(), directFnDVV.end(), dvv_tmp.values());
  sci_err = 
    createMatrixOfDoubleInNamedList(pvApiCtx, "Dakota", piMListAddr, 16,
				    1, numDerivVars, dvv_tmp.values());
  if (sci_err.iErr) {
    Cerr << "Error (ScilabInterface): Failed inserting directFnDVV" << std::endl;
    printError(&sci_err, 0);
    return (SCILAB_FAIL);
  }

  // directFnDVV_bool
  x_tmp = (double *)malloc(sizeof(double) * numDerivVars); // YC
  // x_tmp = new double[numDerivVars];
  for ( j=0; j<numDerivVars; j++) {
    x_tmp[directFnDVV[j] - 1] = 1;
  }
  sci_err = 
    createMatrixOfDoubleInNamedList(pvApiCtx, "Dakota", piMListAddr, 17,
				    1, numDerivVars, x_tmp);
  free(x_tmp);
  //delete [] x_tmp;
  if (sci_err.iErr) {
    Cerr << "Error (ScilabInterface): Failed inserting directFnDVV_bool" 
	 << std::endl;
    printError(&sci_err, 0);
    return (SCILAB_FAIL);
  }

  // fnFlag
  fn_flag = false;
  for (j=0; j<numFns; ++j) {
    if (directFnASV[j] & 1) {
      fn_flag = true;
      break;
    }
  }
  dbl_tmp = fn_flag;
  sci_err = 
    createMatrixOfDoubleInNamedList(pvApiCtx, "Dakota", piMListAddr, 18,
				    1, 1, &dbl_tmp);
  if (sci_err.iErr) {
    Cerr << "Error (ScilabInterface): Failed inserting fnFlag" << std::endl;
    printError(&sci_err, 0);
    return (SCILAB_FAIL);
  }
  
  // gradFlag
  dbl_tmp = gradFlag;
  sci_err = 
    createMatrixOfDoubleInNamedList(pvApiCtx, "Dakota", piMListAddr, 19,
				    1, 1, &dbl_tmp);
  if (sci_err.iErr) {
    Cerr << "Error (ScilabInterface): Failed inserting gradFlag" << std::endl;
    printError(&sci_err, 0);
    return (SCILAB_FAIL);
  }
  
  // hessFlag
  dbl_tmp = hessFlag;
  sci_err = 
    createMatrixOfDoubleInNamedList(pvApiCtx, "Dakota", piMListAddr, 20,
				    1, 1, &dbl_tmp);
  if (sci_err.iErr) {
    Cerr << "Error (ScilabInterface): Failed inserting hessFlag" << std::endl;
    printError(&sci_err, 0);
    return (SCILAB_FAIL);
  }

  /* fn, grad, hess Flags; as needed, allocate & initialize matrices to zero 
     DAKOTA already sizes and initializes fnVals/fnGrads, so just pass in.
     BMA: Is it necessary to create of size zero if not used? */

  // fnVals
  if (fn_flag) {
    sci_err = 
      createMatrixOfDoubleInNamedList(pvApiCtx, "Dakota", piMListAddr, 21,
				      1, numFns, fnVals.values());
  }
  else {
    dbl_tmp = 0;
    sci_err = 
      createMatrixOfDoubleInNamedList(pvApiCtx, "Dakota", piMListAddr, 21, 
				      0, 0, &dbl_tmp);
  }
  if (sci_err.iErr) {
    Cerr << "Error (ScilabInterface): Failed inserting fnVals" << std::endl;
    printError(&sci_err, 0);
    return (SCILAB_FAIL);
  }

  // fnGrads: shouldn't this be a matrix in SciLab, not a vector?
  if (gradFlag) {
    sci_err = 
      createMatrixOfDoubleInNamedList(pvApiCtx, "Dakota", piMListAddr, 22, 
				      1, numFns * numDerivVars, fnGrads.values());
  }
  else {
    dbl_tmp = 0;
    sci_err = 
      createMatrixOfDoubleInNamedList(pvApiCtx, "Dakota", piMListAddr, 22,
				      0, 0, &dbl_tmp);
  }
  if (sci_err.iErr) {
    Cerr << "Error (ScilabInterface): Failed inserting fnGrads" << std::endl;
    printError(&sci_err, 0);
    return (SCILAB_FAIL);
  }

  // fnHessians: this appears to be an oversight (empty in both
  // branches, whereas sized in cases above...)
  if (hessFlag) {
    int * piListInMList = NULL;
    sci_err = createListInNamedList(pvApiCtx, "Dakota", piMListAddr, 23, 
				    numFns, &piListInMList);
    if (sci_err.iErr) {
      Cerr << "Error (ScilabInterface): Failed inserting fnHessians list" 
	   << std::endl;
      printError(&sci_err,0);
      return (SCILAB_FAIL);
    }
    for(i=0; i<numFns; i++) {
      dbl_tmp = 0;
      sci_err = 
	createMatrixOfDoubleInNamedList(pvApiCtx, "Dakota", piListInMList, 23,
					0, 0, &dbl_tmp);
      if (sci_err.iErr) {
	Cerr << "Error (ScilabInterface): Failed inserting fnHessians" 
	     << std::endl;
	printError(&sci_err,0);
	return (SCILAB_FAIL);
      }
    }
  }
  else {
    dbl_tmp = 0;
    sci_err = 
      createMatrixOfDoubleInNamedList(pvApiCtx, "Dakota", piMListAddr, 23,
				      0, 0, &dbl_tmp);
    if (sci_err.iErr) {
      Cerr << "Error (ScilabInterface): Failed inserting fnHessians" 
	   << std::endl;
      printError(&sci_err, 0);
      return (SCILAB_FAIL);
    }
  }
  
  // fnLabels 
  pstStringsTmp = (char **) malloc (sizeof(char*) * xDRLabels.size());
  for(i=0 ; i<xDRLabels.size(); i++) {
    pstStringsTmp[i] = (char *)xDRLabels[i].c_str();
  }
  sci_err = 
    createMatrixOfStringInNamedList(pvApiCtx, "Dakota", piMListAddr, 24,
				    1, xDRLabels.size(), pstStringsTmp);
  free(pstStringsTmp);
  if (sci_err.iErr) {
    Cerr << "Error (ScilabInterface): Failed inserting fnLabels" << std::endl;
    printError(&sci_err, 0);
    return (SCILAB_FAIL);
  }
  
  // failure
  dbl_tmp = 0;
  sci_err = 
    createMatrixOfDoubleInNamedList(pvApiCtx, "Dakota", piMListAddr, 25,
				    1, 1, &dbl_tmp);
  if (sci_err.iErr) {
    Cerr << "Error (ScilabInterface): Failed inserting failure field" << std::endl;
    printError(&sci_err,0);
    return (SCILAB_FAIL);
  }

  // currEvalId
  dbl_tmp = currEvalId;
  sci_err = 
    createMatrixOfDoubleInNamedList(pvApiCtx, "Dakota", piMListAddr,26,
				    1, 1, &dbl_tmp);
  if (sci_err.iErr) {
    Cerr << "Error (ScilabInterface): Failed inserting currEvalId field" 
	 << std::endl;
    printError(&sci_err, 0);
    return (SCILAB_FAIL);
  }
  
  /* put the structure into the Scilab workspace, then 
     iterate over provided analysis components, checking for error each time */
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Calling Scilab for mapping..." << std::endl;
  int testJob;

  // The active analysis_driver is passed in ac_name.  We pass any
  // analysis components as string arguments to the Scilab function.
  // strip away any .sci the user might have included
  size_t pos = ac_name.find(".sci", ac_name.size() - 4);
  std::string function_name = ac_name.substr(0,pos);
  if (function_name.size() == 0) {
    Cerr << "\nError: invalid Scilab function '" << ac_name
	 << "' specified in analysis_drivers." << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  // build up an analysis command to execute in Matlab:
  // Dakota = function_name(Dakota[, analysisComponents]);
  analysis_command = "Dakota_out = " + function_name + "(Dakota";
  // append any optional analysis component data as quoted strings
  if (analysisComponents.size() > 0)
    for (StringArray::const_iterator ac_it = 
	   analysisComponents[analysisDriverIndex].begin();
	 ac_it != analysisComponents[analysisDriverIndex].end(); ++ac_it)
      analysis_command += ", '" + *ac_it + "'";
  analysis_command += ");";

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Scilab command: " << analysis_command << std::endl;

  testJob = SendScilabJob((char *)analysis_command.c_str()); 

  char * env_scilab_postevalcmd = getenv("DAKOTA_SCILAB_POSTEVALCMD");
  if (env_scilab_postevalcmd) 
    SendScilabJob(env_scilab_postevalcmd); 

  if (testJob == -1) {
    Cerr << "Error (ScilabInterface): Call Scilab has not been able to write "
	 << "the job into Scilab." << std::endl;
    return (SCILAB_FAIL);
  }
  else if (testJob == -2 | testJob == -3) {
    Cerr << "Error (ScilabInterface): Call Scilab has not been able to read "
	 << "the error code from Scilab." << std::endl;
    return (SCILAB_FAIL);
  }
  else if (testJob == -4) {
    Cerr << "Error (ScilabInterface): Call Scilab has not been able to " 
	 << "allocate the job Scilab." << std::endl;
    return (SCILAB_FAIL);
  }
  
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Retreiving Dakota_out variable from Scilab..." << std::endl;
  sci_err = readNamedMList(pvApiCtx, "Dakota_out", &piNbItem, &piMListAddr);
  if (sci_err.iErr) {
    Cerr << "Error (ScilabInterface): Failed to get variable Dakota_out from "
	 << "Scilab." << std::endl;
    printError(&sci_err, 0);
    return (SCILAB_FAIL);
  }
  

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Reading/verifying the lables from Scilab..." << std::endl;
  sci_err = 
    readMatrixOfStringInNamedList(pvApiCtx, "Dakota_out", piMListAddr, 1, 
				  &piRows, &piCols, NULL, NULL);
  piLength = (int*) malloc (sizeof(int) * piRows * piCols);
  sci_err = 
    readMatrixOfStringInNamedList(pvApiCtx, "Dakota_out", piMListAddr, 1, 
				  &piRows, &piCols, piLength, NULL);
  pstStrings = (char **) malloc (sizeof(char*) * piRows * piCols);
  for(i = 0 ; i < piRows * piCols ; i++) {
    pstStrings[i] = (char*)malloc(sizeof(char) * (piLength[i] + 1));
  }
  sci_err =
    readMatrixOfStringInNamedList(pvApiCtx, "Dakota_out", piMListAddr, 1,
				  &piRows, &piCols, piLength, pstStrings);
	   if (strcmp(pstStrings[0],SCI_FIELD_NAMES[0]) != 0) {
    Cerr << "Error (ScilabInterface): Returned fields don't match." << std::endl;
    return (SCILAB_FAIL);
  }
  // free temporary memory
  for(i = 0 ; i < piRows * piCols ; i++) 
    free(pstStrings[i]);
  free(pstStrings);
  free(piLength);


  // Failure
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Retreiving fail code from Scilab..." << std::endl;
  sci_err = 
    readMatrixOfDoubleInNamedList(pvApiCtx, "Dakota_out", piMListAddr, 25, 
				  &piRows, &piCols, NULL);
  x_tmp = (double*) malloc (sizeof(double) * piRows * piCols);
  sci_err = 
    readMatrixOfDoubleInNamedList(pvApiCtx, "Dakota_out", piMListAddr, 25,
				  &piRows, &piCols, x_tmp);
  
  if (sci_err.iErr) {
    printError(&sci_err, 0);
    return (SCILAB_FAIL);
  }
  
  if( *x_tmp != 0 ) {
    /* Matlab user indicated failure, don't process */
    fail_code = (int) *x_tmp;
  }
  else {
    fail_code = 0;
    /* get fields by name in case the user somehow reordered, or decided to
       only return some of the fields to us... update all of
       fns:   1 x numFns
       grads: numFns * numDerivVars
       hess:  numFns * numDerivVars * numDerivVars
       if any of these come back wrong, set fail_code
    */    
  }
  free(x_tmp);


  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Retreiving fnVals from Scilab..." << std::endl;
  if (fn_flag) {
    sci_err = 
      readMatrixOfDoubleInNamedList(pvApiCtx,"Dakota_out", piMListAddr, 21, 
				    &piRows, &piCols, NULL);
      
    x_tmp = (double*) malloc (sizeof(double) * piRows * piCols);
      
    sci_err = 
      readMatrixOfDoubleInNamedList(pvApiCtx, "Dakota_out", piMListAddr, 21, 
				      &piRows, &piCols, x_tmp);
      
#ifdef SCILAB_DEBUG
    for(i=0;i<piRows*piCols;i++) {
      printf("fn[%d] = %f\n",i,x_tmp[i]);
    }
#endif
      
    if (x_tmp == NULL) {
      Cerr << "Error (ScilabInterface): Failed to get field fnVals from " 
	   << "Dakota structure." << std::endl;
      fail_code = SCILAB_FAIL;
    }
    else if (piRows != 1 | piCols != numFns ) {
      Cerr << "Error (ScilabInterface): Dakota.fnVals must be [1 x numFns]." 
	   << std::endl;    
      fail_code = SCILAB_FAIL;
    }
    else {
      for (i=0; i<numFns; ++i) {
	fnVals[i] += x_tmp[i];
      }
    }
    free(x_tmp);
  }
  
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Retreiving fnGrads from Scilab..." << std::endl;

  if (gradFlag) {
    sci_err = 
      readMatrixOfDoubleInNamedList(pvApiCtx,"Dakota_out", piMListAddr, 22, 
				    &piRows, &piCols, NULL);
      
    x_tmp = (double*) malloc (sizeof(double) * piRows * piCols);
      
    sci_err = 
      readMatrixOfDoubleInNamedList(pvApiCtx, "Dakota_out", piMListAddr, 22, 
				    &piRows, &piCols, x_tmp);
    if (sci_err.iErr) {
      printError(&sci_err, 0);
      return (SCILAB_FAIL);
    }
#ifdef SCILAB_DEBUG
    for(i=0;i<piRows*piCols;i++) {
      printf("grad[%d] = %f\n",i,x_tmp[i]);
    }
#endif 
    if ( piRows != numFns | piCols != numDerivVars ) {
      Cerr << "Error (ScilabInterface): Dakota.fnVals must be "
	   << "[numFns x numDerivVars]." << std::endl;    
      fail_code = SCILAB_FAIL;
    }
    else {
      for (i=0; i<numFns; ++i) {
	for (j=0; j<numDerivVars; ++j) {
	  fnGrads(j,i) += x_tmp[numFns*j + i];
	}
      }
    }
    free(x_tmp);
  }
  
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Retreiving fnHessians from Scilab..." << std::endl;
  if (hessFlag) { 
     
    sci_err = getMListInList(pvApiCtx, piMListAddr, 23, &piMListAddrHess);
    
    sci_err = getVarDimension(pvApiCtx, piMListAddrHess, &piRows, &piCols);

    if (piRows != numFns | piCols != numDerivVars) {
      Cerr << "Error (ScilabInterface): Dakota.fnVals must be "
	   << "[numFns x numDerivVars]." << std::endl;    
      fail_code = SCILAB_FAIL;
    }
    else {
      for(i=0; i<numFns; i++) {
	pdblReal =  (double *)malloc (sizeof(double) * piRows * piCols); // YC
	sci_err = 
	  readMatrixOfDoubleInNamedList(pvApiCtx, "Dakota_out", piMListAddrHess, 
					i+1, &piRows, &piCols, pdblReal);
	if (sci_err.iErr) {
	  printError(&sci_err, 0);
	  return (SCILAB_FAIL);
	}
	for (j=0; j<numDerivVars; ++j) {
	  for (k=0; k<numDerivVars; ++k)  {
	    fnHessians[i](j,k) += pdblReal[numDerivVars*numFns*k + numFns*j + i];
	  }
	}
      }
    }
  }
     
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Retreiving fnLabels from Scilab..." << std::endl;
  sci_err = 
    readMatrixOfStringInNamedList(pvApiCtx, "Dakota_out", piMListAddr, 24, 
				  &piRows, &piCols, NULL, NULL);
  if (sci_err.iErr) {
    printError(&sci_err, 0);
    return (SCILAB_FAIL);
  }
  piLength = (int*) malloc (sizeof(int) * piRows * piCols);
  
  sci_err = 
    readMatrixOfStringInNamedList(pvApiCtx, "Dakota_out", piMListAddr, 24, 
				  &piRows, &piCols, piLength, NULL);
  if (sci_err.iErr) {
    printError(&sci_err, 0);
    return (SCILAB_FAIL);
  }
  pstStrings = (char **) malloc (sizeof(char*) * piRows * piCols);
  
  for(i = 0 ; i < piRows * piCols ; i++) {
    pstStrings[i] = (char*)malloc(sizeof(char) * (piLength[i] + 1));
  }
  
  sci_err = 
    readMatrixOfStringInNamedList(pvApiCtx, "Dakota_out", piMListAddr, 24,
				  &piRows, &piCols, piLength, pstStrings);
  if (sci_err.iErr) {
    printError(&sci_err, 0);
    return (SCILAB_FAIL);
  }
  for (i=0; i<numFns; ++i) {
    fnLabels[i] = pstStrings[i];
  }
  
  free(piLength);
  free(pstStrings);
  
  return(fail_code);    
}

}  // namespace Dakota
