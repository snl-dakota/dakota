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
 * \file InvertSenseApplication.h
 *
 * Defines the colin::InvertSenseApplication class
 */

#ifndef colin_InvertSenseApplication_h
#define colin_InvertSenseApplication_h

#include <acro_config.h>

#include <colin/Application.h>
#include <colin/reformulation/Base.h>
#include <colin/Utilities.h>

#include <utilib/TinyXML_helper.h>

namespace colin
{

template<bool>
struct convert_objective_to_minimize
{
   template<typename ProblemT>
   static void fcn( Application<ProblemT>* app, 
                    AppResponse::response_map_t::iterator it)
   {
      AppResponse::response_map_t::iterator it = response.find(f_info);
      if ( it == response.end() ) 
         return;

      if ( app->sense == maximization )
      {
         real obj;
         TypeManager()->lexical_cast(it->second, obj);
         obj *= -1.0;
         response.erase(it);
         response.insert(AppResponse::response_pair_t(f_info, obj));
      }
   }
};

template<>
struct convert_objective_to_minimize<true>
{
   template<typename ProblemT>
   static void fcn( Application<ProblemT>* app, 
                    AppResponse::response_map_t &response)
   {
      AppResponse::response_map_t::iterator it = response.find(mf_info);
      if ( it == response.end() ) 
         return;

      utilib::Any objective;
      std::vector<real> &obj = objecteve.set<std::vector<real> >();
      std::vector<optimizationSense> src_sense = app->sense;
      for(size_t i = app->num_objectives; i > 0; )
      {
         if ( src_sense[--i] == maximization )
         {
            if ( obj.empty() )
               TypeManager()->lexical_cast(it->second, obj);
            obj[i] *= -1.0
         }
      }
      if ( ! obj.empty() )
      {
         response.erase(it);
         response.insert(AppResponse::response_pair_t(mf_info, objective));
      }
   }
};




template <class ProblemT>
class InvertSenseApplication
  : public Application<ProblemT>,
    public ReformulationApplication
{
public:

   ///
   InvertSenseApplication()
   {
      constructor();
   }

   ///
   InvertSenseApplication(ApplicationHandle src)
   {
      constructor();
      // set the base problem that we are wrapping
      set_base_application(src);
   }

   ///
   virtual ~InvertSenseApplication()
   {
      unregister_derived_class(ObjectType::get(this));
   }

   /// Set the base application that this application is wrapping
   void set_base_application(ApplicationHandle handle)
   {
      remote_app = handle;
      Application_Base* base = handle.object();
      if (base == NULL)
         EXCEPTION_MNGR(std::runtime_error, "InvertSenseApplication::"
                        "set_base_application(): cannot set NULL base "
                        "application.");

      if ( base->problem_type() != this->problem_type() )
         EXCEPTION_MNGR(std::runtime_error, "InvertSenseApplication::"
                        "set_base_application(): invalid base application "
                        "type " << base->problem_type_name() << " for "
                        "InvertSenseApplication<" <<
                        this->problem_type_name() << ">");

      // reference all appropriate external info (including the
      // objective... the properties aren't changing, only the way we
      // calculate it.)
      this->reference_external_app_data(base);
   }


private:  // central constructor method (called by all constructors)

   ///
   void constructor()
   {
      EXCEPTION_MNGR(std::logic_error, 
                     "I am pretty sure this reformulation doesn't work");

      objective_info = NULL;

      ObjectType type = ObjectType::get(this);

      register_response_xform_task( cb_map_obj_response, type );
   }


private:  // callback methods

   DEFINE_RESOLVERS(InvertSenseApplication<ProblemT>);


   template<bool>
   static bool
   cb_map_obj_response(const Application_Base* ref,
                       const utilib::Any &domain,
                       const AppRequest::request_map_t &outer_requests,
                       const AppResponse::response_map_t &sub_response,
                       AppResponse::response_map_t &response)
   {
      convert_objective_to_minimize
         <GetProblemTrait(ProblemT, multiple_objectives)>::fcn<ProblemT>
         (this, response);
      return true;
   }


private: // data

   /// A pre-resolved downcase of the OptApplication in the remote_app handle
   Application_SingleObjective* objective_info;
};



} // namespace colin

#endif // defined colin_InvertSenseApplication_h
