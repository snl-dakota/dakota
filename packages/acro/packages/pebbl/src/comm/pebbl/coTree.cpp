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
//
// coTree.cpp
//
// This class defines a skeleton for a co-routine that asynchronously
// pursues a distributed computation involving messages moving up
// and down an arbitrary tree.  The tree topology is specified using
// the treeTopology class.
//
// Jonathan Eckstein
//

#include <acro_config.h>
#ifdef ACRO_HAVE_MPI

#include <pebbl/coTree.h>

using namespace std;

namespace pebbl {


coTree::coTree(MPI_Datatype  datatype_,
	       MPI_Request*  request_,
	       MPI_Status*   status_,
	       treeTopology* t_,
	       int           exitForReceives_) :

	       t(t_),
	       datatype(datatype_),
	       request(request_),
	       status(status_),
	       exitForReceives(exitForReceives_),
	       messagesReceived(0)
{
  reset();
};


void coTree::receiveOperation(void* buffer,int size,MessageID& tag,int src)
{
  DEBUGPR(300,ucout << "coTree receive: size=" << size << " src=" << src 
	  << " tag=" << tag << endl);
  uMPI::irecv(buffer,size,datatype,src,tag,request);
};

void coTree::sendOperation(void* buffer,int size,MessageID& tag,int dst)
{
  DEBUGPR(300,ucout << "coTree send: size=" << size << " dst=" << dst
	  << " tag=" << tag << endl);
  uMPI::isend(buffer,size,datatype,dst,tag);
};


int coTree::run()
{
  int canContinue;

#define jumpState(newState) { state = newState; canContinue = TRUE; break; }

  do 

    {

      canContinue = FALSE;

      switch(state) {

      case init:

	DEBUGPR(200,ucout << "coTree starting.\n");
	t->resetChildren();
	initAction();
	state = startWait;

      case startWait:

	if (!readyToStart())
	  break;
	DEBUGPR(300,ucout << "Confirmed coTree ready to start.\n");
	state = upLoop;

      case upLoop:

	DEBUGPR(300,ucout << "coTree up loop.\n");
	if (t->childrenLeft() == 0)
	  jumpState(upRelay);
	DEBUGPR(300,ucout << "Want to receive up message.\n");
	receiveOperation(upRecvBuf(),
			 upRecvBufSize(),
			 upTag,
			 MPI_ANY_SOURCE);
	state = upWait;
	if (exitForReceives)
	  break;

      case upWait:

	if (!requestMet())
	  {
	    upWaitAction();
	    break;
	  }
	messagesReceived++;
	DEBUGPR(200,ucout << "Received message from " 
		<< status->MPI_SOURCE << ", tag = " 
		<< status->MPI_TAG << '\n');
	if (!(t->validChild(status->MPI_SOURCE)))
	   EXCEPTION_MNGR(runtime_error, 
			  "Received upward tree communication from invalid"
			  " child " << status->MPI_SOURCE);
	upMessageAction();
	t->operator++(0);		// (*t)++ doesn't work w/ SUNOS CC
	jumpState(upLoop);

      case upRelay:

	DEBUGPR(300,ucout << "coTree up relay.\n");
	upRelayAction();
	if (t->isRoot())
	  {
	    DEBUGPR(300,ucout << "coTree root activation.\n");
	    rootAction();
	    jumpState(downRelay);
	  }
	DEBUGPR(300,ucout << "Sending to parent = " << t->parent() << " \n");
	sendOperation(upSendBuf(),
		      upSendBufSize(),
		      upTag,
		      t->parent());
	upToDownAction();
	receiveOperation(downRecvBuf(),
			 downRecvBufSize(),
			 downTag,
			 t->parent());
	state = downWait;
	if (exitForReceives)
	  break;

      case downWait:

	if (!requestMet())
	  {
	    downWaitAction();
	    break;
	  }
	messagesReceived++;
	DEBUGPR(200,ucout << "Received message from " 
		<< status->MPI_SOURCE << ", tag = " 
		<< status->MPI_TAG << '\n');
	downReceiveAction();
	state = downRelay;

      case downRelay:

	DEBUGPR(300,ucout << "coTree down relay.\n");
	t->resetChildren();
	downRelayAction();
	{
	  int i = 0;
	  while(t->childrenLeft() > 0)
	    {
	      downRelayLoopAction(i);
	      DEBUGPR(300,ucout << "Send operation " << i << ".\n");
	      sendOperation(downSendBuf(i),
			    downSendBufSize(i),
			    downTag,
			    t->operator++(0)); // (*t)++ fails w/SUNOS CC
	      i++;
	    }
	}
	state = done;

      case done:

	DEBUGPR(300,ucout << "coTree done.\n");
	doneAction();
	
      }
      
    }
  
  while(canContinue);

  return (state != done);

}

} // namespace pebbl 

#endif
