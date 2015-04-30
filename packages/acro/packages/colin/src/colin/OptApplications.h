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

/**
 * \file OptApplications.h
 *
 * Functions to construct COLIN derived Application objects.
 */

#ifndef colin_OptApplications_h
#define colin_OptApplications_h

#include <acro_config.h>
#include <utilib/pvector.h>
#include <colin/ApplicationMngr.h>
#include <colin/DirectFuncApplication.h>
#include <colin/AnalysisCodeApplication.h>
#include <colin/DirectSimpleApplication.h>


namespace colin
{

///
template <class DomainT, class ProblemT>
AnalysisCodeApplication<ProblemT, DomainT> * 
new_application(
   const std::string& name,
   const std::string& program,
   const std::string& input_filename,
   const std::string& output_filename,
   bool ctr_suffix = true,
   bool remove_files = true)
{
   AnalysisCodeApplication<ProblemT, DomainT>* app = new AnalysisCodeApplication<ProblemT, DomainT>();
   app->setup(program, input_filename, output_filename, ctr_suffix, remove_files);
   ApplicationMngr().reregister_application(app, name);
   return app;
}


///
template <class DomainT, class ProblemT>
DirectFuncApplication <ProblemT> *
new_application(
   const std::string& name,
   void(*eval_fn)(const utilib::Any& domain,
                  const AppRequest::request_map_t& request_map,
                  AppResponse::response_map_t& response_map,
                  utilib::seed_t& seed),
   int n_obj,
   int num_ineqc,
   int num_eqc)
{
   DirectFuncApplication <ProblemT> * app = new
           DirectFuncApplicationImpl <
           ProblemT,
           DomainT,
           void(*)(const utilib::Any&,
                   const AppRequest::request_map_t&,
                   AppResponse::response_map_t&,
                   utilib::seed_t&) >
           (eval_fn);

   app->num_nonlinear_constraints = num_ineqc + num_eqc;
   app->num_objectives = n_obj;
   ApplicationMngr().reregister_application(app, name);
   return app;
}


///
template <class DomainT, class ProblemT>
DirectFuncApplication <ProblemT> *
new_application(
   const std::string& name,
   void(*eval_fn)(const utilib::Any& domain,
                  const AppRequest::request_map_t& request_map,
                  AppResponse::response_map_t& response_map,
                  utilib::seed_t& seed),
   int n_obj)
{
   DirectFuncApplicationImpl <
   ProblemT,
   DomainT,
   void(*)(const utilib::Any&,
           const AppRequest::request_map_t&,
           AppResponse::response_map_t&,
           utilib::seed_t&) >
   * app = new
           DirectFuncApplicationImpl <
           ProblemT,
           DomainT,
           void(*)(const utilib::Any&,
                   const AppRequest::request_map_t&,
                   AppResponse::response_map_t&,
                   utilib::seed_t&) >
           (eval_fn);

   app->num_objectives = n_obj;
   ApplicationMngr().reregister_application(app, name);
   return app;
}


///
template <class DomainT, class ProblemT>
DirectFuncApplication <ProblemT> *
new_application(
   const std::string& name,
   void(*eval_fn)(const utilib::Any& domain,
                  const AppRequest::request_map_t& request_map,
                  AppResponse::response_map_t& response_map,
                  utilib::seed_t&),
   int num_ineqc,
   int num_eqc)
{
   DirectFuncApplicationImpl <
   ProblemT,
   DomainT,
   void(*)(const utilib::Any& domain,
           const AppRequest::request_map_t& request_map,
           AppResponse::response_map_t& response_map,
           utilib::seed_t&) >
   * app = new
           DirectFuncApplicationImpl <
           ProblemT,
           DomainT,
           void(*)(const utilib::Any& domain,
                   const AppRequest::request_map_t& request_map,
                   AppResponse::response_map_t& response_map,
                   utilib::seed_t&) >
           (eval_fn);

   app->num_nonlinear_constraints = num_ineqc + num_eqc;
   ApplicationMngr().reregister_application(app, name);
   return app;
}


///
template <class DomainT, class ProblemT>
DirectFuncApplication < ProblemT > *
new_application(
   const std::string& name,
   void(*eval_fn)(const utilib::Any& domain,
                  const AppRequest::request_map_t& request_map,
                  AppResponse::response_map_t& response_map,
                  utilib::seed_t&))
{
   DirectFuncApplicationImpl <
   ProblemT,
   DomainT,
   void(*)(const utilib::Any& domain,
           const AppRequest::request_map_t& request_map,
           AppResponse::response_map_t& response_map,
           utilib::seed_t&) >
   * app = new
           DirectFuncApplicationImpl <
           ProblemT,
           DomainT,
           void(*)(const utilib::Any& domain,
                   const AppRequest::request_map_t& request_map,
                   AppResponse::response_map_t& response_map,
                   utilib::seed_t&) >
           (eval_fn);

   ApplicationMngr().reregister_application(app, name);
   return app;
}


///
template <class DomainT>
DirectFuncApplication < NLP0_problem > *
new_application(
   const std::string& name,
   double(*eval_fn)(DomainT&))
{
   DirectFuncApplicationImpl <
   NLP0_problem,
   DomainT,
   double(*)(DomainT&) >
   * app = new
           DirectFuncApplicationImpl <
           NLP0_problem,
           DomainT,
           double(*)(DomainT&) >
           (eval_fn);

   app->num_nonlinear_constraints = 0;
   ApplicationMngr().reregister_application(app, name);
   return app;
}


///
template <class DomainT>
DirectFuncApplication < NLP0_problem > *
new_application(
   const std::string& name,
   double(*eval_fn)(const DomainT&))
{
   DirectFuncApplicationImpl <
   NLP0_problem,
   DomainT,
   double(*)(const DomainT&) >
   * app = new
           DirectFuncApplicationImpl <
           NLP0_problem,
           DomainT,
           double(*)(const DomainT&) >
           (eval_fn);

   app->num_nonlinear_constraints = 0;
   ApplicationMngr().reregister_application(app, name);
   return app;
}


///
template <class DomainT>
DirectFuncApplication < NLP0_problem > *
new_application(
   const std::string& name,
   void(*eval_fn)(DomainT&, double&))
{
   DirectFuncApplicationImpl <
   NLP0_problem,
   DomainT,
   void(*)(DomainT&, double&) >
   * app = new
           DirectFuncApplicationImpl <
           NLP0_problem,
           DomainT,
           void(*)(DomainT&, double&) >
           (eval_fn);

   app->num_nonlinear_constraints = 0;
   ApplicationMngr().reregister_application(app, name);
   return app;
}


///
template <class DomainT>
DirectFuncApplication < NLP0_problem > *
new_application(
   const std::string& name,
   void(*eval_fn)(const DomainT&, double&))
{
   DirectFuncApplicationImpl <
   NLP0_problem,
   DomainT,
   void(*)(const DomainT&, double&) >
   * app = new
           DirectFuncApplicationImpl <
           NLP0_problem,
           DomainT,
           void(*)(const DomainT&, double&) >
           (eval_fn);

   app->num_nonlinear_constraints = 0;
   ApplicationMngr().reregister_application(app, name);
   return app;
}


///
template <class DomainT>
DirectFuncApplication < NLP0_problem > *
new_application(
   const std::string& name,
   void(*eval_fn)(DomainT&, colin::real&))
{
   DirectFuncApplicationImpl <
   NLP0_problem,
   DomainT,
   void(*)(DomainT&, colin::real&) >
   * app = new
           DirectFuncApplicationImpl <
           NLP0_problem,
           DomainT,
           void(*)(DomainT&, colin::real&) >
           (eval_fn);

   app->num_nonlinear_constraints = 0;
   ApplicationMngr().reregister_application(app, name);
   return app;
}


///
template <class DomainT>
DirectFuncApplication < NLP0_problem > *
new_application(
   const std::string& name,
   void(*eval_fn)(const DomainT&, colin::real&))
{
   DirectFuncApplicationImpl <
   NLP0_problem,
   DomainT,
   void(*)(const DomainT&, colin::real&) >
   * app = new
           DirectFuncApplicationImpl <
           NLP0_problem,
           DomainT,
           void(*)(const DomainT&, colin::real&) >
           (eval_fn);

   app->num_nonlinear_constraints = 0;
   ApplicationMngr().reregister_application(app, name);
   return app;
}


///
template <class DomainT, class CArrayT>
DirectFuncApplication < NLP0_problem > *
new_application(
   const std::string& name,
   void(*eval_fn)(DomainT&, colin::real&, CArrayT&),
   int num_ineqc = 0,
   int num_eqc = 0)
{
   DirectFuncApplicationImpl <
   NLP0_problem,
   DomainT,
   void(*)(DomainT&, colin::real&, CArrayT&) >
   * app = new
           DirectFuncApplicationImpl <
           NLP0_problem,
           DomainT,
           void(*)(DomainT&, colin::real&, CArrayT&) >
           (eval_fn);

   app->num_nonlinear_constraints = num_ineqc + num_eqc;
   ApplicationMngr().reregister_application(app, name);
   return app;
}


///
template <class DomainT, class CArrayT>
DirectFuncApplication < NLP0_problem > *
new_application(
   const std::string& name,
   void(*eval_fn)(const DomainT&, colin::real&, CArrayT&),
   int num_ineqc = 0,
   int num_eqc = 0)
{
   DirectFuncApplicationImpl <
   NLP0_problem,
   DomainT,
   void(*)(const DomainT&, colin::real&, CArrayT&) >
   * app = new
           DirectFuncApplicationImpl <
           NLP0_problem,
           DomainT,
           void(*)(const DomainT&, colin::real&, CArrayT&) >
           (eval_fn);

   app->num_nonlinear_constraints = num_ineqc + num_eqc;
   ApplicationMngr().reregister_application(app, name);
   return app;
}


///
template <class DomainT, class CArrayT, class GArrayT>
DirectFuncApplication < NLP1_problem > *
new_application(
   const std::string& name,
   void(*eval_fn)(DomainT&, int, colin::real&, CArrayT&, GArrayT&),
   int num_ineqc = 0,
   int num_eqc = 0)
{
   DirectFuncApplicationImpl <
   NLP1_problem,
   DomainT,
   void(*)(DomainT&, int, colin::real&, CArrayT&, GArrayT&) >
   * app = new
           DirectFuncApplicationImpl <
           NLP1_problem,
           DomainT,
           void(*)(DomainT&, int, colin::real&, CArrayT&, GArrayT&) >
           (eval_fn);

   app->num_nonlinear_constraints = num_ineqc + num_eqc;
   ApplicationMngr().reregister_application(app, name);
   return app;
}


///
template <class DomainT, class CArrayT, class GArrayT>
DirectFuncApplication < NLP1_problem > *
new_application(
   const std::string& name,
   void(*eval_fn)(const DomainT&, int, colin::real&, CArrayT&, GArrayT&),
   int num_ineqc = 0,
   int num_eqc = 0)
{
   DirectFuncApplicationImpl <
   NLP1_problem,
   DomainT,
   void(*)(const DomainT&, int, colin::real&, CArrayT&, GArrayT&) >
   * app = new
           DirectFuncApplicationImpl <
           NLP1_problem,
           DomainT,
           void(*)(const DomainT&, int, colin::real&, CArrayT&, GArrayT&) >
           (eval_fn);

   app->num_nonlinear_constraints = num_ineqc + num_eqc;
   ApplicationMngr().reregister_application(app, name);
   return app;
}


///
template <class DomainT, class CArrayT, class GArrayT>
DirectFuncApplication < NLP1_problem > *
new_application(
   const std::string& name,
   void(*eval_fn)(DomainT&, int, colin::real&, CArrayT&, CArrayT&, GArrayT&),
   int num_ineqc = 0,
   int num_eqc = 0)
{
   DirectFuncApplicationImpl <
   NLP1_problem,
   DomainT,
   void(*)(DomainT&, int, colin::real&, CArrayT&, CArrayT&, GArrayT&) >
   * app = new
           DirectFuncApplicationImpl <
           NLP1_problem,
           DomainT,
           void(*)(DomainT&, int, colin::real&, CArrayT&, CArrayT&, GArrayT&) >
           (eval_fn);

   app->num_nonlinear_constraints = num_ineqc + num_eqc;
   ApplicationMngr().reregister_application(app, name);
   return app;
}


///
template <class DomainT, class CArrayT, class GArrayT>
DirectFuncApplication < NLP1_problem > *
new_application(
   const std::string& name,
   void(*eval_fn)(const DomainT&, int, colin::real&, CArrayT&, CArrayT&, GArrayT&),
   int num_ineqc = 0,
   int num_eqc = 0)
{
   DirectFuncApplicationImpl <
   NLP1_problem,
   DomainT,
   void(*)(const DomainT&, int, colin::real&, CArrayT&, CArrayT&, GArrayT&) >
   * app = new
           DirectFuncApplicationImpl <
           NLP1_problem,
           DomainT,
           void(*)(const DomainT&, int, colin::real&, CArrayT&, CArrayT&, GArrayT&) >
           (eval_fn);

   app->num_nonlinear_constraints = num_ineqc + num_eqc;
   ApplicationMngr().reregister_application(app, name);
   return app;
}


///
template <class ParamT>
DirectSimpleApplication <
   NLP0_problem,
   std::vector<ParamT>,
   double(*)(ParamT*, int),
   ParamT > *
new_application(
   const std::string& name,
   double(*eval_fn)(ParamT*, int))
{
   DirectSimpleApplication <
   NLP0_problem,
   std::vector<ParamT>,
   double(*)(ParamT*, int),
   ParamT >
   * app = new
           DirectSimpleApplication <
           NLP0_problem,
           std::vector<ParamT>,
           double(*)(ParamT*, int),
           ParamT >
           (eval_fn);

   app->num_nonlinear_constraints = 0;
   ApplicationMngr().reregister_application(app, name);
   return app;
}


///
template <class ParamT>
DirectSimpleApplication <
   NLP0_problem,
   std::vector<ParamT>,
   double(*)(const ParamT*, int),
   ParamT > *
new_application(
   const std::string& name,
   double(*eval_fn)(const ParamT*, int))
{
   DirectSimpleApplication <
   NLP0_problem,
   std::vector<ParamT>,
   double(*)(const ParamT*, int),
   ParamT >
   * app = new
           DirectSimpleApplication <
           NLP0_problem,
           std::vector<ParamT>,
           double(*)(const ParamT*, int),
           ParamT >
           (eval_fn);

   app->num_nonlinear_constraints = 0;
   ApplicationMngr().reregister_application(app, name);
   return app;
}


///
template <class DomainT>
DirectFuncApplication < SNLP0_problem > *
new_application(
   const std::string& name,
   void(*eval_fn)(DomainT&, colin::real&, utilib::seed_t))
{
   DirectFuncApplicationImpl <
   SNLP0_problem,
   DomainT,
   void(*)(DomainT&, colin::real&, utilib::seed_t) >
   * app = new
           DirectFuncApplicationImpl <
           SNLP0_problem,
           DomainT,
           void(*)(DomainT&, colin::real&, utilib::seed_t) >
           (eval_fn);

   app->num_nonlinear_constraints = 0;
   ApplicationMngr().reregister_application(app, name);
   return app;
}


///
template <class DomainT>
DirectFuncApplication < SNLP0_problem > *
new_application(
   const std::string& name,
   void(*eval_fn)(const DomainT&, colin::real&, utilib::seed_t))
{
   DirectFuncApplicationImpl <
   SNLP0_problem,
   DomainT,
   void(*)(const DomainT&, colin::real&, utilib::seed_t) >
   * app = new
           DirectFuncApplicationImpl <
           SNLP0_problem,
           DomainT,
           void(*)(const DomainT&, colin::real&, utilib::seed_t) >
           (eval_fn);

   app->num_nonlinear_constraints = 0;
   ApplicationMngr().reregister_application(app, name);
   return app;
}


}

#endif
