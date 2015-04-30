/*  _________________________________________________________________________
 *
 *  UTILIB: A utility library for developing portable C++ codes.
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README file in the top UTILIB directory.
 *  _________________________________________________________________________
 */

#include <utilib/MPIStream.h>
#include <utilib/mpiUtil.h>

#include <iostream>

using std::endl;

//-----------------------------------------------------------------
#ifndef UTILIB_HAVE_MPI

#define OUT std::cout

int test_mpistream(int , char**)
{
   OUT << "MPI support not included in this build." << endl;
   return 0;
}

//-----------------------------------------------------------------
#else // UTILIB_HAVE_MPI

#define OUT std::cerr << "[" << uMPI::rank << "]: "

using namespace utilib;

void master(int verbose)
{
   int i = 0;

   // Test 0: test the loopback device
   MPIStream lo(0);
   lo << 42;
   lo >> i;
   OUT << "Received data from self: " << i << endl;

   // Test 1: receiving data from a client
   MPIStream io(1);
   io >> i;
   OUT << "Received data from client: " << i << endl;

   // Test 2: echoing data through client with bidirectional comms
   double d;
   OUT << "Echoing data (bidirectional): " << endl;
   for (i = 5; i >= 0; i--)
   {
      io << i;
      io >> d;
      OUT << "    " << d << endl;
   }

   // Test 3: echoing data through client with unidirectional comms
   OUT << "Echoing data (unidirectional): " << endl;
   // NB: Forming pairs of unidirectional comms can be dangerous.  If
   // not properly scoped, you can create deadlock when they fall out of
   // scope with both processes waiting on the other to close their
   // output stream first.  Workarounds are to either use the
   // bidirectional comm, or create the input comm first (as most
   // compilers delete local variables in the reverse order created).
   {
      iMPIStream is(1);
      oMPIStream os(1);
      for (i = 5; i >= 0; i--)
         os << i;
      os.flush();

      do {
         is >> d;
         OUT << "    " << d << endl;
      } while ( d - 0.001 > 0 );
   }

   // Test 4: testing send buffer overrun
   int sent = 1;
   int received = 0;
   while ( ! io.dataAvailable() )
   {
      io << 0;
      ++sent;
   }
   io << 1;
   do {
      io >> i;
      ++received;
   } while ( i == 0 );
   OUT << "Buffer overrun test: " 
       << ( sent == received ? "PASS" : "FAIL" ) << endl;
   if ( verbose > 1 )
      OUT << "   (sent " << sent << "; received " << received << ")" << endl;

   // Test 5:  Broadcast
   {
      MPIBroadcastStream bc(0);
      bc << 21 << 42 << 84;
   }
   io >> i;
   OUT << i << endl;
}


void client()
{
   MPIStream io(0);

   // Test 1
   io << 21;

   // Test 2
   int i = 0;
   int j = 0;
   do {
      io >> i;
      j = i * 10;
      io << j;
   } while ( i > 0 );
   io.flush();

   // Test 3
   {
      iMPIStream is(0);
      oMPIStream os(0);
      do {
         is >> i;
         j = i * 10;
         os << j;
      } while ( i > 0 );
   }

   // Test 4
   do {
      io >> i;
      io << i;
   } while ( i == 0 );
   io.flush();
   
   // Test 5
   j = 0;
   MPIBroadcastStream bc(0);
   Any a;
   // NB: the "while ( bc >> a )" form ONLY works if you are reading
   // into an Any or a SerialObject.  All other destination types imply
   // a lexical_cast that will throw an exception if EOF is encountered
   // (because the eof condition returns an empty Any).
   while ( bc >> a )
   {
      TypeManager()->lexical_cast(a, i);
      j += i;
      OUT << i << endl;
   }
   io << j;
}


void slave()
{
   // Test 5
   int i;
   MPIBroadcastStream bc(0);
   Any a;
   // NB: the "while ( bc >> a )" form ONLY works if you are reading
   // into an Any or a SerialObject.  All other destination types imply
   // a lexical_cast that will throw an exception if EOF is encountered
   // (because the eof condition returns an empty Any).
   while ( bc >> a )
   {
      TypeManager()->lexical_cast(a, i);
      OUT << i << endl;
   }
}


int test_mpistream(int argc, char**)
{
   OUT << "MPI support included in this build." << endl;
   
   if ( uMPI::rank == 0 )
   {
      master(argc);
      OUT << "Server DONE." << endl;
   }
   else if ( uMPI::rank == 1 )
   {
      client();
      OUT << "Client DONE." << endl;
   }
   else
   {
      slave();
      OUT << "Slave DONE." << endl;
   }

   return 0;
}

#endif // UTILIB_HAVE_MPI
