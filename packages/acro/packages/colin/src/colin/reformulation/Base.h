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
 * \file ReformulationApplication.h
 *
 * Defines the colin::ReformulationApplication class.
 */

#ifndef colin_ReformulationApplication_h
#define colin_ReformulationApplication_h

#include <colin/application/Base.h>


namespace colin {

class BasicReformulationApplication : virtual public Application_Base
{
   typedef std::list<boost::signals::connection>  reformulation_cb_list_t;

public:
   BasicReformulationApplication()
   {
      typedef BasicReformulationApplication this_t;
      initializer("BaseProblem").connect
         ( boost::bind(&this_t::cb_initialize, this, _1) );
   }

   virtual ~BasicReformulationApplication();

   void reformulate_application(ApplicationHandle handle);

protected: // methods
   /// Called before changing the reformulated application; derived
   /// class should throw an exception if the reformulation is invalid
   boost::signal<void(ApplicationHandle)> validate_reformulated_application;

   /// Called after setting the remote_app to allow derived class to
   /// configure any linkages (i.e. property onChange callbacks).
   boost::signal<void()> configure_reformulated_application;

   /// Reference properties from a wrapped application (NB: has side effects!)
   void reference_reformulated_application_properties
       ( std::set<ObjectType> exclude = std::set<ObjectType>(),
         std::set<std::string> block_promotion = std::set<std::string>() );

protected: // data

   ApplicationHandle remote_app;
   
   reformulation_cb_list_t  _reformulation_cb_list;

private:
   void cb_initialize(TiXmlElement* elt);
};


class NonexecutableApplication : virtual public Application_Base
{
public:
   NonexecutableApplication() {}
   virtual ~NonexecutableApplication() {}

protected:
   /// True if a spawned evaluation is "available"
   virtual bool evaluation_available()
   {
      EXCEPTION_MNGR(std::logic_error, 
                     "NonexecutableApplication::evaluation_available()"
                     " called by a non-terminal Application (" << 
                     utilib::demangledName(typeid(*this)) << ")");
      return false;
   }

   /// Cause the application to actually start computing the requests
   virtual utilib::Any 
   spawn_evaluation_impl( const utilib::Any &,
                          const AppRequest::request_map_t &,
                          utilib::seed_t & )
   {
      EXCEPTION_MNGR(std::logic_error, 
                     "NonexecutableApplication::spawn_evaluation_impl()"
                     " called by a non-terminal Application (" << 
                     utilib::demangledName(typeid(*this)) << ")");
      return utilib::Any();
   }

   /// Collect the responses from a call to spawn_evaluation
   virtual utilib::Any
   collect_evaluation_impl ( AppResponse::response_map_t &,
                             utilib::seed_t & )
   {
      EXCEPTION_MNGR(std::logic_error, 
                     "NonexecutableApplication::collect_evaluation_impl()"
                     " called by a non-terminal Application (" << 
                     utilib::demangledName(typeid(*this)) << ")");
      return utilib::Any();
   }

   /// Perform a blocking synchronous evaluation
   virtual void 
   perform_evaluation_impl( const utilib::Any &,
                            const AppRequest::request_map_t &,
                            utilib::seed_t &,
                            AppResponse::response_map_t & )
   {
      EXCEPTION_MNGR(std::logic_error, 
                     "NonexecutableApplication::perform_evaluation_impl()"
                     " called by a non-terminal Application (" << 
                     utilib::demangledName(typeid(*this)) << ")");
   }
};


class ReformulationApplication 
   : public BasicReformulationApplication,
     public NonexecutableApplication
{
public:
   ReformulationApplication() 
   {
      typedef ReformulationApplication this_t;
      BasicReformulationApplication::validate_reformulated_application.connect
         ( boost::bind(&this_t::validate_reformulated_application, this, _1) );
      BasicReformulationApplication::configure_reformulated_application.connect
         ( boost::bind(&this_t::configure_reformulated_application, this) );
   }

   virtual ~ReformulationApplication() {}

protected:
   /// Default to using the evaluation manager from the wrapped application
   virtual const EvaluationManager_Handle default_eval_mngr()
   { return remote_app->eval_mngr().handle(); }

   virtual Application_Base* get_reformulated_application() const
   { return remote_app.object(); }


   virtual void 
   validate_reformulated_application(ApplicationHandle handle) = 0;

   virtual void configure_reformulated_application() = 0;
};

} // namespace colin


#endif // colin_ReformulationApplication_h
