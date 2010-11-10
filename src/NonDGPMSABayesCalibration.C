/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDGPMSABayesCalibration
//- Description: Wrapper class for MCMC capability from LANL GPM/SA library
//- Owner:       Laura Swiler
//- Checked by:
//- Version:

#include "NonDGPMSABayesCalibration.H"
#include "system_defs.h"
#include "DakotaModel.H"
#include "DakotaResponse.H"
#include "ProblemDescDB.H"
#include "NonDLHSSampling.H"
// includes from gpmsa TPL
#ifdef DAKOTA_GPMSA
#include "GPmodel.H"
#include "rawData.H"
#include "covMat.H"
#include "Init.H"
#include "Prior.H"
#include "LAparams.H"
#include "LAdata.H"
#include "LAmodel.H"
#include "LAmodelMCMC.H"
#endif

static const char rcsId[]="@(#) $Id$";


namespace Dakota {

/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDGPMSABayesCalibration::NonDGPMSABayesCalibration(Model& model):
  NonDBayesCalibration(model),
  xObsDataFile(probDescDB.get_string("method.x_obs_data_file")),
  yObsDataFile(probDescDB.get_string("method.y_obs_data_file")),
  yStdDataFile(probDescDB.get_string("method.y_std_data_file"))
{
  lhsSampler.assign_rep(new NonDLHSSampling(iteratedModel, numSamples,
					    seedSpec, rngName, ALL), false);
  iteratedModel.init_communicators(lhsSampler.maximum_concurrency());
}


NonDGPMSABayesCalibration::~NonDGPMSABayesCalibration()
{ iteratedModel.free_communicators(lhsSampler.maximum_concurrency()); }


/** This method does all the pre-processing necessary to call the GPM/SA code,
    including running LHS on the model to generate the initial samples, 
    doing some normalization, calling the GPM/SA functions, and 
    returning the posterior parameter distributions. */
void NonDGPMSABayesCalibration::quantify_uncertainty()
{
  // Note: I am assuming the user will use DAKOTA to run the initial 
  // simulations to generate simData.  Thus, the first part of this 
  // is similar to LHS. Currently we have one argument in our input 
  // spec for the number of samples.  I would suggest we leave this 
  // for the number of simulation samples, and add another argument 
  // for number of MCMC samples. 

  // Note that we construct the initial LHS over x and theta.  We need 
  // a better way to specify these separately, currently it is assumed 
  // that the first p input variables are x and the next q are theta. 
  // We construct samples over both. 
 
  //lhsSampler.samplingVarsMode = ALL;
  // run LHS to generate parameter sets and evaluate simulation at those sets
  
  lhsSampler.run_iterator(Cout);
 
  const RealMatrix&    all_samples = lhsSampler.all_samples();
  const ResponseArray& all_resp    = lhsSampler.all_responses();
  double **zt_raw;
  double **ysim;
  double **x_obs;
  double **y_obs;
  double **y_std;

  size_t num_problem_vars = iteratedModel.acv(); 
  Cout << "num_problem_vars " << num_problem_vars << '\n';
  size_t num_resp = iteratedModel.num_functions();
  Cout << "num_responses " << num_resp << '\n';
  
  zt_raw=new double*[numSamples];
  for (int i=0; i<numSamples; ++i)
    zt_raw[i]=new double[num_problem_vars];
  for (int i=0; i<num_problem_vars; ++i)
   for (int j=0; j<numSamples; ++j)
     zt_raw[j][i]=all_samples(i,j);
  
  ysim=new double*[1];
  for (int i=0; i<1; ++i)
    ysim[i]=new double[numSamples];
  for (int i=0; i<1; ++i)
   for (int j=0; j<numSamples; ++j)
     ysim[i][j]=all_resp[j].function_value(0);

 // We read in the experimental data.  
 // We assume for now that the number of calibration 
 // parameters is the number of total uncertain variables 
 // (num_problem_vars) less the number of x_observed data. 
  int i=0, num_obs_data = 0;
  
  std::ifstream data_file1(xObsDataFile);
  if (!data_file1) {
    Cerr << "Could not open user data source for x observations " 
         << xObsDataFile << " in "
         << "GPMSA Bayesian calibration method." << std::endl;
    abort_handler(-1);
  }
  else {
    std::string line;
    while (getline(data_file1,line))
      num_obs_data++;
    xObsData.resize(num_obs_data);
    yObsData.resize(num_obs_data);
    yStdData.resize(num_obs_data);
       
    data_file1.clear();
    data_file1.seekg(0, std::ios::beg);

    i=0;
    while ((i<num_obs_data) && (data_file1 >> xObsData[i]))
    //while (i<num_obs_data) {
    // data_file1 >> *xObsData[i];
     i++;
    data_file1.close();
  }

  std::ifstream data_file2(yObsDataFile);
  if (!data_file2) {
    Cerr << "Could not open user data source for y observations " 
         << yObsDataFile << " in "
         << "GPMSA Bayesian calibration method." << std::endl;
    abort_handler(-1);
  }
  else {
    i = 0;
    while ((i<num_obs_data) && (data_file2 >> yObsData[i]))
      i++;
    data_file2.close();
  }
  std::ifstream data_file3(yStdDataFile);
  if (!data_file3) {
    Cerr << "Could not open user data source for y error on observations " 
         << yStdDataFile << " in "
         << "GPMSA Bayesian calibration method." << std::endl;
  abort_handler(-1);
  }
  else {
    i = 0;
    while ((i<num_obs_data) &&  (data_file3 >> yStdData[i]))
      i++;
    data_file3.close();
  }


 // Assuming we get all of this as input, we need to populate the following 
 // for GPM/SA: 
 
 int verbose=1;
 
  // parse command line arguments for number of draws, rawfile, 
  //  parmsfile, and outfile

 int numDraws=10000;

#ifdef DAKOTA_GPMSA 
 FILE *fp_raw, *fp_optparms, *fp_out, *fp_new;
 int num_cal_parameters = num_problem_vars-1; 
 fp_new=fopen("gpmsa.rawinfo.dat","w");
 //  n, m, p, q, pu, pv
 fprintf(fp_new, "%d %d %d %d %d %d\n", num_obs_data, numSamples, 1,
	 num_cal_parameters, 1, 1);
 // zt [ m x (p+q) ]    read in column major order
 for (int i=0; i<num_problem_vars; ++i)
   for (int j=0; j<numSamples; ++j)
     fprintf(fp_new,"%f ",all_samples(i,j));
 // lsim
 fprintf (fp_new,"\n%d \n",1);
 // ysim [ lsim x m ]
 for (int i=0; i<numSamples;i++)
   fprintf(fp_new,"%f ",all_resp[i].function_value(0));
 // Ksim [ lsim x pu ]
 //fprintf (fp_new,"\n%f \n",1.0);
 // Dsim [ lsim x pv ]
 //fprintf(fp_new,"%f \n",1.0);
   // for ii = 0, n-1
    //   x (ii,jj), jj=0, p-1
    //   lobs[ii] (element ii of vector lobs)
    //   yobs[ii] (elements jj=0, lobs[ii]-1 of gsl vector yobs[ii])
    //   sdobs[ii] (element ii of vector sdobs)
    //   Kobs[ii] (all lobs[ii] x pu elements of gsl matrix Kobs[ii])
    //   Dobs[ii] (all lobs[ii] x pv elements of gsl matrix Dobs[ii])
 
 for (int j=0; j<num_obs_data; j++){
   fprintf(fp_new,"%f \n",xObsData[j]);
   fprintf(fp_new,"%d \n",1);
   fprintf(fp_new,"%f \n",yObsData[j]);
   fprintf(fp_new,"%f \n",yStdData[j]);
   //fprintf(fp_new,"%f \n",1.0);
   //fprintf(fp_new,"%f \n",1.0);
 }  
 fclose(fp_new); 

    x_obs=  new double*[num_obs_data];
    y_obs= new double*[num_obs_data];
    y_std = new double*[num_obs_data];
    for (i=0; i<num_obs_data; ++i) {
      x_obs[i] = new double[1];
      y_obs[i] = new double[1];
      y_std[i] = new double[1];
     }
    for (int j=0; j<num_obs_data; j++){
      for (int i=0; i<1; ++i) {
        x_obs[j][i] = xObsData[j];
        y_obs[j][i] = yObsData[j];
        y_std[j][i] = yStdData[j];
      }
    }
     
 fp_new=fopen("gpmsa.optparms.dat","w");
 // Opt Parms just for scalar data - will need to be updated for functional data
 fprintf(fp_new, "%d\n",0);
 fprintf(fp_new, "%d\n",1);
 fclose(fp_new);

 fp_raw=fopen("gpmsa.rawinfo.dat","r");

 if (verbose) printf("main: calling rawData\n");
 rawData *raw = new rawData(fp_raw);

 fclose(fp_raw);

  // setup some optional parameters
  //  optional parms data consists of
  //  scOut - scalar output
  //  lamVzGroup - see p. 31 of GPMSA Command Reference
 int scOut_raw = 0, lamVzGnum_raw = 1, *lamVzGroup_raw;
 lamVzGroup_raw = new int[raw->pv_raw];
 for (int ii=0; ii<raw->pv_raw; ii++) lamVzGroup_raw[ii] = 1;

 Init *params = new Init();
 int a[num_obs_data];
 float y1[num_obs_data];
 for(int j=0; j<num_obs_data; j++){
  a[j]=1;
  y1[j]=yStdData[0];
 }
 
 if (verbose) printf("main: calling setProbDims\n");
 params->setProbDims(num_obs_data, numSamples, 1, num_cal_parameters, 1, 1);
 params->showProbDims();
 if (verbose) printf("main: calling setSimData\n");
 params->setSimData(zt_raw,1,ysim);
 params->showSimData();
 if (verbose) printf("main: calling setObsData\n");
 params->setObsData(&x_obs[0],&a[0],&y_obs[0],&y1[0]);
 params->showObsData();
 if (verbose) printf("main: calling setOptions\n");
 params->setOptions(scOut_raw, lamVzGnum_raw, lamVzGroup_raw);


 // type = scalar, functional, multivariate (1, 2, or 3)
 // flag = no calibration, calibration (0 or 1)
 int type = 1;
 int flag = 1;
 if (verbose) printf("main: calling preProcess\n");
 params->preProcess(type,flag);
 if (verbose) printf("main: calling setupModel\n");
 params->setupModel();

 if (verbose) printf("main: returned from setupModel\n");
 
  if (verbose) printf("main: calling LAdata copy constructor\n");
  LAdata *dat=new LAdata(params);
  if (verbose) printf("main: returned from LAdata copy constructor\n");
  if (verbose) printf("main: dat->n = %d\n",dat->n);

  if (verbose) printf("main: calling LAparams\n");
  LAparams *p0=new LAparams(dat,params);
  if (verbose) printf("main: returned from LAparams\n");

  if (verbose) printf("main: calling LAmodel\n");
  LAmodel *m0=new LAmodel(dat,p0);
  if (verbose) printf("main: returned from LAmodel\n");

  if (verbose) printf("main: calling LAmodelMCMC\n");
  LAmodelMCMC LAM(m0,params);
  if (verbose) printf("main: returned from LAmodelMCMC\n");

  printf("Completed read of model with n=%d, m=%d, p=%d, q=%d, pv=%d, pu=%d,\n",
          dat->n, dat->m, dat->p, dat->q, dat->pv, dat->pu);


  // these are not just printing, but also doing initial calculations. 
  printf("Input model log likelihood= %f\n",m0->computeLogLik());
  printf("Input model log prior = %f \n",LAM.computeLogPrior());

  // Posterior samples are currently written to gpmsa.pvals.out
  fp_out=fopen("gpmsa.pvals.out","w");
  LAM.writePvalsHeader(fp_out);
  int timer=clock();
  for (int ii=0; ii<numDraws; ii++) {
    LAM.sampleRound();
	LAM.writePvals(fp_out);
	printf("Round %2d, lik=%10.4f, prior=%10.4f \n", ii,
	       LAM.mod->getLogLik(),LAM.computeLogPrior());
  }
  printf("completed in %f\n",1.0*(clock()-timer)/CLOCKS_PER_SEC);
  fclose(fp_out);
#endif

}


void NonDGPMSABayesCalibration::print_results(std::ostream& s)
{ Cout << "Posterior sample results " << '\n'; }

} // namespace Dakota
