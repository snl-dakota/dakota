/*  _________________________________________________________________________
 *
 *  PICO: A C++ library of scalable branch-and-bound and related methods.
 *  Copyright (c) 2001, Sandia National Laboratories.
 *  This software is distributed under the GNU Lesser General Public License.
 *  For more information, see the README file in the top PICO directory.
 *  _________________________________________________________________________
 */

#include <acro_config.h>
#include <utilib/seconds.h>

#ifdef ACRO_USING_COIN_CBC
#include <ClpSimplex.hpp>
#include <CoinPackedMatrix.hpp>
#include <CoinPackedVector.hpp>
#endif

#ifdef ACRO_HAVE_MPI
#include "parMonomial.h"
#define outstream ucout
#define IO(action) if (uMPI::iDoIO) { CommonIO::end_tagging(); action; }
#else
#include "serialMonomial.h"
typedef void parMaxMonomialData;
#define outstream cout
#define IO(action) action;
#endif


using namespace pebblMonom;
using namespace std;


int main(int argc,char** argv)
{
  // JE addition: if CLP is not present (which we detect by looking
  // for CBC) then just compile a standard driver program that reads a
  // single instance and solves it.  Otherwise, do the LP-Boost
  // procedure as originally coded by Noam.

#ifndef ACRO_USING_COIN_CBC
  return driver<maxMonomialData,parMaxMonomialData>(argc,argv);
#else

  maxMonomialData*    instance  = NULL;
  parMaxMonomialData* pinstance = NULL;
  bool parallel = false;
  
#ifdef ACRO_HAVE_MPI
  uMPI::init(&argc,&argv,MPI_COMM_WORLD);
  int nprocessors = uMPI::size;
  ///
  /// Do parallel optimization if MPI indicates that we're using more than
  /// one processor
  ///
  if (parallel_exec_test<parallelBranching>(argc,argv,nprocessors)) {
    ///
    /// Manage parallel I/O explicitly with the utilib::CommonIO tools
    ///
    CommonIO::begin();
    /// CommonIO::setIOFlush(1);  // JE: not sure why this needed
    ///                           // ...keeps you from seeing things run
    /// Create the optimizer, initialize it with command line parameters
    /// and run it.
    ///
    parallel  = true;
    pinstance = new parMaxMonomialData;
    instance  = pinstance;
  }
  else {
#endif

    instance = new maxMonomialData;

#ifdef ACRO_HAVE_MPI
  }
#endif

  IO();  // Turn off output tagging if parallel

  if (!instance->setup(argc,argv))
    {
      cerr << "error reading data file" << endl;
      return EXIT_FAILURE;
    }

  const double D = 3.0/instance->obsNum();
  const double EPSILON = 0.0000001;

  const int iterNum = instance->getIterations();
  assert(iterNum > 0);

  double * dataWts = new double[instance->obsNum()];
  double * col_lb = new double[instance->obsNum()+1];
  double * col_ub = new double[instance->obsNum()+1];
  double * objective = new double[instance->obsNum()+1];
  double objectiveBeta = 0.0;
  double myStartTime   = 0.0;

  try
    {  
      ClpSimplex lp;
      lp.setOptimizationDirection(1);               // minimization
#ifdef ACRO_HAVE_MPI
      if (!uMPI::iDoIO)
	lp.setLogLevel(0);
#endif
      CoinPackedMatrix matrix(false,0,0);
      matrix.setDimensions(0, instance->obsNum());
      CoinPackedVector row;

      for (int i=0; i < instance->obsNum(); i++)
	{
	  if (instance->weightsReadIn())
	    dataWts[i] = instance->getWeight(i);
	  else
	    dataWts[i]=1.0/instance->obsNum(); // initialize data weights
	  col_lb[i] = 0.0;
	  col_ub[i] = D;
	  objective[i] = 0.0;
	  row.insert(i,1.0);
	}
      objective[instance->obsNum()] = 1;
      row.insert(instance->obsNum(),0.0);
      col_lb[instance->obsNum()] = -COIN_DBL_MAX;
      col_ub[instance->obsNum()] = COIN_DBL_MAX;

      matrix.appendRow(row);
      double row_lb = 1.0;  // lb for normalization (equality) constraint
      double row_ub = 1.0;  // ub for normalization constraint

      lp.loadProblem(matrix, col_lb, col_ub, objective, &row_lb, &row_ub);

      for(int j=0; j < iterNum; j++)
	{
	  if ((j > 0) || !(instance->weightsReadIn()))
	    instance->setWeight(dataWts,instance->obsNum());

	  if ((j > 0) && instance->writingInstances())
	    IO(instance->writeInstanceToFile(j+1));

	  InitializeTiming();

	  IO(outstream << "col gen iter: " << j << " objectiveBeta: " 
			<< objectiveBeta << endl;
	     myStartTime = CPUSeconds());

	  instance->reset();

#ifdef ACRO_HAVE_MPI
	  if (parallel)
	    {
	      pinstance->printConfiguration();
	      CommonIO::begin_tagging();
	    }
#endif

	  instance->solve();
	  instance->resetTimers();

	  // obtain optimal monomial
	  solution* s = instance->getSolution();
	  maxMonomSolution* sl = dynamic_cast<maxMonomSolution*>(s);
	  const monomialObj& monom = sl->getMonomialObj();

	  IO(outstream << "CPU time is " << CPUSeconds() - myStartTime << endl;
	     outstream << "Solution is ";
	     monom.printInMonom(outstream);
	     outstream << endl);

	  set<pebblMonom::size_type> posCovg;
	  instance->getPosCovg(monom,posCovg);
	  set<pebblMonom::size_type> negCovg;
	  instance->getNegCovg(monom,negCovg);
	  double posWt = instance->getWeight(posCovg);
	  double negWt = instance->getWeight(negCovg);
      
	  if (abs(posWt-negWt) <= objectiveBeta + EPSILON)
	    {
	      IO(outstream << "terminating with subcube score: " 
		 << abs(posWt-negWt) << endl;);
	      break;
	    }

	  const set<pebblMonom::size_type> * correctCovg = &posCovg;
	  const set<pebblMonom::size_type> * incorrectCovg = &negCovg;
	  if (negWt > posWt)
	    {
	      correctCovg = &negCovg;
	      incorrectCovg = &posCovg;
	    }

	  int totalCovNum = posCovg.size() + negCovg.size();
	  IO(outstream << "allocating dual row of (nonzero) size: " 
	     << totalCovNum + 1 << endl);
	  int * rowCols = new int[totalCovNum+1];
	  double * rowVals = new double[totalCovNum+1];
	  int k = 0;
	  set<pebblMonom::size_type>::const_iterator 
	    correctIter = correctCovg->begin();
	  set<pebblMonom::size_type>::const_iterator 
	    incorrectIter = incorrectCovg->begin();

	  while (correctIter != correctCovg->end()
		 || incorrectIter != incorrectCovg->end())
	    {
	      if (correctIter != correctCovg->end() 
		  && incorrectIter != incorrectCovg->end())
		{
		  assert(*correctIter != *incorrectIter);
		}
	      if ((correctIter != correctCovg->end() 
		   && incorrectIter != incorrectCovg->end() 
		   && *correctIter < *incorrectIter) 
		  || incorrectIter == incorrectCovg->end())
		{
		  rowCols[k] = *correctIter;
		  rowVals[k] = 1.0;
		  correctIter++;
		}
	      else
		{
		  rowCols[k] = *incorrectIter;
		  rowVals[k] = -1.0;
		  incorrectIter++;
		}
	      k++;
	    }
	  assert(k == totalCovNum);
	  rowCols[k] = instance->obsNum();
	  rowVals[k] = -1.0;
	  lp.addRow(totalCovNum+1, rowCols, rowVals, -COIN_DBL_MAX, 0.0);
	  lp.initialSolve();
	  delete [] rowCols;
	  delete [] rowVals;
	  memcpy(dataWts,
		 lp.getColSolution(),
		 instance->obsNum()*sizeof(double));
#ifdef ACRO_VALIDATING
#ifdef ACRO_HAVE_MPI
	  double* tempArray = new double[instance->obsNum()];
	  if (uMPI::rank == 0)
	    memcpy(tempArray,dataWts,instance->obsNum()*sizeof(double));
	  uMPI::broadcast(tempArray,instance->obsNum(),MPI_DOUBLE,0);
	  for (int i=0; i<instance->obsNum(); i++)
	    if (dataWts[i] != tempArray[i])
	      {
		cerr << "Validation error:  processor " << uMPI::rank
		     << " has non-matching weight of " << dataWts[i]
		     << " for weight " << i << ", expecting "
		     << tempArray[i] << endl;
		return EXIT_FAILURE;
	      }
#endif
#endif
	  objectiveBeta = lp.getColSolution()[instance->obsNum()];
	}
      delete instance;
#ifdef ACRO_HAVE_MPI    
      CommonIO::end();
      uMPI::done();
#endif
    }
  catch(CoinError & ex){
    cerr << "Exception:" << ex.message() << endl 
	 << " from method " << ex.methodName() << endl
	 << " from class " << ex.className() << endl;
    return EXIT_FAILURE;
  }
  catch(...)
    {
      cerr << "Unknown exception caught" << endl;
      return EXIT_FAILURE;
    }

  delete [] dataWts;
  delete [] objective;
  delete [] col_lb;
  delete [] col_ub;

  return EXIT_SUCCESS;

#endif     // of ifndef-else for whether we have CLP

}

