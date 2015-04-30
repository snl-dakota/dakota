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
 * \file AmplApplication.h
 *
 * Defines the colin::AmplApplication class.
 */

#ifndef colin_AmplApplication_h
#define colin_AmplApplication_h

#include <colin/SynchronousApplication.h>
#include <colin/reformulation/Base.h>
#include <colin/PointSet.h>

namespace colin
{

class AmplApplication_Core;

/** This application wraps the relaxed AmplApplication_Core application
 *  and presents the "true" mixed-integer problem.
 *
 *  NB: As this wrapper application is intimately tied to the
 *  AmplApplication_Core application (which is created through
 *  set_nl_file()), we do NOT present a generic ApplicationHandle copy
 *  constructor.
 */
class AmplApplication
   : public Application<MO_MINLP2_problem>,
     public NonexecutableApplication
{
public:

   ///
   AmplApplication();

   ///
   virtual ~AmplApplication();

   ///
   utilib::Any set_nl_file(std::string fname);

   ///
   void write_sol_file(PointSet ps, std::stringstream& msg);

   ///
   virtual ApplicationHandle relaxed_application() const
   { return relaxed_app.first; }

   ///
   void keep_nl_file()
   { keepNLFile = true; }

protected:

   /// Default to using the evaluation manager from the wrapped application
   virtual const EvaluationManager_Handle default_eval_mngr()
   { return relaxed_app.first->eval_mngr().handle(); }

   virtual Application_Base* get_reformulated_application() const
   { return relaxed_app.first.object(); }

   /// Convert the domain supplied by the solver to the application domain
   virtual bool map_domain( const utilib::Any &src, utilib::Any &native, 
                            bool forward ) const;

private:



   ///
   int
   cb_map_g_response( response_info_t info,
                      const utilib::Any &domain,
                      const AppRequest::request_map_t &requests,
                      const AppResponse::response_map_t &sub_response,
                      AppResponse::response_map_t &response );

   ///
   int
   cb_map_h_response(const utilib::Any &domain,
                     const AppRequest::request_map_t &requests,
                     const AppResponse::response_map_t &sub_response,
                     AppResponse::response_map_t &response);

   ///
   void cb_initialize(TiXmlElement* elt);

private:
   ///
   std::pair<ApplicationHandle, AmplApplication_Core*> relaxed_app;

   ///
   bool keepNLFile;
   ///
   size_t numReal;
   ///
   size_t numInteger;
   ///
   size_t numBinary;
   ///
   std::vector<char> var_type;
   ///
   std::vector<int>  var_rmap;
   ///
   double EPS;
};




} // namespace colin
#endif // colin_AmplApplication_h
