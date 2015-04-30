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


#include <colin/ProcessManager.h>
#include <colin/ExecuteMngr.h>

#include <utilib/sSerialStream.h>

#include <tinyxml/tinyxml.h>

using utilib::SharedPtr;
using utilib::ioSerialStream;
using utilib::sSerialStream;

namespace colin {

class ProcessManager_Local : public ProcessManager
{
private:
   struct BufferedCommand {
      BufferedCommand()
         : command(),
           has_params(false)
      {}

      BufferedCommand(const BufferedCommand& rhs)
         : command(rhs.command),
           has_params(rhs.has_params)
      {
         params << rhs.params.str();
      }

      BufferedCommand(const std::string& cmd, TiXmlElement* params_)
         : command(cmd),
           has_params(params != NULL)
      {
         if ( has_params )
            params << (*params_);
      }

      std::string        command;
      std::stringstream  params;
      bool               has_params;
   };

public:
   virtual ~ProcessManager_Local()
   {}

   virtual int rank() const
   {
      return 0;
   }

   virtual int num_ranks() const
   {
      return 1;
   }

   virtual void route_command( const std::string& command, 
                               TiXmlElement* params, int to_rank )
   {
      if ( to_rank != rank() )
         EXCEPTION_MNGR(std::runtime_error, "ProcessManager_Local::"
                        "route_command(): invalid destination rank, "
                        << to_rank );

      buffer.push_back(BufferedCommand(command, params));
   }

   virtual void recv_command()
   {
      if ( buffer.empty() )
         EXCEPTION_MNGR(std::runtime_error, "ProcessManager_Local::"
                        "recv_command(): Deadlock: Serial application "
                        "issued a recv_command() call with an empty command "
                        "buffer.");
      BufferedCommand &cmd = buffer.front();
      if ( cmd.has_params )
      {
         TiXmlDocument params;
         cmd.params >> params;
         ExecuteMngr().run_command( cmd.command, ExecuteManager::local, 
                                    params.RootElement() );
      }
      else
         ExecuteMngr().run_command( cmd.command, ExecuteManager::local, NULL );
      buffer.pop_front();
   }


   virtual utilib::SharedPtr<utilib::ioSerialStream> serializer(int to_rank)
   {
      static_cast<void>(to_rank);
      return SharedPtr<ioSerialStream>(new sSerialStream());
   }


   virtual void ping(int to_rank)
   {
      if ( to_rank != rank() )
         EXCEPTION_MNGR(std::runtime_error, "ProcessManager_Local::ping(): "
                        "invalid destination rank, " << to_rank );

      // We actually push an ECHO command onto the stack so that we know
      // when to stop the following loop (in case some of the pending
      // commands route *new* commands).
      buffer.push_back(BufferedCommand("ECHO", NULL));

      // This simulates the remote rank processing commands until the
      // ECHO is received.
      while ( buffer.front().command.compare("ECHO") != 0 )
         recv_command();

      // pop off the ECHO command
      buffer.pop_front();
   }


   static ProcessManager* create()
   { return new ProcessManager_Local; }

private:
   std::list<BufferedCommand> buffer;
};

namespace StaticInitializers {
namespace {
bool RegisterLocalPM()
{
   ExecuteMngr().define_process_manager("local", ProcessManager_Local::create);
   return true;
}

} // namespace colin::StaticInitializers::(local)

extern const volatile bool local_process_mngr = RegisterLocalPM();

} // namespace colin::StaticInitializers
} // namespace colin
