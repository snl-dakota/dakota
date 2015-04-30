/*  _________________________________________________________________________
 *
 *  PICO: A C++ library of scalable branch-and-bound and related methods.
 *  Copyright (c) 2001, Sandia National Laboratories.
 *  This software is distributed under the GNU Lesser General Public License.
 *  For more information, see the README file in the top PICO directory.
 *  _________________________________________________________________________
 */

/**
 * \file serialMonomial.h
 * \author Noam Goldberg
 *
 *  Example class to use object-oriented branching framework.
 *  Solves the maximum monomial problem
 */

#ifndef pebbl_parmonom_h
#define pebbl_parmonom_h
#include <iostream>

#include <acro_config.h>

#ifdef ACRO_HAVE_MPI
#include <pebbl/parBranching.h>

#include "serialMonomial.h"


using namespace std;
using namespace pebbl;


namespace pebblMonom {

  //  The branching class

  class parMaxMonomialData : public parallelBranching,
                             public maxMonomialData
    {
 
    public:

    parMaxMonomialData() : maxMonomialData() 
	{
	  // Default is not to spend time on a dumb ramp up
	  rampUpPoolLimitFac = 1.0;
	  Parameter& p = get_parameter_object("rampUpPoolLimitFac");
	  p.default_value = "1.0";

	  rampUpFeatureFac = 1.0;
	  create_categorized_parameter("rampUpFeatureFac",
				       rampUpFeatureFac,
				       "<double>",
				       "1.0",
				       "Maximum number of subproblems "
				       "in pool to end ramp-up phase,\n\t"
				       "as a fraction of the total number "
				       "of features.",
				       "Maximum Monomial",
				       utilib::ParameterNonnegative<double>());

	  branchChoice::setupMPI();
	};     

      ~parMaxMonomialData() 
	{
	  branchChoice::freeMPI();
	};

      void pack(PackBuffer &outBuffer);
      void unpack(UnPackBuffer & inBuffer);
      int spPackSize();

      void reset(bool VBflag=true)
      {
	maxMonomialData::reset();
	registerFirstSolution(new maxMonomSolution(this));
	parallelBranching::reset();
      }

      virtual parallelBranchSub * blankParallelSub();

      virtual bool continueRampUp()
      {
	return (spCount() <= rampUpFeatureFac*attribNum())
	          && parallelBranching::continueRampUp();
      }

      void setWeight(const double vec[], size_type len);

      // In parallel, restrict writing to verification log to processor
      // 0 when ramping up.
      bool verifyLog()
      {
	return _verifyLog && (!rampingUp() || (uMPI::rank == 0));
      };

      ostream* openVerifyLogFile();

    protected:

      double rampUpFeatureFac;

    }; // parMaxMonomialData


  //  The branchSubTreeWay class
  class parMaxMonomSubThreeWay : 
  public parallelBranchSub,
    public maxMonomSubThreeWay
    {
    public:
    parMaxMonomSubThreeWay() : maxMonomSubThreeWay() {}
      ~ parMaxMonomSubThreeWay() {}
      
      void pack(utilib::PackBuffer &outBuffer);
      void unpack(utilib::UnPackBuffer & inBuffer);

      parallelBranchSub* makeParallelChild(int whichChild);

      parallelBranching* pGlobal() const { return mpGlobal; };

      void setMPGlobal(parMaxMonomialData* ptr) { mpGlobal = ptr; };

      void boundComputation(double* controlParam);

    protected:

      parMaxMonomialData* mpGlobal;

      virtual maxMonomSub * allocateObject() const 
      {
	DEBUGPR(20,ucout << "allocating parallel child" << endl);
	parMaxMonomSubThreeWay* newObj = new parMaxMonomSubThreeWay();
	newObj->setMPGlobal(mpGlobal);
	return newObj;
      }
 
    }; 
}// namespace pebbl

#endif
#endif
