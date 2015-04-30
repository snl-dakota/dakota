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
 * \file parKnapsack.h
 * \author Jonathan Eckstein
 *
 * Definition of a parallel knapsack class for PEBBL.
 */

#ifndef pebbl_parKnapsack_h
#define pebbl_parKnapsack_h

#include <acro_config.h>
#include <pebbl/serialKnapsack.h>
#ifdef ACRO_HAVE_MPI
#include <pebbl/parBranching.h>

namespace pebbl {

using utilib::ParameterSet;


// Before the real business, define a the queue of
// subproblem solutions in the incubment heuristic.
// These are now kept in a simple queue.

class incumbQueueItem : public binKnapSolution
{

friend class parallelBinaryKnapsack;

public:

  incumbQueueItem(binKnapSolution* feeder) :
  binKnapSolution(*feeder),
  next(0)
    { };

  void mutate()
    {
      backTrack(chooseBackTrackItem());
      completeRandom();
    };

private:

  incumbQueueItem* next;
  
};



//  OK, now define the master class for parallel knapsack.

class parBinKnapSub;                  // Forward reference

class parallelBinaryKnapsack : public parallelBranching, public binaryKnapsack
{
protected:

  ///
  int incumbQueueTargetSize;
  int incumbQueueMaxSize;

  // Application-specific stuff consists entirely of stuff to run 
  // the incumbent search.

  void insertInIncQueue(incumbQueueItem* s);
  incumbQueueItem* removeTopOfIncQueue();

  incumbQueueItem* firstInIncQueue;
  incumbQueueItem* lastInIncQueue;
  int              incQueueSize;

  // Now we basically do the standard synthesis.

public:  

  void pack(PackBuffer& outBuffer);
  void unpack(UnPackBuffer& inBuffer);
  int  spPackSize();

  parallelBranchSub* blankParallelSub();

  parallelBinaryKnapsack() : 
    binaryKnapsack(),
    incumbQueueTargetSize(100),
    incumbQueueMaxSize(1000),
    firstInIncQueue(0),
    lastInIncQueue(0),
    incQueueSize(0)
    {
      ParameterSet::create_categorized_parameter("incumbQueueTargetSize",
				     incumbQueueTargetSize,
				     "<int>","100",
                                     "Parallel knapsack heuristic control",
				     "Knapsack",
				     utilib::ParameterPositive<int>());
      ParameterSet::create_categorized_parameter("incumbQueueMaxSize",
                                     incumbQueueMaxSize,
				     "<int>","1000",
                                     "Parallel knapsack heuristic control",
				     "Knapsack",
				     utilib::ParameterPositive<int>());
      branchingInit(maximization, relTolerance, absTolerance); 
    };
  
  // This will create a standard problem routine.  It will automatically
  // call the application-specific serial setup routine.

  bool setup(int& argc,char**& argv) 
    { 
      return parallelBranching::setup(argc,argv);
    }

  void solve() { parallelBranching::solve(); };

  // This will create a standard solution printout routine.

  void printSolution(const char* header = "",
		     const char* footer = "",
		     std::ostream& outStream = ucout)
    {
      parallelBranching::printSolution(header,footer,outStream);
    }

  ~parallelBinaryKnapsack() { };

  /// Note: use VB flag?
  void reset(bool VBflag=true)
    {
      binaryKnapsack::reset();
      registerFirstSolution(new binKnapSolution(this));
      parallelBranching::reset();
    }

  bool hasParallelIncumbentHeuristic() { return true; }; 

  void parallelIncumbentHeuristic(double* controlParam);

  ThreadObj::ThreadState incumbentHeuristicState()
    {
      if (incQueueSize == 0)
	return ThreadObj::ThreadBlocked;
      else
	return ThreadObj::ThreadReady;
    };

  void feedToIncumbentThread(binKnapSolution* soln);

};


// Now, the subproblems...

class parBinKnapSub : public parallelBranchSub, public binKnapSub
{
public:

  parBinKnapSub() {};
  

  void parBinKnapSubFromParKnapsack(parallelBinaryKnapsack* master_)
    {
      globalPtr = master_;
      binKnapSubFromKnapsack(master_);
    };

  void parBinKnapSubAsChildOf(parBinKnapSub* parent,int whichChild)
    {
      globalPtr = parent->globalPtr;
      // This will initialize earlier pieces
      binKnapSubAsChildOf(parent,whichChild);
    };

  ~parBinKnapSub() 
    { };


  parallelBinaryKnapsack* global()  const { return globalPtr; };
  parallelBranching*      pGlobal() const { return globalPtr; };

  void pack(PackBuffer& outBuffer);

  void unpack(UnPackBuffer& inBuffer);

  parallelBranchSub* makeParallelChild(int whichChild)
    {
      parBinKnapSub *temp = new parBinKnapSub;
      temp->parBinKnapSubAsChildOf(this,whichChild);
      return temp;
    }

  void quickIncumbentHeuristic();

  void feedToIncumbentThread();

 protected:

  void valLogDestroyPrint() { parallelBranchSub::valLogDestroyPrint(); };

 private:

  parallelBinaryKnapsack* globalPtr;

};


inline parallelBranchSub* parallelBinaryKnapsack::blankParallelSub()
{
  parBinKnapSub *temp = new parBinKnapSub;
  temp->parBinKnapSubFromParKnapsack(this);
  return temp;
};

} // namespace pebbl

#endif

#endif
