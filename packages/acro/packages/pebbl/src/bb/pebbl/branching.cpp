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
// branching.cpp
//
// Implements lowest level common functions for branching algorithms
// (except those inlined in branching.h).
//
// Jonathan Eckstein
//


#include <acro_config.h>

#include <math.h>

// nextafter is a C99 extension, but MSVS2005 and newer provide _nextafter
#ifdef _MSC_VER
#include <float.h>
#define nextafter _nextafter
#endif

#include <utilib/_math.h>
#include <utilib/nicePrint.h>
#include <utilib/seconds.h>
#include <utilib/hash_fn.h>
#include <utilib/file_util.h>

#include <pebbl/branching.h>
#include <pebbl/gRandom.h>

#include <iostream>
#include <iomanip>


using namespace std;

extern "C" void pebbl_abort_handler(int code)
{
#if defined(SOLARIS) | defined(LINUX)
//
// We call the p4_error routine to allow MPI to clean itself up!
//
//const char* tmp = "Signal Caught by PEBBL";
//p4_error(const_cast<char*>(tmp),code);
#endif

#ifdef ACRO_HAVE_MPI
MPI_Abort(MPI_COMM_WORLD,-1);
abort();
#endif
exit(-code);
}



using namespace utilib;

namespace pebbl {


string branching::version_info = "Unknown version";


// An initializer for branchSubId

void branchSubId::branchSubIdFromBranching(branching* global)
  {
  serial = ++(global->probCounter);
#ifdef ACRO_HAVE_MPI
  creatingProcessor = uMPI::running() ? uMPI::rank : 0;
#else
  creatingProcessor = 0;
#endif
};


// This is used to compare subproblems in a way that varies
// depending on whether we're doing initial diving.

int coreSPInfo::dynamicSPCompare(const coreSPInfo& sp2) const
{
  DEBUGPRX(200,bGlobal(),"Dynamic compare of "; debugPrint(ucout);
           ucout << " and "; sp2.debugPrint(ucout); ucout << endl);

  // If the subproblems are clearly identical, bail out and say
  // They're indistinguishable.

  if (this==&sp2)
    return 0;

  // See if we are doing initial diving
  if (this->bGlobal()->initialDive && 
      (fabs(this->bGlobal()->incumbentValue) == MAXDOUBLE))
    {
      // OK -- we are trying an initial dive and there is no incumbent,
      // so do not use the usual compare method...

      // First test the integrality measure if the user wants to
      // dive on integrality.

      if (bGlobal()->integralityDive && 
	  (integralityMeasure != sp2.integralityMeasure))
	{
	  if (integralityMeasure < sp2.integralityMeasure)
	    return -1;
	  else
	    return 1;
	}

      // Either the user wants to dive on pure depth, or the integrality
      // measures were identical.
      
      if (depth != sp2.depth)
	{
	  if (depth > sp2.depth)
	    return -1;
	  else
	    return 1;
	}
  
    }
  
  // Either no initial diving, or things are identical from the viewpoint of 
  // initial diving.  Just use the regular compare function.

  return compare(sp2);

};


int coreSPInfo::compare(const coreSPInfo& sp2) const
{
  DEBUGPRX(200,bGlobal(),"Comparing "; debugPrint(ucout);
           ucout << " and "; sp2.debugPrint(ucout); ucout << endl);

  // Bail out if the subproblems are a priori identical

  if (this==&sp2) 
    return 0;

  // Decide on bound, if different

  if (bound != sp2.bound) 
    {
      if ((bGlobal()->sense)*(bound - sp2.bound) < 0)
	return -1;
      else 
	return 1;
    }

  // Try to decide based on integrality measure

  if (integralityMeasure != sp2.integralityMeasure)
    {
      if (integralityMeasure < sp2.integralityMeasure)
	return -1;
      else
	return 1;
    }

  // Impose a strong ordering based on serial number and creating processor

  if (id.serial != sp2.id.serial)
    {
      if (id.serial < sp2.id.serial)
	return -1;
      else
	return 1;
    }
 
  if (id.creatingProcessor != sp2.id.creatingProcessor)
    {
      if (id.creatingProcessor < sp2.id.creatingProcessor)
	return -1;
      else
	return 1;
    }

  // These subproblems look indistinguishable.  Hopefully they are just
  // different tokens of the same base subproblem.

  return 0;

};


// Guts of the standard constructors for "branching"

// If anything to do with the sense (optimization direction), and related
// things, such as the bluff change, look at effects on MILP::readIn

void branching::branchingInit(optimType direction,
			     double relTolSet,
			     double absTolSet,
			     double bluff,
			     bool bluffReal)
{
  sense  = direction;
  if (relTolSet != -1.0)
     relTolerance = relTolSet;
  if (absTolSet != -1.0)
     absTolerance = absTolSet;

  if (!bluffReal)
    {
      if (parameter_initialized("startIncumbent"))
	incumbentValue = startIncumbent;
      else incumbentValue = direction*MAXDOUBLE;
    }
  else
    {
      incumbentValue = bluff;
      // Originally, this was always set, since this would avoid checks
      // in the reset method, but this won't work when there are two
      // instances of PEBBL running simultaneously that have, say,
      // different objective senses, and therefore different initial
      // incumbent values (they share the parameter table).  Till that
      // changes, only use the incumbentValue to signal use of a bluff
      // and avoid using the bluff or startHeuristic parameter when using
      // multiple PEBBL instances.
      //
      // WEH - with the use of ParameterSet ... this should not be
      // necessary
      set_parameter("startIncumbent",incumbentValue);
    }

  branchingInitGuts();
 
}

void branching::branchingInitGuts()
{

  for (int i=0; i<numStates; i++)
    subCount[i] = 0;

  probCounter       = 0;
  currentSP         = NULL;
  needPruning       = false;

  vout = 0;
  statusLinePrecision = (int) ceil(-log10(max(relTolerance,1e-16)));

  boundCompCalls  = 0;
  splitCompCalls  = 0;
  boundCompTime   = 0;
  boundCompTimeSq = 0;
  splitCompTime   = 0;
  splitCompTimeSq = 0;
}


void branching::reset(bool /*resetVB*/)
{
  gRandomReSeed();
  branchingInitGuts();
  resetIncumbent();
  if (parameter_initialized("startIncumbent"))
    incumbentValue = startIncumbent;
  else incumbentValue = sense*MAXDOUBLE;
  if (pool) 
    {
      pool->load().reset();
      pool->reset();
    }

  // Figure out if enumeration is being used

  usingEnumCutoff = parameter_initialized("enumCutoff");
  enumerating = usingEnumCutoff   ||
                (enumCount > 1)   || 
                (enumRelTol >= 0) || 
                (enumAbsTol >= 0);

  // Repository and solution tracking stuff (not really needed unless
  // enumerating)

  clearRepository();
  solSerialCounter = 0;
  lastSolId.setWorstPossible(sense);

  if (enumerating)
    reposTable.resize(enumHashSize);

  solsOffered  = 0;
  solsAdmitted = 0;

  //
  // Create the pool -- this was in branchingInit, but it couldn't see
  // the parameters yet!
  //

  if (pool)
    delete pool;

  if ( depthFirst )
    pool = new doublyLinkedPool<branchSub,loadObject>(true);  //stack
  else if ( breadthFirst )
    pool = new doublyLinkedPool<branchSub,loadObject>(false); //queue
  else
    pool = new heapPool<branchSub,loadObject,DynamicSPCompare<branchSub> >();
 
  pool->load().init(this);
  pool->setGlobal(this);

  // Miscellaneous stuff

  lastSolValOutput = sense*MAXDOUBLE;

  lastHeurPrint = 0;

  abortReason = NULL;

  // Set up handler on the assumption the user will not supply one
  // So far, users never supply handlers.

  if (handler)
    delete handler;

  if (lazyBounding)
    {
      DEBUGPR(5, ucout << "Installing lazy bounding handler" << endl);
      handler = new lazyHandler;
    }
  else if (eagerBounding)
    {
      DEBUGPR(5, ucout << "Installing eager bounding handler" << endl);
      handler = new eagerHandler;
    }
  else
    {
      DEBUGPR(5, ucout << "Installing hybrid bounding handler" << endl);
      handler = new hybridHandler;
    }

  handler->setGlobal(this);
}


// Standard serial read-in code.  Returns true if we can continue, false if
// we have to bail out.

bool branching::setup(int& argc, char**& argv)
{
  resetTimers();

  if (!processParameters(argc,argv,min_num_required_args))
    return false;
  if (plist.size() == 0)
    {
      ucout << "Using default values for all solver options" << std::endl;
    }
  else
    {
      ucout << "User-specified solver options: " << std::endl;
      plist.write_parameters(ucout);
      ucout << std::endl;
    }
  set_parameters(plist,false);
  if ((argc > 0) && !checkParameters(argv[0]))
    return false;
  if (!setupProblem(argc,argv))
    return false;
  if (plist.unused() > 0) {
    ucout << "\nERROR: unused parameters: " << std::endl;
    plist.write_unused_parameters(ucout);
    ucout << utilib::Flush;
    return false;
  }
  return true;
}


void branching::resetTimers()
{
  startTime = CPUSeconds();
  startWall = WallClockSeconds();
}


bool branching::processParameters(int& argc, 
				  char**& argv, 
				  unsigned int min_num_required_args__)
{
  if (argc > 0)
     solver_name = argv[0];
  else
     solver_name = "unknown";
  if (!parameters_registered) 
    {
      register_parameters();
      parameters_registered=true;
    }

  if ((argc > 1) && (strcmp(argv[1],"--version")==0)) 
    {
      ucout << endl << version_info << endl << endl;
      return false;
    }

  plist.process_parameters(argc,argv,min_num_required_args__);

  // Set the name of the problem to be the last thing on the command
  // line. setName will extract the filename root. The setupProblem
  // method can overwrite this later.

  if ((argc > 1) && (argv[argc-1] != NULL))
    setName(argv[argc-1]);

  return true;
}


void branching::serialPrintSolution(const char* header,
				    const char* footer,
				    std::ostream& outStream)
{
  if (incumbent)
    {
      int oldPrecision = outStream.precision();
      outStream.precision(statusLinePrecision);
      outStream << header;
      incumbent->print(outStream);
      outStream << footer;
      outStream.precision(oldPrecision);
    }
  else 
    outStream << "No solution identified.\n";
}


// Default destructor.

branching::~branching() 
{
  if (pool)
    delete pool;
  if (handler)
    delete handler;
  resetIncumbent();
  clearRepository();
}


// This sets the official name of the problem by chewing up the
// filename.  It can be overridden.  This version just finds the last
// "/" or "\" in the name and removes it and everything before it.

void branching::setName(const char* cname)
{
#if defined (TFLOPS)
  problemName = cname;
  int i=problemName.size();
  while (i >= 0) {
    if (cname[i] == '/') break;
    i--;
    }
  if (i >= 0)
     problemName.erase(0,i+1);
  // TODO: remove the .extension part for this case
#else
  problemName = cname;
  size_type i = problemName.rfind("/");
  if (i == string::npos)
    i = problemName.rfind("\\");
  if (i != string::npos)
    problemName.erase(0,i+1);

  size_type n = problemName.length();

  if (n < 4) 
    return;

  string endOfName(problemName,n-4,4);
  if ((endOfName == ".dat") || (endOfName == ".DAT"))
    problemName.erase(n-4,n);
#endif
}


// Fathoming-related stuff...


// Standard fathoming function.  

// The canFathom functions are for things NOT already in the repository

// Test fathoming of a subproblem or token (or anything else derived
// from coreSPInfo).

bool coreSPInfo::canFathom()
{
  DEBUGPRX(100,bGlobal(),
	   "Checking on fathoming "; debugPrint(ucout); ucout << endl);
  return (state == dead) || bGlobal()->canFathom(bound);
}


// Test a solution or solution identifier.

bool branching::canFathom(solutionIdentifier* solPtr)
{
  if (canFathom(solPtr->value))
    return true;

  // At this point the solution is most probably OK.  However, if
  // we're using enumCount, there's a possibility that there is a tie
  // in the objective value, but the the (id{,processor}) isn't good
  // enough to get in the repository.  The final check will take care
  // of that. Only say false if the solution definitely wins on all
  // counts.  Also check to make sure that we don't push good things
  // out of the repository while waiting for lastSolId to get updated.

  if (enumCount > 1) 
    {
      if (lastSolId.compare(solPtr) <= 0)
	return true;
      if ((repositorySize() == enumCount) &&
	  (worstReposSol()->compare(solPtr) <= 0))
	return true;
    }

  return false;
}


// To fathom something just based on its bound value

// CanFathom(bound) gets called a lot, and has become quite
// complicated.  A possible TODO is to distill all the possible tests
// into a single value and just to one compare.  But then we would
// have to be sure to update that one value carefully in every piece
// of code that might affect it.

bool branching::canFathom(double boundValue) 
{
  double gap   = absGap(boundValue);
  double denom = gapDenom(boundValue);

  DEBUGPR(1000, ucout << "branching::canFathom - Starting test incumbent = " 
	  << incumbentValue << " bound = " << boundValue 
	  << " Performing Enumeration: " << enumerating << endl);

  // One set of tests if enumerating

  if (enumerating)
    {
      if (usingEnumCutoff && sense*(enumCutoff - boundValue) <= 0)
	{
	  DEBUGPR(1000,ucout << "branching::canFathom - Fathoming with "
		  "enumCutoff! enumCutoff = " << enumCutoff << endl);
	  return true;
	}

      if ((enumAbsTol >= 0) && (gap < -enumAbsTol)) 
	{
	  DEBUGPR(1000, ucout << "branching::canFathom - Fathoming with "
		  "enumAbsTol! gap = " << gap 
		  << " < -enumAbsTol = " 
		  << (-enumAbsTol) << endl);
	  return true;
	}

      if ((enumRelTol >= 0) && (gap < -enumRelTol*denom)) 
	{
	  DEBUGPR(1000, ucout << "branching::canFathom - Fathoming "
		  << "with enumRelTol! gap = " << gap 
		  << " <= -enumRelTol*denom = " 
		  << (-enumRelTol*denom) << endl);
	  return true;
	}

      // Note that lastSolId.value will be sense*MAXDOUBLE if the
      // repository is not full yet.  Here we use the regular
      // tolerances as in normal fathoming.

      if (enumCount > 1)
	{
	  double lsval = lastSolId.value;
	  double lsgap = sense*(lastSolId.value - boundValue);
	  if (lsgap < absTolerance)
	    return true;
	  double lsdenom = std::max(std::fabs(boundValue),std::fabs(lsval));
	  return (lsgap < MAXDOUBLE) && 
	         (lsdenom < MAXDOUBLE) &&
	         (lsgap <= relTolerance*lsdenom);
	}

      DEBUGPR(1000, ucout << "branching::canFathom - NO Fathoming because "
	      "still performing enumeration" << endl);

      return false;

    }

  // Plain old non-enumerating tests

  if (gap < absTolerance) 
    {
      DEBUGPR(1000, ucout << "branching::canFathom - Fathoming  gap = " 
	      << gap << " < absTolerance = " << absTolerance << endl);
      return true;
    }

  if (denom == 0) 
    {
      DEBUGPR(1000, ucout << "branching::canFathom - Fathoming  denom = 0"
	     << endl);
      return true;
    }

  bool status = (gap < MAXDOUBLE) && 
                (denom < MAXDOUBLE) && 
                (gap <= relTolerance*denom);

  DEBUGPR(1000, ucout << "branching::canFathom - "
	  << (status ? "" : "not ") << "fathoming: gap = " 
	  << gap << " absTolerance = " << absTolerance << " denom = " 
	  << denom << " relTolerance*denom = " 
	  << relTolerance*denom << endl);

  return status;
}


// Compute the relative gap for a a specific objective value

double branching::relGap(double boundValue)
{
  double denom = gapDenom(boundValue);
  if (denom == 0)
    return 0;
  return absGap(boundValue)/denom;
}


// Relative gap for a collection of subproblems summarized by a loadObject

double branching::relGap(loadObject& l) 
{ 
  if (l.count() == 0)
    return 0;
  if (l.boundUnknown())
    return 1;
  return relGap(l.aggregateBound); 
}


// Relative gap for the entire search (overridden in parallel layer)

double branching::relGap() 
{ 
  loadObject l = updatedLoad();
  return relGap(l); 
};


// This function computes a value v such that any subproblem with
// bound >= v may be fathomed.  In principle, we could just compute
// this, cache it, and reduce all the canFathom functions to a single
// comparison.  But for now we just implement it so we can set dual LP
// cutoff values in PICO.

double branching::fathomValue()
{
  // Pretend the problem is always minization by multiplying the
  // incumbent by "sense"

  double incv = sense*incumbentValue;

  double fv = MAXDOUBLE;

  if (incv < MAXDOUBLE)
    {
      if (enumerating)
	{
	  if (enumAbsTol >= 0)
	    fv = min(fv,nextafter(incv + enumAbsTol,MAXDOUBLE));
	  if (enumRelTol >= 0)
	    fv = min(fv,nextafter(incv + enumRelTol*abs(incv),MAXDOUBLE));
	  if (usingEnumCutoff)
	    fv = min(fv,sense*enumCutoff);
	  if (enumCount > 1)
	    {
	      // lsv == MAXDOUBLE if not enough solutions yet
	      double lsv = sense*lastSolId.value;
	      if (lsv < MAXDOUBLE)
		{
		  fv = min(fv,lsv - absTolerance);
		  fv = min(fv,lsv - relTolerance*abs(lsv));
		}
	    }
	}
      else 
	{
	  fv = min(fv,incv - absTolerance);
	  fv = min(fv,incv - relTolerance*abs(incv));
	}
    }

  // Multiply final result by sense again in case it was maximization

  return sense*fv;
}


// Change the incumbent solution; assumes ownership of "sol" If "sol"
// is a null pointer, then do nothing.  The "sync" flag *only* applies
// during ramp-up in the parallel layer, and may be ignored here.

void branching::foundSolution(solution* sol,syncType sync)
{
  DEBUGPR(200,ucout << "branching::foundSolution\n");
  if (sol)
    {
      DEBUGPR(200,ucout << "branching::foundSolution:(" 
	      << (void *) sol << ")\n");
      DEBUGPR(4,ucout << "branching::foundSolution: value = " << sol->value 
	      << ", hash = " << sol->computeHashValue() 
	      << ':' << (sol->computeHashValue() % enumHashSize) << endl);
      DEBUGPR(250,sol->print(ucout));
      if (canFathom(sol))
	{
	  DEBUGPR(10,ucout << "Fathomed.\n");
	  deleteSolution(sol);
	}
      else 
	{
	  DEBUGPR(20,ucout << "Cannot be fathomed\n");
	  if (sense*(sol->value - incumbentValue) < 0)
	    {
	      DEBUGPR(10,ucout << "Improves incumbentValue=" << incumbentValue 
		      << endl);
	      DEBUGPR(1,ucout << "Found improved incumbent, value = "
		      << sol->value << endl);
	      resetIncumbent();
	      if (sol->serial < 0)           // Stamp it if the user forgot
		sol->creationStamp(this);
	      incumbent = sol;
	      incumbentValue = sol->value;
	      signalIncumbent();
	      newIncumbentEffect(incumbentValue);
	    }
	  if (enumerating)
	    offerToRepository(sol,sync);
	}
    }
}


// Wipe out the previous incumbent solution.  Note that if we are
// enumerating, the incumbent will be in the repository, so we should
// let the repository reclaim the memory.  If not enumerating, then
// reclaim the memory here.

void branching::resetIncumbent()
{
  if (incumbent)
    {
      if (!enumerating)
	delete incumbent;
      incumbent = NULL;
    };
} 


// Default (and typical) implementation of makeRoot operation.

branchSub* branching::makeRoot() 
{
  branchSub* root = blankSub();
  root->setRoot();
  if (valLogOutput())
    root->valLogCreatePrint(NULL);
  return root;
}

void branchSub::computeBound()
{
  double dummy = MAXDOUBLE;
  computeBound(&dummy);
}

void branchSub::computeBound(double* controlParam)
{
  DEBUGPR(10,ucout << "Trying to bound " << this << '\n');
  if (state == boundable)
    setState(beingBounded);
  else if (state != beingBounded)
    EXCEPTION_MNGR(runtime_error, 
		   "Attempt to bound " << stateString(state) << " subproblem");
  MEMORY_TRACK_SP(this,"Before bound computation");
  double startBCTime = 0;
  double BCTime      = 0;
  int    printSpTimes = bGlobal()->printSpTimes;
  if (printSpTimes)
    startBCTime = WallClockSeconds();
  boundComputation(controlParam);
  bGlobal()->boundCompCalls++;
  if (printSpTimes)
    {
      BCTime = WallClockSeconds() - startBCTime;
      bGlobal()->boundCompTime   += BCTime;
      bGlobal()->boundCompTimeSq += BCTime*BCTime;
      if (printSpTimes > 1)
	{
	  int oldPrec = ucout.precision(8);
	  ucout << "Processor " << uMPI::rank 
                << " call " << bGlobal()->boundCompCalls
		<< " boundComputationTime "
		<< BCTime << " depth " << depth << endl;
	  ucout.precision(oldPrec);
	}
    }
  MEMORY_TRACK_SP(this,"Bound computation");
  DEBUGPR(10,ucout << "Result is " << this << '\n');
  if (valLogOutput() && ((state == bounded) || (state == dead)))
    valLogBoundPrint();
  if ((state == bounded) && !canFathom() && candidateSolution())
    foundSolution(extractSolution(),synchronous);
}


  
// Standard code shell around problem splitting computation.

int branchSub::splitProblem()
{
  if (state == bounded)
    setState(beingSeparated);
  else if (state != beingSeparated)
    EXCEPTION_MNGR(runtime_error, "Attempt to split " << stateString(state) 
		   << " subproblem");
  DEBUGPRP(10,ucout << "Splitting problem " << this << endl);
  MEMORY_TRACK_SP(this,"Before split");
  double startSCTime = 0;
  double SCTime      = 0;
  int    printSpTimes = bGlobal()->printSpTimes;
  if (printSpTimes)
    startSCTime = WallClockSeconds();
  int n = splitComputation();
  if (printSpTimes)
    {
      SCTime = WallClockSeconds() - startSCTime;
      bGlobal()->splitCompCalls++;
      bGlobal()->splitCompTime   += SCTime;
      bGlobal()->splitCompTimeSq += SCTime*SCTime;
      if (printSpTimes > 1)
	{
	  int oldPrec = ucout.precision(8);
	  ucout << "Processor " << uMPI::rank 
                << " call " << bGlobal()->splitCompCalls
		<< " SplitComputationTime "
		<< SCTime << " depth " << depth << endl;
	  ucout.precision(oldPrec);
	}
    }
  MEMORY_TRACK_SP(this,"After split");
  if (canFathom())
    {
      childrenLeft = totalChildren = 0;
      DEBUGPR(10,ucout << "Problem " << this 
	      << " was fathomed during splitComputation()" << endl);
      if (valLogOutput())
	valLogSplitPrint();
    }
  else if (state == separated)
    {
      childrenLeft = totalChildren = n;
      DEBUGPR(10,ucout << "Problem " << this 
	      << " has " << totalChildren << " children.\n");
      if (n == 0)
	setState(dead);
      if (valLogOutput())
	valLogSplitPrint();
      return totalChildren;
    }
  return 0;
}


// Standard code shell around production of new problems.

branchSub* branchSub::child(int whichChild)
{
  if (state != separated)
    EXCEPTION_MNGR(runtime_error, "Attempt to extract child of " 
		   << stateString(state) << " problem");
  if (childrenLeft <= 0)
    EXCEPTION_MNGR(runtime_error, 
		   "Tried to make too many children of a subproblem");
  MEMORY_TRACK(bGlobal()->probCounter + 1,"Before making child");
  branchSub* c = makeChild(chooseChild(whichChild));
  MEMORY_TRACK_SP(c,"Made child");
  childrenLeft--;
  DEBUGPR(10,ucout << "Extracted child " 
	  << totalChildren - childrenLeft
	  << " of "  << this << '\n');
  DEBUGPR(10,ucout << "Made " << c << '\n');
  if (valLogOutput())
    c->valLogCreatePrint(this);
  return c;
}


//  Destructor-like thing.  This can be overridden to something that
//  is efficient about memory allocation.

void branchSub::recycle()
{
  if (valLogOutput())
    valLogDestroyPrint();
  MEMORY_TRACK_SP(this,"Before recycling subproblem");
#ifdef MEMORY_TRACKING
  int temp = id.serial;
#endif
  delete this;
  MEMORY_TRACK(temp,"Recycled subproblem");
};


// Minimal implementation of setRoot() (to tell a problem it's a root).
// Typically called from a more elaborate, specific setRoot in a
// derived class.

void branchSub::setRoot()
{
  branchSubInit(bGlobal(),boundable);
  bound = bGlobal()->aPrioriBound();
  depth = 1;
  id.branchSubIdFromBranching(bGlobal());
  setRootComputation();
}


// Standard code shell around bound computation.
// The basic engine for all search techniques.

double branching::searchFramework(spHandler* handler_)
{
  // Miscellaneous initializations

  vout = valLogFile();                // Null if no validation log.
  valLogInit();

  hlog = (heurLog ? new ofstream("heur.log",ios::out) : NULL);

  double lastPrintTime = WallClockSeconds();
  int    lastPrint     = 0;

  nextOutputTime   = WallClockSeconds() + earlyOutputMinutes*60;

  // If no handler has passed in, create one according to standard
  // command line parameters.  If one was passed in, make sure its
  // global pointer gets linked back correctly to the branching object.

  if (handler_)
    {
      if (handler)
	delete handler;
      handler = handler_;
      handler->setGlobal(this);
    }

  // Preprocess problem, make a root subproblem, and put it in the pool.

  preprocess();
  {
    branchSub *tmp = makeRoot();
    pool->insert(tmp);
  }

  // Guess initial solution

  solution* guessSol = initialGuess();
  if (guessSol)
    {
      DEBUGPR(4,ucout << "Initial guess solution: value = " << guessSol->value 
	      << ", hash = " << guessSol->computeHashValue() 
	      << ':' << (guessSol->computeHashValue() % enumHashSize) << endl);
      if (enumerating)
	offerToRepository(guessSol);
      incumbent = guessSol;
      incumbentValue = incumbent->value;
    }

  MEMORY_BASELINE;

  startLoadLogIfNeeded();

  // The main loop -- process problems until the pool is empty.

  while (haveCurrentSP() || (pool->size() > 0))
    {
      DEBUGPR(20,ucout << "Load is " << load() << '\n');

      if (!haveCurrentSP())
	{ 
	  loadCurrentSP( pool->remove() );
	}
      DEBUGPR(20,ucout << "Load is " << load() << '\n');

      if (currentSP->canFathom())
	eraseCurrentSP();
      else
	{
	  DEBUGPR(5, ucout << "Executing " << currentSP->id.creatingProcessor 
		  << ":" << currentSP->id.serial 
		  << " bound=" << currentSP->bound 
		  << " state=" << currentSP->state 
		  << " depth=" << currentSP->depth << "\n");
	  handler->execute();
	  if (haveCurrentSP() && !(currentSP->forceStayCurrent()))
	    unloadCurrentSPtoPool();
	  if (needPruning)
	    {
	      DEBUGPR(20,ucout << "Pruning.\n");
	      pool->prune();
	      if (enumerating)
		pruneRepository();
	      needPruning = false;
	    }
	}

      if ((earlyOutputMinutes > 0) && serialNeedEarlyOutput())
	{
	  directSolutionToFile();
	  recordEarlyOutput(incumbentValue);
	}

      statusPrint(lastPrint,lastPrintTime);

      recordLoadLogIfNeeded();

      DEBUGPR(2000,
	      ucout << "Pool scan: \n";
	      int s = pool->size();
	      pool->resetScan();
	      for(int i=0; i<s; i++)
	      ucout << "  " << pool->scan() << endl;);

      if (shouldAbort(subCount[beingBounded]))
	{
	  if (haveCurrentSP())
	    unloadCurrentSPtoPool();
	  loadObject l = updatedLoad();
	  printAbortStatistics(l);
	  pool->clear();             // This will terminate the loop.
	}

    }

  // Clean up, record time, and close log files.

  searchTime = CPUSeconds() - startTime;

  finishLoadLogIfNeeded();

  if (valLogOutput())
    {
      valLogFathomPrint();
      delete vout;
    }
  if (hlog)
    delete hlog;

  return incumbentValue;
}


// Abort stuff

bool branching::shouldAbort(int boundCount)
{
  if ((maxSPBounds > 0) && 
      (boundCount > maxSPBounds))
    {
      if (!abortReason)
	abortReason = "too many nodes bounded";
      return true;
    }
  if ((maxCPUMinutes > 0) && 
      (CPUSeconds() - startTime > maxCPUMinutes*60))
    {
      if (!abortReason)
	abortReason = "too much CPU time";
      return true;
    }
  if ((maxWallMinutes > 0) && 
      (WallClockSeconds() - startWall > maxWallMinutes*60))
    {
      if (!abortReason)
	abortReason = "too much wall clock time";
      return true;
    }
  if ((haltOnIncumbent == true) &&
      (haveIncumbent() == true))
    {
      if (!abortReason)
	abortReason = "incumbent was generated";
      return true;
    }
  return false;
}


void branching::printAbortStatistics(loadObject& load)
{
  if (!printAbortMessage)
    return;
  ucout << "\nABORTED: " << abortReason << endl << endl;
  ucout << load.boundedSPs << " subproblem bounds started\n";
  ucout << load.count() << " subproblems still in pool\n";
  if (haveIncumbent())
    {
      ucout << "Best solution value: " << incumbentValue << endl;
      if (!load.boundUnknown())
	{
	  ucout << "Bound: " << load.aggregateBound << "\n";
	  ucout << "Gap: " << absGap(load.aggregateBound);
	  ios::fmtflags defaultFlags = ucout.setf(ios::fixed,ios::floatfield);
	  int oldPrec = ucout.precision(3);
	  ucout << " (" << 100*relGap(load.aggregateBound) << "%)\n";
	  ucout.precision(oldPrec);
	  ucout.setf(defaultFlags,ios::floatfield);
	}
    }
  else
    ucout << "No solution found\n";
}


// Handler stuff...

void spHandler::heuristic()
{
  if (global->haveIncumbentHeuristic() && !p->candidateSolution())
    p->incumbentHeuristic();
}

// This is called if we try to release a subproblem to another processor, 
// but we're running in serial.

void spHandler::releaseChild()
{
  EXCEPTION_MNGR(runtime_error, 
		 "Tried to release a child subproblem from serial code");
}


// This is a common pattern in the lazy and hybrid handlers

void spHandler::spinOffChild()
{
  if (shouldRelease())    // This will always be false in serial.
    releaseChild();       // Will only execute in parallel versions.
  else 
    {
      getChild();
      if (c->canFathom())
	eraseChild();
      else
	insertChild();
    }
}


// Handler for "lazy" search technique.  
// Split every subproblem as soon as it is bounded, extract all children,
// and put them in the pool.

void lazyHandler::execute()
{
  setProblem();

  DEBUGPR(50,ucout << "Entering lazy handler for " << p << '\n');
  
  switch(state())
    {
    case boundable:
      
    case beingBounded:

      computeBound();
      if (canFathom())
	{
	  erase();
	  break;
	}
      if (state() == beingBounded)
	break;
      
    case bounded:

      heuristic();
     
    case beingSeparated:

      split();
      if (canFathom())
	{
	  erase();
	  break;
	}
      if (state() == beingSeparated)
	break;

    case separated:

      heuristic();

      for (int childCount = p->childrenLeft; 
	   (childCount > 0) && !canFathom();
	   childCount--)
	spinOffChild();

    case dead:

      erase();
      break;

    default:

      EXCEPTION_MNGR(runtime_error, 
		     "Lazy search encountered unknown state " << state());

    }

  DEBUGPR(50,ucout << "Exiting lazy handler.\n");

}



// Handler for "hybrid" search technique, which is the most natural for This
// subproblem representation.  Whenever you pick up a subproblem, just try 
// to advance it one state or pull out one child.

void hybridHandler::execute()
{
  setProblem();
  
  DEBUGPR(50,ucout << "Entering hybrid handler for " << p << '\n');
  
  switch(state())
    {
    case boundable:
      
    case beingBounded:

      computeBound();
      if (canFathom())
	{
	  erase();
	}
      else if (state() == bounded)
	heuristic();
      break;
     
    case bounded:

    case beingSeparated:

      split();
      if (canFathom())
	erase();
      else if (state() == separated)
	heuristic();
      break;

    case separated:

      // Pull out children until the parent's bound changes.
      // This should result in sensible things happening without
      // having to use the softReposition method, which we don't 
      // use any more.
      
      {
	double startBound = p->bound;
	while (stillNeedSP() && (p->bound == startBound))
	  spinOffChild();
      }

      if (stillNeedSP())
	break;

    case dead:

      erase();
      break;

    default:

      EXCEPTION_MNGR(runtime_error, 
		     "Hybrid search encountered unknown state " << state());

    }

  DEBUGPR(50,ucout << "Exiting hybrid handler.\n");
 
}


// Handler for "eager" search.  As soon as a bounded problem comes out
// of the pool, expand all its subproblems.

void eagerHandler::execute()
{
  setProblem();
  
   DEBUGPR(50,ucout << "Entering eager handler for " << p << '\n');
  
   switch(state())
    {
    case boundable:               // Actually, should never come here...
      
    case beingBounded:

      setToBoundCurrent();
      if (boundSubHandler())
	{
	  erase();
	  break;
	}
      
    case bounded:

      heuristic();
     
    case beingSeparated:

      split();
      if (canFathom())
	{
	  erase();
	  break;
	}
      if (state() == beingSeparated)
	break;
      heuristic();

    case separated:

      while((p->childrenLeft > 0) && !canFathom())
	{
	  getChild();
	  setToBoundChild();
	  if (boundSubHandler())
	    eraseChild();
	  else
	    insertChild();
	}

    case dead:

      erase();
      break;

    default:

      EXCEPTION_MNGR(runtime_error, "Eager search encountered unknown state " << state());

    }

   DEBUGPR(50,ucout << "Exiting eager handler.\n");

}


// Routines for loading and unloading subproblems

void branching::loadCurrentSP(branchSub* p)
{
#ifdef ACRO_VALIDATING
  if (haveCurrentSP())
    EXCEPTION_MNGR(runtime_error, "Attempt to load a subproblem when one is already loaded");
  DEBUGPR(100,ucout << "Loading " << p << '\n');
#endif
  currentSP   = p;
  currentSPId = p->id;
  p->makeCurrentEffect();
}


void branching::unloadCurrentSP()
{
#ifdef ACRO_VALIDATING
  if (!haveCurrentSP())
    EXCEPTION_MNGR(runtime_error, "Tried to unload current subproblem when none existed");
  DEBUGPR(100,ucout << "Unloading " << currentSP << '\n');
#endif
  previousSPId = currentSPId;
  currentSP->noLongerCurrentEffect();
  currentSP = NULL;
}


void branching::eraseCurrentSP()
{
  branchSub* p = currentSP;
  unloadCurrentSP();
  p->recycle();
}


// loadObject functions that take the current subproblem into account

loadObject branching::load()
{
  loadObject l = pool->load();
  if (haveCurrentSP())
    l += *currentSP;

  return l;
};


loadObject branching::updatedLoad()
{
  loadObject l = pool->updatedLoad();
  if (haveCurrentSP())
    l += *currentSP;

  return l;
};


// To print status lines.

void branching::statusPrint(int&        lastPrint,
			    double&     lastPrintTime,
			    const char* tag) 
{
  loadObject l = updatedLoad();
  statusPrint(lastPrint,lastPrintTime,l,tag);
}


void branching::statusPrint(int&        lastPrint,
			    double&     lastPrintTime,
			    loadObject& l,
			    const char* tag)
{
  DEBUGPR(400,ucout << "statusPrint: count=" << l.boundedSPs
	  << ", last=" << lastPrint << (l.mismatch() ? ", mismatch" : "")
	  << ".\n");

  int needPrint = FALSE;

  if ((statusPrintCount > 0) &&
      (l.boundedSPs >= lastPrint + statusPrintCount))
    needPrint = TRUE;

  double now = WallClockSeconds();
  if ((statusPrintSeconds > 0) &&
      (now >= lastPrintTime + statusPrintSeconds))
    needPrint = TRUE;

  if (needPrint)
    {
#ifdef MEMUTIL_PRESENT
      l.setMemory();
#endif
      if (enumerating)
	adjustReposStats(l);
      statusLine(l,tag);
      lastPrint     = l.boundedSPs;
      lastPrintTime = now;
    }

  // Mikhail's heuristic log stuff

  if (hlog && (l.boundedSPs > lastHeurPrint)) 
    {
      int oldPrecision = ucout.precision();
      *hlog << l.boundedSPs;
      *hlog << " " << CPUSeconds() - startTime;
	hlog->precision(statusLinePrecision);
      *hlog << " " << l.incumbentValue;
      if (!l.boundUnknown() &&
	  (l.count() > 0) &&
	  (sense*l.aggregateBound < MAXDOUBLE))
	{
	  *hlog << " " << l.aggregateBound;
	  hlog->setf(ios::fixed,ios::floatfield);
	  hlog->precision(3);
	  *hlog << " " << 100*relGap(l.aggregateBound);
	  hlog->setf((ios::fmtflags)0,ios::floatfield);
	}
      else
	{
	  *hlog << " " << sense * MAXDOUBLE;
	  *hlog << " 100.000";
	}
      hlog->precision(oldPrecision);
      *hlog << endl;
      lastHeurPrint = l.boundedSPs;
    }
}



void branching::statusLine(loadObject& l, const char* tag) 
{
  int oldPrecision = ucout.precision();
  ucout << tag << '#' << l.boundedSPs;
  ucout << " pool=" << l.count();
  if (l.countIncomplete())
    ucout << '+';
  else
    ucout << ' ';
#ifdef MEMUTIL_PRESENT
  {
    ios::fmtflags defaultFlags = ucout.setf(ios::fixed,ios::floatfield);
    ucout.precision(2);
    ucout << ' ' << l.memUsed << "MB";
    ucout.setf(defaultFlags,ios::floatfield);
  }
#endif
  ucout.precision(statusLinePrecision);
  ucout << " inc=";
  if (l.incumbentValue == sense*MAXDOUBLE)
    ucout << "(none)";
  else
    {
      ucout << l.incumbentValue;
      if (l.mismatch())
	ucout << "(x)";
      if (enumerating && (l.repositorySize > 1))
	{
	  ucout << "<-" << l.repositorySize << "->" << l.worstInRepos;
	  if (l.fathomMismatch())
	    ucout << "(x)";
	}
    }
  if (!l.boundUnknown() && 
      (l.count() > 0) && 
      (sense*l.aggregateBound < MAXDOUBLE))
    {
      ucout << " bnd=" << l.aggregateBound;
      {
#if 0
	long defaultFlags = ucout.setf(ios::fixed,ios::floatfield);
#else
	ios::fmtflags defaultFlags = ucout.setf(ios::fixed,ios::floatfield);
#endif
	ucout.precision(3);
	ucout << " gap=" << 100*relGap(l.aggregateBound) << '%';
	ucout.setf(defaultFlags,ios::floatfield);
      }
    }
  ucout.precision(oldPrecision);
  ucout << '\n' << Flush;
}


int branching::serialNeedEarlyOutput()
{
  if (WallClockSeconds() < nextOutputTime)
    return FALSE;
  return sense*(incumbentValue - lastSolValOutput) < 0;
}


void branching::recordEarlyOutput(double outputVal)
{
  double now = WallClockSeconds();
  DEBUGPR(1,ucout << "Early output completed: inc=" << outputVal
	  << " time=" << now << endl);
  nextOutputTime   = now + earlyOutputMinutes*60;
  lastSolValOutput = outputVal;
}


void branching::solutionToFile()
{
  // If we are enumerating, loop through all solutions and print them

  if (enumerating)
    {
      ostream* outStreamP = openSolutionFile();
      outStreamP->precision(statusLinePrecision);
      BasicArray<solution*> solArray;
      sortRepository(solArray);
      size_type n = solArray.size();
      *outStreamP << "******** " << n << " solution" << plural(n)
		  << " found ********\n";
      for(size_type i=0; i<n; i++)
	{
	  *outStreamP << "\n\n******** Solution " << i+1 << " ********\n";
	  solArray[i]->print(*outStreamP);
	}
      closeSolutionFile(outStreamP);
      return;
    }

  // Now we know we are not enumerating...

  // If we already wrote the solution, do nothing.

  if ((earlyOutputMinutes > 0) && (lastSolValOutput == incumbentValue))
    return;

  // Otherwise, write it.

  directSolutionToFile();
}


void branching::directSolutionToFile()
{
  ostream* outStreamP = openSolutionFile();
  serialPrintSolution("","",*outStreamP);
  closeSolutionFile(outStreamP);
}


ostream* branching::openSolutionFile()
{
  DEBUGPR(50,ucout << "Opening solution file\n");
  ofstream* toReturn = new ofstream("temp-sol.txt",ios::out);
  if (toReturn->bad())
    ucout << "****** Warning ******** could not open solution file.\n";
  return toReturn;
}


void branching::closeSolutionFile(ostream* fileStream)
{
  delete fileStream;
  if (solFileName == "") {
      if (problemName == "")
         solFileName += "unknown";
      else
         solFileName += problemName;
      solFileName += ".sol.txt";
  }
  int delcode = remove(solFileName.c_str());     // Required for Windows OS
  utilib::move_file("temp-sol.txt",solFileName.c_str());
  DEBUGPR(50,ucout << "Closed solution file " << solFileName << endl);
}


void branching::printSPStatistics(ostream& stream)
{
  printSPTable(stream,subCount,17);
}


void branching::printTimings(ostream& stream)
{
  int oldPrecision = stream.precision(1);
  stream.setf(ios::fixed,ios::floatfield);
  stream << "CPU run time          = " << searchTime << " seconds" << endl;
  stream << "CPU total time        = " << (CPUSeconds() - startTime) 
	 << " seconds" << endl;
  stream << "Wall clock total time = " << (WallClockSeconds() - startWall) 
	 << " seconds" << endl;
  stream.unsetf(ios::floatfield);
  stream.precision(oldPrecision);

  if (printSpTimes)
    printSpTimeStats(stream);
}


void branching::printSpTimeStats(ostream& stream)
{
  stream << endl;
  int oldPrec = stream.precision(8);
  double mean = boundCompTime/boundCompCalls;
  stream << "Proc " << uMPI::rank 
	 << " : " << boundCompCalls << " boundComputation calls, mean = "
	 << mean << ", SD = "
	 << sqrt(boundCompTimeSq/boundCompCalls - mean*mean)
	 << endl;
  mean = splitCompTime/splitCompCalls;
  stream << "Proc " << uMPI::rank 
	 << " : " << splitCompCalls << " splitComputation calls, mean = "
	 << mean << ", SD = "
	 << sqrt(splitCompTimeSq/splitCompCalls - mean*mean)
	 << endl;
  stream.precision(oldPrec);
}



void branching::serialPrintSolValue(std::ostream& stream)
{
  int oldPrecision = stream.precision(20);
  stream << endl << "Best Solution:  Value = " << incumbentValue << endl;
  stream.precision(oldPrecision);
  stream << endl;
}


void branching::printAllStatistics(ostream& stream)
{
  printSPStatistics(stream);
  if (enumerating)
    printReposStatistics(stream);
  printTimings(stream);
}


void branching::printReposCommon(ostream& stream,
				 int size,
				 int offer,
				 int admit,
				 double worstVal,
				 int comm)
{
  stream << size << " solution" << plural(size)
	 << " in repository\n";
  stream << offer << " solutions were offered to the repository\n";
  stream << admit << " solutions were admitted to the repository\n";
#ifdef ACRO_HAVE_MPI
  if (comm >= 0)
    stream << comm << " solutions were sent between processors\n";
#endif
  int oldPrecision = stream.precision(1);
  stream.setf(ios::fixed,ios::floatfield);
  stream << "Repository admission rate " << (100.0*admit)/offer 
	 << "%, retention rate " << (100.0*size)/admit 
	 << "%, combined " << (100.0*size)/offer << "%\n";
#ifdef ACRO_HAVE_MPI
  if (comm >= 0)
    stream << (100.0*comm)/offer << "% solution communication rate\n";
#endif
  stream.unsetf(ios::floatfield);
  stream.precision(oldPrecision);
  stream << "\nBest  solution in repository has value " 
	 << incumbentValue << endl;
  stream << "Worst solution in repository has value "
	 << worstVal << endl;
  stream << endl;
}


void branching::printReposStatistics(ostream& stream)
{
  stream << endl;
  printReposCommon(stream,
		   repositorySize(),
		   solsOffered,
		   solsAdmitted,
		   worstReposValue());
}


void branching::printSPTable(ostream& stream,
			     int* table,
			     int nameWidth)
{
  stream << "Subproblems\n";
  hyphens(stream,11) << '\n';
  int numWidth = digitsNeededFor(table[boundable]);
  for(int stateCode = boundable; stateCode < numStates; stateCode++)
    printSPLine(stream,
		table[stateCode],
		stateString(stateCode),
		table[boundable],
		nameWidth,
		numWidth);
  stream << endl;
}


void branching::printSPLine(ostream&    stream,
			    int         n,
			    const char* name,
			    int         denominator,
			    int         nameWidth,
			    int         numWidth)
{
  stream.width(nameWidth);
  stream.setf(ios::left,ios::adjustfield);
  stream << name;
  stream.setf(ios::right,ios::adjustfield);
  stream << "  ";
  stream.width(numWidth);
  stream << n << "  ";
  printPercent(stream,n,denominator) << '\n';
}


//  Validation Log Code.

ostream* branching::valLogFile()
{
  if (validateLog)
    return new ofstream("val00000.log",ios::out);
  else
    return 0;
}


void branching::valLogInit()
{
  if (!vout)
    return;
  vout->setf(ios::scientific,ios::floatfield);  // Output bounds at very
  vout->precision(20);                          // high precision.
}


void branching::valLogFathomPrint()
{
  *vout << "fathoming " << incumbentValue << ' ' << (int) sense << ' ';
  *vout << relTolerance << ' ' << absTolerance << '\n';
}


bool branching::checkParameters(char* progName)
{
  if (version_flag) {
    ucout << endl << branching::version_info << endl << endl;
    return false;
  }
  if (help_parameter)
    {
      write_usage_info(progName,cout);
      return false;
    }
  if (debug_solver_params) 
    {
      ucout << "---- PEBBL Parameters ----" << endl;
      write_parameter_values(ucout);
      ucout << endl << utilib::Flush;
    }
  if (use_abort) 
    {
      utilib::exception_mngr::set_mode(utilib::exception_mngr::Abort);
      // utilib::exception_mngr::set_exit_function(&exit_fn);
      signal(SIGFPE,pebbl_abort_handler);
    }
  return true;
}


void branching::write_usage_info(char* progName,std::ostream& os) const
{
  writeCommandUsage(progName,os);
  os << endl;
  plist.write_registered_parameters(os);
  os << endl;
}


void branching::writeCommandUsage(char* progName,std::ostream& os) const
{
  os << "\nUsage: " << progName 
     << " { --parameter=value ... }";
  if (min_num_required_args == 1)
    os << " <problem data file>";
  else if (min_num_required_args == 1)
    os << " <" << min_num_required_args << " problem data files>";
  os << endl;
}


void branching::solve()
{
  double run_start = CPUSeconds();
  search();
  searchTime = CPUSeconds() - run_start;

  //
  // Final I/O
  //

  printSolValue(ucout);
  if (printFullSolution)
    printSolution("","\n",ucout);

  printAllStatistics();
  ucout << endl;

  solutionToFile();

  if (abortReason)
    ucout << "RUN ABORTED: " << abortReason << endl << endl;
}


void branchSub::valLogWriteID(char separator)
{
  *vout << valLogProc() << ' ' << id.serial;
  if (separator)
    *vout << separator;
}


void branchSub::valLogWriteBound(char separator)
{
  if ((state == dead) && !(bGlobal()->canFathom(bound)))
    *vout << bGlobal()->sense*MAXDOUBLE;
  else
    *vout << bound;
  if (separator)
    *vout << separator;
}


void branchSub::valLogCreatePrint(branchSub* parent)
{
  *vout << "create ";
  valLogWriteID(' ');
  valLogWriteBound(' ');
  if (parent)
    parent->valLogWriteID();
  else
    *vout << "-1 -1";
  valLogCreateExtra(parent);
  *vout << endl;
}


void branchSub::valLogBoundPrint()
{
  *vout << "bound ";
  valLogWriteID(' ');
  valLogWriteBound();
  valLogBoundExtra();
  *vout << endl;
}


void branchSub::valLogSplitPrint()
{
  *vout << "split ";
  valLogWriteID(' ');
  *vout << totalChildren << ' ';
  valLogWriteBound();
  valLogSplitExtra();
  *vout << endl;
}


void branchSub::valLogDestroyPrint()
{
  *vout << "destroy ";
  valLogWriteID(' ');
  *vout << childrenLeft << ' ';
  valLogWriteBound();
  valLogDestroyExtra();
  *vout << endl;
}


//  To help with debug printing


void coreSPInfo::debugPrint(ostream& s) const
{
  int oldPrecision = 0;
  if (bGlobal()->debugPrecision > 0)
    oldPrecision = s.precision(bGlobal()->debugPrecision);
  s << id << " " << bound << " (" << stateString(state) << ')';
  if (bGlobal()->printIntMeasure)
    s << "<im=" << integralityMeasure << '>';
  if (bGlobal()->printDepth)
    s << "<d=" << depth << '>';
  if (bGlobal()->debugPrecision > 0)
    s.precision(oldPrecision);
}

void coreSPInfo::CSPInitfromBranching(branching *global)
{
  //initialize
id.branchSubIdFromBranching(global);
// Integrality measure set in (empty) constructor
}

void coreSPInfo::CPSInitAsCopy(coreSPInfo& toCopy)
{
id = toCopy.id;
bound = toCopy.bound;
integralityMeasure = toCopy.integralityMeasure;
state = toCopy.state;
depth = toCopy.depth;
}

branchSub::branchSub() 
{ 
  poolPtr = 0; 
  childrenLeft = 1;
  state = boundable;
};
  

void branchSub::branchSubAsChildOf(branchSub* parent)
{
  CSPInitfromBranching(parent->bGlobal());
  branchSubInit(parent->bGlobal(),boundable);
  bound = parent->bound;
  integralityMeasure = parent->integralityMeasure;
  depth = parent->depth + 1;
};


// Basic methods for the solutionIdentifier class

solutionIdentifier::solutionIdentifier()
{
  serial = -1;
  sense  = minimization;
#ifdef ACRO_HAVE_MPI
  owningProcessor = -1;
#endif
}


solutionIdentifier::solutionIdentifier(branching* bGlobal)
{
  serial = -1;
  sense  = bGlobal->sense;
#ifdef ACRO_HAVE_MPI
  owningProcessor = uMPI::rank;
#endif
}


solutionIdentifier::solutionIdentifier(solutionIdentifier* toCopy)
{
  copy(toCopy);
}


void solutionIdentifier::copy(solutionIdentifier* toCopy)
{
  value  = toCopy->value;
  serial = toCopy->serial;
  sense  = toCopy->sense;
#ifdef ACRO_HAVE_MPI
  owningProcessor = toCopy->owningProcessor;
#endif
}


solution::solution(branching* bGlobal) : 
  solutionIdentifier(bGlobal),
  typeId(0),
  hashValue(0),
  hashComputed(false)
{
  serial = ++(bGlobal->solSerialCounter);
};


void solution::creationStamp(branching* bGlobal,int typeId_)
{
  serial = ++(bGlobal->solSerialCounter);
#ifdef ACRO_HAVE_MPI
  owningProcessor = uMPI::rank;
#endif
  typeId       = typeId_;
  hashValue    = 0;
  hashComputed = false;
}


void solutionIdentifier::print(ostream& s)
{
  s << "Objective value = " << value;
  s << ", serial = " << serial;
#ifdef ACRO_HAVE_MPI
  s << ", owning processor = " << owningProcessor;
#endif
}


void solutionIdentifier::setWorstPossible()
{
  value  = sense*MAXDOUBLE;
  serial = MAXINT;
#ifdef ACRO_HAVE_MPI
  owningProcessor = uMPI::size;
#endif
}


void solutionIdentifier::setWorstPossible(optimType sense_)
{
  sense = sense_;
  setWorstPossible();
}


#ifdef ACRO_HAVE_MPI

void solutionIdentifier::pack(PackBuffer& outBuf) const
{
  outBuf << value << serial << (int) sense << owningProcessor;
}


void solutionIdentifier::unpack(UnPackBuffer& inBuf)
{
  inBuf >> value;
  inBuf >> serial;
  int temp = 0;
  inBuf >> temp;
  sense = (optimType) temp;
  inBuf >> owningProcessor;
}


int solutionIdentifier::packSize()
{
  return sizeof(double) + 3*sizeof(int);
}

#endif


// Operator to compare two solutions or solutionIdentifiers

int solutionIdentifier::compare(const solutionIdentifier& other) const
{
#ifdef ACRO_VALIDATING
  if (sense != other.sense)
    EXCEPTION_MNGR(runtime_error,"Comparing two solutionIdentifiers with"
		   " different senses.");
#endif
  double valDiff = value - other.value;
  if (valDiff != 0)
    return sense*sgn(valDiff);
  int serialDiff = serial - other.serial;
  if (serialDiff != 0)
    return sgn(serialDiff);
#ifdef ACRO_HAVE_MPI
  return sgn(owningProcessor - other.owningProcessor);
#else
  return 0;
#endif
}


void solutionIdentifier::write(std::ostream& os)
{
#ifdef ACRO_HAVE_MPI
  os << owningProcessor << ':';
#endif
  os << serial << ':' << value;
}


void solution::print(ostream& s)
{
  s << typeDescription() << ':' << endl;
  s << "Objective value = " << value << endl;
  printContents(s);
}


solution::solution(solution* toCopy)
{
  copy(toCopy);
}


void solution::copy(solution* toCopy)
{
  solutionIdentifier::copy(toCopy);

  typeId       = toCopy->typeId;
  hashValue    = toCopy->hashValue;
  hashComputed = toCopy->hashComputed;
}


// To compute the hash value of a solution object from its sequence
// representation

size_type solution::computeHashValue()
{
  if (!hashComputed)
    {
      size_type len = sequenceLength();
      sequenceReset();
      hashValue = 0;
      for(size_type i=0; i<len; i++)
	hashValue = hash_bj(sequenceData(),hashValue);
      hashComputed = true;
    }
  return hashValue;
}


// To check if two solutions are identical

bool solution::duplicateOf(solution& other)
{
  if (computeHashValue() != other.computeHashValue())
    return false;

  size_type len = sequenceLength();

  if (other.sequenceLength() != len)
    return false;

  sequenceReset();
  other.sequenceReset();
  for(size_type i=0; i<len; i++)
    if (sequenceData() != other.sequenceData())
      return false;

  return true;
}


// (Enumeration) Methods to return the worst solution in the
// repository, or its value.

solution* branching::worstReposSol()
{
  if (repositorySize() == 0)
    return NULL;
  return &(reposHeap.top()->key());
}


double branching::worstReposValue()
{
  if (repositorySize() == 0)
    return -sense*MAXDOUBLE;
  return reposHeap.top()->key().value;
}


// Try to put a solution in the repository.  If it is not good enough
// to enter the repository or duplicates something already there,
// delete it instead.  Returns a status of "true" if the entry was
// inserted into the repository, and "false" if it was deleted.  

bool branching::localReposOffer(solution* sol)
{
  DEBUGPR(20, ucout << "branching::localReposOffer " << sol << endl);

  if (sol == NULL)
    return false;

  if (canFathom(sol->value))
    {
      DEBUGPR(20,"branching::localReposOffer: solution fathomed\n");
      delete sol;
      return false;
    }

  // Check for a duplicate already in the repository.  Find the list
  // in the hash table that would have to hold a possible duplicate.

  size_type solHash   = sol->computeHashValue();
  size_type whichList = solHash % enumHashSize;

  DEBUGPR(20,ucout << "Searching list " << whichList << endl);

  // Grab the head of the list and scan the list for a duplicate

  ListItem<solution*>* cursor = reposTable[whichList].head();

  while(cursor)
    {
      DEBUGPR(100,ucout << "Checking against " << cursor 
	      << "->" << (void*) cursor->Data 
	      << "=" << cursor->Data 
	      << " hash=" << cursor->Data->hashValue << endl);
      solution* cursorSol  = cursor->Data;
      size_type cursorHash = cursorSol->hashValue;

      // The list is in hashValue order, so we can give up if the hash
      // is already too large.  Otherwise, check for a duplicate.

      if (cursorHash > solHash)
	break;
      else if ((cursorHash == solHash) && cursorSol->duplicateOf(*sol))
	{
	  DEBUGPR(20,ucout << "localReposOffer: duplicate of "
		  << cursorSol << endl);
	  delete sol;
	  return false;
	}

      // Move on to next element in list

      DEBUGPR(200,ucout << "No match\n");
      cursor = reposTable[whichList].next(cursor);
    }

  DEBUGPR(10,ucout << "Admitted to repository\n");
  solsAdmitted++;

  // At this point, cursor is either NULL, indicating that we scanned
  // the whole ist and found no duplicates, or points to a list item
  // right after where the new solution should go in the list.  In
  // either case, the list "add" operation should insert it in the
  // right place (either the end or the middle, respectively).
  // Remember where we put the solution in the list, so we don't have
  // to scan when we delete.

  sol->hashItem = reposTable[whichList].add(sol,cursor);

  // The solution also needs to go into the heap.  If enumCount is
  // being used and the repository is full, this involves swapping out
  // and deleting the worse entry in the repository.  Otherwise, we
  // just insert into the heap.  Note that this case is very unlikely
  // in parallel, where each processor will normally have only about
  // 1/p-th of the solutions in the repository.

  if ((enumCount > 0) && (repositorySize() == enumCount))
    {
      GenericHeapItem<solution>* oldTop = reposHeap.swapTop(*sol);
      solution* oldTopSol = &(oldTop->key());
      DEBUGPR(10,ucout << "Pushes out " << oldTopSol << endl);
      size_type oldWhichList = oldTopSol->hashValue % enumHashSize;
      reposTable[oldWhichList].remove(oldTopSol->hashItem);
      deleteSolution(oldTopSol);
      delete oldTop;
    }
  else
    reposHeap.add(*sol);

  DEBUGPR(10,ucout << "(Local) repository size now "
	  << repositorySize() << ", worst value "
	  << worstReposValue() << endl);

  return true;
}


// Main serial version of offerToRepository.  This is overridden in
// the parallel layer.  This one just does a localReposOffer, and
// updates lastSolId if necessary.  The "sync" argument is needed only
// when all of the following hold: we are in parallel layer, we are
// enumerating multiple solutions, and we are in the ramp-up phase --
// therefore it is used only in the overridden parallel version of this
// routine.

bool branching::offerToRepository(solution* sol,syncType sync)
{
  DEBUGPR(20, ucout << "branching::offerToRepository " << sol << endl);

  solsOffered++;

  bool accepted = localReposOffer(sol);

  if (accepted && (enumCount > 1) && (repositorySize() == enumCount))
    {
      double oldPruneValue = lastSolId.value;
      updateLastSolId(worstReposSol());
      if (lastSolId.value != oldPruneValue)
	needPruning = true;
    }

  return accepted;
}


// Prune the repository -- used when a better overall incumbent is
// found; in parallel, should also (eventually) be used when enumCount
// is active and other processors find better sub-optimal solutions

void branching::pruneRepository()
{
  DEBUGPR(10,ucout << "Pruning repository\n");
  while((repositorySize() > 0) && canFathomFromRepository(worstReposSol()))
    deleteSolution(removeWorstInRepos());
}


// Clear out the repository -- used in reset method and destructor

void branching::clearRepository()
{
  DEBUGPR(50,ucout << "Emptying repository\n");
  while(repositorySize() > 0)
    deleteSolution(removeWorstInRepos());
}


// Delete a single solution.  In some unusual situations, it might
// also be the incumbent.  In that case, set the incumbent to a null
// pointer to prevent double deletion.

void branching::deleteSolution(solution* sol)
{
  DEBUGPR(100,ucout << "Deleting solution " << sol
	  << ((sol == incumbent) ? " and setting incumbent=NULL" : "")
	  << endl);
  if (sol == incumbent)
    incumbent = NULL;
  delete sol;
};


// Similar to fathoming function, but for solutions in the repository.
// Is only used when enumerating.

bool branching::canFathomFromRepository(solutionIdentifier* solPtr)
{
  double gap   = absGap(solPtr->value);
  double denom = gapDenom(solPtr->value);

  // Usual tolerance tests

  if ((enumAbsTol >= 0) && (gap < -enumAbsTol)) 
    {
      DEBUGPR(1000, ucout << "canFathomFromRepository - Fathoming with "
	      "enumAbsTol! gap = " << gap 
	      << " < -enumAbsTol = " 
	      << (-enumAbsTol) << endl);
      return true;
    }

  if ((enumRelTol >= 0) && (gap < -enumRelTol*denom)) 
    {
      DEBUGPR(1000, ucout << "canFathomFromRepository - Fathoming "
	      << "with enumRelTol! gap = " << gap 
	      << " <= -enumRelTol*denom = " 
	      << (-enumRelTol*denom) << endl);
      return true;
    }

  // If enumCount is active, compare to lastSolId.  If strictly worse
  // than lastSolId, it can be tossed from the repository (note that
  // this situation won't occur in serial because such solutions would
  // have been tossed out in offerToRepsitory; in parallel, however,
  // lastSolId could come from another processor and have a lower
  // value than you could derive locally.

  return(enumCount > 1) && (lastSolId.compare(solPtr) < 0);

}


// Remove the worst solution in the repository and return a pointer to it.

solution* branching::removeWorstInRepos()
{
  GenericHeapItem<solution>* oldTop = reposHeap.top();
  solution* oldTopSol = &(oldTop->key());
  bool status = true;
  reposHeap.remove(oldTop,status);
#ifdef ACRO_VALIDATING
  if (!status)
    EXCEPTION_MNGR(runtime_error,"GenericHeap::remove returned false status"
		   " to branching::removeWorstInRepos");
#endif
  size_type whichList = oldTopSol->hashValue % enumHashSize;
  reposTable[whichList].remove(oldTopSol->hashItem);
  return oldTopSol;
}


// Update lastSolId, which tracks the worst stored solution in the repository

bool branching::updateLastSolId(solutionIdentifier* sol)
{
  DEBUGPR(100,ucout << "Update check of lastSolId=" << &lastSolId
	  << " from " << sol << endl);
  if (lastSolId.compare(*sol) > 0)
    {
      lastSolId.copy(sol);
      DEBUGPR(10,ucout << "lastSolId updated to " << &lastSolId << endl);
      return true;
    }
  return false;
}


// Create a sorted array of solution pointers from the repository,
// best solution first.

void branching::sortRepository(BasicArray<solution*>& solArray)
{
  size_type n = repositorySize();
  solArray.resize(n);

  if (n == 0)
    return;

  // Copy the repository heap, so we don't destroy the original

  reposSolHeap heapCopy(reposHeap);

  DEBUGPR(200,ucout << "Copied heap:\n" << reposHeap << endl);

  // Pull things out of the heap copy one-by-one.  Since the heap is
  // in reverse order, put them in the array from the end, working
  // backwards.

  while(n > 0)
    {
      GenericHeapItem<solution>* topItem = heapCopy.top();
      solArray[--n] = &(topItem->key());
      bool status = true;
      heapCopy.remove(topItem,status);
#ifdef ACRO_VALIDATING
      if (!status)
	EXCEPTION_MNGR(runtime_error,"GenericHeap::remove returned false "
		       " status to branching::sortRepository");
#endif
    }
}


// Create a sorted array of solution pointers from the repository,
// best solution first.

void branching::sortReposIds(BasicArray<solutionIdentifier>& result)
{
  size_type n = repositorySize();
  result.resize(n);

  if (n == 0)
    return;

  // Copy the repository heap, so we don't destroy the original

  reposSolHeap heapCopy(reposHeap);

  DEBUGPR(200,ucout << "Copied heap:\n" << reposHeap << endl);

  // Pull things out of the heap copy one-by-one.  Since the heap is
  // in reverse order, put them in the array from the end, working
  // backwards.

  while(n > 0)
    {
      GenericHeapItem<solution>* topItem = heapCopy.top();
      result[--n].copy(&(topItem->key()));
      bool status = true;
      heapCopy.remove(topItem,status);
#ifdef ACRO_VALIDATING
      if (!status)
	EXCEPTION_MNGR(runtime_error,"GenericHeap::remove returned false "
		       " status to branching::sortReposIds");
#endif
    }
}


//  Return all solutions; whichProcessor argument ignored in serial

void branching::getAllSolutions(BasicArray<solution*>& solArray,
				int /*whichProcessor*/)
{
  if (enumerating)
    sortRepository(solArray);
  else 
    {
      if (incumbent)
	{
	  solArray.resize(1);
	  solArray[0] = incumbent;
	}
      else
	solArray.resize(0);
    }
}


// Set up to return solutions one-by-one.  This is not very
// interesting in serial.

int branching::startRepositoryScan()
{
  getAllSolutions(solPtrArray);
  solPtrCursor = 0;
  return solPtrArray.size();
}


// Get next repository member (argument ignored in serial)

solution* branching::nextRepositoryMember(int /*whichProcessor*/)
{
  return solPtrArray[solPtrCursor++];
}


// Stuff for load logs

loadLogRecord::loadLogRecord(int sense) :
  time(0),
  pool(0),
  boundCalls(0),
  bound(-sense*MAXDOUBLE),
  incVal(sense*MAXDOUBLE),
  offers(0),
  admits(0)
{ }


void branching::startLoadLogIfNeeded()
{
  if (loadLogSeconds > 0)
    {
      needLLAppend = false;
      lastLog = new loadLogRecord(sense);
      beginLoadLog();
    }
}


void branching::beginLoadLog()
{
  loadLogBaseTime = WallClockSeconds();
  lastLog->time   = loadLogBaseTime;
  lastLLWriteTime = loadLogBaseTime;
  recordLoadLogData(loadLogBaseTime);
}


void branching::recordLoadLogIfNeeded()
{
  if (loadLogSeconds > 0)
    {
      double now = WallClockSeconds();
      if (now >= lastLog->time + loadLogSeconds)
	recordLoadLogData(now);
    }
}


void branching::recordLoadLogData(double time)
{
  loadLogRecord* record = new loadLogRecord(sense);
  recordSerialLoadData(record,
		       time,
		       pool->size(),
		       updatedLoad().aggregateBound);
  if (needToWriteLoadLog(time))
    {
      writeLoadLog();
      lastLLWriteTime = time;
    }
}


void branching::recordSerialLoadData(loadLogRecord* record,
				     double         time,
				     size_type      poolSize,
				     double         mainBound)
{
  // Put correct data in the record and update the lastLog structure
  // accordingly

  lastLog->time = time;
  record->time  = time;

  record->pool  = poolSize;

  record->boundCalls  = boundCompCalls - lastLog->boundCalls;
  lastLog->boundCalls = boundCompCalls;

  record->offers  = solsOffered - lastLog->offers;
  lastLog->offers = solsOffered;

  record->admits  = solsAdmitted - lastLog->admits;
  lastLog->admits = solsAdmitted;

  if (depthFirst || breadthFirst)
    mainBound = -sense*MAXDOUBLE;
  record->bound = mainBound;

  record->incVal = incumbentValue;

  // Now stuff the record in the list

  loadLogEntries.add(record);
}


void branching::finishLoadLogIfNeeded()
{
  if (loadLogSeconds > 0)
    {
      recordLoadLogData(WallClockSeconds());  // Record a final data point.
      writeLoadLog();
      delete lastLog;
      lastLog = NULL;
    }
}


std::string branching::loadLogFileName()
{
  std::string filename;
  if (problemName != "")
    filename = problemName;
  else
    filename = "pebbl";
  filename += ".loadLog";
  return filename;
}


void branching::writeLoadLog()
{
  if (loadLogEntries.empty())
    return;
  std::string filename = loadLogFileName();
  ofstream loadLogFile(filename.c_str(),needLLAppend ? ios::app : ios::out);
  if (loadLogFile.bad())
    {
      ucout << "****** Warning ******** could not open load log file.\n";
      return;
    }
  writeLoadLog(loadLogFile,0);
}


void branching::writeLoadLog(ostream& llFile,int proc)
{
  while(!loadLogEntries.empty())
    {
      loadLogRecord* record = NULL;
      loadLogEntries.remove(record);
      record->writeToStream(llFile,sense,loadLogBaseTime,proc);
      delete record;
    }
  needLLAppend = true;
}

void loadLogRecord::writeToStream(ostream& os,
				  int      sense,
				  double  baseTime,
				  int     proc)
{
  double dummyBoundVal = sense*MAXDOUBLE;

  os << proc << '\t'             // Processor, alway 0 in serial
     << time - baseTime << '\t'  // Time
     << boundCalls << '\t'       // Bound calls
     << pool << '\t'             // (Worker) pool size
     << "0\t"                    // Hub pool size
     << pool << '\t'             // Total pool size
     << "0\t0\t0\t0\t"           // Parallel-only stuff
     << bound << '\t'            // Bound
     << incVal << '\t'           // Incumbent value
     << dummyBoundVal << '\t'    // Parallel-only bound stuff...
     << dummyBoundVal << '\t'
     << dummyBoundVal << '\t'
     << dummyBoundVal << '\t'
     << offers << '\t'           // Enumeration stuff
     << admits << '\t'
     << "0\t0\t0\n";             // More parallel-only stuff
}


#ifdef ACRO_HAVE_MPI

void solution::pack(PackBuffer& outBuf)
{
  outBuf << typeId;
  solutionIdentifier::pack(outBuf);
  packContents(outBuf);
}


void solution::unpack(UnPackBuffer& inBuf)
{
  // typeId should already have been read when this gets called
  solutionIdentifier::unpack(inBuf);
  unpackContents(inBuf);
}


int solution::maxBufferSize()
{
  return sizeof(int) 
    + solutionIdentifier::packSize() 
    + maxContentsBufSize()
    + 64;                              // To allow of alignment padding
}


void loadLogRecord::pack(PackBuffer& pb)
{
  pb << time
     << pool
     << boundCalls
     << bound
     << incVal
     << offers
     << admits;
}


void loadLogRecord::unpack(UnPackBuffer& upb)
{
  upb >> time
      >> pool
      >> boundCalls
      >> bound
      >> incVal
      >> offers
      >> admits;
}


#endif



} // namespace pebbl

ostream& operator<<(ostream& s, const pebbl::branchSubId& spid)
{
  s << '{';
#ifdef ACRO_HAVE_MPI
  if (uMPI::running())
    s << spid.creatingProcessor << ':';
#endif
  return s << spid.serial << '}';
}

