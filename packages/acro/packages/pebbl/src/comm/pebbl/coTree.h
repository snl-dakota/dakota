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
 * \file coTree.h
 *
 * Defines the pebbl::coTree class.
 */

#ifndef pebbl_coTree_h
#define pebbl_coTree_h

#include <acro_config.h>
#ifdef ACRO_HAVE_MPI

#include <utilib/CommonIO.h>
#include <utilib/mpiUtil.h>
#include <utilib/BasicArray.h>
#include <pebbl/MessageID.h>
#include <pebbl/treeTopology.h>
#include <pebbl/fundamentals.h>

namespace pebbl {

using utilib::CommonIO;


///
/// This class defines a skeleton for a co-routine that asynchronously
/// pursues a distributed computation involving messages moving up
/// and down an arbitrary tree.  The tree topology is specified using
/// the treeTopology class.
///
class coTree : public CommonIO
{

protected:

  // Basic state information and topology pointer.

  enum coTreeState 
    { 
    init,
    startWait,
    upLoop,
    upWait,
    upRelay,
    downWait,
    downRelay,
    done
    };
  
  coTreeState state;

  treeTopology* t;

  // What to pass MPI when sending/receiving messages.
  // Can be altered during execution by user-supplied methods.

  MessageID     upTag;            // Tag for "up" messages.
  MessageID     downTag;          // Tag for "down" messages.
  MPI_Datatype  datatype;
  MPI_Request*  request;
  MPI_Status*   status;

  int    exitForReceives;        // Flag to force exit before receiving.

  // To create a tree computation, you just override some subset
  // of the the following methods:

  virtual void initAction()          { };
  virtual bool readyToStart()        { return true; };
  virtual void upMessageAction()     { };
  virtual void upWaitAction()        { };
  virtual void upRelayAction()       { };
  virtual void upToDownAction()      { };
  virtual void rootAction()          { upToDownAction(); };
  virtual void downWaitAction()      { };
  virtual void downReceiveAction()   { };
  virtual void downRelayAction()     { };
  virtual void doneAction()          { };

  virtual void downRelayLoopAction(int i) { };

  virtual void receiveOperation(void*      buffer, 
				int        size, 
				MessageID& tag,
				int        src);

  virtual void sendOperation(void*      buffer, 
			     int        size, 
			     MessageID& tag,
			     int        dst);

  virtual void* upSendBuf()        = 0;
  virtual void* upRecvBuf()        = 0;
  virtual void* downSendBuf(int i) = 0;
  virtual void* downRecvBuf()      = 0;

  virtual int upSendBufSize()        { return 1; };
  virtual int upRecvBufSize()        { return 1; };
  virtual int downSendBufSize(int i) { return 1; };
  virtual int downRecvBufSize()      { return 1; };

public:

  virtual void reset() { state = init; };

  int messagesReceived;

  // This method runs the co-routine.  It returns true iff the computation
  // still needs to run.

  int run();

  // Constructor.

  coTree(MPI_Datatype  datatype_,
	 MPI_Request*  request_,
	 MPI_Status*   status_,
	 treeTopology* t_,
	 int           exitForReceives_);

  // Destructor.

  virtual ~coTree()
    {
      if ((state != done) && (state != init))
	EXCEPTION_MNGR(std::runtime_error,"Attempt to kill an incomplete coTree");
    };

private:
  
  bool requestMet()
    {
      if (*request == MPI_REQUEST_NULL)
	return true;
      int flag;
      uMPI::test(request,&flag,status);
      return flag;
    };

};

} // namespace pebbl

#endif

#endif
