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

#include <acro_config.h>

#ifdef ACRO_HAVE_MPI

#include <colin/ProcessManager.h>
#include <colin/ExecuteMngr.h>

#include <utilib/MPIStream.h>

#include <mpi.h>
#include <tinyxml/tinyxml.h>

using std::string;

using utilib::SharedPtr;
using utilib::ioSerialStream;
using utilib::MPIStream;

namespace colin {


//------------------------------------------------------------------
// ProcessManager_MPI
//------------------------------------------------------------------


class ProcessManager_MPI : public ProcessManager
{
public:
   ProcessManager_MPI(MPI_Comm main_comm)
      : comm_mngr(main_comm),
        comm(MPI_COMM_WORLD), 
        buf(new char[ExecuteManager::max_command_length + 2]),
        echo_pending(0),
        echo_waiting4me(false)
   {
      int ans = MPI_Comm_dup(main_comm, &comm);
      if ( ans != 0 )
         EXCEPTION_MNGR(std::runtime_error, "ProcessManager_MPI(): error "
                        "duplicating MPI communicator (Error=" << ans << ")");
   }

   virtual ~ProcessManager_MPI()
   {
      delete [] buf;
   }


   virtual int rank() const
   {
      int rank_id = 0;
      MPI_Comm_rank(comm, &rank_id);
      return rank_id;
   }


   virtual int num_ranks() const
   {
      int ranks = 0;
      MPI_Comm_size(comm, &ranks);
      return ranks;
   }


   virtual void route_command( const std::string& command, 
                               TiXmlElement* params, int to_rank )
   {
      assert( command.size() <= ExecuteManager::max_command_length );

      buf[0] = ( params == NULL ? 0 : parameter_flag );

      strncpy(buf+1, command.c_str(), ExecuteManager::max_command_length);
      buf[1 + ExecuteManager::max_command_length] = 0;

      // 1) Route to self?
      if ( to_rank == ExecuteManager::local )
         to_rank = rank();

      if ( to_rank == ExecuteManager::broadcast )
      {
         // 2) broadcast commands
         buf[0] |= broadcast_flag;

         int my_rank = rank();
         int max = 0;
         MPI_Comm_size(comm, &max);
         for (int to = 0; to < max; ++to)
         {
            if ( to == my_rank )
               continue;
            int ans = MPI_Send( buf, ExecuteManager::max_command_length + 2, 
                                MPI_BYTE, to, command_tag, comm );
            if ( ans != MPI_SUCCESS )
               EXCEPTION_MNGR(std::runtime_error, "ProcessManager_MPI::"
                              "route_command(): error sending command "
                              "(Error=" << ans << ")");
         }
      }
      else
      {
         // 3) standard point-to-point commands
         if ( to_rank < 0 )
            EXCEPTION_MNGR( std::runtime_error, "ProcessManager_MPI::"
                            "route_command(): invalid destination rank: "
                            << to_rank );

         int ans = MPI_Send( buf, ExecuteManager::max_command_length + 2, 
                             MPI_BYTE, to_rank, command_tag, comm );
         if ( ans != MPI_SUCCESS )
            EXCEPTION_MNGR(std::runtime_error, "ProcessManager_MPI::"
                           "route_command(): error sending command "
                           "(Error=" << ans << ")");
      }

      // (all) send the parameters, if present
      if ( params != NULL )
      {
         std::ostringstream param_buf;
         param_buf << (*params);
         if ( to_rank == ExecuteManager::broadcast )
         {
            utilib::MPIBroadcastStream bs(rank(), comm_mngr);
            bs << param_buf.str();
         }
         else
         {
            utilib::oMPIStream os(to_rank, comm_mngr);
            os << param_buf.str();
         }
      }
   }


   virtual void recv_command()
   {
      MPI_Status status;
      int ans = MPI_Recv(buf, ExecuteManager::max_command_length + 2, 
                         MPI_BYTE, MPI_ANY_SOURCE, command_tag, comm, &status);
      if ( ans != MPI_SUCCESS )
         EXCEPTION_MNGR(std::runtime_error, "ProcessManager_MPI::"
                        "recv_command(): error sending command "
                        "(Error=" << ans << ")");
      int count = 0;
      ans = MPI_Get_count(&status, MPI_BYTE, &count);
      if (( count != static_cast<int>(ExecuteManager::max_command_length) + 2 )
          || ( ans != MPI_SUCCESS ))
         EXCEPTION_MNGR(std::runtime_error, "ProcessManager_MPI::"
                        "recv_command(): error verifying command size "
                        "(Error=" << ans << ", size=" << count << ")");

      if ( strcmp(buf+1, "ECHO") == 0 )
      {
         if ( status.MPI_SOURCE == rank() )
         {
            if (( ! echo_waiting4me ) || ( echo_pending == 0 ))
               EXCEPTION_MNGR(std::runtime_error, "ProcessManager_MPI::"
                              "recv_command(): received an ECHO from myself "
                              "that I don't recall sending.");
            --echo_pending;
            echo_waiting4me = false;
         }
         else
            MPI_Send( NULL, 0, MPI_BYTE, status.MPI_SOURCE, echo_tag, comm );
         return;
      }

      if ( buf[0] & parameter_flag )
      {
         string param_str;
         if ( buf[0] & broadcast_flag )
         {
            utilib::MPIBroadcastStream bs(status.MPI_SOURCE, comm_mngr);
            bs >> param_str;
         }
         else
         {
            utilib::iMPIStream is(status.MPI_SOURCE, comm_mngr);
            is >> param_str;
         }

         TiXmlDocument params;
         std::istringstream param_buf(param_str);
         param_buf >> params;
         ExecuteMngr().run_command(buf+1, ExecuteManager::local, 
                                   params.RootElement());
      }
      else
         ExecuteMngr().run_command(buf+1, ExecuteManager::local, NULL);
   }


   virtual utilib::SharedPtr<utilib::ioSerialStream> serializer(int rank)
   {
      return SharedPtr<ioSerialStream>(new MPIStream(rank, comm_mngr));
   }


   virtual void ping(int to_rank)
   {
      if ( echo_pending > 0 )
         EXCEPTION_MNGR(std::runtime_error, "ProcessManager_MPI::ping(): "
                        "Starting a ping before the previous one finished???");

      echo_pending = 1;
      if ( to_rank == ExecuteManager::broadcast )
         MPI_Comm_size(comm, &echo_pending);
      echo_waiting4me = (( to_rank == ExecuteManager::broadcast ) || 
                         ( to_rank == ExecuteManager::local ) || 
                         ( to_rank == rank() ));

      route_command("ECHO", NULL, to_rank);
      MPI_Request request;
      MPI_Status  status;
      MPI_Irecv( NULL, 0, MPI_BYTE, MPI_ANY_SOURCE, echo_tag, comm, &request );
      int flag = false;
      while ( echo_pending > 0 )
      {
         MPI_Test(&request, &flag, &status);
         if ( flag )
         {
            --echo_pending;
            MPI_Irecv( NULL, 0, MPI_BYTE, MPI_ANY_SOURCE, echo_tag, 
                       comm, &request );
         }
         if ( echo_waiting4me )
            irecv_command();
      }
      MPI_Cancel(&request);
   }


   bool irecv_command()
   {
      MPI_Status status;
      int flag;
      int ans = MPI_Iprobe(MPI_ANY_SOURCE, command_tag, comm, &flag, &status);
      if ( ans != MPI_SUCCESS )
         EXCEPTION_MNGR(std::runtime_error, "ProcessManager_MPI::"
                        "irecv_command(): error probing for command "
                        "(Error=" << ans << ")");
      if ( flag )
         recv_command();
      return flag != 0;
   }

   static ProcessManager* create()
   { return new ProcessManager_MPI(MPI_COMM_WORLD); }

private:
   utilib::MPICommManager  comm_mngr;

   MPI_Comm   comm;
   char      *buf;

   int   echo_pending;
   bool  echo_waiting4me;

   static const char broadcast_flag = 1;
   static const char parameter_flag = 2;
   static const int  command_tag    = 1;
   static const int  echo_tag       = 2;
};

namespace StaticInitializers {
namespace {
bool RegisterMPI()
{
   ExecuteMngr().define_process_manager("mpi", ProcessManager_MPI::create);
   return true;
}

} // namespace colin::StaticInitializers::(local)

extern const volatile bool mpi_process_mngr = RegisterMPI();

} // namespace colin::StaticInitializers
} // namespace colin


#else
// Define a fake registration if Acro is configured without MPI support
namespace colin {
namespace StaticInitializers {
extern const volatile bool mpi_process_mngr = false;
} // namespace colin::StaticInitializers
} // namespace colin
#endif // defined(ACRO_HAVE_MPI)
