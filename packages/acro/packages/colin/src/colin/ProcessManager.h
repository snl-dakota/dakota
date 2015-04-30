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
 * \file ProcessMngr.h
 *
 * Function wrapper for managing COLIN execution handlers.
 */

#ifndef colin_ProcessMngr_h
#define colin_ProcessMngr_h

#include <acro_config.h>

#include <utilib/SharedPtr.h>
//#include <utilib/std_headers.h>

class TiXmlElement;
namespace utilib {
class ioSerialStream;
};

namespace colin
{

class ProcessManager
{
public:
   enum ClientServer_t { client, server };

public:
   ///
   virtual ~ProcessManager() {}

   ///
   virtual int rank() const = 0;

   ///
   virtual int num_ranks() const = 0;

   ///
   virtual void route_command( const std::string& command, 
                               TiXmlElement* params,
                               int rank ) = 0;

   ///
   virtual void recv_command() = 0;

   /// Return serializer to the specified rank (local rank returns a loopback)
   virtual 
   utilib::SharedPtr<utilib::ioSerialStream> serializer(int to_rank) = 0;

   ///
   virtual void ping(int to_rank) = 0;
};


} // namespace colin

#endif // defined colin_ProcessMngr_h
