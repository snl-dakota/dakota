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
 * \file branching.h
 * \author Jonathan Eckstein
 *
 * Defines pebbl::branching and related classes.
 */


#ifndef pebbl_branching_h
#define pebbl_branching_h

#include <acro_config.h>
#include <utilib/seconds.h>
#include <utilib/BasicArray.h>
#include <utilib/CharString.h>
#include <utilib/LinkedList.h> 
#include <utilib/GenericHeap.h>
#include <utilib/exception_mngr.h>
#include <utilib/memdebug.h>
#include <utilib/ParameterList.h>
#ifdef   ACRO_HAVE_MPI
#include <utilib/PackBuf.h>
#endif
#include <pebbl/memUtil.h>
#include <pebbl/fundamentals.h>
#include <pebbl/pebblBase.h>
#include <pebbl/pebblParams.h>
#include <pebbl/loadObject.h>

extern "C" void pebbl_abort_handler(int code);


namespace pebbl {


using namespace utilib;


class branchSub;
class branching;
class solution;
class spHandler;


///
template <class Type>
struct DynamicSPCompare : public ComparisonBase<Type>, public Comparison
{
public:

  ///
  compare_type operator()(const Type& _arg1, const Type& _arg2) const
  {
    int ans = _arg1.dynamicSPCompare(_arg2);
    if (ans < 0) return better;
    if (ans > 0) return worse;
    return equal;
  }
};


///
///  We define a general pool class that can hold either subproblems
///  or tokens (for the parallel code).
///
enum poolType {heapPoolType, stackPoolType, queuePoolType};


///
template <class SUB, class LOAD> class branchPool : public pebblBase
{
public:
 
  virtual ~branchPool() {}

  virtual void reset() = 0;

  branching* global() { return globalPtr; };

  void setGlobal(branching* master) { globalPtr = master; };

  virtual int size() = 0;

  virtual int insert(SUB* p) = 0;

  virtual SUB* select() = 0;

  virtual SUB* remove(SUB* p) = 0;
  virtual SUB* remove() { return remove(select()); };
  virtual int kill(SUB* p);

  virtual void clear();

  virtual int  prune() = 0;
  virtual void pretendPrunedOnce() { };

  virtual void resetScan() = 0;
  virtual SUB* scan() = 0;

  virtual SUB* firstToUnload() = 0;
  virtual SUB* nextToUnload()  = 0;

  LOAD& load() { return myLoad; };
  LOAD& updatedLoad()
    {
      myLoad.update();
      if (knowsGlobalBound())
	{
	  myLoad.setBound(globalBound());
	  myLoad.setBoundUnknown(false);
	}
      else
	myLoad.setBoundUnknown();
      return myLoad;
    }

  virtual double loadMeasure() 
    {
      return myLoad.loadMeasure();
    };

  virtual bool   knowsGlobalBound() { return false; };
  virtual double globalBound()
    {
      EXCEPTION_MNGR(std::runtime_error, 
		     "globalBound() called, but none available.");
      return 0;
    }

  virtual void myPrint()=0;

 protected:

  LOAD myLoad;

 private:

  branching*  globalPtr;

};

//-----------------------------------------------------------------------
//
//  Standard pool implementations for depth and
//  breadth-first. 
//

template <class SUB, class LOAD> 
class doublyLinkedPool : public branchPool <SUB,LOAD> 
{
 public:

  int size() { return list.size(); }; 

  SUB* select() { return list.top(); };

  int insert(SUB* p)  
    {
      ListItem<SUB*> *item = list.add(p); 
      p->poolPtr = item; 
      this->myLoad += *p;
      return size();
    };

  SUB* remove(SUB* p) 
    {    
      return removeListItem( (ListItem<SUB*> *)p->poolPtr );
    };

  SUB* remove() { return remove(select()); };
  
  int prune()
    {
      ListItem<SUB*> *l1,*l2;  
        
      l1 = list.head();
      while (l1)
	{  
	  l2 = list.next(l1);
	  if (l1->data()->canFathom())
	    {
	      removeListItem(l1)->recycle();
	    }
	  l1=l2;
	}
      return size();
    };

  void resetScan() { scanCursor = list.head(); };

  SUB* scan()
    {
      SUB* toReturn = scanCursor->data();
      scanCursor = list.next(scanCursor);
      return toReturn;
    }

  SUB* firstToUnload() 
    {
      if (size() == 0)         // Safety valve
        return NULL;

      SUB* toReturn = 0;
      
      unloadCursor = list.head();
      if (list.next(unloadCursor))
	unloadCursor = list.next(unloadCursor);
      if (unloadCursor)
	toReturn = unloadCursor->data();
      
      advanceUnloadCursor();
      advanceUnloadCursor();
      return toReturn;
    };
  
  SUB* nextToUnload() 
    {
      if (unloadCursor == 0)
	return firstToUnload();
      SUB *toReturn = unloadCursor->data();

      advanceUnloadCursor();
      advanceUnloadCursor();
      return toReturn;
    };
  
  doublyLinkedPool(bool stack_or_queue) 
    { 
      if (stack_or_queue) 
	list.stack_mode(); 
      else 
	list.queue_mode();
    };

  // Nothing special to be done here.  Mode should be set from 
  // first construction.
  void reset() { this->clear(); };

  ~doublyLinkedPool() { this->clear(); };  

  virtual void myPrint()
    {
      std::cout<<"\n======doublyLinkedPool=========\n";
      ListItem<SUB*>* l = list.head();
      while(l) 
	{
	  std::cout<<l->data()<<" | ";
	  l = list.next(l);
	}
      std::cout<<std::endl<<std::endl;
    };

 protected:

  LinkedList< SUB* > list; 

  ListItem< SUB* > *scanCursor;

  ListItem< SUB* > *unloadCursor; 

  SUB* removeListItem(ListItem<SUB*> *item)
    {
      SUB *p;

      list.remove(item,p);
      this->myLoad -= *p;
      return p;
    };

  void advanceUnloadCursor() 
    {
      if (unloadCursor)       
	{
	  unloadCursor = list.next(unloadCursor);
	}
      else
	unloadCursor = 0;
    };

};

//--------------------------------------------------------------------------
//
//  A best-first pool implemented via a heap.
//

template <class SUB, class LOAD, class COMPARE=GenericHeapCompare<SUB> > 
class heapPool : public branchPool <SUB,LOAD> 
{
 public:

 int size() { return heap.size(); };
    
 virtual int insert(SUB* p) 
 {
   GenericHeapItem<SUB> *item = heap.add(*p);
   p->poolPtr = item;      
   this->myLoad += *p;
   return 1;// it can return anything
 };
  
 virtual SUB* select () { return  &(heap.top()->key()); };
   
 SUB* remove(SUB* p) 
 { 
   return removeHeapItem( (GenericHeapItem<SUB> *)p->poolPtr );
 };

 SUB* remove() { return remove(select()); };
  
 virtual void clear() 
 {
   for(int e = heap.size(); e > 0; e--)
     removeHeapItem(heap.member(e))->recycle();
 };
    
 virtual int prune() 
 {
   GenericHeapItem<SUB> *item;
   int n = heap.size();

   if (this->global()->initialDive && firstPrune) 
     { 
       heap.reheapify();
       firstPrune = false;
     }

  
   while( n > (heap.size()>>1) )
     {
       item = heap.member(n);
       if ( item->key().canFathom() )
	 {
	   removeHeapItem(item)->recycle(); 
	   if ( n > heap.size() ) 
	     n = heap.size();
	 }
       else
	 n--;
     }

   return heap.size();
 };

 virtual void pretendPrunedOnce()
 {
   firstPrune = false;
 };
  
 heapPool() :
 unloadCursor(0), 
 firstPrune(true) 
 { };

 void reset()
 {
   clear();
   unloadCursor = 0;
   firstPrune = true;
   scanCursor = 1;
 }
  
 virtual ~heapPool() { clear(); };
  
  bool knowsGlobalBound() 
     { 
       // Though the globalBound() method tries to estimate the bound when the pool
       // is empty, it really doesn't know it.  Other methods should then know to use
       // the bound in the current subproblem (not in the pool).
       return (!(this->global()->initialDive && firstPrune) && size() > 0);
     };

 double globalBound()    
 { 
   if (size() > 0)
     return select()->bound;
   else
     return this->global()->sense*MAXDOUBLE;
 };

 void resetScan() { scanCursor = 1; };
  
 SUB* scan() { return &(heap.member(scanCursor++)->key()); };

 SUB* firstToUnload()
    {
      unloadCursor = std::min(heap.size(),2); 
      return nextToUnload();
    };
  
  SUB* nextToUnload()
    {
      if (size() == 0) return 0;
      if (unloadCursor <= heap.size()) 
	return &(heap.member(unloadCursor++)->key());
      return firstToUnload();
    };

  virtual void myPrint()
    {
      std::cout<<"\n=======heapPool=====================\n";
      SUB* he;
      for(int i=1;i<=heap.size();i++) 
	{
	  he = (& (heap.member(i)->key() ) );
	  std::cout<<he<<" || ";
	}
      std::cout<<std::endl<<std::endl;

    };
  
 protected:

  GenericHeap<SUB,COMPARE> heap;
  int unloadCursor; 
  int scanCursor;
  bool firstPrune;

  SUB* removeHeapItem(GenericHeapItem<SUB> *item)
    {
      bool status;
      SUB *p;
      
      p = &( item->key() );
      heap.remove(item,status);
      if (!status) 
	EXCEPTION_MNGR(std::runtime_error,"The item was not found in the heap");
      this->myLoad -= *p;
      return p;
    };
 
};


//
//  Class to identify subproblems.
//  Includes a creating processor field for parallel appliations
//  (Eventually could make that conditionally compiled.)
//

// CAP: This was derived from pebblBase, but it doesn't seem to need
// anything in pebblBase and it's cluttering output in gdb.
// I'm removing for now.
// class branchSubId : public pebblBase

class branchSubId
{

public:

  int serial;
  int creatingProcessor;

  // Check if two id's are the same

  inline int operator ==(branchSubId& other)
  {
    return 
      (serial == other.serial) && 
      (creatingProcessor == other.creatingProcessor);
  };

  // Copy operator

  inline branchSubId& operator= (const branchSubId& other)
  {
    serial = other.serial;
    creatingProcessor = other.creatingProcessor;
    return(*this);
  }

  // Null constructor

  branchSubId() :
    serial(-1),
#ifdef ACRO_HAVE_MPI
    creatingProcessor(uMPI::running() ? uMPI::rank : 0)
#else
    creatingProcessor(0)
#endif
  { };

  // Copy constructor was replaced by a copy operator

  // Initializer for a new subproblem

  void branchSubIdFromBranching(branching* global);

  // This can be used to make an ID "empty", and check if its empty.

  enum { emptyCode = -10 };

  inline void setEmpty() 
  {
    serial = emptyCode;
  };

  inline bool isEmpty()
  {
    return serial == emptyCode;
  }

};


//  Classes to store solutions.  There is a base class
//  solutionIdentifier which is there mainly for various manipulations
//  by the parallel layer.  It's the solution equivalent of a
//  coreSPInfo or a token.  The main solution class derives from
//  solutionIdentifier.

class solutionIdentifier : public pebblBase
{
 public:

  // Basic identification data

  double value;
  int    serial;

  // Which way to compare identifiers

  optimType sense;

  // Constructors

  solutionIdentifier();
  solutionIdentifier(branching* bGlobal);
  solutionIdentifier(solutionIdentifier* toCopy);

  // Copy information from another solution identifier

  void copy(solutionIdentifier* toCopy);

  // Write in text form

  virtual void print(std::ostream& s);

  // Basic comparison method in utilib standard form, and a method to
  // set things up so the current object will always lose such a
  // comparison.

  int compare(const solutionIdentifier& other) const;

  void setWorstPossible();
  void setWorstPossible(optimType sense_);

  virtual void write(std::ostream& os);

#ifdef ACRO_HAVE_MPI

  // In parallel, we also store the "owning" processor of each solution

  int owningProcessor;

  virtual void pack(PackBuffer& OutBuf) const;
  virtual void unpack(UnPackBuffer& InBuf);
  static int packSize();

#endif

};


// The main base class for solutions

class solution : public solutionIdentifier
{
 public:

  // (Public) data members

  int typeId;

  // Repository-related stuff

  size_type hashValue;
  bool      hashComputed;   // Flag that says whether hashValue is initialized

  ListItem<solution*>* hashItem;   // Location in repository hash tables

  // Printout-related stuff

  virtual const char* typeDescription() const { return "Generic solution"; };

  virtual void print(std::ostream& s);
  virtual void printContents(std::ostream& /*s*/) { }

  // Simple constructor that doesn't set the serial number or sense

  solution() : 
    typeId(0),
    hashValue(0),
    hashComputed(false)
    { };

  // Slightly more complicated one that sets the serial number and
  // sense through a branching pointer.

  solution(branching* bGlobal);

  // Copy constructor

  solution(solution* toCopy);

  void copy(solution* toCopy);

  void creationStamp(branching* bGlobal, int typeId = 0);

  // Virtual destructor, so that derived classes can be deleted properly

  virtual ~solution() { };

  // Duplicate detection and hash values

  virtual size_type computeHashValue();

  virtual bool duplicateOf(solution& other);

  void deleteIfNotLocal() 
  {
#ifdef ACRO_HAVE_MPI
    if (owningProcessor != uMPI::rank)
      delete this;
#endif
  };

  // Parallel-environment methods

#ifdef ACRO_HAVE_MPI

  virtual solution* blankClone() { return new solution(this); };

  void pack(PackBuffer& outBuf);
  virtual void packContents(PackBuffer& outBuf) { };

  void unpack(UnPackBuffer& inBuf);
  virtual void unpackContents(UnPackBuffer& inBuf) { };

  int maxBufferSize();
  virtual int maxContentsBufSize() { return 0; };

#endif

  // Internal stuff related to duplicate detection

protected:

  size_type sequenceCursor;

  virtual size_type sequenceLength() { return 0; };

  virtual double sequenceData() { return 0; };

  virtual void sequenceReset() { sequenceCursor = 0; };

};


//
//  Structure for holding load log information (serial version)
//  Parallel version is derived from this
//

class loadLogRecord 
{
 public:

  double time;
  int    pool;
  int    boundCalls;
  double bound;
  double incVal;
  int    offers;
  int    admits;

  // Constructor -- creates an "empty" record
  loadLogRecord(int sense);

  // Placeholder destructor
  virtual ~loadLogRecord() { };

  // Method to write to output
  virtual void writeToStream(std::ostream& os, 
			     int           sense, 
			     double        baseTime,
			     int           proc = 0);
#ifdef ACRO_HAVE_MPI

  // Pack to buffer
  virtual void pack(PackBuffer& pb);

  // Unpack from buffer
  virtual void unpack(UnPackBuffer& upb);

#endif

};


//
//  Base class for each branching structure. 
//

class branching : public pebblBase, public pebblParams
{
  friend class branchSub;
  friend class branchSubId;
  friend class solutionIdentifier;
  friend class loadObject;

public:

  optimType sense;
  double    incumbentValue;

  solution* incumbent;

  branchPool<branchSub,loadObject>* pool;

  spHandler* handler;

  branchSub*  currentSP;
  branchSubId currentSPId;
  branchSubId previousSPId;

  bool needPruning;

  int probCounter;
  int subCount[numStates];

  double searchTime;

  int    boundCompCalls;
  int    splitCompCalls;
  double boundCompTime;
  double boundCompTimeSq;
  double splitCompTime;
  double splitCompTimeSq;

  double nextOutputTime;
  double lastSolValOutput;

  std::string problemName;

  branching() : 
  sense(minimization),    // Default is now minimization unless specified...
      incumbent(NULL),
      pool(NULL),
      handler(NULL),
      enumerating(false),
      usingEnumCutoff(false),
      solSerialCounter(0),
      parameters_registered(false),
      min_num_required_args(0)
    {
      version_info = "PEBBL 1.5";
    }

  double absGap(double boundValue) 
    { return (incumbentValue - boundValue)*sense; };

  virtual double relGap(double boundValue);
  virtual double relGap(loadObject& l);
  virtual double relGap();

  virtual bool canFathom(double boundValue);
  virtual bool canFathom(solutionIdentifier* solPtr);

  virtual double fathomValue();

  virtual void setName(const char* cname);

  virtual void preprocess() { };

  virtual double aPrioriBound() { return -sense*MAXDOUBLE; };

  virtual solution* initialGuess() { return NULL; };

  virtual branchSub* blankSub() = 0;
  virtual branchSub* makeRoot();

  // Incumbent-related methods

  void foundSolution(solution* sol,syncType sync=notSynchronous);

  void resetIncumbent();

  virtual void signalIncumbent() 
    {
      needPruning = true; 
    };

  // Do any operations that all go along with the acquisition 
  // of a new incumbent value, whether generated locally or 
  // (in the parallel case) received from another processor

  virtual void newIncumbentEffect(double /*new_value*/) {}

  virtual bool haveIncumbentHeuristic() { return false; };

  bool haveIncumbent() {return (incumbentValue != sense * MAXDOUBLE);};

  double searchFramework(spHandler *handler_ = NULL);

  virtual double search() { return searchFramework(NULL); };

  bool shouldAbort(int boundCount);
  const char* abortReason;

  inline int haveCurrentSP()
  {
    return currentSP != NULL;
  };
  
  virtual void loadCurrentSP(branchSub* p);
  virtual void unloadCurrentSP();
  virtual void eraseCurrentSP();

  virtual void unloadCurrentSPtoPool()
    {
      pool->insert(currentSP);
      unloadCurrentSP();
    };

  virtual std::ostream* openSolutionFile();
  virtual void closeSolutionFile(std::ostream* fileStream);

  void solutionToFile();

  virtual void serialPrintSolution(const char* header = "",
				   const char* footer = "",
				   std::ostream& outStream = std::cout);

  virtual void printSolution(const char* header = "",
			     const char* footer = "",
			     std::ostream& outStream = std::cout)  
    {
      serialPrintSolution(header,footer,outStream);
    };

  virtual void serialPrintSolValue(std::ostream& stream = std::cout);
  virtual void printSolValue(std::ostream& stream = std::cout)
    {
      serialPrintSolValue(stream);
    }

  virtual void printSPStatistics(std::ostream& stream = std::cout);
  virtual void printTimings(std::ostream& stream = std::cout);
  virtual void printReposStatistics(std::ostream& stream = std::cout);
  virtual void printAllStatistics(std::ostream& stream = std::cout);

  void printSpTimeStats(std::ostream& stream = std::cout);

  void printReposCommon(std::ostream& stream,
			int size,
			int offer,
			int admit,
			double worstVal,
			int comm = -1);

  void printAbortStatistics(loadObject& load);

  virtual ~branching();  // Note: default action is to delete the pool.

  /// Initialize base classes and reset the state of the solver
  virtual void reset(bool resetVB = true);

  /// Setup the solver parameters using command-line information.
  /// This returns false if there is a problem in the setup, and true
  /// if the setup appeared to work normally.
  virtual bool setup(int& argc, char**& argv);

  /// Reset wall clock and overall CPU times (search timer resets automatically)
  void resetTimers();

  ///
  virtual void solve();

  /// Return a pointer to the incumbent after solving
  virtual solution* getSolution(int /*whichProcessor*/ = allProcessors)
  {
    return incumbent;
  };

  /// To return the entire repository of all solutions into an array
  /// Very simple serial implementation.
  virtual void getAllSolutions(BasicArray<solution*>& solArray,
			       int whichProcessor = allProcessors);

  /// To initialize scanning of solutions one-by-one
  virtual int startRepositoryScan();

  /// To get next thing from the repository
  virtual solution* nextRepositoryMember(int whichProcessor = allProcessors);

  ///
  static std::string version_info;

 protected:

  double gapDenom(double boundValue)
    { return std::max(std::fabs(boundValue),std::fabs(incumbentValue)); };
 
  virtual void statusPrint(int&        lastPrint, 
			   double&     lastPrintTime,
			   const char* tag="");
  virtual void statusPrint(int&        lastPrint,
			   double&     lastPrintTime,
			   loadObject& l, 
			   const char* tag="");
  virtual void statusLine(loadObject& l, 
			  const char* tag="");

  int statusLinePrecision;

  virtual loadObject load();
  virtual loadObject updatedLoad();

  void branchingInit(optimType   direction = minimization,
		    double      relTolSet = -1.0,
		    double      absTolSet = -1.0,
		    double      bluff = -1,
		    bool bluffReal = false); 

  void branchingInitGuts();

  void directSolutionToFile();
  int  serialNeedEarlyOutput();
  void recordEarlyOutput(double outputVal);

  void printSPTable(std::ostream& stream,int* table,int nameWidth);
  void printSPLine(std::ostream&    stream,
		   int         n,
		   const char* name,
		   int         denominator,
		   int         nameWidth,
		   int         numWidth);

  virtual bool          valLogOutput() { return validateLog;  };
  virtual std::ostream* valLogFile();
  void                  valLogInit();
  void                  valLogFathomPrint();

  // Load log stuff

  virtual void startLoadLogIfNeeded();
  void recordLoadLogIfNeeded();
  void finishLoadLogIfNeeded();

  void recordSerialLoadData(loadLogRecord* record,
			    double    time,
			    size_type poolSize,
			    double    mainBound);

  std::string loadLogFileName();

  virtual void recordLoadLogData(double time);

  virtual void writeLoadLog();

  void writeLoadLog(std::ostream& llFile, int proc = 0);

  // List of load log records
  LinkedList<loadLogRecord*> loadLogEntries;

  // holds state when load log record created.  Used for information
  // that is a "delta"
  loadLogRecord*  lastLog;

  // Generic code to start recording of log (shared by parallel layer)
  void beginLoadLog();

  double loadLogBaseTime;
  bool   needLLAppend;
  double lastLLWriteTime;

  bool needToWriteLoadLog(double time)
  {
    return (loadLogWriteSeconds > 0) && 
      (time >= lastLLWriteTime + loadLogWriteSeconds);
  }

  // End of load log stuff

  int lastHeurPrint;

  double startTime;
  double startWall;

  // Enumeration stuff, including repository of solutions

 public:

  bool enumerating;
  bool usingEnumCutoff;

  int solSerialCounter;

  // The repository consists of a heap of (references to) solutions in
  // *reverse* bound order, and a hash table of pointers to solutions.
  // Each list in the hash table is sorted in increasing hashValue
  // order.

  typedef GenericHeap<solution,Reverse<GenericHeapCompare<solution> > >
    reposSolHeap;

 protected:

  reposSolHeap reposHeap;

  BasicArray< LinkedList<solution*> > reposTable;

  // This is used when enumCount is active, and represents the worst
  // solution that is currently being stored.  Any solution that does
  // not beat this can been discarded.  It starts at the worst
  // possible value, and must improve monotonically.  It starts
  // changing once the repository is full.

  solutionIdentifier lastSolId;

  // Bookkeeping information

  int solsOffered;
  int solsAdmitted;

  // Basic repository functions

  size_type repositorySize() { return reposHeap.size(); };

  solution* worstReposSol();

  double worstReposValue();

  virtual bool offerToRepository(solution* sol, syncType sync=notSynchronous);

  bool localReposOffer(solution* sol);

  virtual void pruneRepository();

  bool canFathomFromRepository(solutionIdentifier* solPtr);

  void clearRepository();

  solution* removeWorstInRepos();

  void deleteSolution(solution* sol);

  // Creat a sorted list of solution*'s from the repository, best
  // solution first.

  void sortRepository(BasicArray<solution*>& solArray);

  // Similar, but fill an array of solutionIdentifiers; used only in
  // the parallel layer.

  void sortReposIds(BasicArray<solutionIdentifier>& result);

  // Update lastSolId.  If the argument is better, copy it to
  // lastSolId.  Return a true status if lastSolId changed.

  bool updateLastSolId(solutionIdentifier* otherSol);

  // Used to tweak status line printouts when enumerating.  In serial,
  // no tweakage is needed.

  virtual void adjustReposStats(loadObject& /*lo*/) { };

  // Internal data structures for scanning the repository

  BasicArray<solution*> solPtrArray;
  size_type             solPtrCursor;

  // Parameter-related methods
   
  ///
  virtual void write_usage_info(char* progName,std::ostream& os) const;

  ///
  virtual void writeCommandUsage(char* progName,std::ostream& os) const;

  ///
  utilib::ParameterList plist;

  ///
  bool parameters_registered;

  ///
  std::string solver_name;

  //  Looks like Bill or Cindy added this -- redundant?
  //  double start_time;

  unsigned int min_num_required_args;

  ///
  virtual bool processParameters(int& argc, 
				 char**& argv, 
				 unsigned int min_num_required_args__=0);

  /// Register the parameters into a ParameterList object
  virtual void register_parameters()
        {
        plist.register_parameters(*this);
        }

  /// Check parameters for setup problems and perform debugging I/O
  virtual bool checkParameters(char* progName = "");

  /// Sets up the problem from command line arguments.  Should return
  /// true if the setup worked, and false if something went wrong.
  /// The default implementation is just to return true -- in that case, 
  /// the instance must get set up in some internal way.

  virtual bool setupProblem(int& /*argc*/, char**& /*argv*/) { return true; }

};



  

//
//  The following class is used as a common base class by subproblems
//  and tokens (in the parallel code).
//

class coreSPInfo : public pebblBase
{
 public:

  int dynamicSPCompare(const coreSPInfo& sp2) const;
  int compare(const coreSPInfo& sp2) const;

  void write(std::ostream& /*s*/) const {}
  void read(std::istream& /*s*/){}

  branchSubId id;
  
  double   bound;
  double   integralityMeasure;
  subState state;
  int      depth;

  void*    poolPtr;

  virtual branching* bGlobal() const = 0;

  virtual bool canFathom();

  coreSPInfo() : 
    integralityMeasure(MAXDOUBLE)
  { }

  void CSPInitfromBranching(branching *global);
  void CPSInitAsCopy(coreSPInfo& toCopy);

  virtual double boundEstimate() const { return bound; }

  virtual int loadXFactor() const { return 1; }

  virtual void recycle() { delete this; }

  virtual ~coreSPInfo() { }

  // To assist in debug printouts

  virtual void debugPrint(std::ostream& s) const;
  
};



//
//  Root class for all subproblems.         
//


class branchSub : public coreSPInfo
{
friend class branching;

public:
  
  branching* bGlobal() const = 0;
  
  REFER_DEBUG(bGlobal())

  int childrenLeft;
  int totalChildren;
  
  void* poolPtr;             // For use by pool.
    
  double relGap()         { return bGlobal()->relGap(bound);  }
  double absGap()         { return bGlobal()->absGap(bound);  }
  double incumbentValue() { return bGlobal()->incumbentValue; }
  
  virtual void setRoot();
  virtual void setRootComputation() = 0;

  virtual bool forceStayCurrent()      { return false; };
  virtual void makeCurrentEffect()     { };
  virtual void noLongerCurrentEffect() { };
  
  virtual void boundComputation(double* controlParam) = 0;
  virtual void computeBound(double* controlParam);
  virtual void computeBound();

  virtual solution* extractSolution() = 0;
  
  virtual int splitProblem();
  virtual int splitComputation() = 0;
  
  virtual branchSub* child(int whichChild = anyChild);
  virtual branchSub* makeChild(int whichChild = anyChild) = 0;
  virtual int chooseChild(int desired)
    {
      if ((desired >= 0) && (desired < totalChildren))
	return desired;
      if (desired != anyChild)
	EXCEPTION_MNGR(std::runtime_error, "Request for child " 
		       << desired << " out of range");
      return totalChildren - childrenLeft;
    };
  
  virtual bool candidateSolution() = 0;
  
  virtual void incumbentHeuristic() { };

  virtual void foundSolution(solution* sol,syncType sync=notSynchronous)
    {
      bGlobal()->foundSolution(sol,sync);
    };

  int loadXFactor() const
    {
      if (state == separated)
	return childrenLeft;
      else
	return state != dead;
    };


  // Constructors:
  // Moving these to regular functions to help (I hope) with debugging.
  // We can move back later for inlining.

  branchSub();
  
  void branchSubAsChildOf(branchSub* parent);
  
  void setState(subState newState,branching* globalP)
    {
#ifdef ACRO_VALIDATING
      if (newState < state)
	EXCEPTION_MNGR(runtime_error,"Invalid subproblem state transition");
#endif
      // The check of the serial number is for applications that 
      // generate "fake" subproblems (e.g. for strong branching purposes)
      // that might never go into the tree.  We don't want them to skew
      // the subproblem statistics.
      if (id.serial > 0 && state != newState) 
	(globalP->subCount[newState])++;

      state = newState;
    }

  void setState(subState newState)
    {
      setState(newState,bGlobal());
    };

  void recycle();

  // Memory tracking

#ifdef MEMORY_TRACKING
  virtual void printMemDetails() { std::cout << this << ' '; };
#endif

  // Validation log stuff...

  int valLogOutput() { return bGlobal()->valLogOutput(); };

  virtual int valLogProc() { return 0; };

  virtual void valLogWriteID(char separator = 0);
  virtual void valLogWriteBound(char separator = 0);

  virtual void valLogCreatePrint(branchSub* parent);
  virtual void valLogBoundPrint();
  virtual void valLogSplitPrint();
  virtual void valLogDestroyPrint();

  // Application-specific validation log stuff -- null by default

  virtual void valLogCreateExtra(branchSub* /*parent*/) { }
  virtual void valLogBoundExtra()                   { }
  virtual void valLogSplitExtra()                   { }
  virtual void valLogDestroyExtra()                 { }

  // This has to be moved back to private after we fix the constructor-
  // calling problem

  // If this changes, it will affect packGenericChild, which constructs a
  // packed child.

  void branchSubInit(branching* master,
		    subState initialState = boundable) 
    {
      totalChildren = childrenLeft = 0;
      state = initialState;
      (master->subCount[state])++;
      poolPtr = 0;
    };


private:

};


//  Some stuff related to memory tracking

#ifdef MEMORY_TRACKING
#define MEMORY_TRACK_SP(sp,s) \
    MEMORY_IF(sp->id.serial) {sp->printMemDetails(); memUtil::trackPrint(s);}
#else
#define MEMORY_TRACK_SP(sp,s)  // Nothing
#endif


// Template for simple solution classes made around BasicArrays.
// The template argument T should be something for which the following
// are defined:  
//
//      operator double() (casting to a double)
//      operator!=(T&,int&)
//      operator<<(ostring&,T)
//      operator<<(PackBuffer&,T)     (if MPI is active)
//      operator>>(T,UnPackBuffer&)   (if MPI is active)
//
// These will automatically be defined for all numeric types.  The !=
// operator is used to test whether to print.  It can just be defined
// to return true if you want everything to print.

template <class T> class arraySolution :
public solution
{
 public:

  // Data members

  optimType minOrMax;                // Direction of optimization

  BasicArray<T> array;               // Data (including size)

  BasicArray<CharString>* vNamePtr;  // Optional names of the variables.
                                     // NULL if no names
  const char* typeNamePtr;           // Pointer to type description string
                                     // NULL if no special name
  // Standard overrides

  const char* typeDescription() const
    {
      if (typeNamePtr)
	return typeNamePtr;
      else
	return "Array Solution";
    };

  void printContents(std::ostream& s)
    {
      for (size_type j=0; j<array.size(); j++)
	{
	  T xj = array[j];
	  if (xj != 0)
	    {
	      if (vNamePtr)
		s << (*vNamePtr)[j];
	      else
		s << "Variable(" << j << ')';
	      s << " = ";
	      s << xj << std::endl;
	    }
	}
    };

  // Constructors

  // Construct a solution of known size.  Typically used to create a
  // reference solution for registration and buffer size calculation.

  arraySolution(size_type               size,
		branching*              bGlobal,
		const char*             typeNamePtr_ = NULL,
		BasicArray<CharString>* vNamePtr_    = NULL) :
    solution(bGlobal),
    array(size),
    vNamePtr(vNamePtr_),
    typeNamePtr(typeNamePtr_)
    { };

  // Copy-like constructor for use by blankClone().  Does *not* copy
  // data itself.

  arraySolution(arraySolution<T>* otherArraySol) :
    solution(otherArraySol),
    vNamePtr(otherArraySol->vNamePtr),
    typeNamePtr(otherArraySol->typeNamePtr)
    { };

  // Explicitly construct a solution with contents from a supplied array
  // and objective value

  arraySolution(double                  obj,
		BasicArray<T>&          suppliedArray,
		branching*              bGlobal,
		const char*             typeNamePtr_ = NULL,
		BasicArray<CharString>* vNamePtr_    = NULL) :
    solution(bGlobal),
    array(suppliedArray),
    vNamePtr(vNamePtr_),
    typeNamePtr(typeNamePtr_)
    { 
      value = obj;
    };
  
  // Explicitly construct a solution with contents from a supplied STL vector
  // and objective value

  arraySolution(double                  obj,
		std::vector<T>&         suppliedArray,
		branching*              bGlobal,
		const char*             typeNamePtr_ = NULL,
		BasicArray<CharString>* vNamePtr_    = NULL) :
    solution(bGlobal),
    vNamePtr(vNamePtr_),
    typeNamePtr(typeNamePtr_)
    { 
      value = obj;
      array << suppliedArray;
    };
  
  // Communication-related methods

#ifdef ACRO_HAVE_MPI

  virtual solution* blankClone() { return new arraySolution<T>(this); };

  virtual void packContents(PackBuffer& outBuf)    { outBuf << array; };

  virtual void unpackContents(UnPackBuffer& inBuf) { inBuf >> array;  };

  // The following method will only work correctly if the array has
  // been sized to the anticipated maximum size.  In the parallel
  // layer, the reference solution an application registers should be
  // sized accordingly.  This method assumes packing an object of size
  // T requires sizeof(T) bytes.  This should be fine for simple
  // datatypes, but we may want to revisit it if we end up using
  // exotic types.

  virtual int maxContentsBufSize() 
    {
      return sizeof(size_type) + array.size()*sizeof(T);
    };

#endif

  // Sequence representation

protected:

  size_type sequenceLength() { return array.size(); };

  double sequenceData() { return array[sequenceCursor++]; };

};


// A base class for all subproblem handlers.

class spHandler : public pebblBase
{
 public:

  REFER_DEBUG(global)

  virtual ~spHandler() { };

  virtual void setGlobal(branching* global_) { global = global_; };

  virtual void execute() = 0;

 protected:

  branching* global;   // Pointer back to branching class
  branchSub* p;        // Subproblem currently being operated on
  branchSub* c;        // A child of that problem

  virtual void setProblem()    { p = global->currentSP;    };
  subState state()             { return p->state;          };
  virtual void erase()         { global->eraseCurrentSP(); };
  virtual void computeBound()  { p->computeBound();        };
  virtual bool canFathom()     { return p->canFathom();    };   
  virtual void heuristic();    // Defined in cpp file
  virtual void split()         { p->splitProblem();        };
  virtual bool shouldRelease() { return false;             };
  virtual void releaseChild(); // Returns error, defined in cpp file
  virtual void getChild()      { c = p->child();           };
  virtual void insertChild()   { global->pool->insert(c); };
  virtual void eraseChild()    { c->recycle();             };

  void spinOffChild();         // Defined in cpp file

};


// The lazy version of the handler.

class lazyHandler : virtual public spHandler
{
 public:

  virtual ~lazyHandler() { };

  void execute();

};


// The hybrid version

class hybridHandler : virtual public spHandler
{
 public:

  virtual ~hybridHandler() { };

  void execute();

 protected:

  virtual bool stillNeedSP() { return (p->childrenLeft > 0) && !canFathom(); };

};


// The eager version needs a few special functions of its own

class eagerHandler : virtual public spHandler
{
 public:

  virtual ~eagerHandler() { };

  void execute();

 protected:

  branchSub* bp;     // The subproblem to be bounded.
  
  virtual void setToBoundCurrent()   { bp = p; };
  virtual void setToBoundChild()     { bp = c; };

  virtual bool boundSubHandler()      // Try to bound bp and return true
    {                                // if it can be thrown away now.
      if (bp->canFathom())
	return true;
      bp->computeBound();
      return bp->canFathom();
    };

};


template <class SUB, class LOAD>
int branchPool<SUB,LOAD>::kill(SUB* p)  
{
  remove(p);
  p->recycle();
  return size();
}

template <class SUB, class LOAD>
void branchPool<SUB,LOAD>::clear() 
{
  while(size() > 0)
    remove()->recycle();
}


/// Guts of prepackaged serial main program

template <class B> bool runSerial(int argc,char** argv)
{
  B instance;
  bool flag = instance.setup(argc,argv);
  if (flag)
    {
      instance.reset();
      instance.solve();
    }
  return flag;
}


/// Prepackaged serial main program

template <class B> int driver(int argc, char** argv)
{
  bool flag = true;

  try 
    {
      InitializeTiming();
      flag = runSerial<B>(argc,argv);
    }

  STD_CATCH(;);

  return !flag;
}


#ifndef ACRO_HAVE_MPI

/// Define serial/parallel prepackaged main program when there is no
/// parallelism.  If we do have MPI, a proper version of this routine
/// will be defined in parBranching.h instead.

template <class B,class PB> int driver(int argc, char** argv)
{
  return driver<B>(argc,argv);
}

#endif



} // namespace pebbl


std::ostream& operator<<(std::ostream& s, const pebbl::branchSubId& spid);

inline std::ostream& operator<<(std::ostream& s, pebbl::coreSPInfo* p)
{
  p->debugPrint(s);
  return s;
};


inline std::ostream& operator<<(std::ostream& s, pebbl::solutionIdentifier* p)
{
  p->write(s);
  return s;
};

#endif
