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
 * \file ExecuteMngr.h
 *
 * Function wrapper for managing COLIN execution handlers.
 */

#ifndef colin_ExecuteMngr_h
#define colin_ExecuteMngr_h

#include <acro_config.h>

#include <utilib/SharedPtr.h>

class TiXmlElement;
namespace utilib {
class ioSerialStream;
};

namespace colin
{

class ExecuteManager;
class ProcessManager;

///
ExecuteManager& ExecuteMngr();


class ExecuteManager
{
public:
   typedef ProcessManager*(*process_mngr_create_fcn)();

   class ExecuteFunctor
   {
   public:
      virtual ~ExecuteFunctor() {}
      virtual void execute(TiXmlElement* parameters) = 0;
   };

   /// The name of the default process manager
   static const std::string default_process_mngr;
   /// The maximum length that any command can be (sets a buffer size)
   static const size_t max_command_length;
   /// An alias for the "local" process id
   static const int local;
   /// Broadcast the command to ALL process ids
   static const int broadcast;

public:

   ///
   ExecuteManager();

   ///
   ~ExecuteManager();

   /// 
   void define_process_manager(const std::string& manager,
                               process_mngr_create_fcn fcn);

   /// 
   void set_process_manager(const std::string& manager);

   /// 
   ProcessManager* 
   get_process_manager(const std::string& manager = default_process_mngr);

   ///
   int rank();

   ///
   int num_ranks();

   ///
   bool Exiting() const;

   ///
   void Exit();

   ///
   void Abort(int error_code);

   /// Define a new command handler (ExecuteManager will own & delete pointer)
   bool register_command( const std::string& name,
                          ExecuteFunctor* fcn );

   ///
   void unregister_command(const std::string& name);

   ///
   void run_command( const std::string& command, 
                     int on_rank = local,
                     TiXmlElement* params = NULL );

   ///
   void route_command( const std::string& command, 
                       int to_rank = local,
                       TiXmlElement* params = NULL );

   ///
   utilib::SharedPtr<utilib::ioSerialStream> serialstream(int to_rank);

public: // Public so that command functors can hold a reference
   /// 
   struct Data;
private:
   ///
   Data *data;
};


} // namespace colin

#endif // defined colin_ExecuteMngr_h
