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

#include <colin/ApplicationMngr.h>
#include <colin/application/Base.h>
#include <colin/XMLProcessor.h>
#include <colin/ColinRegistrations.h>
#include <utilib/TinyXML_helper.h>


using std::string;
using std::ostringstream;
using std::cerr;
using std::endl;

using std::map;
using std::pair;
using std::make_pair;

using utilib::Any;
using utilib::get_string_attribute;

namespace colin {

// This call guarantees that all factory components are linked in when
// we build against static libraries.  This is here because the
// StaticInitializers register problem transformations with the
// ApplicationMngr(), so if your application uses the ApplicationMngr,
// it will automatically link in all the statically-registered colin
// objects.
namespace StaticInitializers {
namespace {

int string2application(const Any& from, Any& to)
{
   ApplicationHandle ans = 
      ApplicationMngr().get_application(from.expose<std::string>());
   to = ans;
   return ans.empty() ? utilib::Type_Manager::CastWarning::ValueMissing : 0;
}

bool RegisterApplicationManager()
{
   bool ans = static_colin_registrations;
   utilib::TypeManager()->register_lexical_cast
      (typeid(std::string), typeid(ApplicationHandle), &string2application);
   return ans;
}

const bool register_all = RegisterApplicationManager();
} // namespace colin::StaticInitializers::(local)
} // namespace colin::StaticInitializers


class ProblemElementFunctor : public XML_Processor::ElementFunctor
{
   virtual void process(TiXmlElement* root, int version)
   {
      static_cast<void>(version);
      string type;
      get_string_attribute(root, "type", type, "");
      string app_name;
      get_string_attribute(root, "application", app_name, "AnalysisCode");
      string name;
      get_string_attribute( root, "id", name, 
                            ApplicationManager::default_application_name );

      ApplicationHandle tmp;
      tmp = ApplicationMngr().create_application(app_name, type);
      tmp->initialize(root);
      ApplicationMngr().register_application(tmp, name);

      // HACK so new XML processing diffs correctly with old QA results
      bool verbose = true;
      if ( verbose )
      {
         string name = ApplicationMngr().get_newest_application();
         ucout << "\n------------------------" << std::endl;
         ucout << "XML Application Summary" << std::endl;
         ucout << "------------------------" << std::endl;
         ucout << "Problem Name: " << name << std::endl;
         ucout << "Problem Type: " << type << std::endl;
         ApplicationMngr().get_application(name)->print_summary(ucout);
      }
   }

   virtual void describe(std::ostream &os, size_t indent = 0) const
   {
      for( ; indent > 0; --indent )
         os << XML_Processor::indent_string;
      os << "Please implement ProblemElementFunctor::describe" << endl;
   }
};

//------------------------------------------------------------------
// ApplicationManager::Data
//------------------------------------------------------------------

struct ApplicationManager::Data {
   Data() : newest("") {}

   //struct AppInfo {
   //   Application_Base*  app;
   //   utilib::Any           holder;
   //   utilib::Any           problem;
   //};

   struct AppTypes {
      string                       default_type;
      map<string, create_app_fcn>  type;
   };

   typedef map<string, ApplicationHandle>                         name_map_t;
   typedef map<const Application_Base*, name_map_t::iterator>  ptr_map_t;
   typedef map<string, AppTypes >                                 create_map_t;

   ///
   name_map_t    app_byname;
   ///
   ptr_map_t     app_byptr;
   ///
   create_map_t  create;
   ///
   string        newest;
};


const std::string ApplicationManager::default_application_name = "default";


//------------------------------------------------------------------
// ApplicationManager
//------------------------------------------------------------------

ApplicationManager& ApplicationMngr()
{
   static ApplicationManager a_manager;
   return a_manager;
}


ApplicationManager::ApplicationManager()
   : data(new Data)
{
   XMLProcessor().register_element("Problem", 1, new ProblemElementFunctor);
}


ApplicationManager::~ApplicationManager()
{
   clear();
   delete data;
}


void
ApplicationManager::clear()
{
   data->app_byname.clear();
   data->app_byptr.clear();
}


bool
ApplicationManager::declare_application_type( std::string type, 
                                              std::string problem_type,
                                              create_app_fcn fcn )
{
   Data::AppTypes &app_type = data->create[type];
   if ( app_type.type.empty() )
      app_type.default_type = problem_type;

   if ( app_type.type.insert(make_pair( problem_type, fcn )).second == false )
      EXCEPTION_MNGR(std::runtime_error, 
                     "ApplicationManager::declare_application_type(): "
                     "duplicate application declaration: " << type 
                     << "::" << problem_type);
   return true;
}


Application_Base*
ApplicationManager::register_application( ApplicationHandle handle,
                                          const std::string &name )
{
   
   pair<Data::name_map_t::iterator, bool> ok = data->app_byname.insert
      ( make_pair( name.empty() ? default_application_name : name, handle ) );

   if ( ! ok.second )
      EXCEPTION_MNGR( std::runtime_error, 
                      "ApplicationManager::register_application(): "
                      "duplicate application name, '" <<
                      ( name.empty() ? default_application_name : name ) <<
                      "'");
   
   Application_Base* app = handle.object();
   if ( ! data->app_byptr.insert(make_pair(app, ok.first)).second )
   {
      data->app_byname.erase(ok.first);
      EXCEPTION_MNGR(std::runtime_error,
                     "ApplicationMngr::register_application(): "
                     "Application pointer already registered as '"
                     << data->app_byptr[app]->first 
                     << "', refusing registration under name '" << 
                     (name.empty() ? default_application_name : name) << "'");
   }

   data->newest = ok.first->first;

   return app;
}


void
ApplicationManager::reregister_application( Application_Base* app,
                                            const std::string& name )
{
   Data::ptr_map_t::iterator p_it = data->app_byptr.find(app);
   if ( p_it == data->app_byptr.end() )
   {
#if 0
      EXCEPTION_MNGR(std::runtime_error,
                     "ApplicationMngr::reregister_application(): "
                     "Application pointer not registered with the application "
                     "manager.  Refusing to reregister under name '" << name
                     << "'");
#else
      //ucerr << "WARNING: placeholder until XML parsing rework is complete."
      //      << endl;
      //ucerr << "ApplicationMngr::reregister_application(): "
      //   "Application pointer not registered with the application "
      //   "manager.  Pseudo-registering application named '" << name << "'";
      
      pair<Data::name_map_t::iterator, bool> ok = data->app_byname.insert
         ( make_pair( name, app->get_handle() ) );
      if ( ! ok.second )
         EXCEPTION_MNGR(std::runtime_error,
                        "ApplicationMngr::reregister_application(): "
                        "Application with name '" << name
                        << "' already registered");
      data->app_byptr[app] = ok.first;
      data->newest = ok.first->first;
      return;
#endif
   }

   if ( name.empty() )
      EXCEPTION_MNGR(std::runtime_error,
                     "ApplicationMngr::reregister_application(): "
                     "Refusing to reregister with an empty name");
      
   pair<Data::name_map_t::iterator, bool> ok 
      = data->app_byname.insert(make_pair(name, p_it->second->second));
   if ( ! ok.second )
      EXCEPTION_MNGR(std::runtime_error,
                     "ApplicationMngr::reregister_application(): "
                     "Application with name '" << name
                     << "' already registered");

   if (  data->newest == p_it->second->first )
      data->newest = name;

   data->app_byname.erase(p_it->second);
   p_it->second = ok.first;
}


void
ApplicationManager::unregister_application(const std::string& name)
{
   Data::name_map_t::iterator it = data->app_byname.find(name);
   if ( it == data->app_byname.end() )
      EXCEPTION_MNGR(std::runtime_error,
                     "ApplicationMngr::unregister_application(): "
                     "Application with name '" << name << "' not registered");

   if ( data->newest == name )
      data->newest = "";

   data->app_byptr.erase(it->second.object());
   data->app_byname.erase(it);
}


void 
ApplicationManager::unregister_application(Application_Base* app)
{
   Data::ptr_map_t::iterator it = data->app_byptr.find(app);
   if ( it == data->app_byptr.end() )
      EXCEPTION_MNGR(std::runtime_error,
                     "ApplicationMngr::unregister_application(): "
                     "Application '" << utilib::demangledName(typeid(*app))
                     << "' not registered");

   if ( data->newest == it->second->first )
      data->newest = "";

   data->app_byname.erase(it->second);
   data->app_byptr.erase(it);
}


ApplicationHandle
ApplicationManager::create_application( const std::string type,
                                        const std::string problem_type )
{
   Data::create_map_t::iterator it = data->create.find(type);
   if ( it == data->create.end() )
      EXCEPTION_MNGR(std::runtime_error, 
                     "ApplicationManager::create_application(): "
                     "unknown application, \"" << type << "\"");
   
   map<string, create_app_fcn>::iterator it2 
      = it->second.type.find(problem_type);
   if ( it2 == it->second.type.end() )
      EXCEPTION_MNGR(std::runtime_error, 
                     "ApplicationManager::create_application(): "
                     "application \"" << type << "\" is not defined for "
                     "problem type \"" << problem_type << "\"");

   return it2->second();
}


ApplicationHandle
ApplicationManager::get_application(const std::string& name) const
{
   Data::name_map_t::const_iterator it = data->app_byname.find(name);
   if ( it == data->app_byname.end() )
   {
      return ApplicationHandle();
      //EXCEPTION_MNGR(std::runtime_error, 
      //               "ApplicationMngr::get_application(): Application "
      //               "with name '" << name << "' not registered");
   }
   return it->second;
}

///
std::string 
ApplicationManager::get_application_name(const Application_Base* app) const
{
   Data::ptr_map_t::iterator it = data->app_byptr.find(app);
   if ( it == data->app_byptr.end() )
      return "";
   else
      return it->second->first;
}

///
std::string 
ApplicationManager::get_newest_application() const
{
   return data->newest;
}

} // namespace colin
