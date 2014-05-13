/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDUnilevelRBDO
//- Description: Implementation code for NonDUnilevelRBDO class
//- Owner:       Mike Eldred
//- Checked by:
//- Version:

#include "system_defs.h"
#include "ProblemDescDB.H"
#include "ParallelLibrary.H"
#include "NonDUnilevelRBDO.H"
#include "Epetra_SerialDenseVector.h"
#include "Epetra_SerialSpdDenseSolver.h"
#ifdef HAVE_NPSOL
#include "NPSOLOptimizer.H"
#elif defined(HAVE_OPTPP)
#include "SNLLOptimizer.H"
#endif


static const char rcsId[] = "@(#) $Id: NonDUnilevelRBDO.C 6972 2010-09-17 22:18:50Z briadam $";

// initialization of statics
Epetra_SerialDenseVector NonDUnilevelRBDO::fnValsMeanX;
Epetra_SerialDenseVector NonDUnilevelRBDO::fnGradX;
Epetra_SerialDenseVector NonDUnilevelRBDO::fnGradU;
Epetra_SerialDenseMatrix NonDUnilevelRBDO::cholCorrMatrix;
Epetra_SerialDenseVector NonDUnilevelRBDO::mostProbPointX;
Epetra_SerialDenseVector NonDUnilevelRBDO::mostProbPointU;
DakotaIntVector          NonDUnilevelRBDO::ranVarType;
Epetra_SerialDenseVector NonDUnilevelRBDO::ranVarMeans;
Epetra_SerialDenseVector NonDUnilevelRBDO::ranVarSigmas;
int                      NonDUnilevelRBDO::respFnCount(0);
int                      NonDUnilevelRBDO::levelCount(0);
DakotaModel              NonDUnilevelRBDO::staticNonDModel;
size_t                   NonDUnilevelRBDO::staticNumUncVars(0);
size_t                   NonDUnilevelRBDO::staticNumFuncs(0);
Real                     NonDUnilevelRBDO::requestedRespLevel(0.);
Real                     NonDUnilevelRBDO::requestedCDFProbLevel(0.);
DakotaRealVectorArray    NonDUnilevelRBDO::computedRespLevels;
short                    NonDUnilevelRBDO::mppSearchFlag(0);
int                      NonDUnilevelRBDO::npsolDerivLevel(3);


NonDUnilevelRBDO::NonDUnilevelRBDO(DakotaModel& model): DakotaNonD(model),
  requestedProbLevels(probDescDB.get_drva("method.nond.probability_levels")),
  integrationMethod(
    probDescDB.get_string("method.nond.reliability_integration"))
{
  // Copy the correlation matrix and response/probability levels to Petra types
  copy_data(uncertainCorrelations, petraCorrMatrix);

  const DakotaString& mpp_search
    = probDescDB.get_string("method.nond.reliability_search_type");
  if (mpp_search == "mv")
    mppSearchFlag = 0;
  else if (mpp_search == "amv_x")
    mppSearchFlag = 1;
  else if (mpp_search == "amv_plus_x")
    mppSearchFlag = 2;
  else if (mpp_search == "amv_plus_u")
    mppSearchFlag = 3;
  else if (mpp_search == "form")
    mppSearchFlag = 4;
  else {
    Cerr << "Error: only MV, AMV, AMV+, transformed AMV+, and FORM are "
         << "currently supported in unilevel_reliability." << std::endl;
    abort_handler(-1); 
  }

  // When specifying probability levels, a specification with an index key
  // (number of levels = list of ints) will result in multiple vectors of
  // levels, one for each response fn.  A specification without an index key
  // will result in a single vector of levels.  This is as much logic as the
  // parser can support since it cannot access the number of response fns.
  // Here, support the shorthand specification case where there are multiple
  // response fns but only one vector of levels (no index key provided), which
  // are to be evenly distributed among the response fns.  This provides some
  // measure of backwards compatibility.
  size_t i, j, num_pl_vec = requestedProbLevels.length();
  if (num_pl_vec != numFunctions) {
    if (num_pl_vec == 0) // create array of empty vectors
      requestedProbLevels.reshape(numFunctions);
    else if (num_pl_vec == 1) { // evenly distribute vector among all resp fns
      DakotaRealVector req_pl = requestedProbLevels[0];
      size_t total_len = req_pl.length();
      // check for divisibility
      if (total_len%numFunctions) {
        Cerr << "\nError: number of probability levels not evenly divisible by "
             << "the number of response functions." << std::endl;
        abort_handler(-1);
      }
      requestedProbLevels.reshape(numFunctions);
      size_t new_len = total_len/numFunctions;
      DakotaRealVector new_vec(new_len);
      for (i=0; i<numFunctions; i++) {
        for (j=0; j<new_len; j++)
          new_vec[j] = req_pl[i*new_len+j];
        requestedProbLevels[i] = new_vec;
      }
    }
    else {
      Cerr << "\nError: number of probability level vectors differ from the "
           << "number of response functions." << std::endl;
      abort_handler(-1);
    }
  }

  // size the output arrays
  computedRespLevels.reshape(numFunctions);
  computedProbLevels.reshape(numFunctions);
  size_t total_levels = 0;
  for (i=0; i<numFunctions; i++) {
    size_t num_levels = requestedRespLevels[i].length()
                      + requestedProbLevels[i].length();
    computedRespLevels[i].reshape(num_levels);
    computedProbLevels[i].reshape(num_levels);
    total_levels += num_levels;
  }

  // initialize finalStatistics
  size_t num_final_stats = 2*numFunctions + total_levels;
  DakotaIntArray asv(num_final_stats, 1);
  finalStatistics = DakotaResponse(numUncertainVars, asv);
  // Assign meaningful fn tags to finalStatistics (appear in NestedModel output)
  size_t num_levels, cntr = 0;
  char tag_string[10], lev_string[10];
  DakotaStringArray stats_tags(num_final_stats);
  using std::sprintf;
  for (i=0; i<numFunctions; i++) {
    sprintf(tag_string, "_r%i", i+1);
    stats_tags[cntr++] = DakotaString("mean")    + DakotaString(tag_string);
    stats_tags[cntr++] = DakotaString("std_dev") + DakotaString(tag_string);
    num_levels = requestedRespLevels[i].length();
    for (j=0; j<num_levels; j++) {
      sprintf(lev_string, "_plev%i", j+1);
      stats_tags[cntr] = (cdfFlag) ? DakotaString("cdf") : DakotaString("ccdf");
      stats_tags[cntr] += DakotaString(lev_string) + DakotaString(tag_string);
      cntr++;
    }
    num_levels = requestedProbLevels[i].length();
    for (j=0; j<num_levels; j++) {
      sprintf(lev_string, "_rlev%i", j+1);
      stats_tags[cntr] = (cdfFlag) ? DakotaString("cdf") : DakotaString("ccdf");
      stats_tags[cntr] += DakotaString(lev_string) + DakotaString(tag_string);
      cntr++;
    }
  }
  finalStatistics.fn_tags(stats_tags);

  // Check for proper variables and response function definitions.
  // NOTE: numResponseFunctions implies UQ usage.  numFunctions has a broader
  // interpretation (e.g., general DACE usage of NonDLHSSampling iterator).
  if (!numUncertainVars || !numResponseFunctions) {
    Cerr << "\nError: number of uncertain variables and response functions "
	 << "must be nonzero." << std::endl;
    abort_handler(-1);
  }
}


NonDUnilevelRBDO::~NonDUnilevelRBDO()
{ }


void NonDUnilevelRBDO::quantify_uncertainty()
{
  // static copies used in FORM constraint calculations.
  staticNonDModel  = userDefinedModel;
  staticNumFuncs   = numFunctions;
  staticNumUncVars = numUncertainVars;

  // build static ranVar arrays containing the uncertain variable distribution
  // types (0=normal, 1=lognormal, ...) and their corresponding means/std devs.
  size_t i, j, k, uncertain_var_count = 0;
  ranVarType.reshape(numUncertainVars);
  ranVarMeans.Size(numUncertainVars);
  ranVarSigmas.Size(numUncertainVars);
  for (i=0; i<numNormalVars; i++) {
    ranVarType[uncertain_var_count]   = 0;
    ranVarMeans(uncertain_var_count)  = normalMeans[i];
    ranVarSigmas(uncertain_var_count) = normalStdDevs[i];
    uncertain_var_count++;
  }
  for (i=0; i<numLognormalVars; i++) {
    ranVarType[uncertain_var_count]  = 1;
    ranVarMeans(uncertain_var_count) = lognormalMeans[i];
    if (lognormalStdDevs.length())
      ranVarSigmas(uncertain_var_count) = lognormalStdDevs[i];
    else if (lognormalErrFacts.length()) {
      // Pre-process the error factors to compute the std dev of the lognormal
      // distribution.  See formula on SAND Report #98-0210, "A User's Guide to
      // LHS: Sandia's Latin Hypercube Sampling Software" page 39-41.
      double temp_sigma = log(lognormalErrFacts[i])/Pecos::Phi_inverse(0.95),
             temp_var   = pow(temp_sigma, 2);
      ranVarSigmas(uncertain_var_count)
        = sqrt( (exp(2.*lognormalMeans[i] + temp_var))*(exp(temp_var) - 1.) );
    }
    uncertain_var_count++;
  }
  for (i=0; i<numUniformVars; i++) {
    ranVarType[uncertain_var_count] = 2;
    ranVarMeans(uncertain_var_count)
      = (uniformDistLowerBnds[i] + uniformDistUpperBnds[i])/2.;
    double sqr_diff_bnds
      = pow((uniformDistUpperBnds[i] - uniformDistLowerBnds[i]), 2);
    ranVarSigmas(uncertain_var_count) = sqrt(sqr_diff_bnds/12.);
    uncertain_var_count++;
  }

  // Response values/gradients needed at mean values of the uncertain vars for
  // MV analysis or for computing an initial linearization model (AMV/AMV+).
  // The MV analysis for meanStats/stdDevStats is also performed for strategy
  // usage of NonDUnilevelRBDO to fill out finalStatistics.
  if (mppSearchFlag < 4 || strategyFlag) {
    // Evaluate response fn values/grads at mean values of uncertain vars.
    Cout << "\n>>>>> Evaluating response at mean values\n";
    DakotaRealVector local_unc_vars(numUncertainVars);
    copy_data(ranVarMeans, local_unc_vars);
    userDefinedModel.continuous_variables(local_unc_vars);
    DakotaIntArray asv(numFunctions, 3);
    userDefinedModel.compute_response(asv);
    const DakotaResponse& local_response = userDefinedModel.current_response();
    copy_data(local_response.function_values(),    fnValsMeanX);
    copy_data(local_response.function_gradients(), fnGradsMeanX);

    // set meanStats and compute stdDevStats using MVFOSM formula including
    // effect of input variable correlations.  NOTE: if fnGradsMeanX is zero,
    // then stdDevStats will be zero which is bad for MV CDF estimates.
    copy_data(fnValsMeanX, meanStats);
    for (i=0; i<numFunctions; i++) {
      stdDevStats[i] = 0.;
      for (j=0; j<numUncertainVars; j++)
        for (k=0; k<numUncertainVars; k++)
          stdDevStats[i] += petraCorrMatrix(j,k)*ranVarSigmas(j)*ranVarSigmas(k)
                         *  fnGradsMeanX(i,j)*fnGradsMeanX(i,k);
      stdDevStats[i] = sqrt(stdDevStats[i]);
    }
  }
  else {
    meanStats   = 0.;
    stdDevStats = 0.;
  }

  if (mppSearchFlag)
    iterated_mean_value();
  else
    mean_value();
}


void NonDUnilevelRBDO::mean_value()
{
  // For MV, compute approximate mean, standard deviation, and requested
  // CDF/CCDF data points for each response function and store in 
  // finalStatistics.  Additionally, if uncorrelated variables, compute
  // importance factors.

  // initialize arrays
  impFactor.reshape_2d(numFunctions, numUncertainVars);
  size_t i, j, k, cntr = 0;
  DakotaRealVector final_stats = finalStatistics.function_values();

  // loop over response functions
  for (i=0; i<numFunctions; i++) {
    // approximate means and std devs already computed
    final_stats[cntr++] = meanStats[i];
    final_stats[cntr++] = stdDevStats[i];

    // if inputs are uncorrelated, compute importance factors
    if (!correlationFlag && stdDevStats[i] > 1.e-25)
      for (j=0; j<numUncertainVars; j++)
        impFactor[i][j]
          = pow(ranVarSigmas(j)/stdDevStats[i]*fnGradsMeanX(i,j), 2);

    // compute probability levels for specified response levels and response
    // levels for specified probability levels.  For g<0 = failed, beta is
    // simply mean/sigma.  This is modified for g>z = failed.
    size_t rl_len = requestedRespLevels[i].length(),
           pl_len = requestedProbLevels[i].length();
    for (j=0; j<rl_len; j++) {
      // computed = requested in MV case since no validation fn evals
      computedRespLevels[i][j] = requestedRespLevels[i][j];

      if (stdDevStats[i] > 1.e-25) {
        double beta = (requestedRespLevels[i][j] - meanStats[i])/stdDevStats[i];
        computedProbLevels[i][j] = (cdfFlag) ? phi(beta) : phi(-beta);
      }
      else {
        if (requestedRespLevels[i][j] >= meanStats[i])
          computedProbLevels[i][j] = (cdfFlag) ? 1. : 0.;
        else
          computedProbLevels[i][j] = (cdfFlag) ? 0. : 1.;
      }
      final_stats[cntr++] = computedProbLevels[i][j];
    }
    for (j=0; j<pl_len; j++) {
      // computed = requested in MV case since no validation fn evals
      computedProbLevels[i][j+rl_len] = requestedProbLevels[i][j];

      double beta = (cdfFlag) ? phi_inverse(requestedProbLevels[i][j])
                              : phi_inverse(1.-requestedProbLevels[i][j]);
      computedRespLevels[i][j+rl_len] = beta * stdDevStats[i] + meanStats[i];
      final_stats[cntr++] = computedRespLevels[i][j+rl_len];
    }
  }
  finalStatistics.function_values(final_stats);
}


void NonDUnilevelRBDO::iterated_mean_value()
{
  // Size some arrays
  size_t i, j, cntr = 0;
  mostProbPointX.Size(numUncertainVars);
  mostProbPointU.Size(numUncertainVars);
  fnGradX.Size(numUncertainVars);
  fnGradU.Size(numUncertainVars);
  for (i=0; i<numUncertainVars; i++)
    fnGradX(i) = fnGradU(i) = 0.;
  DakotaIntArray asv(numFunctions);
  DakotaRealVector init_pt(numUncertainVars);
  Epetra_SerialDenseMatrix jacobian_xu;
  DakotaRealVector final_stats = finalStatistics.function_values();

  // Modify the correlation matrix and compute its cholesky decomp in place
  Epetra_SerialSymDenseMatrix mod_corr_matrix;
  Epetra_SerialSpdDenseSolver corr_solve_object;
  cholCorrMatrix.Shape(numUncertainVars, numUncertainVars);
  transNataf(mod_corr_matrix);
  corr_solve_object.SetMatrix(mod_corr_matrix);
  corr_solve_object.Factor();
  // mod_corr_matrix is cholesky-factored (LL^T) in place.  Define
  // cholCorrMatrix to be L by zeroing-out the upper triangle.
  for (i=0; i<numUncertainVars; i++)
    for (j=0; j<numUncertainVars; j++)
      cholCorrMatrix(i, j) = (j <= i) ? mod_corr_matrix(i, j) : 0.;

  // Determine median of limit state function for AMV/AMV+/FORM/SORM.  This is
  // used to determine the sign of the safety index in computing probability.
  Cout << "\n>>>>> Evaluating response at median values\n";
  DakotaRealVector median_u(numUncertainVars, 0.), median_x(numUncertainVars);
  transUToX(median_u, median_x);
  userDefinedModel.continuous_variables(median_x);
  asv = 1; // fn. values only for median value
  userDefinedModel.compute_response(asv);
  medianFnVals = userDefinedModel.current_response().function_values();

  // Set up optimizer data that does not change.
  // Bound to 10 std devs in u space.  This is particularly important for PMA
  // since an SQP-based optimizer will not enforce the constraint immediately
  // and min +/-g has been observed to have significant excursions early on
  // prior to the u'u = beta^2 constraint enforcement bringing it back.  A
  // large excursion can cause overflow; a medium excursion can cause poor 
  // performance since far-field info is introduced into the BFGS Hessian.
  DakotaRealVector var_lower_bnds(numUncertainVars, -10),
    var_upper_bnds(numUncertainVars, 10);
  DakotaRealMatrix lin_ineq_coeffs, lin_eq_coeffs;
  DakotaRealVector lin_ineq_lower_bnds, lin_ineq_upper_bnds, lin_eq_targets,
    nln_ineq_lower_bnds, nln_ineq_upper_bnds, nln_eq_targets(1, 0.);

  // Loop over each response function in the responses specification.  It is
  // important to note that the MPP iteration is different for each response 
  // function, and it is not possible to combine the model evaluations for
  // multiple response functions.
  for (respFnCount=0; respFnCount<numFunctions; respFnCount++) {

    // approximate means and std devs already computed
    final_stats[cntr++] = meanStats[respFnCount];
    final_stats[cntr++] = stdDevStats[respFnCount];

    // The most general case is to allow a combination of response and
    // probability level specifications for each response function.
    size_t rl_len = requestedRespLevels[respFnCount].length(),
           pl_len = requestedProbLevels[respFnCount].length(),
           num_levels = rl_len + pl_len;

    // Loop over response/probability levels
    for (levelCount=0; levelCount<num_levels; levelCount++) {

      // The rl_len response levels are performed first, followed by
      // the pl_len probability levels.
      bool ria_flag = (levelCount < rl_len) ? true : false;
      if (ria_flag)
        requestedRespLevel = requestedRespLevels[respFnCount][levelCount];
      else
        requestedCDFProbLevel = (cdfFlag)
          ? requestedProbLevels[respFnCount][levelCount-rl_len]
          : 1.-requestedProbLevels[respFnCount][levelCount-rl_len];

      // Initialize mostProbPointX/U, fnGradX/U, and computed response level.
      // For the first level or for a switch from RIA to PMA, the initial
      // linearization point is the mean x/u.  For subsequent levels continuing
      // either an RIA progression or a PMA progression, warm start by using the
      // MPP from the previous level as the initial linearization point.
      if (levelCount == 0 || levelCount == rl_len) {
        // Initialize optimizer initial point once per response function or for
        // an RIA/PMA switch, since MPP solutions for one response/probability
        // level are used as the starting point for the next level.
        init_pt = 1.; // u space

        // Initialize mostProbPointX/U and computed response level to mean
        // variables/responses for AMV+.
        if (mppSearchFlag == 2 || mppSearchFlag == 3) {
          mostProbPointX = ranVarMeans;
          transXToU(mostProbPointX, mostProbPointU);
          computedRespLevels[respFnCount][levelCount]
            = fnValsMeanX[respFnCount];
        }
        // Initial values of fnGradX/U are the gradients at the mean x values.
        if (mppSearchFlag < 4) { // gradients needed for linearizations
          for (i=0; i<numUncertainVars; i++)
            fnGradX(i) = fnGradsMeanX(respFnCount, i);
          jacXToU(ranVarMeans, jacobian_xu);
          fnGradU.Multiply('T', 'N', 1., jacobian_xu, fnGradX, 0.);
        }
      }
      else {
        // No action is required for warm start of mostProbPointX/U and
        // fnGradX/U (not indexed by level) but the carryover must be performed
        // explicitly for computedRespLevels (indexed by level).
        if (mppSearchFlag == 2 || mppSearchFlag == 3)
          computedRespLevels[respFnCount][levelCount]
            = computedRespLevels[respFnCount][levelCount-1];
      }

      // For AMV+ linearizations, iterate until linearization expansion point
      // converges to MPP.
      size_t amv_plus_iters = 0;
      bool   amv_plus_converged = false;
      while (!amv_plus_converged) {

        // Use NPSOL/OPT++ in "user_functions" mode to perform the MPP search
        Cout << "\n>>>>> Initiating search for most probable point (MPP)\n";
#ifdef HAVE_NPSOL
        // NPSOL deriv. level: 1 = supplied grads of objective fn, 2 = supplied
        // grads of constraints, 3 = supplied grads of both.  Always use the
        // supplied grads of u'u (deriv. level = 1 for RIA, deriv. level = 2 for
        // PMA).  In addition, use supplied gradients of G(u) in most cases.
        // Exception: deriv. level = 3 results in a gradient-based line search,
        // which could be too expensive for FORM with numerical gradients unless
        // seeking parallel load balance.
        //if (ria_flag)
        //  npsolDerivLevel = ( mppSearchFlag == 4
        //                  &&  staticNonDModel.gradient_method() != "analytic"
        //                  && !staticNonDModel.asynch_flag() ) ? 1 : 3;
        //else
        //  npsolDerivLevel = ( mppSearchFlag == 4
        //                  &&  staticNonDModel.gradient_method() != "analytic"
        //                  && !staticNonDModel.asynch_flag() ) ? 2 : 3;
        //Cout << "Derivative level = " << npsolDerivLevel << '\n';

        // The gradient-based line search (deriv. level = 3) appears to be
        // outperforming the value-based line search in PMA testing.  In
        // addition, the RIA warm start needs fnGradU so deriv. level = 3 has
        // superior performance there as well.  Therefore, deriv level = 3 can
        // be used for all cases.
        npsolDerivLevel = 3;

        // run a tighter tolerance on linearization-based MPP searches
        //Real conv_tol = (mppSearchFlag == 4) ? 1.e-4 : 1.e-6;
        Real conv_tol = -1.; // use NPSOL default

        void (*obj_eval) (int&,int&,Real*,Real&,Real*,int&)
          = (ria_flag) ? RIA_objective_eval : PMA_objective_eval;
        void (*con_eval) (int&,int&,int&,int&,int*,Real*,Real*,Real*,int&)
          = (ria_flag) ? RIA_constraint_eval : PMA_constraint_eval;
        NPSOLOptimizer mpp_optimizer(init_pt, var_lower_bnds, var_upper_bnds,
          0, 0, 0, 1, lin_ineq_coeffs, lin_ineq_lower_bnds, lin_ineq_lower_bnds,
          lin_eq_coeffs, lin_eq_targets, nln_ineq_lower_bnds,
          nln_ineq_upper_bnds, nln_eq_targets, obj_eval, con_eval,
          npsolDerivLevel, conv_tol);
#elif defined(HAVE_OPTPP)
        void (*obj_eval) (int,int,const ColumnVector&,Real&,ColumnVector&,int&)
          = (ria_flag) ? RIA_objective_eval : PMA_objective_eval;
        void (*con_eval)(int,int,const ColumnVector&,ColumnVector&,Matrix&,int&)
          = (ria_flag) ? RIA_constraint_eval : PMA_constraint_eval;
        SNLLOptimizer mpp_optimizer(init_pt, var_lower_bnds, var_upper_bnds,
          0, 0, 0, 1, lin_ineq_coeffs, lin_ineq_lower_bnds, lin_ineq_lower_bnds,
          lin_eq_coeffs, lin_eq_targets, nln_ineq_lower_bnds,
          nln_ineq_upper_bnds, nln_eq_targets, obj_eval, con_eval);
#else
        DakotaIterator mpp_optimizer; // empty envelope
        Cerr << "Error: this executable not configured with NPSOL or OPT++.\n"
             << "       NonDUnilevelRBDO cannot perform MPP search."<<std::endl;
        abort_handler(-1);
#endif

        // Execute MPP search and retrieve results
	// no summary output since on-the-fly constructed:
        mpp_optimizer.run_iterator(Cout);
        const DakotaVariables& vars_star
          = mpp_optimizer.iterator_variable_results(); // MPP in u space
        const DakotaResponse& resp_star
          = mpp_optimizer.iterator_response_results(); // final obj fn & eq con.
        Cout << "Results of MPP optimization:\nInitial point =\n"
             << init_pt << "Final point = \n" << vars_star << resp_star;

        // Update MPP arrays from optimization results
        const DakotaRealVector& mpp_u = vars_star.continuous_variables();
        Epetra_SerialDenseVector del_u;
        if (mppSearchFlag == 2 || mppSearchFlag == 3) {
          del_u.Size(numUncertainVars);
          for (i=0; i<numUncertainVars; i++)
            del_u(i) = mpp_u[i] - mostProbPointU(i);
        }
        copy_data(mpp_u, mostProbPointU);
        DakotaRealVector mpp_x(numUncertainVars);
        transUToX(mpp_u, mpp_x);
        copy_data(mpp_x, mostProbPointX);

        // Set computedRespLevels to the current g(x) value by either performing
        // a validation function evaluation (AMV/AMV+) or retrieving data from
        // resp_star (FORM).  Also update linearizations and convergence tols.
        if (mppSearchFlag == 1) { // AMV
          amv_plus_converged = true; // break out of while loop
          asv = 0; asv[respFnCount] = 1;
          userDefinedModel.continuous_variables(mpp_x);
          userDefinedModel.compute_response(asv); 
          const DakotaRealVector& local_fns
            = userDefinedModel.current_response().function_values();
          computedRespLevels[respFnCount][levelCount] = local_fns[respFnCount];
        }
        else if (mppSearchFlag == 2 || mppSearchFlag == 3) { // AMV+
          // Update linearization in x or u
          asv = 0; asv[respFnCount] = 3;
          userDefinedModel.continuous_variables(mpp_x);
          userDefinedModel.compute_response(asv);
          const DakotaRealVector& local_fns
            = userDefinedModel.current_response().function_values();
          const DakotaRealMatrix& local_grads
            = userDefinedModel.current_response().function_gradients();

          computedRespLevels[respFnCount][levelCount] = local_fns[respFnCount];
          copy_data(local_grads[respFnCount], fnGradX);
          jacXToU(mostProbPointX, jacobian_xu);
          fnGradU.Multiply('T', 'N', 1., jacobian_xu, fnGradX, 0.);
          // Assess AMV+ iteration convergence.  ||del_u|| is not a perfect
          // metric since cycling between MPP estimates can occur.  This is
          // why a maximum number of amv_plus_iters is enforced.
          //amv_plus_tol = fabs(local_fns[respFnCount] - requestedRespLevel);
          double amv_plus_tol = 0.;
          for (i=0; i<numUncertainVars; i++)
            amv_plus_tol += pow(del_u(i), 2);
          amv_plus_tol = sqrt(amv_plus_tol);
          amv_plus_iters++;
          if (amv_plus_tol < 1.e-5 || amv_plus_iters >= 10)
            amv_plus_converged = true;
        }
        else if (mppSearchFlag == 4) { // FORM
          // direct optimization converges to MPP: no relinearization to compute
          amv_plus_converged = true; // break out of while loop
          if (ria_flag) // RIA computed response = response target + eq_con_star
            computedRespLevels[respFnCount][levelCount] = 
              resp_star.function_value(1) + requestedRespLevel;
          else { // PMA computed response = obj_fn_star
            double obj_fn_star = resp_star.function_value(0);
            computedRespLevels[respFnCount][levelCount]
              = (requestedCDFProbLevel <= 0.5) ? obj_fn_star : -obj_fn_star;
          }
        }

        // Set computedProbLevels by retrieving beta from resp_star and
        // converting to a probability using phi(beta).
        double norm_u_sq;
        if (ria_flag)
          norm_u_sq = resp_star.function_value(0); // obj_fn_star
        else {
          double beta_target = phi_inverse(requestedCDFProbLevel);
          double eq_con_star = resp_star.function_value(1); // constr. viol.
          norm_u_sq = eq_con_star + pow(beta_target, 2);
        }
        double beta = (computedRespLevels[respFnCount][levelCount] >=
          medianFnVals[respFnCount]) ? sqrt(norm_u_sq) : -sqrt(norm_u_sq);
        computedProbLevels[respFnCount][levelCount]
          = (cdfFlag) ? phi(beta) : phi(-beta);

        // Warm start the initial guess for the next MPP optimization.
        // NOTE: premature opt. termination can occur if the RIA/PMA 1st-order
        // optimality conditions (u + lamba*grad_g = 0 or grad_g + lambda*u = 0)
        // remain satisfied for the new level, even though the new equality
        // constraint will be violated.  Therefore, when advancing to the next
        // level, project from the current MPP out to the new beta radius or
        // response level.
        if ( (mppSearchFlag == 2 || mppSearchFlag == 3) && !amv_plus_converged )
          // Warm start init_pt for the next AMV+ iteration
          copy_data(mostProbPointU, init_pt);
        else { // Warm start init_pt for the next level
          if (ria_flag) { // For RIA case, project along fnGradU to next g level
            // using linear Taylor series.  g2 = g1 + dg/gu^T (u2 - u1) where
            // u2 - u1 = alpha*dg/du gives alpha = (g2 - g1)/(dg/gu^T dg/gu).
            if (levelCount < rl_len-1) {
              Real norm_grad_sq = 0.;
              for (i=0; i<numUncertainVars; i++)
                norm_grad_sq += pow(fnGradU(i), 2);
              if ( norm_grad_sq > 1.e-10 ) {// also handles NPSOL numerical case
                Real alpha = (requestedRespLevels[respFnCount][levelCount+1] -
                              requestedRespLevel)/norm_grad_sq;
                for (i=0; i<numUncertainVars; i++)
                  init_pt[i] = mostProbPointU(i) + fnGradU(i)*alpha;
              }
            }
          }
          else { // For PMA case, scale mostProbPointU so that its magnitude
                 // equals the next beta_target.
            if (levelCount < num_levels-1) {
              Real next_pl
                = requestedProbLevels[respFnCount][levelCount-rl_len+1];
              Real next_cdf_pl = (cdfFlag) ? next_pl : 1. - next_pl;
              if ( std::fabs(next_cdf_pl - 0.5)           > 1.e-10 &&
                   std::fabs(requestedCDFProbLevel - 0.5) > 1.e-10 ) {
                Real scale_factor
                  = phi_inverse(next_cdf_pl)/phi_inverse(requestedCDFProbLevel);
                for (i=0; i<numUncertainVars; i++)
                  init_pt[i] = mostProbPointU(i)*scale_factor;
              }
            }
          }
        }
      } // end AMV+ while loop
      final_stats[cntr++] = (ria_flag)
        ? computedProbLevels[respFnCount][levelCount]
        : computedRespLevels[respFnCount][levelCount];
    } // end loop over levels
  } // end loop over response fns
  finalStatistics.function_values(final_stats);
}


#ifdef HAVE_NPSOL
void NonDUnilevelRBDO::
RBDO_objective_eval(int& mode, int& n, Real* d_and_u, Real& f, Real* grad_f,
		    int&)
{
  // ----------------------------------------
  // The RBDO RIA/PMA objective function is f
  // ----------------------------------------

  // NPSOL mode: 0 = get f, 1 = get grad_f, 2 = get both
  // promote mode to standard asv request codes
  int i, asv_request = mode + 1;

  DakotaRealVector d_and_mean_x(staticNumDesVars + staticNumUncVars);
  for (i=0; i<staticNumDesVars; i++)
    d_and_mean_x[i] = d_and_u[i];
  for (i=0; i<staticNumUncVars; i++)
    d_and_mean_x[i+staticNumDesVars] = ranVarMeans[i];

  if (asv_request) {
    staticNonDModel.continuous_variables(d_and_mean_x);
    // perform a new fn evaluation (separate from constraint evaluations)
    // with ASV[0] = mode.
    DakotaIntArray local_asv(staticNumFuncs, 0);
    local_asv[0] = asv_request; // eval the obj fn only
    staticNonDModel.compute_response(local_asv);
  }
  if (asv_request & 1) {
    const DakotaRealVector& local_fns
      = staticNonDModel.current_response().function_values();
    f = local_fns[0];
  }
  if (asv_request & 2) {
    const DakotaRealMatrix& local_grads
      = staticNonDModel.current_response().function_gradients();
    copy_data(local_grads[0], fnGradX);
    Epetra_SerialDenseMatrix jacobian_xu;
    jacXToU(x, jacobian_xu);
    fnGradU.Multiply('T', 'N', 1., jacobian_xu, fnGradX, 0.);
    for (int i=0; i<n; i++)
      grad_f[i] = fnGradU(i);
  }
}


void NonDUnilevelRBDO::
RBDO_RIA_constraint_eval(int& mode, int& ncnln, int& n, int& nrowj, int* needc,
			 Real* d_and_u, Real* c, Real *cjac, int& nstate)
{
  // ------------------------------------------------------
  // The RBDO RIA inequality constraint is (norm u)^2 - beta^2 = 0
  // The RBDO RIA equality constraints are ...
  // Plus any additional deterministic design inequalities...
  // ------------------------------------------------------

  // NPSOL mode: 0 = get active constraint values using needc
  //             1 = get active constraint gradients using needc
  //             2 = get active constraint values and gradients using needc
  // Currently, needc is ignored which is OK for two reasons: (1) there is only
  // one constraint (so one would be expect that constraint to be active if the
  // function is called), and (2) SOLBase::constraint_eval() sets *all* of cjac
  // and inactive entries are successfully ignored.


  // promote mode to standard asv request codes
  int i, asv_request = mode + 1, cntr = 0;

  DakotaRealVector d_and_x_i(staticNumDesVars + staticNumUncVars),
    u_i(staticNumUncVars), x_i(staticNumUncVars);
  for (i=0; i<staticNumDesVars; i++)
    d_and_x_i[i] = d_and_u[i];

  if (asv_request & 1) { // TO DO: needc for multiple constraints

    for (i=0; i<numLimitStates; i++) { // TO DO

      double norm_u_sq = 0.;
      for (j=0; j<staticNumUncVars; j++) {
	u_i[j] = d_and_u[j+i*staticNumUncVars];
	norm_u_sq += pow(u_i[j], 2);
      }
      double norm_u = sqrt(norm_u_sq);
      transUToX(u_i, x_i);
      for (j=0; j<staticNumUncVars; j++)
	d_and_x_i[j+staticNumDesVars] = x_i[j];

      // Perform a separate fn eval for d and x_i with ASV[respFn] = mode
      staticNonDModel.continuous_variables(d_and_x_i);
      DakotaIntArray local_asv(staticNumFuncs, 0);
      local_asv[i+1] = 3; // need g/grad_g: KKT eq constr always active
      staticNonDModel.compute_response(local_asv);
      const DakotaRealVector& local_fns
	= staticNonDModel.current_response().function_values();
      const DakotaRealMatrix& local_grads
	= staticNonDModel.current_response().function_gradients();
      copy_data(local_grads[respFnCount], fnGradX);
      Epetra_SerialDenseMatrix jacobian_xu;
      jacXToU(x, jacobian_xu);
      fnGradU.Multiply('T', 'N', 1., jacobian_xu, fnGradX, 0.);

      // -------------------------------------
      // 1 design inequality for limit state i
      // -------------------------------------
      double beta_target = phi_inverse(requestedCDFProbLevel);
      c[cntr++] = norm_u_sq - pow(beta_target, 2); // u'u - beta_target^2

      // ----------------------------------
      // 2 KKT equalities for limit state i
      // ----------------------------------
      // h1_i = fnGradU^T u_i + norm(fnGradU)*norm(u_i) = 0.
      c[cntr] = 0.;
      double norm_fn_grad_u = 0.;
      for (j=0; j<staticNumUncVars; j++) {
	norm_fn_grad_u += pow(fnGradU[j], 2);
	c[cntr] += fnGradU(i)*u_i[i];
      }
      norm_fn_grad_u = sqrt(norm_fn_grad_u);
      c[cntr++] += norm_u*norm_fn_grad_u;

      // h2_i = g_i - requestedRespLevel_i = 0.
      c[cntr++] = local_fns[i+1] - requestedRespLevel;
    }

    // -----------------------------------------
    // Additional design inequalities/equalities
    // -----------------------------------------


  if (asv_request & 2) {
    for (int i=0; i<n; i++)
      grad_f[i] = fnGradU(i);
  }

  if ( (asv_request & 2) && npsolDerivLevel >= 2) {
    for (i=0; i<n; i++)
      cjac[i] = 2*u[i]; // gradient of constraint = 2u
                        // f77 loop reversal can be ignored for 1 constraint
  }
}


void NonDUnilevelRBDO::
RBDO_PMA_constraint_eval(int& mode, int& ncnln, int& n, int& nrowj, int* needc,
			 Real* u, Real* c, Real* cjac, int& nstate)
{
  // --------------------------------------------------------
  // The RBDO PMA inequality constraint is g* - response level >= 0
  // The RBDO PMA equality constraints are ...
  // --------------------------------------------------------

  // NPSOL mode: 0 = get active constraint values using needc
  //             1 = get active constraint gradients using needc
  //             2 = get active constraint values and gradients using needc
  // Currently, needc is ignored which is OK for two reasons: (1) there is only
  // one constraint (so one would be expect that constraint to be active if the
  // function is called), and (2) SOLBase::constraint_eval() sets *all* of cjac
  // and inactive entries are successfully ignored.

  Epetra_SerialDenseVector petra_u;
  copy_data(u, n, petra_u);

  // suppress gradient requests if NPSOL derivative level is not 2 or 3.
  int offset_mode = mode + 1;
  int asv_request = (offset_mode & 1) ? 1 : 0;
  if ( (offset_mode & 2) && npsolDerivLevel >= 2 )
    asv_request += 2;

  Real g;
  g_eval(asv_request, petra_u, g); // convenience function

  if (asv_request & 1)
    c[0] = g - requestedRespLevel;
  if (asv_request & 2) {
    for (int i=0; i<n; i++)
      cjac[i] = fnGradU(i); // f77 loop reversal can be ignored for 1 constr.
  }
}


#elif defined (HAVE_OPTPP)
void NonDUnilevelRBDO::
RIA_objective_eval(int mode, int n, const ColumnVector& u, Real& f,
                   ColumnVector& grad_f, int& result_mode)
{
  // ----------------------------------------
  // The RIA objective function is (norm u)^2
  // ----------------------------------------

  int i;
  if (mode & 1) {
    f = 0.;
    for (i=1; i<=n; i++)
      f += pow(u(i), 2); // objective function is u'u
    result_mode = NLPFunction;
  }
  if (mode & 2) {
    for (i=1; i<=n; i++)
      grad_f(i) = 2*u(i); // gradient of objective function is 2u
    result_mode = result_mode | NLPGradient;
  }
}


void NonDUnilevelRBDO::
RIA_constraint_eval(int mode, int n, const ColumnVector& u, ColumnVector& c,
                    Matrix& grad_c, int& result_mode)
{
  // --------------------------------------------------------
  // The RIA equality constraint is G(u) - response level = 0
  // --------------------------------------------------------

  Real g;
  Epetra_SerialDenseVector petra_u;
  copy_data(u, petra_u);
  g_eval(mode, petra_u, g); // convenience function

  if (mode & 1) {
    c(1) = g - requestedRespLevel;
    result_mode = NLPFunction;
  }
  if (mode & 2) {
    for (int i=0; i<n; i++)
      grad_c(i+1, 1) = fnGradU(i); // grad_c transposed from DAKOTA grads
    result_mode = result_mode | NLPGradient;
  }
}


void NonDUnilevelRBDO::
PMA_objective_eval(int mode, int n, const ColumnVector& u, Real& f,
                   ColumnVector& grad_f, int& result_mode)
{
  // ----------------------------------
  // The PMA objective function is G(u)
  // ----------------------------------

  Epetra_SerialDenseVector petra_u;
  copy_data(u, petra_u);

  Real g;
  g_eval(mode, petra_u, g); // convenience function

  // The following formulation corresponds to a failure region defined by
  // G(u) > z.  Many other researchers use G(u) < 0 for the failure region,
  // which flips the minimize/maximize sense.
  // CDF probability < 0.5  -->  p_fail > 0.5  -->  beta < 0  -->  minimize g
  // CDF probability > 0.5  -->  p_fail < 0.5  -->  beta > 0  -->  maximize g
  // CDF probability = 0.5  -->  p_fail = 0.5  -->  beta = 0  -->  compute g
  // Note: "compute g" means that minimize/maximize is irrelevant since there 
  // should only be one G(u) intersection when the circle of radius beta
  // collapses to a point.
  if (mode & 1) {
    f = (requestedCDFProbLevel <= 0.5) ? g : -g; // no additional terms
    result_mode = NLPFunction;
  }
  if (mode & 2) {
    for (int i=0; i<n; i++)
      grad_f(i+1) = (requestedCDFProbLevel <= 0.5) ? fnGradU(i) : -fnGradU(i);
    result_mode = result_mode | NLPGradient;
  }
}


void NonDUnilevelRBDO::
PMA_constraint_eval(int mode, int n, const ColumnVector& u, ColumnVector& c,
                    Matrix& grad_c, int& result_mode)
{
  // ------------------------------------------------------
  // The PMA equality constraint is (norm u)^2 - beta^2 = 0
  // ------------------------------------------------------

  int i;
  if (mode & 1) {
    // calculate the reliability index (beta)
    double beta_sq = 0.;
    for (i=1; i<=n; i++)
      beta_sq += pow(u(i), 2); // better to use beta^2 = u'u since its gradient
                               // is not singular at the origin.

    // Convert the desired probability to a desired reliability index.
    double beta_target = phi_inverse(requestedCDFProbLevel);

    // calculate the constraint
    c(1) = beta_sq - pow(beta_target, 2); // u'u - beta_target^2
    result_mode = NLPFunction;
  }
  if (mode & 2) {
    for (i=1; i<=n; i++)
      grad_c(i, 1) = 2*u(i); // gradient of constraint is 2u
                             // grad_c transposed from DAKOTA grads
    result_mode = result_mode | NLPGradient;
  }
}
#endif


void NonDUnilevelRBDO::
g_eval(int& mode, const Epetra_SerialDenseVector& u, Real& g)
{
  // -------------------------------------------------------
  // Convenience function for evaluating G(u) and fnGradU(u)
  // -------------------------------------------------------

  Epetra_SerialDenseVector x;
  transUToX(u, x);

  int i, n = u.Length();
  switch (mppSearchFlag) {
  case 1: // AMV: linear approx in x centered at means
    if (mode & 1) {
      g = fnValsMeanX(respFnCount);
      for (i=0; i<n; i++)
        g += fnGradX(i) * (x(i) - ranVarMeans(i));
    }
    if (mode & 2) {
      // dg/du = dg/dx * dx/du
      // where dg/dx = fnGradX and is a constant (single linearization in x)
      // and dx/du is jacobian_xu and varies with x
      Epetra_SerialDenseMatrix jacobian_xu;
      jacXToU(x, jacobian_xu);
      // OK to update fnGradU array at x since it is not used elsewhere for AMV.
      fnGradU.Multiply('T', 'N', 1., jacobian_xu, fnGradX, 0.);
    }
    break;
  case 2: // AMV+: linear approx in x centered at MPP
    if (mode & 1) {
      g = computedRespLevels[respFnCount][levelCount];
      for (i=0; i<n; i++)
        g += fnGradX(i) * (x(i) - mostProbPointX(i));
    }
    if (mode & 2) {
      // dg/du = dg/dx * dx/du
      // where dg/dx = fnGradX and is a constant for each linearization in x
      // and dx/du is jacobian_xu and varies with x
      Epetra_SerialDenseMatrix jacobian_xu;
      jacXToU(x, jacobian_xu);
      // OK to update fnGradU array at x since it is not used elsewhere for AMV+
      fnGradU.Multiply('T', 'N', 1., jacobian_xu, fnGradX, 0.);
    }
    break;
  case 3: // transformed AMV+: linear approx in u centered at MPP
    if (mode & 1) {
      g = computedRespLevels[respFnCount][levelCount];
      for (i=0; i<n; i++)
        g += fnGradU(i) * (u(i) - mostProbPointU(i));
    }
    // dg/du = fnGradU and is a constant for each linearization in u
    break;
  case 4: // FORM: no linearization
    if (mode) {
      DakotaRealVector local_unc_vars(staticNumUncVars);
      copy_data(x, local_unc_vars);
      staticNonDModel.continuous_variables(local_unc_vars);
      DakotaIntArray local_asv(staticNumFuncs, 0);
      local_asv[respFnCount] = mode; // eval the active response fn only
      staticNonDModel.compute_response(local_asv);
    }
    if (mode & 1) {
      const DakotaRealVector& local_fns
        = staticNonDModel.current_response().function_values();
      g = local_fns[respFnCount];
    }
    if (mode & 2) {
      const DakotaRealMatrix& local_grads
        = staticNonDModel.current_response().function_gradients();
      // OK to update fnGradX/fnGradU arrays at x since they are not used
      // elsewhere for any linearizations.
      copy_data(local_grads[respFnCount], fnGradX);
      Epetra_SerialDenseMatrix jacobian_xu;
      jacXToU(x, jacobian_xu);
      fnGradU.Multiply('T', 'N', 1., jacobian_xu, fnGradX, 0.);
    }
    break;
  default:
    Cerr << "Bad mppSearchFlag in g_eval." << std::endl;
    abort_handler(-1);
  }
  //Cout << "u: " << u << "\njacobian_xu: " << jacobian_xu << "\nfnGradU: "
  //     << fnGradU;
}


void NonDUnilevelRBDO::print_iterator_results(std::ostream& s) const
{
  size_t i, j;
  const DakotaStringArray& uv_labels
    = userDefinedModel.continuous_variable_labels();
  const DakotaStringArray& fn_tags
    = userDefinedModel.current_response().fn_tags();
  DakotaString dist_type_long = (cdfFlag) ?
    "Cumulative Distribution Function (CDF) for " :
    "Complementary Cumulative Distribution Function (CCDF) for ";
  DakotaString dist_type_short = (cdfFlag) ? "  CDF at Response Level = "
                                           : "  CCDF at Response Level = ";
  s << "-----------------------------------------------------------------\n";

  for (i=0; i<numFunctions; i++) {

    // output MV-specific statistics
    if (!mppSearchFlag) {
      s << "MV Statistics for " << fn_tags[i] << ":\n";
      // approximate response means and std deviations and importance factors
      s << "  Approximate Mean Response                  = " << meanStats[i]
	<< "\n  Approximate Standard Deviation of Response = " << stdDevStats[i]
	<< '\n';
      if (correlationFlag || stdDevStats[i] <= 1.e-25)
	s << "  Importance Factors not available.\n";
      else {
	s << std::setiosflags(std::ios::left);
	for (j=0; j<numUncertainVars; j++)
	  s << "  Importance Factor for variable " << std::setw(11)
	    << uv_labels[j].data() << " = " << impFactor[i][j] << '\n';
	s << std::resetiosflags(std::ios::adjustfield);
      }
    }

    // output CDF/CCDF response/probability pairs
    size_t num_levels = computedRespLevels[i].length();
    if (num_levels) {
      if (!mppSearchFlag && stdDevStats[i] <= 1.e-25)
        s << "\nWarning: negligible standard deviation renders CDF results "
          << "suspect.\n\n";
      s << dist_type_long << fn_tags[i] << ":\n";
      for (j=0; j<num_levels; j++)
        s << dist_type_short << std::setw(write_precision+7)
          << computedRespLevels[i][j] << "  is " << std::setw(write_precision+7)
          << computedProbLevels[i][j] << '\n';
    }
  }
  //s << "\n  UQ results should be examined carefully to verify successful "
  //  << "completion\n  of the optimization.  Stagnation sometimes occurs "
  //  << "with the message:\n  Current solution cannot be improved on.\n";

  //Cout << "Final statistics:\n" << finalStatistics;

  s << "-----------------------------------------------------------------"
    << std::endl;
}
