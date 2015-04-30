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
 * \file ApplicationMngr.h
 *
 * Function wrapper for managing COLIN application objects.
 */

#ifndef colin_ApplicationMngr_h
#define colin_ApplicationMngr_h

#include <acro_config.h>
//#include <colin/Problem.h>
#include <colin/Application.h>

#include <utilib/exception_mngr.h>

namespace colin
{

class Application_Base;
class ApplicationManager;

///
ApplicationManager& ApplicationMngr();


class ApplicationManager
{
public:
   ///
   typedef ApplicationHandle(*create_app_fcn)();
   ///
   static const std::string default_application_name;

public:

   ///
   ApplicationManager();

   ///
   ~ApplicationManager();

   ///
   void clear();

   ///
   bool declare_application_type( std::string type, 
                                  std::string problem_type,
                                  create_app_fcn fcn );

   template<typename TYPE>
   bool declare_application_type(std::string type)
   {
      return declare_application_type
         (type, ProblemType<typename TYPE::problem_t>::name(), &create<TYPE>);
   }



   ///
   Application_Base* register_application( ApplicationHandle handle,
                                           const std::string &name );

   ///
   void reregister_application( Application_Base* app,
                                const std::string &name );

   ///
   void unregister_application(const std::string& name);

   ///
   void unregister_application(Application_Base* app);

   ///
   ApplicationHandle create_application( const std::string type, 
                                         const std::string problem_type );

   ///
   ApplicationHandle get_application(const std::string& name) const;

   ///
   std::string get_application_name(const Application_Base* app) const;

   ///
   std::string get_newest_application() const;


   ///
   template<typename ProblemT>
   static Application<ProblemT>* resolve(Application_Base* app)
   {
      Application<ProblemT>* ans 
         = dynamic_cast<Application<ProblemT>*>(app);
      if ( ans != NULL )
         return ans;
      if ( app->problem_type() == ProblemT::value )
         return static_cast<Application<ProblemT>*>(app);

      EXCEPTION_MNGR(std::runtime_error, "ApplicationManager::resolve(): "
                     "unable to cast " << utilib::demangledName(typeid(*app))
                     << " into " <<
                     utilib::demangledName(typeid(Application<ProblemT>)));
      return NULL;
   }


private:
   ///
   template<typename TYPE>
   static ApplicationHandle create()
   {
      return ApplicationHandle::create<TYPE>().first;
   }

private:
   ///
   struct Data;
   ///
   Data *data;
};


} // namespace colin

#endif // defined colin_ApplicationMngr_h
