/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
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
#include "tabular_io.h"
// includes from gpmsa TPL
#ifdef DAKOTA_GPMSA
#include "GPmodel.H"
#include "rawD.H"
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
  numEmulatorSamples(probDescDB.get_int("method.nond.emulator_samples")),
  numDraws(probDescDB.get_int("method.samples")),
  seedSpec(probDescDB.get_int("method.random_seed")),
  rngName(probDescDB.get_string("method.random_number_generator"))
{
  //Cout << "The Bayesian Calibration method GPMSA is not currently operational."
    //   << " We expect that it will be operational in 2011." << std::endl;
  //abort_handler(-1);
  String sample_type; // empty string: use default sample type
  lhsSampler.assign_rep(new NonDLHSSampling(iteratedModel, sample_type,
    numEmulatorSamples, seedSpec, rngName, ALL_UNIFORM), false);

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
  // construct emulatorModel, if needed
  // NonDBayesCalibration::quantify_uncertainty();
  // *** TO DO ***: eliminate redundant lhsSampler

  // Note: I am assuming the user will use DAKOTA to run the initial 
  // simulations to generate simData.  
  // Note that we construct the initial LHS over x and theta.  
  // We assume that x are given by the state variables (of dimension p) 
  // and the uncertain variables specificy theta (of dimension q). 
  // We construct samples over both. 
 
  // run LHS to generate parameter sets and evaluate simulation at those sets
  
  lhsSampler.run_iterator(Cout);
  const RealMatrix&     all_samples = lhsSampler.all_samples();
  const IntResponseMap& all_resp    = lhsSampler.all_responses();
  double **zt_raw;

  double **ysim;

  size_t num_problem_vars = iteratedModel.acv(); 
  Cout << "num_problem_vars " << num_problem_vars << '\n';
  size_t num_resp = iteratedModel.num_functions();
  Cout << "num_responses " << num_resp << '\n';
  if (num_resp > 1)
    Cout << "WARNING: GPMSA is only calibrating the first response currently." << '\n';

  int i,j;
  zt_raw=new double*[numEmulatorSamples];
  for (int i=0; i<numEmulatorSamples; ++i)
    zt_raw[i]=new double[num_problem_vars];
  for (int i=0; i<num_problem_vars; ++i)
   for (int j=0; j<numEmulatorSamples; ++j)
     zt_raw[j][i]=all_samples(i,j);
  
  ysim=new double*[1];
  for (i=0; i<1; ++i)
    ysim[i]=new double[numEmulatorSamples];
  for (int i=0; i<1; ++i) {
    IntRespMCIter resp_it = all_resp.begin();
    for (j=0, resp_it=all_resp.begin(); j<numEmulatorSamples; ++j, ++resp_it)
      ysim[i][j]=resp_it->second.function_value(0);
  }
  
  Cout << "zt_raw" << '\n';
  for (int i=0; i<num_problem_vars; ++i)
   for (int j=0; j<numEmulatorSamples; ++j)
     Cout << "zt_raw" << zt_raw[j][i];
  Cout << '\n' ;
  Cout << 'ysim', '\n';
  for (int j=0; j<numEmulatorSamples; ++j)
    Cout << ysim[0][j];

  // We read in the experimental data.  
  // We read a matrix with numExperiments rows and cols
  // numExpConfigVars X, numFunctions Y, [numFunctions Sigma]
  RealMatrix experimental_data;

  size_t num_sigma_read = numExpStdDeviationsRead;
  size_t num_cols = numExpConfigVars + numFunctions + num_sigma_read;

  TabularIO::read_data_tabular(expDataFileName, "GPMSA Bayes Calibration", 
		    experimental_data, numExperiments,  num_cols, 
		    expDataFileAnnotated);

  // Get views of the data in 3 matrices for convenience
  size_t start_row, start_col;
  if (numExpConfigVars == 0) {
    Cerr << "\nError (NonDGPMSABayesCalibration): must specify "
           << "experimental_config_variables in experimental_data_file " 
           << std::endl;
   abort_handler(-1);
  }

  Cout << "num_sigma_read " << num_sigma_read << '\n';
  Cout << "num_cols " << num_cols << '\n';
  Cout << "numExpConfVariables " << numExpConfigVars << '\n';

  start_row = 0;
  start_col = 0;
  RealMatrix x_obs_data(Teuchos::View, experimental_data,
			numExperiments, numExpConfigVars,
			start_row, start_col);
  Cout << 'xobs_data' << x_obs_data << '\n';
  
  start_row = 0;
  start_col = numExpConfigVars;
  RealMatrix y_obs_data(Teuchos::View, experimental_data,
			numExperiments, numFunctions,
			start_row, start_col);
  Cout << 'yobs_data' << y_obs_data << '\n';

  // BMA: note that we now allow reading 1 or N sigmas from the file...
  RealMatrix y_std_data;
  y_std_data.reshape(numExperiments,numFunctions);
  if (num_sigma_read > 0) {
    start_row = 0;
    start_col = numExpConfigVars + numFunctions;
    RealMatrix y_std_data2(Teuchos::View, experimental_data,
			numExperiments, numFunctions,
			start_row, start_col);
    for (int i=0; i<numExperiments; i++) 
      for (int j=0; j<numFunctions; j++) 
        y_std_data(i,j)=y_std_data2(i,j);
  }
  else { 
    if (expStdDeviations.length()==1) {
      for (int i=0; i<numExperiments; i++) 
        for (int j=0; j<numFunctions; j++)
          y_std_data(i,j)=expStdDeviations(0);
    }
    else if (expStdDeviations.length()==numFunctions) {
      for (int i=0; i<numExperiments; i++) 
	for (int j=0; j<numFunctions; j++)
          y_std_data(i,j)=expStdDeviations(j);
    }
    else {
      Cerr << "\nError (NonDGPMSABayesCalibration): must specify "
           << "experimental standard deviations either from experimental_data_file " 
           << "or in the experimental_std_deviations specification." 
           << std::endl;
      abort_handler(-1);
    }
  }
  Cout << 'ystd_data' << y_std_data << '\n';

 // Assuming we get all of this as input, we need to populate the following 
 // for GPM/SA: 
 
 int verbose=1;
 
  // parse command line arguments for number of draws, rawfile, 
  //  parmsfile, and outfile

 // type = scalar, functional, multivariate (0, 1, or 2)
 // flag = no calibration, calibration (0 or 1)
 int type = 0;
 int flag = 1;

#ifdef DAKOTA_GPMSA 
 FILE *fp_raw, *fp_out, *fp_new;
 int num_cal_parameters = num_problem_vars-numExpConfigVars; 
 fp_new=fopen("gpmsa.rawinfo.dat","w");
 // type
 fprintf(fp_new, "%d \n", type); 
 //  n, m, p, q
 fprintf(fp_new, "%d %d %d %d \n", numExperiments, numEmulatorSamples, numExpConfigVars,num_cal_parameters);
 // zt [ m x (p+q) ]    read in column major order
 for (int i=0; i<num_problem_vars; ++i)
   for (int j=0; j<numEmulatorSamples; ++j)
     fprintf(fp_new,"%f ",all_samples(i,j));
 // lsim
 fprintf (fp_new,"\n%d \n",1);
 // ysim [ lsim x m ]
 IntRespMCIter resp_it = all_resp.begin();
 IntRespMCIter resp_end = all_resp.end();
 for ( ; resp_it != resp_end; ++resp_it)
    fprintf(fp_new,"%f ", resp_it->second.function_value(0));

 // lsup
 fprintf (fp_new,"\n%d \n",1);
 // gsim [ lsim x lsup ]
 fprintf (fp_new,"%f \n",1.0);
 // nkern
 fprintf (fp_new,"%d \n",1);
   // for ii = 0, n-1
    //   x (ii,jj), jj=0, p-1
    //   lobs[ii] (element ii of vector lobs)
    //   yobs[ii] (elements jj=0, lobs[ii]-1 of vector yobs[ii])
    //   gobs[ii] (lobs[ii]*lsup, obs support matrix)
    //   lobs[ii] (element ii of vector lobs)
    //   yobs[ii] (elements jj=0, lobs[ii]-1 of vector yobs[ii])
    //   gobs[ii] (lobs[ii]*lsup, obs support matrix)
    //   Sigy[ii] (lobs[ii] x lobs[ii])
 
 for (int j=0; j<numExperiments; j++){
   for (int w=0; w<numExpConfigVars; w++) 
     fprintf(fp_new,"%f ",x_obs_data(j,w));
   fprintf(fp_new,"\n");
   fprintf(fp_new,"%d \n",1);
   fprintf(fp_new,"%f \n",y_obs_data(j,0));
   fprintf(fp_new,"%d \n",1);
   fprintf(fp_new,"%f \n",y_std_data(j,0));
 }  
 fclose(fp_new); 
     
 fp_raw=fopen("gpmsa.rawinfo.dat","r");

 if (verbose) printf("main: calling rawData\n");
 rawD *raw = new rawD(fp_raw);

 fclose(fp_raw);

 Init *params = new Init();
 
 if (verbose) printf("main: calling Init::setProbDims\n");
 params->setProbDims(raw->type_raw,raw->cal_raw,raw->n_raw,raw->m_raw,raw->p_raw,raw->q_raw);
 if (verbose) printf("main: calling Init::setSimData\n");
 params->setSimData(raw->zt_raw,raw->lsim_raw,raw->ysim_raw,raw->lsup_raw,raw->gsim_raw,raw->nkern_raw);
 if (raw->n_raw) {
   if (verbose) printf("main: calling Init::setObsData\n");
   params->setObsData(raw->x_raw,raw->lobs_raw,raw->yobs_raw,raw->gobs_raw,raw->Sigy_raw);
  }

 if (verbose) printf("main: calling Init::setOptions\n");
 params->setOptions();

 if (verbose) printf("main: calling Init::preProcess\n");
 params->preProcess();
 if (verbose) printf("main: calling Init::setDefaults\n");
 params->setDefaults();
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
