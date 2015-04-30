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

/**
 * \file mpiUtil.h
 *
 * Defines the utilib::uMPI class, which provides convient wrappers to 
 * MPI routines.
 *
 * \author Jonathan Eckstein
 */

#ifndef utilib_mpiUtil_H
#define utilib_mpiUtil_H

#include <utilib_config.h>
#ifdef UTILIB_HAVE_MPI
#include <mpi.h>
#endif
#include <utilib/_generic.h>
#include <utilib/exception_mngr.h>

//
// To compile code for logging of all messages.
//

#if defined(UTILIB_HAVE_MPE) || defined(DOXYGEN)

#include <mpe.h>

/// Send a log message
#define LOG_SEND(a,b,c,d)   if (messageLog) logSend(a,b,c,d)
/// Receive a log message
#define LOG_RECV(s)         if (messageLog) logRecv(s)

#else

#define LOG_SEND(a,b,c,d)               // Nothing
#define LOG_RECV(s)                     // Nothing

#endif

namespace utilib {

#if defined(UTILIB_HAVE_MPI)

/**
 * \class uMPI
 *
 *  Using MPI a little easier.
 */
class uMPI 
{
public:

  /// The MPI communicator group used for communication.
  static MPI_Comm comm;

  /// The rank of the current process in \c comm.
  static int rank;

  /// The number of processes in \c comm.
  static int size;

  /**
   * The rank of a single process that can do I/O, guaranteed to 
   * always be the same value across all processors.
   */
  static int ioProc;

  /// A flag that is \c TRUE if the current process can perform I/O.
  static int iDoIO;

  /// The error code from the previous MPI call.
  static int errorCode;

#if defined(UTILIB_HAVE_MPE) || defined(DOXYGEN)

  /// A class parameter for logging
  static bool messageLog;

  /// Log a send event.
  static void logSend(int dest,int tag,int count,MPI_Datatype t);

  /// Log a receive event.
  static void logRecv(MPI_Status* status);

#endif
  
  /// Returns \c TRUE if MPI is already running.
  static int running()
	{
	int running;
	MPI_Initialized(&running);
	return(running);
	}

  /**
   * Takes a buffer of length \p m and expands it to account for 
   * possible wastage due to packing alignment.  This code is very
   * conservative at present.
   */
  static int packSlop(int m)
	{ return 2*m + 32; }

  /// Returns the size of an MPI datatype
  static int sizeOf(MPI_Datatype t);

  /// Initializes MPI.
  static void init(int* argcP,char*** argv, MPI_Comm comm_=MPI_COMM_WORLD);

  /// Initializes MPI with a comm object (assumes that MPI is running)
  static void init(MPI_Comm comm_=MPI_COMM_WORLD);
  
  /// Cleans up and finishes MPI.
  static void done();

  /// Executes a synchronous barrier command.
  static void barrier()
	{
	errorCode = MPI_Barrier(comm);
	if (errorCode)
	   EXCEPTION_MNGR(std::runtime_error, "MPI_Barrier failed, code " 
								<< errorCode);
	}

  /// Executes a parallel reduction.
  static void reduce(void* sendbuf,void* recvbuf,int count,
		     MPI_Datatype datatype,MPI_Op op,int root)
	{
	errorCode = MPI_Reduce(sendbuf,recvbuf,count,datatype,op,root, comm);
      	if (errorCode)
	   EXCEPTION_MNGR(std::runtime_error, "MPI_Reduce failed, code " 
								<< errorCode);
    	}

  /// Computes a parallel sum of integers.
  static int sum(int value,int root = ioProc)
	{
	int result = 0;
	errorCode = MPI_Reduce(&value,&result,1,MPI_INT,MPI_SUM,root, comm);
        if (errorCode)
	   EXCEPTION_MNGR(std::runtime_error, "MPI_Reduce failed, code " 
								<< errorCode);
        return result;
        }

  /// Computes a parallel sum of doubles.
  static double sum(double value,int root = ioProc)
	{
        double result = 0;
        errorCode = MPI_Reduce(&value,&result,1,MPI_DOUBLE,MPI_SUM,root, comm);
        if (errorCode)
	   EXCEPTION_MNGR(std::runtime_error, "MPI_Reduce failed, code " 
								<< errorCode);
        return result;
        }

  /// Computes a parallel maximization of integers.
  static int max(int value,int root = ioProc)
	{
        int result = 0;
        errorCode = MPI_Reduce(&value,&result,1,MPI_INT,MPI_MAX,root, comm);
        if (errorCode)
	   EXCEPTION_MNGR(std::runtime_error, "MPI_Reduce failed, code " 
								<< errorCode);
        return result;
        }

  /// Computes a parallel maximization of doubles.
  static double max(double value,int root = ioProc)
	{
        double result = 0;
        errorCode = MPI_Reduce(&value,&result,1,MPI_DOUBLE,MPI_MAX,root, comm);
        if (errorCode)
	   EXCEPTION_MNGR(std::runtime_error, "MPI_Reduce failed, code "
								<< errorCode);
        return result;
        }

  /// Perform a broadcast.
  static void broadcast(void* buffer,int count,MPI_Datatype datatype,int root)
	{
       	errorCode = MPI_Bcast(buffer,count,datatype,root,comm);
        if (errorCode)
	   EXCEPTION_MNGR(std::runtime_error, "MPI_Broadcast failed, code "
								<< errorCode);
        }

  /// Perform a reduction followed by a broadcast of the result.
  static void reduceCast(void* sendbuf,void* recvbuf,int count,
			 MPI_Datatype datatype,MPI_Op op)
	{
	  errorCode = MPI_Allreduce(sendbuf,recvbuf,count,datatype,op,comm);
	  if (errorCode)
	    EXCEPTION_MNGR(std::runtime_error,"MPI_Allreduce failed, code "
			   << errorCode);
	}
      
  /// Perform MPI_Isend
  static void isend(void* buffer,int count,MPI_Datatype datatype,int dest,
		    int tag,MPI_Request* request)
	{
	LOG_SEND(dest,tag,count,datatype);
	errorCode = MPI_Isend(buffer,count,datatype,dest,tag,comm, request);
	if (errorCode)
	   EXCEPTION_MNGR(std::runtime_error, "MPI_Isend failed, code "
								<< errorCode);
	}

  /**
   * Perform an Isend without a user-supplied request object.
   * This method automatically frees the request object.
   */
  static void isend(void* buffer,int count,MPI_Datatype datatype,int dest,
		    int tag)
	{
	MPI_Request request;
	isend(buffer,count,datatype,dest,tag,&request);
	requestFree(&request);
	}


  /// Perform a Send.
  static void send(void* buffer,int count,MPI_Datatype datatype,int dest,
		   int tag)
	{
	LOG_SEND(dest,tag,count,datatype);
	errorCode = MPI_Send(buffer,count,datatype,dest,tag,comm);
	if (errorCode)
   	   EXCEPTION_MNGR(std::runtime_error, "MPI_Send failed, code "
								<< errorCode);
	}

  /// Perform a Ssend.
  static void ssend(void* buffer,int count,MPI_Datatype datatype,int dest,
		    int tag)
	{
	LOG_SEND(dest,tag,count,datatype);
	errorCode = MPI_Ssend(buffer,count,datatype,dest,tag,comm);
	if (errorCode)
   	   EXCEPTION_MNGR(std::runtime_error, "MPI_Ssend failed, code "
								<< errorCode);
	}

  /// Free a request object.
  static void requestFree(MPI_Request* request)
    	{
      	errorCode = MPI_Request_free(request);
      	if (errorCode)
	   EXCEPTION_MNGR(std::runtime_error, "MPI_Request_free failed, code "
								<< errorCode);
    	}

  /// Perform an Issend.
  static void issend(void* buffer,int count,MPI_Datatype datatype,int dest,
		     int tag,MPI_Request* request)
    	{
      	LOG_SEND(dest,tag,count,datatype);
      	errorCode = MPI_Issend(buffer,count,datatype,dest,tag,comm, request);
      	if (errorCode)
	   EXCEPTION_MNGR(std::runtime_error, "MPI_Issend failed, code "
								<< errorCode);
    	}

  /// Perform an Irecv.
  static void irecv(void* buffer,int count,MPI_Datatype datatype,int source,
		    int tag,MPI_Request* request)
	{
	errorCode = MPI_Irecv(buffer,count,datatype,source,tag,comm, request);
	if (errorCode)
	   EXCEPTION_MNGR(std::runtime_error, "MPI_Irecv failed, code "
								<< errorCode);
	}

  /// Perform a Recv.
  static void recv(void* buffer,int count,MPI_Datatype datatype,int source,
		   int tag,MPI_Status* status)
	{
	errorCode = MPI_Recv(buffer,count,datatype,source,tag,comm, status);
	if (errorCode)
	   EXCEPTION_MNGR(std::runtime_error, "MPI_Recv failed, code "
								<< errorCode);
	LOG_RECV(status);
	}

  /// Get the count from a data type.
  static int getCount(MPI_Status* status,MPI_Datatype datatype)
	{
	int count;
	errorCode = MPI_Get_count(status,datatype,&count);
	if (errorCode)
	   EXCEPTION_MNGR(std::runtime_error, "MPI_Get_count failed, code "
								<< errorCode);
	return count;
	}

  /// Test a request, returning the result in argument \a flag.
  static void test(MPI_Request* request,int* flag,MPI_Status* status)
	{
	errorCode = MPI_Test(request,flag,status);
	if (errorCode)
	   EXCEPTION_MNGR(std::runtime_error, "MPI_Test failed, code "
								<< errorCode);
#ifdef UTILIB_HAVE_MPI
	if (*flag)
	   LOG_RECV(status);
#endif
	}

  /// Test a send request, returning the result in argument \a flag.
  /// Similar to test, but does not attempt logging.
  static void testSend(MPI_Request* request,int* flag,MPI_Status* status)
	{
	errorCode = MPI_Test(request,flag,status);
	if (errorCode)
	   EXCEPTION_MNGR(std::runtime_error, "MPI_Test failed, code "
								<< errorCode);
	}

  /// Test a request, returning the result.
  static int test(MPI_Request* request,MPI_Status* status)
	{
	int flag;
	test(request,&flag,status);
	return flag;
	}

  /// Test a send request, returning the result.  Similar to above,
  /// but will not attempt logging.
  static int testSend(MPI_Request* request,MPI_Status* status)
	{
	int flag;
	testSend(request,&flag,status);
	return flag;
	}

  /// Test a request, returning the result and ignoring the status.
  static int test(MPI_Request* request)
	{
	MPI_Status status;
	return test(request,&status);
	}

  /// Test a send request, returning the result and ignoring the
  /// status.  Similar to previous call, but will not attempt logging.
  static int testSend(MPI_Request* request)
	{
	MPI_Status status;
	return testSend(request,&status);
	}

  /// Call Testsome.
  static int testsome(int incount, MPI_Request* request_array,
		int& outcount, int* array_of_indeces, MPI_Status* status_array)
	{
	errorCode = MPI_Testsome(incount,request_array,&outcount,
				array_of_indeces,status_array);
	if (errorCode != MPI_SUCCESS)
	   EXCEPTION_MNGR(std::runtime_error, "MPI_Testsome failed, code "
								<< errorCode);
#ifdef UTILIB_HAVE_MPI
	for (int i=0; i<outcount; i++)
	  LOG_RECV(status_array+i);
#endif
	return (outcount > 0);
	}

  /// Cancel a request.
  static void cancel(MPI_Request* request)
	{
	errorCode = MPI_Cancel(request);
	if (errorCode)
	   EXCEPTION_MNGR(std::runtime_error, "MPI_Cancel failed, code "
								<< errorCode);
    	}

  /// Wait on a request.
  static void wait(MPI_Request* request,MPI_Status* status,bool killing=false)
	{
	errorCode = MPI_Wait(request,status);
	if (errorCode)
	EXCEPTION_MNGR(std::runtime_error, 
		       "MPI_Wait failed, code " << errorCode);
	if (!killing)
	  {
	    LOG_RECV(status);
	  }
	}

  /// Cancel send requests.
  static void killSendRequest(MPI_Request* request);

  /// Cancel receive requests.
  static void killRecvRequest(MPI_Request* request);

};


#else


//  If no MPI, don't define anything except running(), which returns
//  FALSE.  Also define the rank to be zero.

class uMPI 
{
public:
  static int running() { return FALSE; };

  static int rank;
};


#endif

} // namespace utilib 

#endif
