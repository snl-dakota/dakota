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
// knapsack.cpp
//
//  Implements larger methods in example of how to use object-oriented 
//  branching framework (for binary knapsack problems).
//
// Jonathan Eckstein
//

#include <acro_config.h>
#include <utilib/exception_mngr.h>
#include <utilib/GenericHeap.h>
#include <utilib/comments.h>
#include <pebbl/serialKnapsack.h>
#include <pebbl/fundamentals.h>
#include <pebbl/gRandom.h>

using namespace utilib;
using namespace std;

namespace pebbl {



//  Set up the efficient allocation of knapsack items from the free store

CHUNK_ALLOCATOR_DEF(knapsackItem,100);


//  Constructors for branching class
  binaryKnapsack::binaryKnapsack() :
    maxGenSize(0),
    workingSol(this)
{
  version_info += ", knapsack example 1.1";
  min_num_required_args = 1;
  branchingInit(maximization, relTolerance, absTolerance);
  workingSol.serial = 0;
  workingSol.sense  = maximization;
};


//  To read in the problem.  This routine now makes use of UTILIB
//  containers.

bool binaryKnapsack::setupProblem(int& argc,char**& argv)
{
  numItems       = 0;
  sumOfAllValues = 0;

  if (argc <= 1)
    {
      cerr << "No filename specified\n";
      return false;
    }
  ifstream s(argv[1]);
  if (!s)
    {
      cerr << "Could not open file \"" << argv[1] << "\"\n";
      return false;
    }

  s >> capacity >> whitespace;
  if (capacity < 0)
    {
      cerr << "Negative knapsack capacity of " << capacity 
	   << " specified.\n";
      return false;
    }

  // Read in all the items and throw them on a heap

  int allInteger = TRUE;

  GenericHeap<knapsackItem> heap;

  while(!(s.eof())) 
    {
      double w,v;

      utilib::CharString tempName;
      
      s >> tempName;
      if (s.eof())
	break;

      s >> w;
      if (s.eof())
	break;

      s >> v >> whitespace;

      if (w < 0)
	cerr << "Ignoring negative weight item " << tempName << '\n';
      else if (v <= 0)
	cerr << "Ignoring nonpositive value item " << tempName << '\n';
      else if (w > capacity)
	cerr << "Ignoring item " << tempName << " larger than knapsack\n";
      else 
	{
	  heap.add(*(new knapsackItem(w,v,tempName)));
	  if (v != floor(v))
	    allInteger = FALSE;
	  numItems++;
	  sumOfAllValues += v;
	}
    }

  if (allInteger && (absTolerance < 1))
    absTolerance = 1;

  // Pull the items out in order of "bang for the buck" and put in the
  // "item" array.

  item.resize(numItems);

  for(int i=0; i<numItems; i++)
    {
      GenericHeapItem<knapsackItem>* topOfHeap = heap.top();
      knapsackItem* itemPtr = &(topOfHeap->key());
      item[i] = *itemPtr;
      bool status;
      heap.remove(topOfHeap,status);
      delete itemPtr;
    }

  // Reclaim memory we used for storing the original items read in.

  knapsackItem::memClear();

  return true;

};


void binaryKnapsack::preprocess() 
{
  partialSumW.resize(numItems + 1);
  partialSumV.resize(numItems + 1);

  minWeight = MAXDOUBLE;
  double runningSumV = 0;
  double runningSumW = 0;

  for(int j=0; j<=numItems; j++) 
    {
      partialSumV[j] = runningSumV;
      partialSumW[j] = runningSumW;
      if (runningSumW <= capacity)
	fitSize = j;

      if (j < numItems)
	{
	  runningSumV += item[j].value;
	  double temp  = item[j].weight;
	  runningSumW += temp;
	  if (temp < minWeight)
	    minWeight = temp;
	}
    }

  DEBUGPR(10,ucout << "Capacity = " << capacity << '\n');
  DEBUGPR(200,ucout << "Items in order:\n");
  for(int ii=0; verbosity(20) && ii<numItems; ii++) 
    {
      DEBUGPR(200,ucout << "  " << item[ii].name
	      << ": weight=" << item[ii].weight
	      << " value="   << item[ii].value
	      << " ratio="   << item[ii].value/item[ii].weight
	      << endl);
    }
  DEBUGPR(10,ucout << "Abstolerance=" << absTolerance << "\n\n");

}


solution* binaryKnapsack::initialGuess() 
{
  binKnapSolution* guess = new binKnapSolution(this);
  guess->reset(fitSize);
  guess->completeGreedy();
  DEBUGPR(200,ucout << "Initial Greedy solution:\n"; guess->print(ucout));
  return guess;
};


binKnapSolution::binKnapSolution(binaryKnapsack* global_) :
  solution(global_),
  global(global_)
{ 
  DEBUGPRX(200,global,"Creating binKnapSolution at " << (void*) this 
	   << " with global=" << global << endl);
  // Only one solution representation in this application,
  // so typeId can just be 0.
  typeId = 0;
  genItem.resize(maxGenSize());
};


binKnapSolution::binKnapSolution(binKnapSolution* toCopy) 
{ 
  DEBUGPRX(200,toCopy->global,"Copy constructing binKnapSolution at "
	   << (void*) this << " from " << toCopy << endl);
  copy(toCopy);
  serial = ++(global->solSerialCounter);
};



void binKnapSolution::reset(int k)
{
  genItems        = 0;
  initialSequence = k;
  value           = global->partialSumV[k];
  left            = global->capacity - global->partialSumW[k];
  lastItem        = k - 1;
  DEBUGPRX(200,global,"Reset to " 
	   << (lastItem < 0 ? "beginning" : itemName(lastItem).data())
	   << " {" << value << ',' << left << "} shared="
	   << genItem.shared_mem() << '\n');
  DEBUGPRX(210,global,"Contains: "; 
	   for(int i=0; i<k; i++) ucout << itemName(i) << ' ';
	   ucout << endl);
}


void binKnapSolution::copy(binKnapSolution* toCopy)
{
  solution::copy(toCopy);

  global          = toCopy->global;
  initialSequence = toCopy->initialSequence;
  genItems        = toCopy->genItems;
  left            = toCopy->left;

  // Copy over array of general items.

  genItem << toCopy->genItem;

}


void 
binKnapSub::growList(IntVector& newList, IntVector& oldList, int newElement)
{
  newList.resize(oldList.size() + 1);
  
  int newCursor = 0;
  int oldCursor = 0;
  size_t i;

  for(i=0; (i < oldList.size()) && (oldList[i] < newElement); i++)
    newList[newCursor++] = oldList[oldCursor++];

  newList[newCursor++] = newElement;

  for(; i < oldList.size(); i++)
    newList[newCursor++] = oldList[oldCursor++];
}


void binKnapSub::binKnapSubFromKnapsack(binaryKnapsack* master)
{
  capBase = master->capacity;
  splitItem = notSplit;
  globalPtr = master;

  tSplitInitial = notSplit;

  DEBUGPR(20,ucout << "Created blank problem.\n");
};


void binKnapSub::binKnapSubAsChildOf(binKnapSub* parent,int whichChild)
{
  capBase = parent->capBase;
  splitItem = notSplit;
  globalPtr = parent->global();

  tSplitInitial = notSplit;

  branchSubAsChildOf(parent);

  DEBUGPR(10,ucout << "Creating child " << whichChild << " of " 
	  << parent << '\n');
  DEBUGPR(200,parent->dumpLists("Parent "));

  int splitter = parent->splitItem;

  if (splitter == terminal)
    {
#ifdef ACRO_VALIDATING
      if (!bGlobal()->enumerating)
	EXCEPTION_MNGR(runtime_error, "Trying to split terminal problem,"
		       " but not enumerating");
#endif
      int parentNumIn = parent->numIn;
      numIn  = parentNumIn + whichChild;
      numOut = parent->numOut + 1;
      inList.resize(numIn);

      int i = 0;
      int p = 0;

      for(int t=0; t<whichChild; t++)
	{
	  int inItem = parent->tSplitItem(t);
	  while((p < parentNumIn) && (parent->inList[p] < inItem))
	    inList[i++] = parent->inList[p++];
	  inList[i++] = inItem;
	  capBase -= itemWeight(inItem);
	}
      while(p < parentNumIn)
	inList[i++] = parent->inList[p++];

      growList(outList,parent->outList,parent->tSplitItem(whichChild));
    }
  else if ((splitter < 0) || (splitter >= numItems()))
     EXCEPTION_MNGR(runtime_error, "Bad splitting item, index = " << splitter);
  else if (whichChild == 0)
    {
      int parentNumIn = parent->numIn;   // The "in" child
      numIn = parentNumIn + 1;
      growList(inList,parent->inList,splitter);
      numOut = parent->numOut;
      outList &= parent->outList;
      capBase -= itemWeight(splitter);
      if (capBase < 0)
	{
	  bound = -MAXDOUBLE;
	  setState(dead,globalPtr);
	  DEBUGPR(20,ucout << "(Dead problem)\n";);
	}
      DEBUGPR(20,ucout << "Must include " << itemName(splitter) 
	      << '\n');
    }
  else if (whichChild == 1)
    {
      numIn = parent->numIn;              // The "out" child
      inList &= parent->inList;
      int parentNumOut = parent->numOut;
      numOut = parentNumOut + 1;
      growList(outList,parent->outList,splitter);
      DEBUGPR(20,ucout << "Cannot include " << itemName(splitter) 
	      << '\n');
    }
  else
    EXCEPTION_MNGR(runtime_error,
		    "Bad child number = " << whichChild 
		   << " passed to knapsack child constructor");

  DEBUGPR(100,ucout << "inList:  ";  inList.debug_print(ucout));
  DEBUGPR(100,ucout << "outList: "; outList.debug_print(ucout));
  DEBUGPR(200,dumpLists("New "));
  DEBUGPR(20,ucout << "capBase=" << capBase << '\n');
}


int binKnapSub::tSplitItem(int i)
{
  if (i < tSplitInitial)
    return i;
  return tSplitGenItem[i - tSplitInitial];
}


void binKnapSub::setRootComputation()
{
  numIn  = 0;
  numOut = 0;
  inList.resize(0);
  outList.resize(0);
  DEBUGPR(10,ucout << "binKnapSub::setRootComputation() invoked.\n");
  DEBUGPR(100,ucout << "inList:  ";  inList.debug_print(ucout));
  DEBUGPR(100,ucout << "outList: "; outList.debug_print(ucout));
};



binKnapSub::~binKnapSub() 
{
  DEBUGPR(10,ucout << "Destroying " << this << '\n');
  DEBUGPR(100,ucout << "inList:  ";  inList.debug_print(ucout));
  DEBUGPR(100,ucout << "outList: "; outList.debug_print(ucout));
}


void binKnapSub::dumpLists(const char* extraString)
{
  cout << extraString << "numIn=" << numIn << ':';
  int i;
  for(i=0; i<numIn; i++)
    cout << ' ' << itemName(inList[i]);
  cout << endl;
  cout << extraString << "numOut=" << numOut << ':';
  for(i=0; i<numOut; i++)
    cout << ' ' << itemName(outList[i]);
  cout << endl;
}


void binKnapSub::boundComputation(double* controlParam) 
{
  *controlParam = 1;

  DEBUGPR(200,dumpLists());

  itemListCursor forceIn(numIn,&inList);
  itemListCursor forceOut(numOut,&outList);

  binKnapSolution* wsol = workingSol();

  splitItem = notSplit;

  // The initial sequence of the solution must not contain any forced-in
  // or forced-out items.

  int r = global()->fitSize;
  if ((forceIn.next  != noItem) && (forceIn.next  < r))
    r = forceIn.next;
  if ((forceOut.next != noItem) && (forceOut.next < r))
    r = forceOut.next;

  // It must also have total weight small enough to leave room for all
  // the forced-in items.  Figure this out and set up the initial solution.
  
  int i = global()->fitSequence(capBase,r);
  wsol->reset(i);
  double spaceLeft = capBase - global()->partialSumW[i];

  // Bookkeeping needed in case this turns out to be a terminal node
  // and it nees to be split for enumeration.

  tSplitInitial = i;
  int inNotForcedCount = 0;
  IntVector inNotForced(numItems() - i - numIn - numOut);

  // Do a loop over all the items not in the initial solution.

  for(; (i < numItems()) && (spaceLeft > 0); i++)
    {
      if (i == forceOut.next)
	forceOut.advance();
      else if (i == forceIn.next)
	{
	  wsol->addItem(i);
	  forceIn.advance();
	}
      else if (itemWeight(i) > spaceLeft)
	{
	  splitItem = i;
	  integralityMeasure = spaceLeft/itemWeight(i);
	  DEBUGPRP(200,cout << '(' << spaceLeft << '/' << itemWeight(i) << ')'
		   << itemName(i) << ' ');
	  break;
	}
      else 
	{
	  wsol->addItem(i);
	  spaceLeft -= itemWeight(i);
	  inNotForced[inNotForcedCount++] = i;
	}
    }

  while(forceIn.next != itemListCursor::noItem)
    {
      wsol->addItem(forceIn.next);
      forceIn.advance();
    }
  DEBUGPR(200,cout << '\n');

  bound = wsol->value;

  if (splitItem == notSplit)
    {
      DEBUGPR(20,cout << "[Terminal]\n");
      splitItem = terminal;
      integralityMeasure = 0;
      if (bGlobal()->enumerating)
	{
	  tSplitGenItem.resize(inNotForcedCount);
	  for(int k=0; k<inNotForcedCount; k++)
	    tSplitGenItem[k] = inNotForced[k];
	}
    }
  else
    bound += integralityMeasure*itemValue(splitItem);

  setState(bounded);
}


//  Add an item to a knapsack solution.  Note that items MUST be added in 
//  order, with the best items first (as determined by the preprocess sort).

void binKnapSolution::addItem(int i)
{
#ifdef ACRO_VALIDATING
  if ((i < 0) || (i >= numItems()) || (i <= lastItem))
    EXCEPTION_MNGR(runtime_error, "Invalid knapsack insertion: i=" << i << " lastItem=" << lastItem);
#endif
  if (i == initialSequence)
    initialSequence++;
  else
    {
      if ((unsigned int)genItems == genItem.size())
	{
	  if ((unsigned int)maxGenSize() == genItem.size())
	    maxGenSize() += global->itemArraySizeQuantum;
	  genItem.resize(maxGenSize());
	}
      genItem[genItems++] = i;
    }
  
  value  += itemValue(i);
  left   -= itemWeight(i);

  lastItem = i;
  
  DEBUGPRXP(200,global,itemName(i) << ' ');
  DEBUGPRX(210,global," shared=" << genItem.shared_mem() << ' ');
  
}


//  To complete a partial solution in a greedy way.  Only consider things
//  after "lastItem".

void binKnapSolution::completeGreedy()
{
  for(int i = lastItem + 1; 
      (i < numItems()) && (left > global->minWeight); 
      i++)
    if (itemWeight(i) <= left) 
      addItem(i);
  DEBUGPRX(20,global,'{' << left << ',' << value << "}\n");
  DEBUGPRX(200,global,"shared= " << genItem.shared_mem() << '\n');
};


// Complete a partial solution in a random, GRASP-like way.  Only consider 
// things after "lastItem".  Then try to squeeze in as many items as can
// fit in a greedy manner.

void binKnapSolution::completeRandom()
{
  for(int i = lastItem + 1; 
      (i < numItems()) && (left > global->minWeight); i++)
    if ((itemWeight(i) <= left) &&
	(gRandom() < global->randomCompletionGreediness)) 
      addItem(i);
  DEBUGPRXP(20,global,'{' << left << ',' << value << "} (squeezeIn) ");
  DEBUGPRX(210,global,"shared= " << genItem.shared_mem() << '\n');
  squeezeInGreedy();
};


//  This is similar to completeGreedy, but considers objects that we might
//  have skipped over earlier.  That makes it quite a bit more complicated,
//  unless we only have a simple initial sequence.

void binKnapSolution::squeezeInGreedy()
{
  if (genItems > 0)
    {
      binKnapSolution temp(global);  // Make a temporary solution.
      --(global->solSerialCounter);  // but don't use up a serial number
      temp.reset(initialSequence);
      int changesMade = FALSE;
      itemListCursor alreadyIn(genItems,&genItem);

      // Scan items that we may have skipped before.

      for(int i = initialSequence; 
	  (i < lastItem) && (left <= global->minWeight); 
	  i++)
	{
	  if (i == alreadyIn.next)
	    {
	      temp.addItem(i);
	      alreadyIn.advance();
	    }
	  else if (itemWeight(i) <= left)
	    {
	      temp.addItem(i);
	      left -= itemWeight(i);
	      changesMade = TRUE;
	    }
	}

      // If we accomplished anything, stick any remaining forced-in 
      // stuff and replace the current solution with the temporary.

      if (changesMade)
	{
	  while(alreadyIn.next != noItem)
	    {
	      temp.addItem(alreadyIn.next);
	      alreadyIn.advance();
	    }
	  copy(&temp);
	}
      
    }

  completeGreedy();  // No matter what, try to fill out greedily.

}


int binKnapSolution::chooseBackTrackItem()
{
  double approxHowFar = 1 - pow(1.0 - gRandom(),global->randomBackTrackPower);
  int howFar = (int) floor(items()*approxHowFar);
  DEBUGPRX(20,global,"Backtrack amount is " << howFar << '\n');
  DEBUGPRX(210,global,"shared= " << genItem.shared_mem() << '\n');
  return howFar;
}


void binKnapSub::incumbentHeuristic() 
{
  DEBUGPR(100,ucout << "Entered binKnapSub::incumbentHeuristic\n");
  if (state != bounded)
    return;
  DEBUGPR(10,ucout << "Trying to improve incumbent...\n");
  if (splitItem == terminal)
    { 
      if (bGlobal()->enumerating)
	return;
      else
	EXCEPTION_MNGR(runtime_error, "Trying to improve terminal solution");
    }
  workingSol()->heuristic();
}


// This runs when we load a subproblem.  If the problem is bounded, but
// some other problem was just loaded, we have to recompute the solution.

void binKnapSub::makeCurrentEffect()
{
  DEBUGPR(150,ucout << "Making " << this << " current problem.\n");
  if ((state == bounded) && !(id == bGlobal()->previousSPId))
    {
      DEBUGPR(20,ucout << "Regenerating solution...\n");
      double junk = 1;
      boundComputation(&junk);
    }
}



void binKnapSolution::heuristic()
{
  completeGreedy();
  if (value > global->incumbentValue) 
    foundSolution(synchronous);
  while(gRandom() <= global->randomSearchPersistence)
    {
      backTrack(chooseBackTrackItem());
      completeRandom();
      if (value > global->incumbentValue) 
	foundSolution(notSynchronous);
    }
  DEBUGPRX(210,global,"Heuristic: shared= " << genItem.shared_mem() << '\n');
}


void binKnapSolution::backTrack(int where)
{
#ifdef ACRO_VALIDATING
  if ((where < 0) || (where > items()))
    EXCEPTION_MNGR(runtime_error, "Invalid backtrack: where=" << where << " items()=" << items());
#endif
  if (where <= initialSequence)
    reset(where);
  else 
    {
      while(items() > where)
	{
	  int item  = genItem[--genItems];
	  value    -= itemValue(item);
	  left     += itemWeight(item);
	}
      lastItem = genItem[genItems - 1];
    }
  DEBUGPRX(20,global,"Backtrack(" << where 
	   << ") {" << left << ',' << value << "}\n");
  DEBUGPRX(210,global,"shared= " << genItem.shared_mem() << '\n');
}


//  Find the largest intialSequence of objects that is within "weightLimit"
//  and is no longer than "maxLength".  Since we have computed partial sums, 
//  this can be done by binary search.

int  binaryKnapsack::fitSequence(double weightLimit,int maxLength)
{
  int top    = maxLength;
  int bottom = 0;

  while(partialSumW[top] > weightLimit)
    {
      int middle = (top + bottom) >> 1;
      if ((partialSumW[middle] > weightLimit) || (middle == bottom))
	top = middle;
      else
	bottom = middle;
    }
  
  return top;
}


void binKnapSolution::printContents(ostream& outStream) 
{
  int i;

  // TODO: this may be doing some unnecessary string copying;
  // it might be better use an array of pointers or references.
  GenericHeap<utilib::CharString> nameHeap;
    
  for(i=0; i<items(); i++)
    nameHeap.add(itemName(item(i)));

  int lineLength = 1;
  for(i=0; i<items(); i++) 
    {
      GenericHeapItem<utilib::CharString>* topOfHeap = nameHeap.top();
      utilib::CharString& name = topOfHeap->key();
      
      int newLength =  lineLength + name.size() + 1;
      if ((lineLength > 1) && (newLength >= 79))
	{
	  outStream << endl;
	  newLength -= lineLength;
	}
      outStream << ' ' << name;
      lineLength = newLength;
      bool status;
      nameHeap.remove(topOfHeap,status);
    }
  outStream << '\n';
  outStream << "Free Space = " << left << endl;
};


double binKnapSolution::sequenceData() 
{
  if (sequenceCursor == 0)
    {
      sequenceCursor++;
      return initialSequence;
    }
  else
    return genItem[(sequenceCursor++) - 1];
}


#ifdef MEMORY_TRACKING

void binaryKnapsack::memoryUsageTest()
{
  cout << "IntVector: " << sizeof(IntVector) << " bytes.\n";
  cout << "BitArray: "  << sizeof(BitArray)  << " bytes.\n";
  cout << "binKnapSolution: " << sizeof(binKnapSolution) << " bytes.\n";
  cout << "coreSPInfo: " << sizeof(coreSPInfo) << " bytes.\n";
  cout << "branchSub: " << sizeof(branchSub) << " bytes.\n";
  cout << "binKnapSub: " << sizeof(binKnapSub) << " bytes.\n";

  memUtil::baseline();

  binKnapSub* sp = new binKnapSub();
  sp->binKnapSubFromKnapsack(this);
  memUtil::trackPrint("Creation");
  workingSol.reset(0);
  memUtil::trackPrint("Reset solution");
  delete sp;
  memUtil::trackPrint("Deletion");
}


void binKnapSub::printMemDetails()
{
  cout << this << " numIn=" << numIn << "numOut=" << numOut << endl;
}

#endif

#ifdef ACRO_HAVE_MPI

void binKnapSolution::packContents(PackBuffer& outBuf)
{
  outBuf << initialSequence;
  outBuf << genItems;
  for (int i=0; i<genItems; i++)
    outBuf << genItem[i];
  outBuf << left;
}

void binKnapSolution::unpackContents(UnPackBuffer& inBuf)
{
  int k = -1;
  inBuf >> k;
  reset(k);
  inBuf >> k;
  if (k > maxGenSize())
    {
      genItem.resize(k);
      maxGenSize() = k;
    }
  for(; k>0; k--)
    {
      int readItem = -1;
      inBuf >> readItem;
      addItem(readItem);
    }
  inBuf >> left;
}


int binKnapSolution::maxContentsBufSize()
{
  return (global->numItems + 2)*sizeof(int);
}

#endif // ACRO_HAVE_MPI


} // namespace pebbl

//  Operators to read and write knapsack items to streams

ostream& operator<<(ostream& os, pebbl::knapsackItem& item)
{
  item.write(os);
  return os;
}

istream& operator>>(istream& is, pebbl::knapsackItem& item)
{
  item.read(is);
  return is;
}

