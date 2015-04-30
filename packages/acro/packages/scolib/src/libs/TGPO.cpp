/*  _________________________________________________________________________
 *
 *  Acro: A Common Repository for Optimizers
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README.txt file in the top Acro directory.
 *  _________________________________________________________________________
 */


#include <acro_config.h>
#include <scolib/TGPO.h>

#if defined(ACRO_HAVE_R)

#include <colin/SolverMngr.h>
#include <colin/cache/View_Unlabeled.h>
#include <colin/cache/View_Pareto.h>

using utilib::Any;
using utilib::Property;

using std::string;
using std::vector;
using std::set;
using std::pair;
using std::make_pair;
using std::cout;
using std::endl;

#include <R.h>
#include <Rembedded.h>
#include <Rinternals.h>
#include <Rdefines.h>

#define ADD_TERM(EXPR, ARG)                                             \
   if ( !ARG.empty() ) {                                                \
      if ( ARG.is_type(typeid(SEXP)) ) {                                \
         SETCAR(EXPR, ARG.expose<SEXPREC* >());                         \
      } else if ( ARG.is_type(typeid(pair<const char*, SEXP >)) ) {     \
         const pair<const char*, SEXP > &tmp =                          \
            ARG.expose<pair<const char*, SEXP> >();                     \
         SETCAR(EXPR, tmp.second);                                      \
         SET_TAG(EXPR, install(tmp.first));                             \
      } else if ( ARG.is_type(typeid(pair<string, SEXP >)) ) {     \
         const pair<string, SEXP > &tmp =                          \
            ARG.expose<pair<string, SEXP> >();                     \
         SETCAR(EXPR, tmp.second);                                      \
         SET_TAG(EXPR, install(tmp.first.c_str()));                     \
      } else                                                            \
         EXCEPTION_MNGR(std::runtime_error, "Bad type");                \
      EXPR = CDR(EXPR);                                                 \
   } else static_cast<void>(0)

namespace {

SEXP Reval( Any arg1 = Any(), Any arg2 = Any(), 
            Any arg3 = Any(), Any arg4 = Any(),
            Any arg5 = Any(), Any arg6 = Any(),
            Any arg7 = Any(), Any arg8 = Any() )
{
   SEXP expr, expr_exec;
   int nargs = 0;
   if ( ! arg1.empty() ) ++nargs;
   if ( ! arg2.empty() ) ++nargs;
   if ( ! arg3.empty() ) ++nargs;
   if ( ! arg4.empty() ) ++nargs;
   if ( ! arg5.empty() ) ++nargs;
   if ( ! arg6.empty() ) ++nargs;
   if ( ! arg7.empty() ) ++nargs;
   if ( ! arg8.empty() ) ++nargs;

   PROTECT(expr = expr_exec = allocList(nargs));
   SET_TYPEOF(expr_exec, LANGSXP);
   ADD_TERM(expr, arg1);
   ADD_TERM(expr, arg2);
   ADD_TERM(expr, arg3);
   ADD_TERM(expr, arg4);
   ADD_TERM(expr, arg5);
   ADD_TERM(expr, arg6);
   ADD_TERM(expr, arg7);
   ADD_TERM(expr, arg8);
   SEXP ans = eval(expr_exec, R_GlobalEnv);
   UNPROTECT(1);
   return ans;
}

string tgp_label = "TGPO_seen_by_tgp";

void 
MCMC_BTE_setfcn( utilib::Any& ans, const utilib::Any val )
{
   if ( val.expose<vector<int> >().size() == 3 )
      ans = val;
   else
   {
      vector<int> &tmp = ans.set<vector<int> >();
      tmp.resize(3);
      tmp[0] = 2000;
      tmp[1] = 0;
      tmp[2] = 2;
   }
}


bool
MCMC_BTE_validate( const utilib::ReadOnly_Property&, const utilib::Any& value )
{
   const vector<int> &val = value.expose<vector<int> >();
   if ( val.size() == 0 || val.size() == 3 )
      return true;
   return false;
}

SEXP getRListElement(SEXP list, const char *str)
{
   SEXP names = getAttrib(list, R_NamesSymbol);
   for ( R_len_t i = 0; i < length(list); i++ )
   {
      //cout << CHAR(STRING_ELT(names, i)) << endl;
      if ( strcmp(CHAR(STRING_ELT(names, i)), str) == 0 ) 
         return VECTOR_ELT(list, i);
   }
   return R_NilValue;
}


} // namespace (local)


namespace scolib {

struct TGPO::Data
{
   Data()
      : BTE(NULL), X(NULL), Z(NULL), found_improving_point(false)
   {
      const char *argv[]= {"REmbeddedAcro", "--silent", "--no-save"};
      Rf_initEmbeddedR(sizeof(argv)/sizeof(argv[0]), const_cast<char**>(argv));

      // import the library with "improvedLHS" and "btgp"
      Reval(install("library"), install("lhs"));
      Reval(install("library"), install("tgp"));

      PROTECT(BTE = Rf_allocVector(REALSXP, 3));
      PROTECT_WITH_INDEX(X = Rf_allocMatrix(REALSXP, 0, 0), &X_index);
      PROTECT_WITH_INDEX(Z = Rf_allocVector(REALSXP, 0), &Z_index);
   }

   ~Data()
   {
      UNPROTECT(1);
      Rf_endEmbeddedR(0);
   }

   SEXP BTE; // input 3-vector (BTE parameter)
   SEXP X;   // input matrix (calculated domain points)
   SEXP Z;   // response vector (responses calculated from X)
   

   PROTECT_INDEX X_index;
   PROTECT_INDEX Z_index;

   bool found_improving_point;

   void resize_X(int nrow, int ncol)
   {
      SEXP tmp;
      int nr = Rf_nrows(X);
      int nc = Rf_ncols(X);
      int min_c = std::min(nc, ncol);
      int min_r = std::min(nr, nrow);

      // Note: protection not needed here, as we are not calling ANY R
      // methods until AFTER we REPROTECT the newly-allocated matrix
      tmp = Rf_allocMatrix(REALSXP, nrow, ncol);
      double *src = REAL(X);
      double *dest = REAL(tmp);

      if ( nr == nrow )
         memcpy(dest, src, nr*min_c*sizeof(double));
      else
         for(int i = 0; i < min_c; ++i)
            memcpy(dest+(i*nrow), src+(i*nr), min_r*sizeof(double));

      X = tmp;
      REPROTECT(X, X_index);
   }

   void resize_Z(int len)
   {
      REPROTECT(Z = lengthgets(Z, len), Z_index);
   }

};


TGPO::TGPO()
   : LHS_sample_size(Property::Bind<int>(0)),
     TGP_sample_size(Property::Bind<int>(0)),
     DOptimal_designs(Property::Bind<int>(-1)),
     eval_ego_point(Property::Bind<bool>(true)),
     eval_max_stdev_point(Property::Bind<bool>(true)),
     eval_confidence_bounded_point(Property::Bind<bool>(true)),
     local_optimizer(Property::Bind<string>("sco:ps")),
     local_search_frequency(Property::Bind<int>(5)),
     optimality_tolerance(Property::Bind<double>(0.05)),
     maximum_iterations(Property::Bind<int>(100)),
     data_filter_threshold(Property::Bind<double>(0.1)),
     MCMC_BTE( Property::Bind<vector<int> >(),
               MCMC_BTE_setfcn, 
               Property::get_fcn_t() ),
     data(NULL)
{
   properties.declare
      ("LHS_sample_size",
       "The number of LHS points to use for the initial problem sample.  "
       "This must be at least {number of variables} "
       " [default: 0 {10 * number of variables}]",
       LHS_sample_size);

   properties.declare
      ("TGP_sample_size",
       "The number of LHS points to use when evaluating the TGP model for "
       "estimating expected improvement and D-Optimal designs.  "
       "This must be at least {number of variables} "
       " [default: 0 {10 * number of variables**2}]",
       TGP_sample_size);

   properties.declare
      ("DOptimal_designs",
       "The number of D-Optimal designs to generate and evaluate using "
       "the TGP model; set to 0 to disable"
       " [default: -1 {number of variables}]",
       DOptimal_designs);

   properties.declare
      ("eval_ego_point",
       "Evaluate the EGO point (best expected improvement) returned "
       "by the TGP model"
       " [default: true]",
       eval_ego_point);

   properties.declare
      ("eval_max_stdev_point",
       "Evaluate the point returned by the TGP model with the highest "
       "reported standard deviation (uncertainty)"
       " [default: true]",
       eval_max_stdev_point);

   properties.declare
      ("eval_confidence_bounded_point",
       "Evaluate the point returned by the TGP model with best "
       "uncertainty-bounded expected value (95% confidence around prediction)"
       " [default: true]",
       eval_confidence_bounded_point);

   properties.declare
      ("local_optimizer",
       "Solver to use to refine the EGO point returned by TGP"
       " [default: sco:ps]",
       local_optimizer);

   properties.declare
      ("local_search_frequency",
       "How often to run the local search algorithm (local search every "
       "NN TGP iterations)"
       " [default: 5]",
       local_search_frequency);

   properties.declare
      ("optimality_tolerance",
       "Relative tolerance between best calculated solution and the 95% "
       "confidence bound for the TGP model prediction sufficient for "
       "termination"
       " [default: 0.05]",
       optimality_tolerance);

   properties.declare
      ("maximum_iterations",
       "Maximum number of TGP iterations before optimize() terminates"
       " [default: 100]",
       maximum_iterations);

   properties.declare
      ("data_filter_threshold",
       "Scaling threshold (scaled euclidean distance) for filtering "
       "data from the evaluation cache"
       " [default: 0.1]",
       data_filter_threshold);

   MCMC_BTE.validate().connect(MCMC_BTE_validate);
   MCMC_BTE = vector<int>();
   properties.declare
      ("MCMC_BTE",
       "MCMC parameters for tgp sampling: a 3-element vector "
       "{Burn in, Total, Every}"
       " [default: (2000,B+4000+1000*num_var,2)]",
       MCMC_BTE);
}

void TGPO::optimize()
{
   //if ( initial_points->size() == 0 )
   //   EXCEPTION_MNGR(std::runtime_error, "No-Real-Params");

   if ( ! problem->finite_bound_constraints() )
      EXCEPTION_MNGR
         (std::runtime_error, "TGPO::optimize(): Invalid problem\n\t"
          "The TGPO optimizer requires all variables to have finite "
          "lower and upper bounds.");

   // We need a specialized "view" to track all the points that the TGP
   // surrogate has *not* seen yet.
   colin::cache::View_Unlabeled new_tgp_points;
   new_tgp_points.label = tgp_label;
   if ( eval_mngr().evaluation_cache().empty() )
      EXCEPTION_MNGR(std::runtime_error, "TGPO::optimize(): "
                     "ERROR instantiating solution cache view.\n\t"
                     "The TGPO optimizer requires the Evaluation Manager to "
                     "define an evaluation cache.");
   new_tgp_points.set_core_cache(eval_mngr().evaluation_cache());

   colin::cache::View_Pareto best_point;
   best_point.application_context = problem->get_handle();
   best_point.set_core_cache(eval_mngr().evaluation_cache());
   best_point.onInsert.connect
      ( boost::bind(&TGPO::cb_new_best_point, this, _1) );

   data = new Data;
   data->found_improving_point = false;
   solver_status.model_status = colin::model_intermediate_nonoptimal;

   /* R code goes here :) */

   // Draw & evaluate the initial problem sample
   cout << "TGPO: Drawing and evaluating initial LHS sample (" 
        << get_lhs_num_samples() << " samples)" << endl;
   sample_problem_LHS();
   processEvaluationCache(new_tgp_points);
   printResponse( "Best initial sample point", 
                  best_point.begin()->second.asResponse(problem) );

   double sense = problem->sense == colin::maximization ? -1.0 : 1.0;

   size_t iter_since_ls = 0;
   size_t iter = 0;
   size_t max_iter = maximum_iterations;
   tgp_results_t tgp;
   while ( iter++ < max_iter || ! max_iter )
   {
      cout << "*****************************************************" << endl;
      cout << "TGPO Iteration " << iter << endl;
      ++iter_since_ls;

      TGP(tgp);

      double fVal;
      best_point.begin()->second.asResponse(problem).get(colin::f_info, fVal);
      fVal = sense * ( fVal - tgp.conf_q1 ) / std::fabs(fVal);
      if ( fVal <= optimality_tolerance.as<double>() 
           && ! data->found_improving_point )
      {
         solver_status.termination_condition=colin::termination_optimal;
         solver_status.model_status=colin::model_optimal;
         break;
      }

      processEvaluationCache(new_tgp_points);

      if ( ! tgp.ego_eval.empty() )
         cout << "TGPO: EGO point objective evaluation:  " 
              << tgp.ego_eval << endl;
      if ( ! tgp.conf_eval.empty() )
         cout << "TGPO: Min 5% quantile point objective evaluation:  " 
              << tgp.conf_eval << endl;
      if ( ! tgp.sd_eval.empty() )
         cout << "TGPO: Max norm (95%-5%) point objective evaluation:  "
              << tgp.sd_eval << endl;

      if ( ! local_optimizer.as<string>().empty() && 
           ( data->found_improving_point 
             || iter_since_ls >= local_search_frequency.as<int>() ) )
      {
         cout << "TGPO: Begin local optimization" << endl;

         colin::SolverHandle lo
            = colin::SolverMngr().create_solver(local_optimizer);
         lo->set_problem(problem);
         if ( data->found_improving_point )
            lo->add_initial_point(best_point.begin()->second.asResponse(problem));
         else
            lo->add_initial_point(tgp.ego_domain);
         printResponse
            ( "TGPO: Initial point for local optimization",
              lo->get_initial_points()->begin()->second.asResponse(problem) );

         lo->reset();
         lo->optimize();

         printResponse
            ( "TGPO: Final point from local optimization",
              lo->get_final_points()->begin()->second.asResponse(problem) );

         processEvaluationCache(new_tgp_points);
         data->found_improving_point = false;
         iter_since_ls = 0;
      }

      printResponse( "TGPO: Current best solution: ",
                     best_point.begin()->second.asResponse(problem) );
   }

   processEvaluationCache(new_tgp_points);
   cout << "*****************************************************" << endl;
   printResponse( "TGPO: Final solution: ",
                  best_point.begin()->second.asResponse(problem) );
   cout << endl
        << "TGPO: Final TGP model information: {estimate, 5% quantile}: "
        << "<domain point>" << endl;
   cout << "      NOTE: This is Exactly copied from the final iteration above"
        << endl;
   if ( iter == max_iter )
      cout << "      WARNING: TGPO terminated on maximum_iterations; the "
           << "TGP model " << endl
           << "               is out of date." << endl;
   cout << "      EGO point:         {" << tgp.ego_mean << ", " 
        << tgp.ego_q1 << "}: " << tgp.ego_domain << endl;
   if ( ! tgp.ego_eval.empty() )
      cout << "                         evaluated objective: " 
           << tgp.ego_eval << endl;
   cout << "      Min 5% quantile:   {" << tgp.conf_mean << ", " 
        << tgp.conf_q1 << "}: " << tgp.conf_domain << endl;
   if ( ! tgp.conf_eval.empty() )
      cout << "                         evaluated objective: " 
           << tgp.conf_eval << endl;
   cout << "      Max norm (95%-5%): {" << tgp.sd_mean << ", " 
        << tgp.sd_q1 << "}: " << tgp.sd_domain << endl << std::flush;
   if ( ! tgp.sd_eval.empty() )
      cout << "                         evaluated objective: " 
           << tgp.sd_eval << endl;


   final_points->insert(best_point.begin());

   if ( max_iter && iter >= max_iter )
      solver_status.termination_condition = colin::termination_maxIterations;

   delete data;
}

void TGPO::cb_new_best_point(colin::Cache::cache_t::iterator)
{
   data->found_improving_point = true;
}


int TGPO::get_lhs_num_samples()
{
   int ans = LHS_sample_size;
   int num_var = problem->num_real_vars;
   if ( ans <= 0 )
      ans = 10 * num_var;
   else if  ( ans <= num_var )
   {
      std::cerr << "TGPO: WARNING: LHS_sample_size set to less than the "
         "number of variables.  Resetting to the number of variables (" <<
         num_var << ")" << std::endl;
      LHS_sample_size = num_var;
      ans = num_var;
   }
   return ans;
}


int TGPO::get_tgp_num_samples()
{
   int ans = TGP_sample_size;
   int num_var = problem->num_real_vars;
   if ( ans <= 0 )
      ans = 10 * num_var * num_var;
   else if  ( ans <= num_var )
   {
      std::cerr << "TGPO: WARNING: TGP_sample_size set to less than the "
         "number of variables.  Resetting to the number of variables (" <<
         num_var << ")" << std::endl;
      TGP_sample_size = num_var;
      ans = num_var;
   }
   return ans;
}


void TGPO::sample_problem_LHS()
{
   size_t num_var = problem->num_real_vars;
   size_t partitions = get_lhs_num_samples();

   vector<double> domain(num_var);
   vector<double> lb = problem->real_lower_bounds;
   vector<double> range = problem->real_upper_bounds;
   for (size_t i = 0; i < num_var; ++i)
      range[i] -= lb[i];

   // Run LHS to generate initial samples
   SEXP lhs;
   PROTECT( lhs = 
            Reval( install("improvedLHS"), 
                   ScalarInteger(partitions), 
                   ScalarInteger(num_var), 
                   make_pair("dup",ScalarInteger(1)) ) );
   // NB: improvedLHS returns N samples (partitions) of K variables in
   // an N x K matrix (row-major, K rows, N columns)
   double* r_lhs = REAL(lhs);
   for (size_t i = 0; i < partitions; ++i)
   {
      for (size_t j = 0; j < num_var; ++j)
         domain[j] = lb[j] + range[j]*r_lhs[i+j*partitions];
      problem->AsyncEvalF(eval_mngr(), domain);
   }
   UNPROTECT(1);
}


void TGPO::processEvaluationCache(colin::CacheHandle cache)
{
   // Wait for any queued sample evaluations to complete (either LHS
   // samples, or TGP D-Optimal designs)
   eval_mngr().synchronize();

   int num_var = problem->num_real_vars;
   
   // NB: TGP assumes *minimization* (as does our using std::set for sorting)
   bool invertSense = problem->sense == colin::maximization;

   // Retrieve all points
   typedef set<pair<double, vector<double> > > points_t;
   points_t new_points;
   colin::Cache::iterator c_it = cache->begin(problem);
   colin::Cache::iterator c_itEnd = cache->end();
   vector<double> domain;
   double response;
   for ( ; c_it != c_itEnd; ++c_it )
   {
      colin::AppResponse r = c_it->second.asResponse(problem);
      r.get_domain(domain, problem);
      try {
         r.get(colin::f_info, response);
      } catch (...) {
         cout << "WARNING: model evaluation did not return an objective value."
              << endl
              << "     domain: " << domain << endl;
         continue;
      }
      if ( invertSense )
         response *= -1;
      new_points.insert(make_pair(response, domain));
   }


   // Filter points that are "unnecessarily close"
   double threshold = data_filter_threshold.as<double>();
   threshold *= threshold;
   //
   // TBD: should we filter against points that are already in the TGP model?
   // I think we might: I occasionally see errors thrown by TGP due to
   // poorly scaled/conditioned matrices.
   points_t::iterator it = new_points.begin();
   points_t::iterator itEnd = new_points.end();
   vector<double> scale = problem->real_upper_bounds;
   vector<double> tmp = problem->real_lower_bounds;
   double pts_per_dim = 1.0;//get_tgp_num_samples() / num_var;
   for( size_t i = 0; i < num_var; ++i )
      scale[i] = (scale[i] - tmp[i]) / pts_per_dim;
   int filtered = new_points.size();
   while ( it != itEnd )
   {
      points_t::iterator test_it = it;
      ++test_it; 
      while ( test_it != itEnd )
      {
         double dist = 0;
         for( size_t i = 0; i < num_var; ++i )
         {
            double tmp = ( it->second[i] - test_it->second[i] ) / scale[i];
            dist += tmp*tmp;
         }
         if ( dist < threshold )
            new_points.erase(test_it++);
         else
            ++test_it;
      }
      ++it;
   }
   
   // Add the remaining points to the TGP model
   int r = nrows(data->X);
   int nr = r + new_points.size();

   data->resize_X(nr, num_var);
   data->resize_Z(nr);
   cout << "TGPO: Processing evaluation cache: adding " 
        << nr-r << " points to the TGP model (new total: " << nr 
        << ")." << endl 
        << "      (Ignored " << (filtered - new_points.size()) 
        << " points due to the data_filter_threshold.)" << endl
        << "      objective: <domain>" << endl;

   double* x = REAL(data->X);
   double* z = REAL(data->Z);

   for ( it = new_points.begin() ; it != itEnd; ++it, ++r )
   {
      cout << "      " << it->first << ": " << it->second << endl;
      for ( size_t j = 0; j < num_var; ++j )
         x[r + j*nr] = it->second[j];
      z[r] = it->first;
   }

   cache->clear();
}

void TGPO::printResponse(std::string prompt, colin::AppResponse r)
{
   cout << prompt << ": ";
   try {
      double response_val = 0.0;
      r.get(colin::f_info, response_val);
      cout << "Objective: " << response_val << "; ";
   } catch (...) { }
   cout << r;
}


void TGPO::TGP(tgp_results_t &ans)
{
   size_t num_var = problem->num_real_vars;
   size_t samples = get_tgp_num_samples();

   cout << "TGPO: Drawing " << samples 
        << " LHS samples for estimating EGO point" << endl << std::flush;

   SEXP XX;
   PROTECT( XX = 
            Reval( install("improvedLHS"), 
                   ScalarInteger(samples), 
                   ScalarInteger(num_var), 
                   make_pair("dup",ScalarInteger(1)) ) );

   // Rescale the samples to the actual problem domain
   double* r_XX = REAL(XX);
   vector<double> lb = problem->real_lower_bounds;
   vector<double> range = problem->real_upper_bounds;
   for (size_t i = 0; i < num_var; ++i)
      range[i] -= lb[i];
   for (size_t i = 0; i < samples; ++i)
      for (size_t j = 0; j < num_var; ++j)
         r_XX[i+j*samples] = lb[j] + range[j]*r_XX[i+j*samples];


   cout << "TGPO: Generating TGP model" << endl << std::flush;

   std::vector<int> bte = MCMC_BTE;
   if ( bte[1] == 0 )
      bte[1] = bte[0] + 4000 + 1000*num_var;
   for( size_t i = 0; i < bte.size(); ++i )
      REAL(data->BTE)[i] = bte[i];

   // Build the TGP Model
   SEXP tgp;
   PROTECT( tgp = 
            Reval( install("btgp"), 
                   make_pair("X",data->X),
                   //make_pair("XX",XX),
                   make_pair("Z",data->Z),
                   make_pair("BTE",data->BTE),
                   make_pair("corr",mkString("expsep")),
                   //make_pair("improv",ScalarLogical(1)),
                   make_pair("verb",ScalarInteger(0)) 
                   ) );

   cout << "TGPO: Estimating EGO point from " 
        << samples << " LHS sample points." << endl << std::flush;

   SEXP pred;
   PROTECT( pred = 
            Reval( install("predict"),
                   tgp,
                   make_pair("XX",XX),
                   make_pair("BTE",data->BTE),
                   make_pair("improv",ScalarLogical(1)),
                   make_pair("verb",ScalarInteger(0)) 
                   ) );


   // NB: We directly use the ordering reported in the second column,
   // because some versions of tgp return INTEGER constants for the
   // first column instead of the fractional expected improvement
   int* improv = INTEGER(VECTOR_ELT(getRListElement(pred, "improv"),1));
   double* sd = REAL(getRListElement(pred, "ZZ.q"));
   double* cb = REAL(getRListElement(pred, "ZZ.q1"));

   // Identify the EGO point and point with max(stddev)
   size_t ego_idx = 0;
   size_t sd_idx = 0;
   size_t conf_idx = 0;
   for( size_t r = 1; r < samples; ++r )
   {
      if ( improv[r] < improv[ego_idx] )
         ego_idx = r;
      if ( sd[r] > sd[sd_idx] )
         sd_idx = r;
      if ( cb[r] < cb[conf_idx] )
         conf_idx = r;
   }

   // prepare the return values
   ans = tgp_results_t();
   ans.ego_mean = REAL(getRListElement(pred, "ZZ.mean"))[ego_idx];
   ans.ego_q1 = cb[ego_idx];
   vector<double> &ego_v = ans.ego_domain.set<vector<double> >();
   ego_v.resize(num_var);
   for( size_t c = 0; c < num_var; ++c )
      ego_v[c] = r_XX[ego_idx + c*samples];

   ans.conf_mean = REAL(getRListElement(pred, "ZZ.mean"))[conf_idx];
   ans.conf_q1 = cb[conf_idx];
   vector<double> &conf_v = ans.conf_domain.set<vector<double> >();
   conf_v.resize(num_var);
   for( size_t c = 0; c < num_var; ++c )
      conf_v[c] = r_XX[conf_idx + c*samples];

   ans.sd_mean = REAL(getRListElement(pred, "ZZ.mean"))[sd_idx];
   ans.sd_q1 = cb[sd_idx];
   vector<double> &sd_v = ans.sd_domain.set<vector<double> >();
   sd_v.resize(num_var);
   for( size_t c = 0; c < num_var; ++c )
      sd_v[c] = r_XX[sd_idx + c*samples];

   cout << endl
        << "TGPO: Model information: {estimate, 5% quantile}: <domain point>"
        << endl;
   cout << "      EGO point:         {" << ans.ego_mean << ", " 
        << ans.ego_q1 << "}: " << ans.ego_domain << endl;
   cout << "      Min 5% quantile:   {" << ans.conf_mean << ", " 
        << ans.conf_q1 << "}: " << ans.conf_domain << endl;
   cout << "      Max norm (95%-5%): {" << ans.sd_mean << ", " 
        << ans.sd_q1 << "}: " << ans.sd_domain << endl << std::flush;
   
   if ( eval_ego_point )
   {
      problem->AsyncEvalF( eval_mngr(), ans.ego_domain, 
                           ans.ego_eval.set<double>() );
      cout << "TGPO: queuing EGO point evaluation: " << ans.ego_domain
           << endl;
   }

   // Evaluate the sd and confidence bound points, if requested
   if ( eval_confidence_bounded_point )
   {
      problem->AsyncEvalF( eval_mngr(), ans.conf_domain, 
                           ans.conf_eval.set<double>() );
      cout << "TGPO: queuing Min Q1 point evaluation: " << ans.conf_domain
           << endl;
   }
   if ( eval_max_stdev_point )
   {
      problem->AsyncEvalF( eval_mngr(), ans.sd_domain, 
                           ans.sd_eval.set<double>() );
      cout << "TGPO: queuing Max Q norm point evaluation: " << ans.sd_domain
           << endl;
   }

   // Calculate D-Optimal designs and queue for evaluation
   int num_dopt = DOptimal_designs;
   if ( num_dopt < 0 )
      num_dopt = num_var;
   if ( num_dopt > 0 )
   {
      cout << "TGPO: Generating TGP D-optimal design for " << num_dopt
           << " points" << endl;
      SEXP DOpt;
      PROTECT( DOpt = 
               Reval( install("tgp.design"),
                      ScalarInteger(num_dopt),
                      XX,
                      tgp ) );
      double *DOpt_r = REAL(DOpt);

      vector<double> domain(num_var);
      size_t nr = nrows(DOpt);
      cout << "TGPO: queuing " << nr << " D-optimal design point evaluations: "
           << endl;
      for(size_t r = 0; r < nr; ++r)
      {
         for(size_t c = 0; c < num_var; ++c)
            domain[c] = DOpt_r[r+c*nr];
         problem->AsyncEvalF(eval_mngr(), domain);
         cout << "      " << domain << endl;
      }
      UNPROTECT(1);
   }

   UNPROTECT(3);
}


REGISTER_COLIN_SOLVER_WITH_ALIAS
   (TGPO, "sco:TGPO", "sco:tgpo", "The SCO TGP-guided hybrid optimizer");

} // namespace scolib


#else // (not) if ACRO_HAVE_R
//
// Stub in a non-functional (and non-registered) TGPO class.
//

namespace scolib {

TGPO::TGPO() {}

void TGPO::optimize() {}

namespace StaticInitializers {

extern const volatile bool TGPO_bool = false;

} // namepace StaticInitializers
} // namespace scolib

#endif // if ACRO_HAVE_R
