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

#include <colin/ExecuteMngr.h>
#include <colin/ProcessManager.h>
#include <colin/SolverMngr.h>
#include <colin/XMLProcessor.h>
#include <utilib/TinyXML_helper.h>
#include <utilib/exception_mngr.h>

using std::cerr;
using std::endl;
using std::string;

using std::map;
using std::pair;
using std::make_pair;

namespace colin {

//---------------------------------------------------------------------
// ExecuteMngr() singleton
//---------------------------------------------------------------------

ExecuteManager& ExecuteMngr()
{
   static ExecuteManager manager;
   return manager;
}

//---------------------------------------------------------------------
// ExecuteElementFunctor: the actual processor for the <Execute> element
// in the XML spec
//---------------------------------------------------------------------

class ExecuteElementFunctor : public XML_Processor::ElementFunctor
{
   virtual void process(TiXmlElement* root, int version)
   {
      static_cast<void>(version);
      std::string mngr;
      utilib::get_string_attribute( root, "process_manager", mngr, 
                            ExecuteManager::default_process_mngr );
      ExecuteMngr().set_process_manager(mngr);
      

      // each child element should be a valid command...
      TiXmlElement* node = root->FirstChildElement();

      // special case: if it is an empty element, just execute the last
      // solver
      if ( node == NULL )
      {
         string solver = SolverMngr().get_newest_solver();
         if ( ! solver.empty() )
            ExecuteMngr().run_command(string("solve:") + solver);
         return;
      }
      
      int rank;
      int myRank = ExecuteMngr().rank();

      // step through all the commands
      for( ; node != NULL; node = node->NextSiblingElement() )
      {
         utilib::get_num_attribute( node, "rank", rank, ExecuteManager::local );
         if (( rank == ExecuteManager::local ) || ( rank == myRank ))
            ExecuteMngr().run_command(node->ValueStr(), rank, node);
      }
   }

   virtual void describe(std::ostream &os, size_t indent = 0) const
   {
      for( ; indent > 0; --indent )
         os << XML_Processor::indent_string;
      os << "Please implement ExecuteElementFunctor::describe" << endl;
   }
};



//---------------------------------------------------------------------
// ExecuteManager::Data definition
//---------------------------------------------------------------------

struct ExecuteManager::Data {
   Data() : active_manager(NULL), exiting(false) {}

   struct ProcessMngrHandle {
      ProcessMngrHandle()
         : manager(NULL), create(NULL) 
      {}
      ProcessMngrHandle(process_mngr_create_fcn fcn) 
         : manager(NULL), create(fcn)
      {}
      ~ProcessMngrHandle()
      {
         delete manager;
      }

      ProcessManager          *manager;
      process_mngr_create_fcn  create;
   };
   

   typedef map<string, ExecuteFunctor*>  command_map_t;

   typedef map<string, ProcessMngrHandle>  processMngr_map_t;

   /// The registered command handlers (by name)
   command_map_t  commands;

   /// The registered command handlers (by name)
   processMngr_map_t  managers;

   ///
   ProcessManager  *active_manager;
   ///
   bool  exiting;
};


const std::string ExecuteManager::default_process_mngr = "local";
const size_t      ExecuteManager::max_command_length   = 20;
const int         ExecuteManager::local                = -1;
const int         ExecuteManager::broadcast            = -2;


//---------------------------------------------------------------------
// AbortFunctor
//---------------------------------------------------------------------

class AbortFunctor : public ExecuteManager::ExecuteFunctor
{
public:
   AbortFunctor()
   {}

   virtual void execute(TiXmlElement* parameters)
   {
      int code = 0;
      utilib::get_num_attribute(parameters, "errorcode", code, 1);
      ExecuteMngr().Abort(code);
   }
};



//---------------------------------------------------------------------
// CommandLoopFunctor
//---------------------------------------------------------------------

class CommandLoopFunctor : public ExecuteManager::ExecuteFunctor
{
public:
   CommandLoopFunctor()
   {}

   virtual void execute(TiXmlElement* parameters)
   {
      static_cast<void>(parameters);

      ProcessManager *pm = ExecuteMngr().get_process_manager();

      while ( ! ExecuteMngr().Exiting() )
         pm->recv_command();
   }
};



//---------------------------------------------------------------------
// ExitFunctor
//---------------------------------------------------------------------

class ExitFunctor : public ExecuteManager::ExecuteFunctor
{
public:
   ExitFunctor()
   {}

   virtual void execute(TiXmlElement* parameters)
   {
      static_cast<void>(parameters);
      ExecuteMngr().Exit();
   }
};


//---------------------------------------------------------------------
// ExitImplFunctor
//---------------------------------------------------------------------

class ExitImplFunctor : public ExecuteManager::ExecuteFunctor
{
public:
   ExitImplFunctor(ExecuteManager::Data *data_)
      : data(data_)
   {}

   virtual void execute(TiXmlElement* parameters)
   {
      static_cast<void>(parameters);
      data->exiting = true;
   }

private:
   ExecuteManager::Data *data; 
};



//---------------------------------------------------------------------
// ExecuteManager public methods
//---------------------------------------------------------------------


ExecuteManager::ExecuteManager()
   : data(new Data)
{
   register_command("Abort", new AbortFunctor);
   register_command("CommandLoop", new CommandLoopFunctor);
   register_command("Exit", new ExitFunctor);
   register_command("ExitImplementation", new ExitImplFunctor(data));
}


ExecuteManager::~ExecuteManager()
{
   map<string, ExecuteFunctor*>::iterator it = data->commands.begin();
   map<string, ExecuteFunctor*>::iterator itEnd = data->commands.end();
   for( ; it != itEnd; ++it )
      delete it->second;

   delete data;
}


void
ExecuteManager::define_process_manager(const std::string& manager,
                                       process_mngr_create_fcn fcn)
{
   if ( ! data->managers.insert
        (make_pair( manager, Data::ProcessMngrHandle(fcn) )).second )
      EXCEPTION_MNGR(std::runtime_error, "ExecuteManager::"
                     "define_process_manager(): duplicate definition of "
                     "process manager, \"" << manager << "\"");
}


void 
ExecuteManager::set_process_manager(const std::string& manager)
{
   if ( data->active_manager != NULL )
      EXCEPTION_MNGR(std::runtime_error, "ExecuteManager::"
                     "set_process_manager(): process manager already set.");

   data->active_manager = get_process_manager(manager);
}


ProcessManager*
ExecuteManager::get_process_manager(const std::string& manager)
{
   Data::processMngr_map_t::iterator it = data->managers.find(manager);
   if ( it == data->managers.end() )
      EXCEPTION_MNGR(std::runtime_error, "ExecuteManager::"
                     "get_process_manager(): unknown process manager, \""
                     << manager << "\"");

   if ( it->second.manager == NULL )
      it->second.manager = it->second.create();
   return it->second.manager;
}


int
ExecuteManager::rank() 
{
   if ( data->active_manager == NULL )
      data->active_manager = get_process_manager();

   return data->active_manager->rank();
}


int
ExecuteManager::num_ranks() 
{
   if ( data->active_manager == NULL )
      data->active_manager = get_process_manager();

   return data->active_manager->num_ranks();
}


bool
ExecuteManager::Exiting() const
{
   return data->exiting;
}


void
ExecuteManager::Exit()
{
   run_command("ExitImplementation", broadcast);
}


void
ExecuteManager::Abort(int error_code)
{
#if defined (ACRO_HAVE_MPI)
   if ( utilib::uMPI::running() )
      MPI_Abort(MPI_COMM_WORLD, error_code);
#endif
   exit(error_code);
}


bool
ExecuteManager::register_command( const std::string& name, 
                                  ExecuteFunctor* fcn )
{
   pair<Data::command_map_t::iterator, bool> ans;
   if ( name.size() > max_command_length )
      ans = data->commands.insert(make_pair(name.substr(0,max_command_length), 
                                            fcn));
   else
      ans = data->commands.insert(make_pair(name, fcn));
      
   if ( ! ans.second )
      EXCEPTION_MNGR(std::runtime_error, "ExecuteManager::register_command(): "
                     "duplicate command name, \"" << ans.first->first << "\"");

   return true;
}


void
ExecuteManager::unregister_command( const std::string& name )
{
   size_t ans;
   if ( name.size() > max_command_length )
      ans = data->commands.erase(name.substr(0,max_command_length));
   else
      ans = data->commands.erase(name);
      
   if ( ans == 0 )
      EXCEPTION_MNGR(std::runtime_error, "ExecuteManager::"
                     "unregister_command(): unknown command, \""
                     << name << "\"");
}


void
ExecuteManager::run_command( const std::string& command, 
                             int on_rank,
                             TiXmlElement* params )
{
   if ( data->active_manager == NULL )
      data->active_manager = get_process_manager();

   if (( on_rank != local ) && ( on_rank != data->active_manager->rank() ))
   {
      data->active_manager->route_command(command, params, on_rank);
      data->active_manager->ping(on_rank);
   }
   else
   {
      Data::command_map_t::iterator it;
      if ( command.size() > max_command_length )
         it = data->commands.find(command.substr(0,max_command_length));
      else
         it = data->commands.find(command);
      if ( it == data->commands.end() )
         EXCEPTION_MNGR(std::runtime_error, "ExecuteManager::run_command(): "
                        "unknown command, \"" << command << "\"");
      it->second->execute(params);
   }
}


void
ExecuteManager::route_command( const std::string& command, 
                               int to_rank,
                               TiXmlElement* params )
{
   if ( data->active_manager == NULL )
      data->active_manager = get_process_manager();
   data->active_manager->route_command(command, params, to_rank);
}


utilib::SharedPtr<utilib::ioSerialStream> 
ExecuteManager::serialstream(int to_rank)
{
   if ( data->active_manager == NULL )
      data->active_manager = get_process_manager();
   return data->active_manager->serializer(to_rank);
}


//---------------------------------------------------------------------

namespace {

bool RegisterExecuteManager()
{
   XMLProcessor().register_element("Execute", 3, new ExecuteElementFunctor);
   return true;
}
const bool tmp = RegisterExecuteManager();

} // namespace colin::(local)
} // namespace colin
