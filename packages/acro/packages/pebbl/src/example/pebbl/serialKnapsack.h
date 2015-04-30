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
 * \file knapsack.h
 * \author Jonathan Eckstein
 *
 *  Example class to use object-oriented branching framework.
 *  Solves binary knapsack problems.
 */

#ifndef pebbl_knapsack_h
#define pebbl_knapsack_h

#include <acro_config.h>
#include <utilib/CharString.h>
#include <utilib/BasicArray.h>
#include <utilib/BitArray.h>
#include <utilib/IntVector.h>
#include <utilib/DoubleVector.h>
#include <utilib/_math.h>
#include <utilib/ParameterSet.h>
#ifdef ACRO_HAVE_MPI
#include <utilib/PackBuf.h>
#endif
#include <pebbl/branching.h>
#include <pebbl/chunkAlloc.h>


namespace pebbl {


using namespace utilib;


// Static class for enums

class binKnapBase
{
public:

  enum { noItem = -1 };

};


// Class for parameters

 class binKnapParams :  virtual public ParameterSet
   {
   public:

     binKnapParams()
       {
	 randomCompletionGreediness=0.9;
	 create_categorized_parameter("randomCompletionGreediness",
				      randomCompletionGreediness,
				      "<double>","0.9",
				      "Heuristic greediness",
				      "Knapsack",
				      ParameterBounds<double>(0.0,1.0));

	 randomBackTrackPower=2;
	 create_categorized_parameter("randomBackTrackPower",
				      randomBackTrackPower,
				      "<int>","2",
				      "Controls heuristic backtracks",
				      "Knapsack",
				      ParameterLowerBound<int>(1));

	 randomSearchPersistence=0.5;
	 create_categorized_parameter("randomSearchPersistence",
				      randomSearchPersistence,
				      "<double>","0.5",
				      "Controls length of heuristic runs",
				      "Knapsack",
				      ParameterBounds<double>(0.0,1.0));
	
	 itemArraySizeQuantum = 8;
	 create_categorized_parameter("itemArraySizeQuantum",
				      itemArraySizeQuantum,
				      "<int>","8",
				      "Number of slots added at a time to "
				      "solution item arrays",
				      "Knapsack",
				      ParameterPositive<int>());
       };

     double randomCompletionGreediness;
     int    randomBackTrackPower;
     double randomSearchPersistence;
     int    itemArraySizeQuantum;

   };


// Helper classes for storing information about the objects to be packed.

class knapsackItem 
{
public:

  // Basic properties of the item

  double     weight;
  double     value;
  CharString name;

  // Constructors

  knapsackItem(double weight_, 
	       double value_, 
	       CharString& name_) :
    weight(weight_),
    value(value_),
    name(name_)
    { };

  knapsackItem() :
    weight(0),
    value(0),
    name()
    { };

  // Assignment operator

  knapsackItem& operator=(const knapsackItem& other)
    {
      weight = other.weight;
      value  = other.value;
      name  &= const_cast<knapsackItem&>(other).name;   // Share name memory
      return *this;
    };
  
    
  // These are needed for heap operations.  Actual comparison is
  // between the values (value/weight), but things are multiplied
  // through for efficiency.

  int compare(const knapsackItem& other) const
    {
      return sgn((other.value)*weight - value*(other.weight));
    };
  
  int write(std::ostream& os) const

    {
      os << name << ' ' << weight << ' ' << value;
      return 0;
    };

  int read(std::istream& is)
    {
      is >> name >> weight >> value;
      return 0;
    };

  // This causes us to use efficient block memory allocation for
  // knapsackItems.

  INSERT_CHUNKALLOC

};


// Shortcut operators for reading writing knapsack items to/from streams


// Forward declarations...

class binaryKnapsack;
class binKnapSub;       



//  Class that describes solutions to binary knapsack problems.
//  This version uses a compacted representation: if the solution starts with 
//  k most desirable items in sequence with no gaps, we just store k rather 
//  than explicitly storing the sequence 0,1,...,k-1.

class binKnapSolution :
public binKnapBase, public solution
{
 public:

  // Overrides of basic solution class methods

  const char* typeDescription() const { return "Binary knapsack solution"; };

  void printContents(std::ostream& s);

#ifdef ACRO_HAVE_MPI

  void packContents(PackBuffer& outBuf);
  void unpackContents(UnPackBuffer& inBuf);
  int  maxContentsBufSize();

  solution* blankClone() { return new binKnapSolution(global); };

#endif

  // Publically accessible data members.

  double left;

  // These let you access the actual explicit set of items.

  int items() { return initialSequence + genItems; };

  int item(int i)
    {
      if (i < initialSequence)
	return i;
      else
	return genItem[i - initialSequence];
    };

  void addItem(int i);      // Insert an item
  void reset(int k);        // Roll back to just the k best items

  // These relate to the incumbent heuristic

  void heuristic();            // Full incumbent heuristic
  void completeGreedy();       // Fill out knapsack from end
  void squeezeInGreedy();      // Fill out, possibly using items we've skipped
  void completeRandom();       // Fill out from end, but randomly skip items
  int  chooseBackTrackItem();  // Randomly choose an amount to roll back
  void backTrack(int toWhere); // Roll back to "toWhere" first items here

  void copy(binKnapSolution* toCopy);

  void foundSolution(syncType sync = notSynchronous);

  binKnapSolution(binaryKnapsack* global_);

  binKnapSolution(binKnapSolution* toCopy);

  ~binKnapSolution() { };

protected:

  binaryKnapsack* global;

  int       initialSequence;
  int       genItems;
  IntVector genItem;

  int lastItem;

  int& maxGenSize();

  inline int         numItems();
  inline double      itemWeight(int i);
  inline double      itemValue(int i);
  inline CharString& itemName(int i);

  size_type sequenceLength() { return genItems + 1; };

  double sequenceData();

};


//  The branching class...

class binaryKnapsack : 
virtual public branching,
public binKnapBase,
public binKnapParams
{
public:

  double capacity;
  int    numItems;

  BasicArray<knapsackItem> item;  // Items sorted by "bang for buck"

  DoubleVector partialSumW;       // Element k is sum of all items with
  DoubleVector partialSumV;       // index < k.  W is weight, V is value.

  int fitSize;       // Largest sequence of initial elemets 
                     // that fits the knapsack
  double minWeight;  // Size of smallest element

  double sumOfAllValues;

  int maxGenSize;    // Current size to make solution arrays

  binKnapSolution workingSol;

  binaryKnapsack(); 
                
  ~binaryKnapsack() { };

  bool setupProblem(int& argc,char**& argv);

  void preprocess();
  double aPrioriBound() { return sumOfAllValues; };
  solution* initialGuess();
  branchSub* blankSub();
  bool haveIncumbentHeuristic() { return true; };

  int fitSequence(double weightLimit,int maxLength);

#ifdef MEMORY_TRACKING
  // Temporary(?) addition to test memory use
  void memoryUsageTest();
#endif

};


inline int& binKnapSolution::maxGenSize() { return global->maxGenSize; };


//  For use in the constructor compiled code, and also in derived
//  parallel classes
#ifdef  BRANCHING_CONSTRUCTOR
#undef  BRANCHING_CONSTRUCTOR
#endif
#define BRANCHING_CONSTRUCTOR \
branching(maximization,relTolerance(),absTolerance())


// These definitions for the binKnapSolution class had to be deferred
// until binaryKnapsack was defined.

inline int binKnapSolution::numItems() 
{
  return global->numItems; 
};
  
inline double binKnapSolution::itemWeight(int i) 
{ 
  return global->item[i].weight; 
};
  
inline double binKnapSolution::itemValue(int i)
{ 
  return global->item[i].value; 
};
  
inline CharString& binKnapSolution::itemName(int i)
{ 
  return global->item[i].name; 
};

inline void binKnapSolution::foundSolution(syncType sync) 
{ 
  global->foundSolution(new binKnapSolution(this),sync); 
};


//  The branchSub class...

class binKnapSub : 
  virtual public branchSub,
  public binKnapBase
{
public:

  inline binaryKnapsack* global() const { return globalPtr; };

  branching* bGlobal() const { return global(); };

  REFER_DEBUG(global())

  double capBase;

  binKnapSolution* workingSol() { return &(globalPtr->workingSol); };

  IntVector inList;
  IntVector outList;
  int numIn;
  int numOut;

  int splitItem;
  enum { terminal = -1, notSplit = -2 };

  int       tSplitInitial;  // Data needed when splitting terminal
  IntVector tSplitGenItem;  // subproblems for enumeration

  binKnapSub() {};

  void binKnapSubFromKnapsack(binaryKnapsack* master);
  void binKnapSubAsChildOf(binKnapSub* parent,int whichChild);
  virtual ~binKnapSub();

  virtual void setRootComputation();

  void boundComputation(double* controlParam);

  solution* extractSolution()
    {
      return new binKnapSolution(workingSol());
    }

  void incumbentHeuristic();

  virtual int splitComputation() 
    { 
      setState(separated);
      if (splitItem == terminal)
	{
	  DEBUGPR(20,ucout << "Splitting terminal problem for enumeration\n");
	  return tSplitInitial + tSplitGenItem.size();
	}
      return 2; 
    };

  virtual branchSub* makeChild(int whichChild)
    {
      binKnapSub *temp = new binKnapSub;
      temp->binKnapSubAsChildOf(this, whichChild);
      return temp;
    };
  
  bool candidateSolution() { return (splitItem == terminal); };

  void foundSolution(syncType sync = notSynchronous) 
    { 
      workingSol()->foundSolution(sync); 
    };

  void makeCurrentEffect();

#ifdef MEMORY_TRACKING
  void printMemDetails();
#endif

protected:

  binaryKnapsack* globalPtr;

  static void growList(IntVector& newList, 
		       IntVector& oldList, 
		       int newElement);

  inline int         numItems()        { return global()->numItems;       };
  inline double      itemWeight(int i) { return global()->item[i].weight; };
  inline double      itemValue(int i)  { return global()->item[i].value;  };
  inline CharString& itemName(int i)   { return global()->item[i].name;   };

  void dumpLists(const char* extraString = "");

  int tSplitItem(int i);
};


// Now we have enough information to define...

inline branchSub* binaryKnapsack::blankSub()
{
  binKnapSub *temp = new binKnapSub();
  temp->binKnapSubFromKnapsack(this);
  return temp;
};


// This little utility class is used by binaryKnapsack::boundComputation...

class itemListCursor :
public binKnapBase
{
public:

  IntVector* array;
  int size,cursor,next;

  inline void advance() 
    { 
      if (cursor < size) 
	next = (*array)[cursor++]; 
      else
	next = noItem;
    };     

  itemListCursor(int size_,IntVector* array_) :
    array(array_),
    size(size_),
    cursor(0)
    {
      advance();
    };
};

} // namespace pebbl

std::ostream& operator<<(std::ostream& os, pebbl::knapsackItem& item);
std::istream& operator>>(std::istream& is, pebbl::knapsackItem& item);

#endif
