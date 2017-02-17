/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       NLPQLPOptimizer
//- Description: Implementation code for the NLPQLPOptimizer class
//- Owner:       Bengt Abel
//- Checked by:  Mike Eldred

#include "DakotaModel.hpp"
#include "DakotaResponse.hpp"
#include "NLPQLPOptimizer.hpp"
#include "ProblemDescDB.hpp"
#include "ParallelLibrary.hpp"

static const char rcsId[]="@(#) $Id: NLPQLPOptimizer.cpp 7029 2010-10-22 00:17:02Z mseldre $";

#define QL_F77     F77_FUNC(ql,QL)
#define NLPQLP_F77 F77_FUNC(nlpqlp,NLPQLP)

extern "C" {

void QL_F77(int&, int&, int&, int&, int&, int&, double*,
	    double*, double*, double*, double*, double*,
	    double*, double*, double&, int&, int&, int&,
	    int&, double*, int&, int*, int&);

void NLPQLP_F77(int&, int&, int&, int&, int&, int&, int&, double*, double*,
		double*, double*, double*, double*, double*, double*, double*,
		double*, double&, double&, double&, int&, int&, int&, double&,
		int&, int&, int&, int&, double*, int&, int*, int&, int*, int&,
		int&, void (*QL_F77)(int&, int&, int&, int&, int&, int&,
				     double*, double*, double*, double*,
				     double*, double*, double*, double*,
				     double&, int&, int&, int&, int&, double*,
				     int&, int*, int&));

}

namespace Dakota {

NLPQLPOptimizer::NLPQLPOptimizer(ProblemDescDB& problem_db, Model& model):
  Optimizer(problem_db, model)
{ initialize(); }


NLPQLPOptimizer::NLPQLPOptimizer(Model& model): Optimizer(NLPQL_SQP, model)
{ initialize(); }


#ifdef HAVE_DYNLIB_FACTORIES
NLPQLPOptimizer* new_NLPQLPOptimizer(ProblemDescDB& problem_db, Model& model)
{
#ifdef DAKOTA_DYNLIB
  not_available("NLPQLP");
  return 0;
#else
  return new NLPQLPOptimizer(problem_db, model);
#endif // DAKOTA_DYNLIB
}

NLPQLPOptimizer* new_NLPQLPOptimizer(Model& model)
{
#ifdef DAKOTA_DYNLIB
  not_available("NLPQLP");
  return 0;
#else
  return new NLPQLPOptimizer(model);
#endif // DAKOTA_DYNLIB
}
#endif // HAVE_DYNLIB_FACTORIES


void NLPQLPOptimizer::initialize()
{
  // NLPQLP does not support internal calculation of numerical derivatives
  if (vendorNumericalGradFlag) {
    Cerr << "\nError: vendor numerical gradients not supported by nlpql_sqp."
	 << "\n       Please select dakota numerical instead." << std::endl;
    abort_handler(-1);
  }

  // Prevent nesting of an instance of a Fortran iterator within another
  // instance of the same iterator (which would result in data clashes since
  // Fortran does not support object independence).  Recurse through all
  // sub-models and test each sub-iterator for NLPQL presence.
  Iterator sub_iterator = iteratedModel.subordinate_iterator();
  if (!sub_iterator.is_null() && 
       ( sub_iterator.method_name() == NLPQL_SQP  ||
	 sub_iterator.uses_method() == NLPQL_SQP ) )
    sub_iterator.method_recourse();
  ModelList& sub_models = iteratedModel.subordinate_models();
  for (ModelLIter ml_iter = sub_models.begin();
       ml_iter != sub_models.end(); ml_iter++) {
    sub_iterator = ml_iter->subordinate_iterator();
    if (!sub_iterator.is_null() && 
	 ( sub_iterator.method_name() == NLPQL_SQP  ||
	   sub_iterator.uses_method() == NLPQL_SQP ) )
      sub_iterator.method_recourse();
  }

  // Set NLPQL optimization controls 
  L      = 1;
  ACC    = 1.0e-9;
  ACCQP  = 1.0e-11;
  STPMIN = 0;
  MAXFUN = 10; // max fn evals per line search
  MAXIT  = maxIterations;
  MAX_NM = 10;
  TOL_NM = 0.1;
  MODE   = 0;
  IOUT   = 6;
  LQL    = 1;

  switch (outputLevel) {
  case DEBUG_OUTPUT:
    IPRINT = 4; break;
  case VERBOSE_OUTPUT:
    IPRINT = 2; break;
  case SILENT_OUTPUT:
    IPRINT = 0; break;
  case NORMAL_OUTPUT: default:
    IPRINT = 1; break;
  }
}


NLPQLPOptimizer::~NLPQLPOptimizer()
{
  // Virtual destructor handles referenceCount at Iterator level
}


void NLPQLPOptimizer::allocate_constraints()
{
  // NLPQL handles equality constraints = 0 and 1-sided inequalities >= 0.
  // Compute the number of equalities and 1-sided inequalities to pass to NLPQL
  // as well as the mappings (indices, multipliers, offsets) between the DAKOTA
  // constraints and the NLPQL constraints.
  size_t i, num_nln_ineq = iteratedModel.num_nonlinear_ineq_constraints(),
    num_lin_ineq   = iteratedModel.num_linear_ineq_constraints(),
    num_nlpql_ineq = 0;
  const RealVector& nln_ineq_lwr_bnds
    = iteratedModel.nonlinear_ineq_constraint_lower_bounds();
  const RealVector& nln_ineq_upr_bnds
    = iteratedModel.nonlinear_ineq_constraint_upper_bounds();
  const RealVector& lin_ineq_lwr_bnds
    = iteratedModel.linear_ineq_constraint_lower_bounds();
  const RealVector& lin_ineq_upr_bnds
    = iteratedModel.linear_ineq_constraint_upper_bounds();
  numEqConstraints = iteratedModel.num_nonlinear_eq_constraints()
                   + iteratedModel.num_linear_eq_constraints();
  for (i=0; i<num_nln_ineq; i++) {
    if (nln_ineq_lwr_bnds[i] > -bigRealBoundSize) {
      num_nlpql_ineq++;
      // dakota_constraint - nln_ineq_lower_bnd >= 0
      nonlinIneqConMappingIndices.push_back(i);
      nonlinIneqConMappingMultipliers.push_back(1.0);
      nonlinIneqConMappingOffsets.push_back(-nln_ineq_lwr_bnds[i]);
    }
    if (nln_ineq_upr_bnds[i] < bigRealBoundSize) {
      num_nlpql_ineq++;
      // nln_ineq_upper_bnd - dakota_constraint >= 0
      nonlinIneqConMappingIndices.push_back(i);
      nonlinIneqConMappingMultipliers.push_back(-1.0);
      nonlinIneqConMappingOffsets.push_back(nln_ineq_upr_bnds[i]);
    }
  }
  for (i=0; i<num_lin_ineq; i++) {
    if (lin_ineq_lwr_bnds[i] > -bigRealBoundSize) {
      num_nlpql_ineq++;
      // Ax - lin_ineq_lower_bnd >= 0
      linIneqConMappingIndices.push_back(i);
      linIneqConMappingMultipliers.push_back(1.0);
      linIneqConMappingOffsets.push_back(-lin_ineq_lwr_bnds[i]);
    }
    if (lin_ineq_upr_bnds[i] < bigRealBoundSize) {
      num_nlpql_ineq++;
      // lin_ineq_upper_bnd - Ax >= 0
      linIneqConMappingIndices.push_back(i);
      linIneqConMappingMultipliers.push_back(-1.0);
      linIneqConMappingOffsets.push_back(lin_ineq_upr_bnds[i]);
    }
  }
  numNlpqlConstr = numEqConstraints + num_nlpql_ineq;
}


void NLPQLPOptimizer::allocate_workspace()
{
  N    = numContinuousVars;
  MMAX = (numNlpqlConstr > 0) ? numNlpqlConstr : 1;
  NMAX = N + 1;
  MNN2 = numNlpqlConstr + 2*N + 2;

  LWA     = (int)std::floor(1.5*(double)(NMAX*NMAX))
          + 10*NMAX + 23*N + 5*numNlpqlConstr + 4*MMAX + 217;
  LKWA    = N + 10;
  LACTIVE = 2*numNlpqlConstr + 10;

  X      = new Real [NMAX*L];
  F      = new Real [L];
  G      = new Real [MMAX*L];
  DF     = new Real [NMAX];
  DG     = new Real [MMAX*NMAX];
  // Need to initialize this data to avoid valgrind errors (TPL should do it but doesn't)
  for( int i=0; i<MMAX*NMAX; ++i )
    DG[i] = 0;
  U      = new Real [MNN2];
  C      = new Real [NMAX*NMAX];
  D      = new Real [NMAX];
  WA     = new Real [LWA];
  KWA    = new  int [LKWA];
  ACTIVE = new  int [LACTIVE];
  // Need to initialize this data to avoid valgrind errors (TPL should do it but doesn't)
  for( int i=0; i<LACTIVE; ++i )
    ACTIVE[i] = 1; // These need to be non-zero so that all constraints are active. - RWH
}


void NLPQLPOptimizer::deallocate_workspace()
{
  delete [] X;
  delete [] F;
  delete [] G;
  delete [] DF;
  delete [] DG;
  delete [] U;
  delete [] C;
  delete [] D;
  delete [] WA;
  delete [] KWA;
  delete [] ACTIVE;
}


void NLPQLPOptimizer::initialize_run()
{
  Optimizer::initialize_run();

  allocate_constraints();
  allocate_workspace();

  const RealVector& local_cdv = iteratedModel.continuous_variables();
  for (size_t i=0; i<numContinuousVars; i++)
    X[i] = local_cdv[i]; // Note: X is [NMAX,L]

  IFAIL = 0; // initialize to zero prior to first NLPQLP call
}


void NLPQLPOptimizer::core_run()
{
  // TO DO: utilize L concurrency with evaluate_nowait()/synchronize()

  const RealVector& cdv_lower_bnds
    = iteratedModel.continuous_lower_bounds();
  const RealVector& cdv_upper_bnds
    = iteratedModel.continuous_upper_bounds();
  size_t i, j, fn_eval_cntr,
    num_nln_ineq = iteratedModel.num_nonlinear_ineq_constraints(),
    num_nln_eq   = iteratedModel.num_nonlinear_eq_constraints();
  const RealMatrix& lin_ineq_coeffs
    = iteratedModel.linear_ineq_constraint_coeffs();
  const RealMatrix& lin_eq_coeffs
    = iteratedModel.linear_eq_constraint_coeffs();
  const RealVector& nln_eq_targets
    = iteratedModel.nonlinear_eq_constraint_targets();
  const RealVector& lin_eq_targets
    = iteratedModel.linear_eq_constraint_targets();

  // Any MOO/NLS recasting is responsible for setting the scalar min/max
  // sense within the recast.
  const BoolDeque& max_sense = iteratedModel.primary_response_fn_sense();
  bool max_flag = (!max_sense.empty() && max_sense[0]);

  // Prior to first call to NLPQLP, initialize X to initial guess,
  // IFAIL to 0, and F/G/DF/DG to objective/constraint values/gradients
  // (refer to p. 18 in NLPQLP20.pdf)

  for (fn_eval_cntr=1; fn_eval_cntr<=maxFunctionEvals; fn_eval_cntr++) {

    // map ACTIVE array into activeSet
    if (IFAIL == -1) // need all functions
      activeSet.request_values(1);
    else if (IFAIL == -2) { // need ACTIVE gradients
      activeSet.request_values(0);
      for (i=0; i<numObjectiveFns; i++)
        activeSet.request_value(2, i); // objective function(s)
      for (i=0; i<num_nln_eq; i++)     // nonlinear eq
	if (ACTIVE[i])
	  activeSet.request_value(2, numObjectiveFns+num_nln_ineq+i);
      size_t  cntr = numEqConstraints;
      StLIter i_iter;
      for (i_iter  = nonlinIneqConMappingIndices.begin(); // nonlinear ineq
	   i_iter != nonlinIneqConMappingIndices.end(); i_iter++)
	if (ACTIVE[cntr++])
	  activeSet.request_value(2, (*i_iter)+numObjectiveFns);
    }
    else // initial evaluation: need all functions/gradients
      activeSet.request_values(3);

    // compute the response
    if (fn_eval_cntr > 1) {
      RealVector local_cdv(N, false);
      copy_data(X, N, local_cdv); // Note: X is [NMAX,L]
      iteratedModel.continuous_variables(local_cdv);
    }
    iteratedModel.evaluate(activeSet);
    const Response& local_response = iteratedModel.current_response();

    // pack up the response function values
    if (IFAIL == 0 || IFAIL == -1) {
      const RealVector& local_fns = local_response.function_values();
      F[0] = (max_flag) ? -local_fns[0] : local_fns[0];
      for (i=0; i<numEqConstraints; i++) {
	if (i<num_nln_eq)                // nonlinear eq
	  G[i] = local_fns[i+num_nln_ineq+1] - nln_eq_targets[i];
	else {                           // linear eq
	  size_t index = i - num_nln_eq;
          Real Ax = 0.;
	  for (j=0; j<numContinuousVars; j++)
	    Ax += lin_eq_coeffs(index,j) * X[j];
	  G[i] = Ax - lin_eq_targets[index];
	}
      }
      StLIter i_iter;
      RLIter  m_iter, o_iter;
      size_t  cntr = numEqConstraints;
      for (i_iter  = nonlinIneqConMappingIndices.begin(),
	   m_iter  = nonlinIneqConMappingMultipliers.begin(),
	   o_iter  = nonlinIneqConMappingOffsets.begin();
	   i_iter != nonlinIneqConMappingIndices.end();
	   i_iter++, m_iter++, o_iter++)   // nonlinear ineq
	G[cntr++] = (*o_iter) + (*m_iter) * local_fns[(*i_iter)+1];
      for (i_iter  = linIneqConMappingIndices.begin(),
	   m_iter  = linIneqConMappingMultipliers.begin(),
	   o_iter  = linIneqConMappingOffsets.begin();
	   i_iter != linIneqConMappingIndices.end();
	   i_iter++, m_iter++, o_iter++) { // linear ineq
	size_t index = *i_iter;
	Real Ax = 0.;
	for (j=0; j<numContinuousVars; j++)
	  Ax += lin_ineq_coeffs(index,j) * X[j];
	G[cntr++] = (*o_iter) + (*m_iter) * Ax;
      }
    }

    // pack up the response function gradients
    if (IFAIL == 0 || IFAIL == -2) {
      const RealMatrix& local_grads = local_response.function_gradients();
      size_t index;
      if (max_flag)
	for (j=0; j<numContinuousVars; j++)
	  DF[j] = -local_grads(j,0);
      else
	for (j=0; j<numContinuousVars; j++)
	  DF[j] =  local_grads(j,0);
      for (i=0; i<numEqConstraints; i++) {
	if (ACTIVE[i]) {
	  if (i<num_nln_eq) {              // nonlinear eq
	    index = i + num_nln_ineq + 1;
	    for (j=0; j<numContinuousVars; j++)
	      DG[i+MMAX*j] = local_grads(j,index);
	  }
	  else {                           // linear eq
	    index = i - num_nln_eq;
	    for (j=0; j<numContinuousVars; j++)
	      DG[i+MMAX*j] = lin_eq_coeffs(index,j);
	  }
	}
      }
      StLIter i_iter;
      RLIter  m_iter, o_iter;
      size_t  cntr = numEqConstraints;
      Real    mult;
      for (i_iter  = nonlinIneqConMappingIndices.begin(),
	   m_iter  = nonlinIneqConMappingMultipliers.begin(),
	   o_iter  = nonlinIneqConMappingOffsets.begin();
	   i_iter != nonlinIneqConMappingIndices.end();
	   i_iter++, m_iter++, o_iter++) { // nonlinear ineq
	if (ACTIVE[cntr]) {
	  index = *i_iter;
	  mult  = *m_iter;
	  for (j=0; j<numContinuousVars; j++)
	    DG[cntr+MMAX*j] = mult * local_grads(j,index+1);
	}
	cntr++;
      }
      for (i_iter  = linIneqConMappingIndices.begin(),
	   m_iter  = linIneqConMappingMultipliers.begin(),
	   o_iter  = linIneqConMappingOffsets.begin();
	   i_iter != linIneqConMappingIndices.end();
	   i_iter++, m_iter++, o_iter++) { // linear ineq
	if (ACTIVE[cntr]) {
	  index = *i_iter;
	  mult  = *m_iter;
	  for (j=0; j<numContinuousVars; j++)
	    DG[cntr+MMAX*j] = mult * lin_ineq_coeffs(index,j);
	}
	cntr++;
      }
    }

    // call NLPQLP
    NLPQLP_F77(L, numNlpqlConstr, numEqConstraints, MMAX, N, NMAX, MNN2, X, F,
	       G, DF, DG, U, cdv_lower_bnds.values(), cdv_upper_bnds.values(),
	       C, D, ACC, ACCQP, STPMIN, MAXFUN, MAXIT, MAX_NM, TOL_NM, IPRINT,
	       MODE, IOUT, IFAIL, WA, LWA, KWA, LKWA, ACTIVE, LACTIVE, LQL,
	       QL_F77);

    if (IFAIL >= 0)
      break;
  }

  // NLPQLP completed
  Cout << "\nNLPQLP exits with IFAIL code = " << IFAIL;
  switch (IFAIL) {
  case -2: case -1:
    Cout << " (max function evals exceeded).\n";
    break;
  case 0:
    Cout << " (optimal solution found).\n";
    break;
  case 1:
    Cout << " (max iterations exceeded).\n";
    break;
  case 2: case 4:
    Cout << " (line search failure).\n"
	 << "Refer to NLPQLP20.pdf for additional information.\n";
    break;
  case 5: case 6: case 8: case 9:
    Cout << " (NLPQLP input error).\n"
	 << "Refer to NLPQLP20.pdf for additional information.\n";
    break;
  default:
    Cout << " (numerical problem).\n"
	 << "Refer to NLPQLP20.pdf for additional information.\n";
    break;
  }

  // Publish optimal solution
  RealVector local_cdv(N, false);
  copy_data(X, N, local_cdv); // Note: X is [NMAX,L]
  bestVariablesArray.front().continuous_variables(local_cdv);
  if (!localObjectiveRecast) { // else local_objective_recast_retrieve()
                               // is used in Optimizer::post_run()
    RealVector best_fns(numFunctions);
    best_fns[0] = (max_flag) ? -F[0] : F[0];

    StLIter i_iter;
    RLIter  m_iter, o_iter;
    size_t  cntr = numEqConstraints;
    for (i_iter  = nonlinIneqConMappingIndices.begin(),
	 m_iter  = nonlinIneqConMappingMultipliers.begin(),
	 o_iter  = nonlinIneqConMappingOffsets.begin();
	 i_iter != nonlinIneqConMappingIndices.end();
	 i_iter++, m_iter++, o_iter++)   // nonlinear ineq
      best_fns[(*i_iter)+1] = (G[cntr++] - (*o_iter))/(*m_iter);

    size_t i, 
      num_nln_ineq = iteratedModel.num_nonlinear_ineq_constraints(),
      num_nln_eq = iteratedModel.num_nonlinear_eq_constraints();
    const RealVector& nln_eq_targets
      = iteratedModel.nonlinear_eq_constraint_targets();
    for (i=0; i<num_nln_eq; i++)
      best_fns[i+num_nln_ineq+1] = G[i] + nln_eq_targets[i];

    bestResponseArray.front().function_values(best_fns);
  }
  deallocate_workspace();
}

} // namespace Dakota
