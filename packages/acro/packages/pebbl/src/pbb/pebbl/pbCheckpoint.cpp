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
// pbCheckpoint.cpp
//
// Parallel branching class code for PEBBL -- methods relating specifically
// to checkpoints
//
// Jonathan Eckstein
//


#include <acro_config.h>
#ifdef ACRO_HAVE_MPI

#include <sys/types.h>
#include <dirent.h>

#include <utilib/CharString.h>
#include <utilib/mpiUtil.h>
#include <utilib/nicePrint.h>
#include <pebbl/parBranching.h>
#include <pebbl/gRandom.h>

using namespace std;
using namespace utilib;

namespace pebbl {

  // Does a little bookkeeping to set the time of a possible checkpoint abort.

  void parallelBranching::prepareCPAbort()
  {
    cpAbortNum = 0;
    if (abortCheckpointCount > 0)
      cpAbortNum = checkpointNumber + abortCheckpointCount;
    if ((abortCheckpoint > checkpointNumber) &&
	((abortCheckpoint < cpAbortNum) || (cpAbortNum == 0)))
      cpAbortNum = abortCheckpoint;
  }
  

  // This declares that a checkpoint has started.  If on a hub, tell workers
  // too.

  void parallelBranching::setupCheckpoint()
  {
    checkpointStartTime = WallClockSeconds();
    checkpointNumber++;
    if (uMPI::rank == uMPI::ioProc)
      ucout << "Starting checkpoint " << checkpointNumber
	    << " at " << checkpointStartTime << " seconds.\n";
    DEBUGPR(2,ucout << "Setting up checkpoint " << checkpointNumber << endl);
    checkpointing = true;
    if (iAmHub())
      alertWorkers(startCheckpointSignal);
  }


  // This actually writes a checkpoint

  void parallelBranching::writeCheckpoint()
  {
    LOG_EVENT(1,start,checkpointLogState);

    writingCheckpoint = true;

    if (iAmHub())
      alertWorkers(writeCheckpointSignal);

    ofstream bstream(checkpointFilename(checkpointNumber,uMPI::rank).c_str(),
		     (ios::out | ios::binary));

    PackBuffer cpBuf;

    // Write global data

    cpBuf << incumbentValue << incumbentSource << probCounter;
    if (uMPI::rank == incumbentSource)
      incumbent->pack(cpBuf);
    cpBuf.writeBinary(bstream);

    // Write application-specific data

    appCheckpointWrite(cpBuf);
    cpBuf.writeBinary(bstream);

    // Write all worker pool and server pool subproblems

    if (iAmWorker())
      {
	int wCount = workerPool->size();
	int sCount = serverPool.size();
	int tCount = wCount + sCount;
	bstream.write((char *) &tCount,sizeof(int));
	if (cpDebugCount > 0)
	  ucout << "Writing " << tCount << " subproblems\n";
	workerPool->resetScan();
	for(int i=0; i<wCount; i++)
	  {
	    parallelBranchSub* sp = workerPool->scan();
	    sp->packProblem(cpBuf);
	    if (i < cpDebugCount)
	      ucout << "Writing " << cpBuf.curr() << " bytes for " 
		    << sp << endl;
	    cpBuf.writeBinary(bstream);
	  }
	serverPool.resetScan();
	for(int i=0; i < sCount; i++)
	  {
	    parallelBranchSub* sp = serverPool.scan();
	    sp->packProblem(cpBuf);
	    if (wCount+i < cpDebugCount)
	      ucout << "Writing " << cpBuf.curr() << " bytes for " 
		    << sp << endl;
	    cpBuf.writeBinary(bstream);
	  }
      }
    else   // If not a worker, just say there were 0 subproblems
      {
	int dummy = 0;
	bstream.write((char *) &dummy,sizeof(int));
      }

    // In case we are enumerating, write the number of solutions in
    // the repository, and all solutions in the repository.  If we are
    // not enumerating, we just write a single integer "0".

    int rsize = 0;

    if (enumerating)
      rsize = repositorySize();

    if (cpDebugCount > 0)
      ucout << "Repository size " << rsize << endl;

    bstream.write((char *) &rsize,sizeof(int));

    for(int s=1; s<=rsize; s++)   // Loop 1..rsize because we have a heap
      {
	reposHeap.member(s)->key().pack(cpBuf);
	if (s <= cpDebugCount)
	  ucout << "Writing " << cpBuf.curr() << " bytes for solution "
		<< &(reposHeap.member(s)->key()) << endl;
	cpBuf.writeBinary(bstream);
      }

    // Done with writing the checkpoint!

    bstream.close();
    uMPI::barrier();

    if (checkpointNumber > 1)
      deleteCheckpointFile(checkpointNumber-1,uMPI::rank);
  
    double cpEndTime = WallClockSeconds();
    double cpTime    = cpEndTime - checkpointStartTime;

    checkpointTriggerTime = max(checkpointStartTime + checkpointMinutes*60,
				cpEndTime + checkpointMinInterval*60);
  
    checkpointTotalTime += cpTime;

    if (uMPI::rank == uMPI::ioProc)
      {
	statusLine(globalLoad,"c");
	ucout << "Checkpoint " << checkpointNumber 
	      << " done -- took " << cpTime << " seconds.\n";
      }

    writingCheckpoint = false;
    checkpointing     = false;

    LOG_EVENT(1,end,checkpointLogState);

    // If we have to abort now, do it.

    if (checkpointNumber == cpAbortNum)
      {
	// Say we're aborting and write a "flag" file

	if (uMPI::rank == uMPI::ioProc)
	  {
	    ucout << "Aborting at checkpoint " << checkpointNumber 
		  << endl << Flush;
	    ofstream flagStream("pebbl-cp-abort.flag",ios::out);
	    flagStream << "PEBBL run for '" << problemName
		       << "' aborted at checkpoint " << checkpointNumber
		       << endl << "Wall clock time was " << cpEndTime
		       << " seconds, and there were " << globalLoad.count()
		       << " subproblems\n";
	  }

	cleanAbort();
	
      }
    
  }


  string parallelBranching::checkpointFilename(int k, int p)
  {
    stringstream s;
    s << checkpointDir;
    int n = checkpointDir.length();
    if ((n > 0) && (checkpointDir[n-1] != '/'))
      s << '/';
    s << problemName << ".cp" << k << ".p" << p << ".bdat";
    return s.str();
  }


  void parallelBranching::deleteCheckpointFile(int k, int p)
  {
    int code = remove(checkpointFilename(k,p).c_str());
    DEBUGPR(1000,
            if (code) {
               ucerr << "Could not remove checkpoint file (" << k 
                     << "," << p << ")" << endl;
            });
  }


  //  Try to match a substring at position 'cursor'.  If it matches
  //  return true and advance 'cursor' to matach.

  bool parallelBranching::stringMatch(string& str,
				      int& cursor,
				      const char* pattern)
  {
    size_type sublen = strlen(pattern);
    if (str.substr(cursor,sublen) != pattern)
      return false;
    cursor += sublen;
    return true;
  }


  //  Read an unsigned integer of at most 9 digits starting from
  //  position "cursor" of a string, and place the result in "result"
  //  Return true if it worked.

  bool parallelBranching::intFromString(string& s,int& cursor,int& result)
  {
    result = 0;
    int i = cursor;
    int len = min((int) s.length(),cursor+9);
    for(; i<len; i++) 
      {
	int d = ((int) s[i]) - '0';
	if ((d < 0) || (d > 9))
	  break;
	result = 10*result + d;
      }
    if (i == cursor)
      return false;
    cursor = i;
    return true;
  }


  //  Identify which checkpoint we should be reading and set 
  //  "checkpointNumber".  Return the number of processors found
  //  and the also set the highest processor number found.  
  //  The last argument specifies if we should look for a particular
  //  processor's files (=MPI_ANY_SOURCE if all processors)

  int parallelBranching::scanForCheckpointFiles(int processor)
  {
    string scanDir = checkpointDir;
    if (scanDir.size() == 0)
      scanDir = ".";

    DIR* dirHandle = opendir(scanDir.c_str());
    struct dirent* fileHandle;

    int highestProcFound = -1;
    int numFound = 0;

    while((fileHandle = readdir(dirHandle)))
      {
	string filename(fileHandle->d_name);
	int k = 0;
	int p = -1;
	DEBUGPR(90,ucout << "Scanning file " << filename << endl);

	if (checkpointFileMatch(filename,k,p))
	  {
	    DEBUGPR(90,ucout << "Matched: k=" << k << " p=" << p << endl);
	    if (k == 0)
	       EXCEPTION_MNGR(runtime_error, 
			      "0 is not a valid checkpoint number in file "
			      << filename);
	    if (checkpointNumber == 0)
	      checkpointNumber = k;
	    else if (checkpointNumber != k)
	      EXCEPTION_MNGR(runtime_error, 
			     "Found files for two checkpoints: " 
			     << checkpointNumber << " and " << k);

	    if ((processor == MPI_ANY_SOURCE) || (p == processor))
	      {
		DEBUGPR(90,ucout << "Match for this processor\n");
		numFound++;
		if (p > highestProcFound)
		  highestProcFound = p;
	      }

	    if ((processor != MPI_ANY_SOURCE) && (p >= uMPI::size))
	       EXCEPTION_MNGR(runtime_error, 
			      "Processor number " << p << " too high in file " 
			      << filename);
	  }
      }
	    
    closedir(dirHandle);

    if (processor == MPI_ANY_SOURCE)
      {
	if (highestProcFound != numFound - 1)
	   EXCEPTION_MNGR(runtime_error, "Found " << numFound << 
			" files for checkpoint " << checkpointNumber << 
			", but higest processor is " << highestProcFound << 
                          ", not " << (numFound-1));
      }

    return numFound;
  }


  // This tries to parse individual file names to see if they
  // match the checkpoint standard form.  If so, it sets k and p
  // accordingly.

  bool parallelBranching::checkpointFileMatch(string& filename,int& k,int& p)
  {
    int cursor = problemName.size();
    if (filename.substr(0,cursor) != problemName)
      return false;
    if (!stringMatch(filename,cursor,".cp"))
      return false;
    if (!intFromString(filename,cursor,k))
      return false;
    if (!stringMatch(filename,cursor,".p"))
      return false;
    if (!intFromString(filename,cursor,p))
      return false;
    if (!stringMatch(filename,cursor,".bdat"))
      return false;
    return ((size_type) cursor == filename.size());
  }


  // Method to implement restarts from checkpoints.
  // Note that time, messages, pool, and bounds are 
  // stored in the same variables as for ramp-up.

  bool parallelBranching::restartFromCheckpoint()
  {
    DEBUGPR(10,ucout << "restartFromCheckpoint called\n");

    double startRampUpTime = CPUSeconds();

    bool ableToRead = false;

    if (reconfigure)
      ableToRead = reconfigureRestart();
    else 
      ableToRead = parallelRestart();

    if (!ableToRead)
      return false;

    restartCPNum = checkpointNumber;   // Remember starting checkpoint number

    // Deal with any side effects from an incumbent that might have
    // been loaded with the checkpoint.  Set the pools to think they've
    // been pruned at least once (so --initialDive will work properly
    // and report aggregate bounds).

    if (abs(incumbentValue) != MAXDOUBLE)
      {
	newIncumbentEffect(incumbentValue);
	if (iAmHub())
	  {
	    hubPool->pretendPrunedOnce();
	    for(int w=0; w<numWorkers(); w++)
	      workerTransitPool[w].pretendPrunedOnce();
	  }
	if (iAmWorker())
	  {
	    workerPool->pretendPrunedOnce();
	    serverPool.pretendPrunedOnce();
	  }
      }

    // Compute the load, print a status line, remember some
    // statistics, and return.  If enumCount is active, make sure all
    // the solution ID arrays are set correctly and the lastSolId
    // cutoff is set.

    restartSetLoads();

    if (enumCount > 1)
      syncLastSol();

    if (uMPI::rank == uMPI::ioProc)
      statusLine(globalLoad,"l");
    
    rampUpBounds = 0;
    rampUpPool   = globalLoad.count();
    rampUpTime   = CPUSeconds() - startRampUpTime;

    return true;
  }
  

  bool parallelBranching::parallelRestart()
  {
    DEBUGPR(2,ucout << "parallelRestart called\n");

    // Find which checkpoint we're going to start from, and check
    // for various errors.  Note: this sets checkPointNumber

    int filesFound = scanForCheckpointFiles(uMPI::rank);

    // Check how many files were found.

    int minF = 0;
    int maxF = 0;
    uMPI::reduceCast(&filesFound,&minF,1,MPI_INT,MPI_MIN);
    uMPI::reduceCast(&filesFound,&maxF,1,MPI_INT,MPI_MAX);
    DEBUGPR(10,ucout << "filesFound=" << filesFound 
	    << " minF=" << minF << " maxF=" << maxF << endl);
    if (uMPI::rank == 0)
      {
	if (minF != maxF)
	   EXCEPTION_MNGR(runtime_error, "Found " << minF << 
			" checkpoint file" << plural(minF) << 
			" for some processors, and " << maxF <<
                          "file" << plural(maxF) << " for others");
      }
    else
      rampUpMessages += 4;

    // If there are no files, return false, meaning that nothing happened.

    if (minF == 0) 
      return false;
    
    // Make sure that we agree on which checkpoint we're starting from

    int minK = 0;
    int maxK = 0;
    uMPI::reduce(&checkpointNumber,&minK,1,MPI_INT,MPI_MIN,0);
    uMPI::reduce(&checkpointNumber,&maxK,1,MPI_INT,MPI_MAX,0);
    DEBUGPR(10,ucout << "checkpointNumber=" << checkpointNumber
	    << " minK=" << minK << " maxK=" << maxK << endl);
   if (uMPI::rank == 0)
      {
	if (minK != maxK)
	   EXCEPTION_MNGR(runtime_error, "Found files for two checkpoints: "
                          << minK << " and " << maxK);
      }
    else
      rampUpMessages += 2;

    // Things look good -- read the file!

    ifstream bstream(checkpointFilename(checkpointNumber,uMPI::rank).c_str(),
		     (ios::in | ios::binary));

    UnPackBuffer cpBuf;

    // Read global information

    cpBuf.readBinary(bstream);
    cpBuf >> incumbentValue >> incumbentSource >> probCounter;
    if (uMPI::rank == incumbentSource)
      incumbent = unpackSolution(cpBuf);
    DEBUGPR(10,ucout << "incumbentValue=" << incumbentValue
	    << " incumbentSource=" << incumbentSource << endl);

    // Read application information

    cpBuf.readBinary(bstream);
    appCheckpointRead(cpBuf);

    // If a worker, read all subproblems for this processor

    int numSPs = -1;
    bstream.read((char *) &numSPs,sizeof(int));
    DEBUGPR(10,ucout << "Reading " << numSPs << " subproblems...\n");

    if (iAmWorker())
      {
	for (int i=0; i<numSPs; i++)
	  {
	    cpBuf.readBinary(bstream);
	    if (i < cpDebugCount)
	      ucout << "Got " << cpBuf.message_length() << " bytes\n";
	    parallelBranchSub* sp = blankParallelSub();
	    sp->unpackProblem(cpBuf);
	    if (i < cpDebugCount)
	      ucout << "Problem looks like " << sp << endl;
	    if (sp->canFathom())
	      sp->recycle();
	    else
	      workerPool->insert(sp);
	  }
      }
    else if (numSPs > 0)
      EXCEPTION_MNGR(runtime_error, "Checkpoint has " << numSPs << 
		     " subproblems for non-worker processor " <<
		     uMPI::rank);

    // If enumerating, read and store all solutions stored on this
    // processor.

    if (enumerating)
      {
	int rsize = -1;
	bstream.read((char *) &rsize,sizeof(int));
	if (cpDebugCount > 0)
	  ucout << "Repository size " << rsize << endl;
	for(int s=0; s<rsize; s++)
	  {
	    cpBuf.readBinary(bstream);
	    solution* sol = unpackSolution(cpBuf);
	    if (s < cpDebugCount)
	      ucout << "Got solution " << sol << endl;
	    parallelSolCounter = max(parallelSolCounter,sol->serial);
	    localReposOffer(sol);
	  }
      }

    // Done reading!

    bstream.close();
    uMPI::barrier();
    if (uMPI::rank != 0)
      rampUpMessages += 2;

    DEBUGPR(2,ucout << "parallelRestart done\n");

    // Return true to say that we read something

    return true;
  }


  // Do a serial read-in of the checkpoint files and distribute the
  // the contents evenly to all workers.

  bool parallelBranching::reconfigureRestart()
  {
    DEBUGPR(2,ucout << "reconfigureRestart called\n");

    // Figure out how many files we have; if none, bail out.

    int filesFound = 0;

    if (uMPI::rank == uMPI::ioProc)
      filesFound = scanForCheckpointFiles(MPI_ANY_SOURCE);
    else
      rampUpMessages++;

    uMPI::broadcast(&filesFound,1,MPI_INT,uMPI::ioProc);

    DEBUGPR(10,ucout << "checkpointNumber=" << checkpointNumber
	    << " filesFound=" << filesFound << endl);

    if (filesFound == 0)
      return false;

    uMPI::broadcast(&checkpointNumber,1,MPI_INT,uMPI::ioProc);
    rampUpMessages += (uMPI::rank != uMPI::ioProc);

    // OK, we have some non-zero number of files. The I/O processor
    // reads these and sends out the work.  The incumbent always
    // resides on the reading processor.

    incumbentSource = uMPI::ioProc;

    if (uMPI::rank == uMPI::ioProc)
      reconfigureRestartRoot(filesFound);
    else 
      reconfigureRestartLeaf(filesFound);

    // All processors start numbering subproblems from the largest
    // value of all prior counters.  This will prevent overlaps
    // and complaints from logAnalyze.

    uMPI::broadcast(&probCounter,1,MPI_INT,uMPI::ioProc);
    rampUpMessages += (uMPI::rank != uMPI::ioProc);

    DEBUGPR(2,ucout << "reconfigureRestart done\n");

    return true;
  }
  


  // This is what the IO processor does when we reconfigure

  void parallelBranching::reconfigureRestartRoot(int filesFound)
  {
    DEBUGPR(2,ucout << "reconfigureRestartRoot called\n");
    
    UnPackBuffer cpBuf;

    // The workerList array keeps the processor numbers of all workers.
    // (Later on, we periodically shuffle its order)

    int wNum = 0;
    IntVector workerList(totalWorkers());
    for (int i=0; i<totalWorkers(); i++)
      workerList[i] = overallWorkerProc(i);

    // We keep track of the buffer sizes of all other processors
	
    IntVector rBufSize(uMPI::size);

    int rBufStart = spPackSize();
    if (parameter_initialized("spReceiveBuf"))
      rBufStart = spReceiveBuf;
    for(int i=0; i<uMPI::size; i++)
	  rBufSize[i] = rBufStart;

    // Main loop over checkpoint files.

    for (int p=0; p<filesFound; p++)
      {
	// Read the file for old processor p

	ifstream bstream(checkpointFilename(checkpointNumber,p).c_str(),
			 (ios::in | ios::binary));

	// Get the incumbent value and source.  Broadcast incumbent value if
	// this is the first file.  If this is the file with the incumbent
	// itself, read it and keep local.

	cpBuf.readBinary(bstream);
	int oldIncumbentSource;
	int fileProbCounter;
	cpBuf >> incumbentValue >> oldIncumbentSource >> fileProbCounter;
	DEBUGPR(10,ucout << "p=" << p << " incumbentValue=" << incumbentValue
		<< " oldIncumbentSource=" << oldIncumbentSource << endl);
	if (fileProbCounter > probCounter)
	  probCounter = fileProbCounter;
	if (p == 0)
	  uMPI::broadcast(&incumbentValue,1,MPI_DOUBLE,uMPI::ioProc);
	if (p == oldIncumbentSource)
	  incumbent = unpackSolution(cpBuf);

	// Application-specific data.  Read it and then replicate
	// the information to all processors, and do the "merge"
	// operation.

	cpBuf.readBinary(bstream);
	int appDataSize = cpBuf.message_length();
	uMPI::broadcast(&appDataSize,1,MPI_INT,uMPI::ioProc);
	uMPI::broadcast((void *)cpBuf.buf(),
			appDataSize,
			MPI_PACKED,
			uMPI::ioProc);
	DEBUGPR(10,ucout << "Broadcast " << appDataSize  
		<< " bytes of application data\n");

	appMergeGlobalData(cpBuf);

	// Adjust buffer tracking to reflect broadcast of application data

	for(int i=0; i<uMPI::size; i++)
	  if (rBufSize[i] < appDataSize)
	    rBufSize[i] = appDataSize;

	// Now read subproblems and send them out

	int numSPs = -1;
	bstream.read((char *) &numSPs,sizeof(int));
	DEBUGPR(10,ucout << numSPs << " subproblems in this file\n");

	for (int i=0; i<numSPs; i++)
	  {
	    cpBuf.readBinary(bstream);
	    if (i < cpDebugCount)
	      ucout << "Subproblem " << i << " has " << cpBuf.message_length()
		    << " bytes\n";
	    int wProc = workerList[wNum];
	    if (wProc == uMPI::rank)
	      {
		parallelBranchSub* sp = blankParallelSub();
		sp->unpackProblem(cpBuf);
		if (i < cpDebugCount)
		  ucout << "Keeping " << sp << " local\n";
		if (sp->canFathom())
		  sp->recycle();
		else
		  workerPool->insert(sp);
	      }
	    else
	      {
		reconfigureBufferCheck(cpBuf,wProc,rBufSize);
		if (i < cpDebugCount)
		  ucout << "Sending to processor " << wProc << endl;
		uMPI::send((void *) cpBuf.buf(),cpBuf.message_length(),
			   MPI_PACKED,wProc,reconfigSPTag);
	      }

	    // Advance so the next subproblem goes to a different worker.
	    // Wrap around if we are out of workers.  When that happens,
	    // randomly permute the list of workers.

	    if (++wNum == totalWorkers())
	      {
		wNum = 0;
		for(int k=totalWorkers(); k>1; )
		  {
		    int j = (int) floor(gRandom()*k);
		    k--;
		    int temp = workerList[j];
		    workerList[j] = workerList[k];
		    workerList[k] = temp;
		  }
	      }
	    
	  }

	// If enumerating, read any solutions and sent them out.

	if (enumerating)
	  {
	    int rsize = -1;
	    bstream.read((char *) &rsize,sizeof(int));
	    if (cpDebugCount > 0)
	      ucout << "Repository size " << rsize << endl;
	    for(int s=0; s<rsize; s++)
	      {
		cpBuf.readBinary(bstream);
		solution* sol = unpackSolution(cpBuf);
		if (s < cpDebugCount)
		  ucout << "Read solution " << sol << endl;
		int owner = owningProcessor(sol);
		if (owner == uMPI::rank)
		  {
		    if (s < cpDebugCount)
		      ucout << "Keeping\n";
		    assignId(sol);
		    localReposOffer(sol);
		  }
		else
		  {
		    reconfigureBufferCheck(cpBuf,owner,rBufSize);
		    if (s < cpDebugCount)
		      ucout << "Sending to " << owner << endl;
		    uMPI::send((void *) cpBuf.buf(),cpBuf.message_length(),
			       MPI_PACKED,owner,reconfigSolTag);
		    delete sol;
		  }
	      }
	  }
	
	// Inform everybody that we are done with this file by sending
	// an empty, specially-tagged message.

	DEBUGPR(10,ucout << "Declaring done with file " << p << endl);

	for (int p=0; p<uMPI::size; p++)
	  uMPI::send(&p,0,MPI_PACKED,p,reconfigDoneTag);
	
      }
    
    DEBUGPR(2,ucout << "reconfigureRestartRoot done\n");
    
  }


  // This routine is used at te root to make sure leaf processor p has
  // a sufficiently large buffer to receive the UnPackBuffer 'buf'.
  // If not, it sends a "please make a bigger buffer" message.

  void parallelBranching::reconfigureBufferCheck(UnPackBuffer& buf,
						 int p,
						 IntVector& rBufSize)
  {
    int thisSize = buf.message_length();
    if (thisSize > rBufSize[p])
      {
	PackBuffer auxBuf(sizeof(int));
	auxBuf << thisSize;
	DEBUGPR(100,ucout << "Requesting processor " << p
		<< " to expand its buffer from " << rBufSize[p]
		<< " to " << thisSize << endl);
	uMPI::send((void *) auxBuf.buf(),auxBuf.size(),
		   MPI_PACKED,p,reconfigResizeTag);
	rBufSize[p] = thisSize;
      }
  }


  // This is what all the other processors do in a configuration
  // restart.

  void parallelBranching::reconfigureRestartLeaf(int filesFound)
  {
    DEBUGPR(2,ucout << "reconfigureRestartLeaf called\n");

    int bufSize = spPackSize();
    if (parameter_initialized("spReceiveBuf"))
      bufSize = spReceiveBuf;
    UnPackBuffer cpBuf(bufSize);

    for (int p=0; p<filesFound; p++)
      {
	// Get the incumbent value, but only for the first time
	// through the loop.

	if (p == 0)
	  {
	    uMPI::broadcast(&incumbentValue,1,MPI_DOUBLE,uMPI::ioProc);
	    rampUpMessages++;
	    DEBUGPR(10,ucout << "Got incumbentValue=" 
		    << incumbentValue << endl);
	  }
	
	// Application-specific data.  Receive it and do the "merge"
	// operation.  But first get the data size and grow the buffer
	// if necessary

	int appDataSize = -1;
	uMPI::broadcast(&appDataSize,1,MPI_INT,uMPI::ioProc);
	if (appDataSize > (int) cpBuf.size())
	  cpBuf.resize(appDataSize);
	uMPI::broadcast((void *) cpBuf.buf(),
			appDataSize,
			MPI_PACKED,
			uMPI::ioProc);
	cpBuf.reset(appDataSize);
	DEBUGPR(10,ucout << "Got " << appDataSize << 
		" bytes of application data, with p=" << p << endl);

	appMergeGlobalData(cpBuf);

	// Now get subproblems (and solutions, if enumerating) until
	// we receive a "done" tag.

	MPI_Status status;

	do
	  {
	    uMPI::recv((void *) cpBuf.buf(),
		       cpBuf.size(),
		       MPI_PACKED,
		       uMPI::ioProc,
		       MPI_ANY_TAG,
		       &status);

	    cpBuf.reset(&status);
	    rampUpMessages++;

	    if (status.MPI_TAG == reconfigSPTag)
	      {
		if (iAmWorker())
		  {
		    parallelBranchSub* sp = blankParallelSub();
		    sp->unpackProblem(cpBuf);
		    DEBUGPR(10,ucout << "Got subproblem " << sp << endl);
		    if (sp->canFathom())
		      sp->recycle();
		    else
		      workerPool->insert(sp);
		  }
		else
		  EXCEPTION_MNGR(runtime_error, 
				 "Non-worker got a subproblem during "
				 "reconfigure");
	      }
	    else if (status.MPI_TAG == reconfigResizeTag)
	      {
		int newSize = -1;
		cpBuf >> newSize;
		if (newSize <= (int) cpBuf.size())
		   EXCEPTION_MNGR(runtime_error, "Buffer shrinkage during "
				  "reconfigure");
		cpBuf.resize(newSize);
		DEBUGPR(100,ucout << "Buffer expansion to " 
			<< newSize << endl);
	      }
	    else if (enumerating && (status.MPI_TAG == reconfigSolTag))
	      {
		solution* sol = unpackSolution(cpBuf);
		DEBUGPR(10,ucout << "Got solution " << sol << endl);
		assignId(sol);
		localReposOffer(sol);
	      }
	    else if (status.MPI_TAG != reconfigDoneTag)
	      EXCEPTION_MNGR(runtime_error, "Bad tag received during "
			     "reconfigure");

	  }
	while(status.MPI_TAG != reconfigDoneTag);

	DEBUGPR(10,ucout << "Done with p=" << p << endl);

      }

    DEBUGPR(2,ucout << "reconfigureRestartLeaf done\n");

  }


  // Helper objects to restartSetLoads...

  static parallelBranching* reduceOpPBPtr = NULL;

  void loadReduceOp(void* invec,void* inoutvec,int* len,MPI_Datatype* dt)
  {
    if (*len != 1)
       EXCEPTION_MNGR(runtime_error, "loadReduceOp got len=" << *len);
    reduceOpPBPtr->addPackedLoads((char*) invec,(char*) inoutvec);
  }

  void parallelBranching::addPackedLoads(char* inbuf,char* inoutbuf)
  {
    parLoadObject tempLoad1(this,false,false);
    parLoadObject tempLoad2(this,false,false);
    int ploSize = parLoadObject::packSize(this);
    UnPackBuffer up1buf(inbuf,ploSize,false);
    UnPackBuffer up2buf(inoutbuf,ploSize,false);
    up1buf.reset(ploSize);
    up2buf.reset(ploSize);
    up1buf >> tempLoad1;
    up2buf >> tempLoad2;
    tempLoad2 += tempLoad1;
    PackBuffer pbuf(ploSize);
    pbuf << tempLoad2;
    const char* cursor = pbuf.buf();
    for(int i=0; i<ploSize; i++)
      *(inoutbuf++) = *(cursor++);
  }
  

  // Method to set up load information after restarting from a checkpoint.

  void parallelBranching::restartSetLoads()
  {
    DEBUGPR(2,ucout << "restartSetLoads called\n");

    int ploSize  = parLoadObject::packSize(this);
    int ploSizeS = uMPI::packSlop(ploSize);
    int ierr;
    
    clusterLoad.reset();
    globalLoad.reset();

    // First, all pure workers send their load to their hub,
    // and the hub should digest them.  Then send clusterloads out 
    // to workers again.  This is done with multiple communicators
    // and collective communication.

    // Make a buffer to store each worker's load.

    PackBuffer loadPackBuf(ploSize);
    parLoadObject myLoad(this,false,false);
    myLoad = updatedPLoad();
    DEBUGPR(10,ucout << "Worker load is " << myLoad << endl);
    loadPackBuf << myLoad;

    // Make a buffer to receive load data

    char* gatherBuf = NULL;
    if (iAmHub())
      gatherBuf = new char[numWorkers()*ploSizeS];

    // Make a communicator for each cluster

    MPI_Comm clusterComm;
    ierr = MPI_Comm_split(uMPI::comm,myHub(),uMPI::rank,&clusterComm);
    if (ierr)
       EXCEPTION_MNGR(runtime_error, "MPI_Comm_split returned " << ierr);
 
    // Gather the load information to each hub

    ierr = MPI_Gather((void*) loadPackBuf.buf(),ploSize,MPI_PACKED,
		      gatherBuf,ploSizeS,MPI_PACKED,
		      0,clusterComm);
    if (ierr)
       EXCEPTION_MNGR(runtime_error, "MPI_Gather returned " << ierr);

    // Process the load information at each hub

    if (iAmHub())
      {
	clusterLoad.update();  // Sets incumbent so no mismatch flag!
	if (iAmWorker())
	  clusterLoad += myLoad;
	for(int i=iAmWorker(); i<numWorkers(); i++)
	  {
	    UnPackBuffer upgBuf(gatherBuf + i*ploSizeS,ploSizeS,false);
	    upgBuf.reset(ploSizeS);
	    upgBuf >> workerLoadReport[i];
	    workerLoadEstimate[i] = workerLoadReport[i];
	    clusterLoad += workerLoadReport[i];
	    DEBUGPR(10,ucout << "Relative worker " << i 
		    << " reports a load of " << workerLoadReport[i] << endl);
	    rampUpMessages++;
	  }

	DEBUGPR(10,ucout << "clusterLoad calculated: " << clusterLoad << endl);

	delete[] gatherBuf;

	loadPackBuf.reset();
	loadPackBuf << clusterLoad;
      }

    // Broadcast the cluster load to the whole cluster

    ierr = MPI_Bcast((void *) loadPackBuf.buf(),
		     ploSize,MPI_PACKED,0,clusterComm);
    if (ierr)
       EXCEPTION_MNGR(runtime_error, "MPI_Bcast returned " << ierr);

    // On workers, extract the data

    if (iAmWorker() && !iAmHub())
      {
	UnPackBuffer unpackBCastBuf((char*) loadPackBuf.buf(),ploSize,false);
	unpackBCastBuf.reset(ploSize);
	unpackBCastBuf >> clusterLoad;
	DEBUGPR(10,ucout << "worker has clusterLoad of " 
		<< clusterLoad << endl);
      }

    MPI_Comm_free(&clusterComm);

    // Now add load information for all hubs.  Try to use collective 
    // communication for this too.  This gets a little tricky.

    // Define a communicator for just hubs.

    MPI_Comm hubComm;

    ierr = MPI_Comm_split(uMPI::comm,iAmHub(),uMPI::rank,&hubComm);
    if (ierr)
       EXCEPTION_MNGR(runtime_error, "MPI_Comm_split returned " << ierr);

    // Define a datatype for packed load objects

    MPI_Datatype ploType;
    ierr = MPI_Type_contiguous(ploSize,MPI_PACKED,&ploType);
    if (ierr)
       EXCEPTION_MNGR(runtime_error, "MPI_Type_contiguous returned " << ierr);
    ierr = MPI_Type_commit(&ploType);
    if (ierr)
       EXCEPTION_MNGR(runtime_error, "MPI_Type_commit returned " << ierr);

    // Define a reduction operation to add loads

    MPI_Op reduceOp;
    ierr = MPI_Op_create(&loadReduceOp,true,&reduceOp);
    if (ierr)
       EXCEPTION_MNGR(runtime_error, "MPI_Op_create returned " << ierr);
    
    reduceOpPBPtr = this;

    // OK -- try to add all loads into globalLoad on the first hub

    UnPackBuffer globalBuf(ploSizeS);

    if (iAmHub())
      {
	ierr = MPI_Reduce((void*) loadPackBuf.buf(),
			  (void*) globalBuf.buf(),
			  1,ploType,reduceOp,0,hubComm);
	if (ierr)
	   EXCEPTION_MNGR(runtime_error, "MPI_Reduce returned " << ierr);
      }

    // Now broadcast to everybody and unpack.

    uMPI::broadcast((void *) globalBuf.buf(),ploSize,MPI_PACKED,firstHub());
    
    globalBuf.reset(ploSize);
    globalBuf >> globalLoad;
    DEBUGPR(10,ucout << "Global load reduction yields " <<
	    globalLoad << endl);

    DEBUGPR(2,ucout << "restartSetLoads done\n");

  }


} // namespace pebbl

#endif
