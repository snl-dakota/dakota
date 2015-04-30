/*  _________________________________________________________________________
 *
 *  PICO: A C++ library of scalable branch-and-bound and related methods.
 *  Copyright (c) 2001, Sandia National Laboratories.
 *  This software is distributed under the GNU Lesser General Public License.
 *  For more information, see the README file in the top PICO directory.
 *  _________________________________________________________________________
 */
//
// Maximum Monomial
// parMonomial.cpp
// Noam Goldberg 11/2/2008

#include <acro_config.h>
#ifdef ACRO_HAVE_MPI

#include <utilib/exception_mngr.h>

#include <stdlib.h>
#include <sstream>
#include <algorithm>
#include <vector>

#include <utilib/stl_auxiliary.h>
#include <utilib/PackBuf.h>
#include <utilib/BitArray.h>
#include <utilib/std_headers.h>
#include "parMonomial.h"
#include "serialMonomial.h"


using namespace utilib;
using namespace std;

namespace pebblMonom {

 
  void parMaxMonomialData::pack(PackBuffer& outBuf)
  {
    DEBUGPR(10,ucout << "parMaxMonomialData::pack invoked..." << endl);
    outBuf <<  _attribNum << _obsNum << _improvedBound << _saveCoverages
	   << _threeWayBranching << _branchingFactor << _nonUniformWt;
#ifdef ACRO_USING_COIN_CBC
    outBuf << _iterations;
#endif
    outBuf << _wts;
    outBuf << _posIdx << _negIdx;
    // outBuf << _dataStore;
    BitArray compressedStore((_attribNum+1)*_obsNum);
    compressedStore.reset();
    for(size_type i=0; i<_obsNum; i++)
      for(size_type j=0; j<=_attribNum; j++)
	  if (_dataStore[i][j])
	    compressedStore.set((_attribNum+1)*i + j);
    outBuf << compressedStore;
    outBuf << _watching;
    if (_watching)
      outBuf << _watchVector;
    DEBUGPR(10,ucout << "parMaxMonomialData::pack done." << endl);
  }

  void parMaxMonomialData::unpack(UnPackBuffer &inBuffer)
  {
    DEBUGPR(10,ucout << "parMaxMonomialData::unpack invoked... " << endl);

    inBuffer >> _attribNum >> _obsNum >> _improvedBound >> _saveCoverages
	     >> _threeWayBranching >> _branchingFactor >> _nonUniformWt;
#ifdef ACRO_USING_COIN_CBC
    inBuffer >> _iterations;
#endif
    DEBUGPR(10,ucout << "parMaxMonomialData::unpack before unpacking _wts" 
	    << endl);
    inBuffer >> _wts;
    inBuffer >> _posIdx >> _negIdx;
    DEBUGPR(10,ucout << "parMaxMonomialData::unpack before unpacking "
	    "_dataStore" << endl);
    // inBuffer >>_dataStore;
    BitArray compressedStore;
    inBuffer >> compressedStore;
    _dataStore.resize(_obsNum);
    size_type cursor = 0;
    for(size_type i=0; i<_obsNum; i++)
      {
	_dataStore[i].resize(_attribNum+1,false);
	for(size_type j=0; j<=_attribNum; j++)
	  if (compressedStore[cursor++])
	    _dataStore[i][j] = true;
      }
    inBuffer >> _watching;
    if (_watching)
      inBuffer >> _watchVector;
    DEBUGPR(10,ucout << "parMaxMonomialData::unpack after unpacking "
	    "_dataStore" << endl);
    calculateVarCovg();
    DEBUGPR(10,ucout << "parMaxMonomialData::unpack done." << endl);
    DEBUGPR(20,for(size_type i=0; i<_obsNum; i++)
		 {
		   observationEntry_t obsE(i,_dataStore[i]);
		   ucout << obsE << endl;
		   if (_nonUniformWt)
		     ucout <<" wt: " << _wts[i] << endl;
		 });

    startVerifyLogIfNeeded();
  }


  int parMaxMonomialData::spPackSize()
  {
    int sizeOfOneSP =
      sizeof(size_type) +                      // monomialObj::_lastSetIdx
      _attribNum*sizeof(variable_val_t) +      // monomialObj::vars
      2*sizeof(double) +                       // _posCovg, _negCovg
      2*((_obsNum+1)*sizeof(size_type)) +      // _posCovgIdx, _negCovgIdx
      sizeof(double) +                         // _insepWeight
      sizeof(size_type);                       // branching variable

    return 4*sizeOfOneSP + 3*spGenericPackSize();
  }


  void parMaxMonomialData::setWeight(const double vec[], size_type len)
  {
    DEBUGPR(20,ucout << "In parMaxMonomialData::setWeight" << endl);
    maxMonomialData::setWeight(vec,len);
    if(uMPI::iDoIO)            // If we are the special IO processor
      {
	// Pack everything into a buffer.
	PackBuffer outBuf(_obsNum *sizeof(double) + 500);
	outBuf << _wts;
	int probSize = outBuf.size();        // Figure out length.
	DEBUGPR(70,ucout << "Broadcast size is " << probSize << " bytes.\n");
	uMPI::broadcast(&probSize,     // Broadcast length.
			1,
			MPI_INT,
			uMPI::ioProc);
	uMPI::broadcast((void*) outBuf.buf(), // Now broadcast buffer itself.
			probSize,
			MPI_PACKED,
			uMPI::ioProc);
      }
    else   // On the other processors, we receive the same information...
      {
	int probSize;                          // Get length of buffer
	uMPI::broadcast(&probSize,             // we're going to get.
			1,
			MPI_INT,
			uMPI::ioProc);
	DEBUGPR(70,ucout << "Received broadcast size is " << probSize <<
		" bytes.\n");
	UnPackBuffer inBuf(probSize);          // Create a big enough
	inBuf.reset(probSize);                 // temporary buffer.
	uMPI::broadcast((void *) inBuf.buf(),  // Get the data...
			probSize,
			MPI_PACKED,
			uMPI::ioProc);
	DEBUGPR(100,ucout << "Broadcast received.\n");
	inBuf >> _wts;                  
	DEBUGPR(100,ucout << "Unpack seems successful.\n");
      } 
    DEBUGPR(20,ucout << "In parMaxMonomialData::setWeight" << endl);
  }


  ostream* parMaxMonomialData::openVerifyLogFile()
  {
    char name[32];
      sprintf(name,"verify%05d.mmaLog",uMPI::rank);
      return new ofstream(name,ios::out);
  }


  ////////////// parMaxMonomSubThreeWay /////////

  void parMaxMonomSubThreeWay::pack(utilib::PackBuffer &outBuffer)
  {
    DEBUGPRXP(20,pGlobal(), "parMaxMonomSubThreeWay::pack invoked..." << endl);

    outBuffer << _posCovg << _negCovg << _posCovgIdx << _negCovgIdx << _insepWt;
    _monom.pack(outBuffer);  
    DEBUGPRXP(20, pGlobal(), "parMaxMonomSubThreeWay::pack packed monom:" 
	      << _monom << endl);
    outBuffer << getBranchVar();

    outBuffer << (size_t)_children.size();

    DEBUGPRXP(20, pGlobal(), "parMaxMonomSubThreeWay::pack, before "
	      "packing children, num of children: " << _children.size() 
	      << endl);
    childrenVecType::iterator it = _children.begin();
    for (; it != _children.end(); it++)
      {
	if (*it)
	  {
	    outBuffer << (bool)true;
	    parMaxMonomSubThreeWay * parChild 
	      = static_cast<parMaxMonomSubThreeWay *>(*it);
            parChild->packProblem(outBuffer);
	    DEBUGPRXP(20,pGlobal(),"parMaxMonomSubThreeWay::pack packed child:" 
		      << parChild->getMonomialObj() << endl);
	  }
	else
	  {
	    outBuffer << (bool)false;
	  }
      }

    DEBUGPRXP(20, pGlobal(), "parMaxMonomSubThreeWay::pack done. monom: " 
	      << _monom << " bound: " << bound << endl);
  }


  void parMaxMonomSubThreeWay::unpack(utilib::UnPackBuffer &inBuffer)
  {
    DEBUGPRXP(20,pGlobal(), "parMaxMonomSubThreeWay::unpack invoked... monom:" 
	      << _monom << endl);

    inBuffer >> _posCovg >> _negCovg >> _posCovgIdx >> _negCovgIdx >> _insepWt;
    DEBUGPRXP(20,pGlobal(), "parMaxMonomSubThreeWay::unpack before "
	      "unpacking monom" << _monom << endl);
    _monom.unpack(inBuffer);
    inBuffer >> _branchChoice.branchVar;

    DEBUGPRXP(20,pGlobal(), "parMaxMonomSubThreeWay::unpack unpacked monom:" 
	      << _monom << " before unpacking children" << endl);

    size_t childNum;
    inBuffer >> childNum;

    DEBUGPRXP(20,pGlobal(), "parMaxMonomSubThreeWay::unpack before "
	      "unpacking children, num of children: " << childNum << endl);
    _children.resize(childNum);

    for (size_type i = 0; i < childNum; i++)
      {
	bool nonNull;
        inBuffer >> nonNull;
        DEBUGPRXP(20,pGlobal(), "parMaxMonomSubThreeWay::unpack before unpacking child num: " << i << " nonNull: " << nonNull << endl);

	if (nonNull)
	  {
	    parMaxMonomSubThreeWay * tmp = new parMaxMonomSubThreeWay();
	    tmp->setGlobalInfo(globalPtr);
	    tmp->setMPGlobal(mpGlobal);
	    tmp->unpackProblem(inBuffer);
	    DEBUGPRXP(20,global(),"parMaxMonomSubThreeWay::pack "
		      "unpacked child:" << tmp->getMonomialObj() << endl);
	    _children[i]= static_cast<maxMonomSub *>(tmp);
	  }
	else
	  {
	    _children[i] = NULL;
	    DEBUGPRX(25,pGlobal(),"parMaxMonomSubThreeWay::pack unpacked "
		     "null child" << i << endl);
          }
      }

    DEBUGPRX(20,pGlobal(),"parMaxMonomSubThreeWay::unpack done. monom:" 
	     << _monom << " bound: " << bound << endl);
  }


  // makeParallelChild
  parallelBranchSub * parMaxMonomSubThreeWay::makeParallelChild(int whichChild)
  {
    DEBUGPRX(20,global(), "parMaxMonomSubThreeWay::makeParallelChild "
	     "invoked for: " << _monom << ", whichChild: " << whichChild 
	     << ", ramp-up flag: " << rampingUp() << endl);

    parMaxMonomSubThreeWay* ret = NULL;

#ifdef ACRO_VALIDATING
    if (whichChild < 0 || whichChild > 2)
      {
	ucout << "parMaxMonomSubThreeWay::makeParallelChild: invalid request "
	      << "for child " << whichChild << endl;
	return NULL;
      }
    if ((_branchChoice.branchVar < 0) || 
	 (_branchChoice.branchVar >= global()->attribNum()))
      {
	ucout << "parMaxMonomSubThreeWay::makeParallelChild: "
	      << "invalid branching variable\n";
	return NULL;
      }
#endif

    // If there are no cached children (because this subproblem was
    // sent from somewhere else), recreate a child, not necessarily in
    // bound-sorted order.  Otherwise, grab it from the cache.

    // NOTE -- it appears this does not work, so sending embedded
    // children for now

    if (_children.size() == 0)
      {
	ret = new parMaxMonomSubThreeWay();
	ret->setMPGlobal(mpGlobal);
	ret->maxMonomSubAsChildOf(this, 3*getBranchVar() + whichChild);
      }
    else
      {
	ret = dynamic_cast<parMaxMonomSubThreeWay *>(_children[whichChild]);
	_children[whichChild] = NULL;
#ifdef ACRO_VALIDATING
	if (ret == NULL)
	  {
	    ucout << "Casting problem in "
		  << "parMaxMonomSubThreeWay::makeParallelChild";
	    return NULL;
	  }
#endif
      }

    double saveBound = ret->bound;
    ret->branchSubAsChildOf(this);   // Overwrites bound with parent bound
    ret->bound = saveBound;          // Put bound back how it was

    DEBUGPR(10,ucout << "Parallel MakeChild produces " << ret 
	    << ' ' << ret->getMonomialObj() << endl);

    if (global()->watching() && 
	(ret->getMonomialObj().covers(global()->watchVector())))
      {
	ucout << "Watched subproblem: depth ";
	int oldWidth = ucout.width(6);
	ucout <<  ret->depth;
	ucout.width(oldWidth);
	ucout << ' ' << ret << ", child of " << this;
	ucout << ", branched on x" << getBranchVar();
	ucout << endl;
      }

    if (global()->verifyLog())
      {
	ostream& vlFile = global()->verifyLogFile();
	vlFile << "create ";
	ret->vlWriteDescription();
	vlFile << ' ' << ret->id.creatingProcessor
	       << ' ' << ret->id.serial
	       << ' ' << ret->bound << ' ';
	vlWriteDescription();
	vlFile << endl;
      }

    DEBUGPRX(20,global(),"Out of parMaxMonomSubThreeWay::makeParallelChild, "
	     "whichChild: " << whichChild << " bound: " << bound << endl);	

    return ret;
  }

 
  inline parallelBranchSub* parMaxMonomialData::blankParallelSub()
  {
    parMaxMonomSubThreeWay *temp = NULL;
    if (threeWayBranching())
      temp = new parMaxMonomSubThreeWay();
    else
      {
	cerr << "ERROR: parallel branching supported only "
	     << "in three way mode" << endl;
	assert(false);
      }

    ((maxMonomSubThreeWay *)temp)->maxMonomSubFromData(this);
    temp->setGlobalInfo(this);
    temp->setMPGlobal(this);
    return temp;
  };


  // Bound computation -- unless we're in ramp-up, just do the same
  // thing as the serial three-way code.  If we're in ramp-up, try to
  // parallelize the strong branching procedure

  void parMaxMonomSubThreeWay::boundComputation(double* controlParam)
  {
    DEBUGPR(10,ucout << "In parMaxMonomSubThreeWay::boundComputation, "
	    << "ramp-up flag=" << rampingUp() << endl);

    if (!rampingUp())
      {
	maxMonomSubThreeWay::boundComputation(controlParam);
	return;
      }

    // Special handling of ramp-up

    DEBUGPR(10,ucout << "Ramp-up bound computation\n");

    // Recompute coverages if they were not stored.

    if (global()->getLowNodeMemory())
      {
	global()->getPosCovgFast(_monom,_posCovgIdx);
	global()->getNegCovgFast(_monom,_negCovgIdx);
      }

    // Make an array of all the free variables, and count them

    size_type n = global()->attribNum();
    vector<size_type> freeVariable;
    freeVariable.reserve(n);

    for (size_type i = 0; i < global()->attribNum(); i++)
      if (_monom.getVarVal(i) == NULL_VAL)
	freeVariable.push_back(i);

    size_type numFree = freeVariable.size();

    DEBUGPR(10,ucout << numFree << " free variables out of " << n << endl);

    // Figure which variables go on which processor.  Make them as
    // even as possible -- the first (remainder) processors have one
    // more variable

    size_type quotient  = numFree / uMPI::size;
    size_type remainder = numFree % uMPI::size;

    size_type myFirstIndex = uMPI::rank*quotient + min(uMPI::rank,remainder);
    size_type indexAfterMe = myFirstIndex + quotient + (uMPI::rank < remainder);

    DEBUGPR(10,ucout << "My first index is " << myFirstIndex
	    << ", index limit is " << indexAfterMe << endl);

    // Now try all the indices assigned to this processor (this could
    // be none if numFree < #processors and rank >= remainder

    for (size_type i = myFirstIndex; 
	 i < indexAfterMe && state != dead; 
	 i++)
      tryThreeWaySplit(freeVariable[i]);

    DEBUGPR(10,ucout << "Best local choice is " << _branchChoice << endl);

    // Better incumbents may have been found along the way

    pGlobal()->rampUpIncumbentSync();

    // Now determine the globally best branching choice by global reduction.
    // Use the special MPI type and combiner for branch choices.

    branchChoice bestBranch;

    uMPI::reduceCast(&_branchChoice,
		     &bestBranch,
		     1,
		     branchChoice::mpiType,
		     branchChoice::mpiCombiner);

    pGlobal()->rampUpMessages += 2*(uMPI::rank > 0);

    DEBUGPR(10,ucout << "Best global choice is " << bestBranch << endl);

    // Now figure out if we had the best choice.  If so, there is
    // nothing to do.  Otherwise, adjust everything so it looks like
    // we made the globally best choice.

    if (bestBranch.branchVar != _branchChoice.branchVar)
      {
	DEBUGPR(10,ucout << "Adjusting local choice\n");
	// Kill off any unneeded children from local choice
	childrenVecType::iterator iter;
	for (iter = _children.begin(); iter != _children.end(); iter++)
	  delete *iter;
	_children.resize(0);
	// Set choice
	_branchChoice = bestBranch;
	// Recreate children
	for (size_type j=0; j<3; j++)
	  {
	    // If whichChild < 0, that means there are no more children
	    // in this branch choice, so bail out
	    if (bestBranch.branch[j].whichChild < 0)
	      break;
	    // Make the correct child for this branch choice, in order
	    maxMonomSub* tmp = allocateObject();
	    int childCode = 3*bestBranch.branchVar 
	                       + bestBranch.branch[j].whichChild;
	    // As we create the child, specify that the bound is
	    // already known, so we don't waste time recomputing it
	    tmp->maxMonomSubAsChildOf(this,childCode,
				      bestBranch.branch[j].exactBound);
	    DEBUGPR(15,ucout << "Created " << tmp << endl);
	    _children.push_back(tmp);
	  }
      }

    // If no children, this branching disjunctions proves we are at a 
    // dead end, so set the state to dead.
    if (_children.size() == 0)
      setState(dead);

    // Now clean up.  This is exactly the same as in serial or after ramp-up

    boundComputationCleanUp(controlParam);
    
    DEBUGPR(9,ucout << "Ending ramp-up bound computation for: "
	    << getMonomialObj() << " bound: " << bound << endl);

  }


}
#endif
