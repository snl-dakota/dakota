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

#include <colin/SolverMngr.h>
#include <colin/ExecuteMngr.h>
#include <colin/ApplicationMngr.h>
#include <colin/solver/Base.h>
#include <colin/Cache.h>
#include <colin/XMLProcessor.h>
#include <utilib/TinyXML_helper.h>

using std::string;
using std::ostringstream;
using std::cerr;
using std::endl;

using std::map;
using std::pair;
using std::make_pair;

namespace colin
{

//------------------------------------------------------------------

class SolverElementFunctor : public XML_Processor::ElementFunctor
{
public:
   virtual void process(TiXmlElement* root, int version)
   {
      static_cast<void>(version);
      std::string type;
      utilib::get_string_attribute(root, "type", type);

      SolverMngr().register_solver(SolverMngr().create_solver(type), type)
         ->construct(root);
   }

   virtual void describe(std::ostream &os, size_t indent = 0) const
   {
      for( ; indent > 0; --indent )
         os << XML_Processor::indent_string;
      os << "Please implement SolverElementFunctor::describe" << endl;
   }
};

//------------------------------------------------------------------

class SolverExecuteFunctor : public ExecuteManager::ExecuteFunctor
{
public:
   SolverExecuteFunctor(SolverHandle solver_)
      : solver(solver_)
   {
      if ( solver.empty() )
         EXCEPTION_MNGR(std::runtime_error, "SolverExecuteFunctor: "
                        "constructed with empty solver handle!");
   }

   virtual void execute(TiXmlElement* parameters)
   {
      // Process any XML sub-elements
      solver->construct(parameters);

      // Assign the implicit problem (if needed)
      if ( solver->get_problem_handle().empty() )
      {
         // Attempt to use the most recently created application 
         string problem = ApplicationMngr().get_newest_application();
         if ( problem.empty() )
            EXCEPTION_MNGR(std::runtime_error, 
                           "SolverExecuteFunctor::execute(): cannot assign "
                           "implicit problem: no problems defined");

         solver->set_problem( ApplicationMngr().get_application(problem) );
      }

      // initialize the intial & final point caches as specified by the
      // XML (using the default Inter Solver cache if necessary).
      solver->initialize_xml_init_cache();
      solver->initialize_xml_final_cache();

      solver->reset();

      solver->optimize();
      solver->postsolve();
   }

private:
   SolverHandle solver;
};


//------------------------------------------------------------------
// SolverManager::Data
//------------------------------------------------------------------

struct SolverManager::Data {
   Data() : newest("") {}

   struct SolverTypes {
      string             description;
      create_solver_fcn  fcn;
   };

   typedef map<string, pair<SolverHandle, string> >          name_map_t;
   typedef map<const Solver_Base*, name_map_t::iterator>  ptr_map_t;

   typedef map<string, SolverTypes >              create_map_t;

   ///
   name_map_t    solver_byname;
   ///
   ptr_map_t     solver_byptr;
   ///
   create_map_t  create;
   ///
   string        newest;

};

const std::string SolverManager::default_solver_name = "default";


//------------------------------------------------------------------
// SolverManager
//------------------------------------------------------------------

SolverManager& SolverMngr()
{
   static SolverManager manager;
   return manager;
}


SolverManager::SolverManager()
   : data(new Data)
{
   XMLProcessor().register_element("Solver", 2, new SolverElementFunctor);
}


SolverManager::~SolverManager()
{
   clear();
   delete data;
}


void
SolverManager::clear()
{
   data->solver_byname.clear();
   data->solver_byptr.clear();
}


bool
SolverManager::declare_solver_type_impl( std::string type, 
                                         std::string description,
                                         create_solver_fcn fcn )
{
   pair<Data::create_map_t::iterator, bool> ok 
      = data->create.insert(make_pair( type, Data::SolverTypes() ));
 
   if ( ! ok.second )
      EXCEPTION_MNGR(solverManager_error, 
                     "SolverManager::declare_solver_type(): "
                     "duplicate solver declaration: " << type);
   
   ok.first->second.description = description;
   ok.first->second.fcn = fcn;
   return true;
}


SolverHandle
SolverManager::register_solver( SolverHandle handle, std::string type )
{
   pair<Data::name_map_t::iterator, bool> ok 
      = data->solver_byname.insert(make_pair( default_solver_name, 
                                              make_pair(handle, type) ));

   if ( ! ok.second )
      EXCEPTION_MNGR(solverManager_registration_error, "SolverManager::"
                     "register_solver(): duplicate solver name, \""
                     << default_solver_name << "\"");
   
   if (!data->solver_byptr.insert(make_pair(handle.object(), ok.first)).second)
   {
      data->solver_byname.erase(ok.first);
      EXCEPTION_MNGR(solverManager_registration_error,
                     "SolverMngr::register_solver(): "
                     "Solver pointer already registered as '"
                     << data->solver_byptr[handle.object()]->first 
                     << "', refusing registration "
                     "under name '" << default_solver_name << "'");
   }

   data->newest = ok.first->first;
   ExecuteMngr().register_command( string("solve:") + ok.first->first,
                                   new SolverExecuteFunctor(handle) );
   return handle;
}


void
SolverManager::reregister_solver( SolverHandle solver,
                                  const std::string& name )
{
   Data::ptr_map_t::iterator p_it = data->solver_byptr.find(solver.object());
   if ( p_it == data->solver_byptr.end() )
   {
#if 1
      EXCEPTION_MNGR(solverManager_registration_error,
                     "SolverMngr::reregister_solver(): "
                     "Solver pointer not registered with the solver "
                     "manager.  Refusing to reregister under name '" << name
                     << "'");
#else
      //ucerr << "WARNING: placeholder until XML parsing rework is complete."
      //      << endl;
      //ucerr << "SolverMngr::reregister_solver(): "
      //   "Solver pointer not registered with the solver "
      //   "manager.  Pseudo-registering solver named '" << name << "'";
      
      pair<Data::name_map_t::iterator, bool> ok = data->solver_byname.insert
         (make_pair(name, make_pair(solver, solver->type())));
      if ( ! ok.second )
         EXCEPTION_MNGR(solverManager_registration_error,
                        "SolverMngr::reregister_solver(): "
                        "Solver with name '" << name
                        << "' already registered");
      ok.first->second.solver = solver;
      data->solver_byptr[solver.object()] = ok.first;
      data->newest = ok.first->first;

      ExecuteMngr().register_command( string("solve:") + ok.first->first, 
                                      new SolverExecuteFunctor(solver) );
      return;
#endif
   }

   if ( name.empty() )
      EXCEPTION_MNGR(solverManager_registration_error,
                     "SolverMngr::reregister_solver(): "
                     "Refusing to reregister with an empty name");
      
   pair<Data::name_map_t::iterator, bool> ok 
      = data->solver_byname.insert(make_pair(name, p_it->second->second));
   if ( ! ok.second )
      EXCEPTION_MNGR(solverManager_registration_error,
                     "SolverMngr::reregister_solver(): "
                     "Solver with name '" << name
                     << "' already registered");

   if ( data->newest == p_it->second->first )
      data->newest = name;

   ExecuteMngr().unregister_command( string("solve:") + p_it->second->first );
   ExecuteMngr().register_command( string("solve:") + name, 
                                   new SolverExecuteFunctor(solver) );

   data->solver_byname.erase(p_it->second);
   p_it->second = ok.first;
}


void
SolverManager::unregister_solver(const std::string& name)
{
   Data::name_map_t::iterator it = data->solver_byname.find(name);
   if ( it == data->solver_byname.end() )
      EXCEPTION_MNGR(solverManager_registration_error,
                     "SolverMngr::unregister_solver(): "
                     "Solver with name '" << name << "' not registered");

   if ( data->newest == name )
      data->newest = "";

   ExecuteMngr().unregister_command( string("solve:") + name );

   data->solver_byptr.erase(it->second.first.object());
   data->solver_byname.erase(it);
}


void 
SolverManager::unregister_solver(SolverHandle solver)
{
   Data::ptr_map_t::iterator it = data->solver_byptr.find(solver.object());
   if ( it == data->solver_byptr.end() )
      EXCEPTION_MNGR(solverManager_registration_error,
                     "SolverMngr::unregister_solver(): Solver '" << 
                     utilib::demangledName(typeid(*solver.object()))
                     << "' not registered");

   if ( data->newest == it->second->first )
      data->newest = "";

   ExecuteMngr().unregister_command( string("solve:") + it->second->first );

   data->solver_byname.erase(it->second);
   data->solver_byptr.erase(it);
}


SolverHandle
SolverManager::create_solver( const std::string type )
{
   Data::create_map_t::iterator it = data->create.find(type);
   if ( it == data->create.end() )
      EXCEPTION_MNGR(solverManager_error, 
                     "SolverManager::create_solver(): "
                     "unknown solver, \"" << type << "\"");
   
   return it->second.fcn();
}


SolverHandle
SolverManager::get_solver(const std::string& name) const
{
   Data::name_map_t::const_iterator it = data->solver_byname.find(name);
   if ( it == data->solver_byname.end() )
      EXCEPTION_MNGR(solverManager_error, 
                     "SolverMngr::get_solver(): "
                     "Solver with name '" << name << "' not registered");

   return it->second.first;
}

std::string
SolverManager::get_solver_name(const Solver_Base* solver) const
{
   Data::ptr_map_t::iterator it = data->solver_byptr.find(solver);
   return ( it == data->solver_byptr.end() ? "" : it->second->first );
}

std::string
SolverManager::get_solver_type(const Solver_Base* solver ) const
{
   Data::ptr_map_t::iterator it = data->solver_byptr.find(solver);
   return ( it == data->solver_byptr.end() ? "" : it->second->second.second );
}



///
std::string 
SolverManager::get_newest_solver() const
{
   return data->newest;
}


void
SolverManager::get_solver_types
(std::list<std::pair<std::string, std::string> > &solvers) const
{
   solvers.clear();

   Data::create_map_t::const_iterator it = data->create.begin();
   Data::create_map_t::const_iterator itEnd = data->create.end();
   for(; it != itEnd; ++it )
      solvers.push_back(make_pair(it->first, it->second.description));
}


void
SolverManager::print_solver_types(std::ostream& os) const
{
   if ( data->create.empty() )
   {
      os << "  solvers: None" << endl;
      return;
   }

   os << "  solvers: " << std::endl;
   Data::create_map_t::const_iterator it = data->create.begin();
   Data::create_map_t::const_iterator itEnd = data->create.end();
   for(; it != itEnd; ++it )
   {
      os << "    " << it->first << std::endl;
      os << "         " << it->second.description << std::endl;
   }
}

} // namespace colin
