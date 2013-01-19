/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        DataStrategy
//- Description:  Class implementation
//- Owner:        Mike Eldred

#include "DataStrategy.hpp"
#include "data_io.hpp"


using namespace std;

namespace Dakota {

// Default constructor:
DataStrategyRep::DataStrategyRep():
  strategyType("single_method"), graphicsFlag(false), tabularDataFlag(false),
  tabularDataFile("dakota_tabular.dat"), outputPrecision(0), 
  resultsOutputFlag(false), resultsOutputFile("dakota_results.txt"), 
  iteratorServers(0), hybridLSProb(0.1), //hybridProgThresh(0.5),
  concurrentRandomJobs(0), concurrentSeed(0), referenceCount(1)
{ }


void DataStrategyRep::write(MPIPackBuffer& s) const
{
  s << strategyType << graphicsFlag << tabularDataFlag << tabularDataFile
    << outputPrecision << resultsOutputFlag << resultsOutputFile 
    << iteratorServers << iteratorScheduling << methodPointer
    << hybridMethodList << hybridType //<< hybridProgThresh
    << hybridGlobalMethodPointer << hybridLocalMethodPointer << hybridLSProb
    << concurrentRandomJobs << concurrentSeed << concurrentParameterSets;
}


void DataStrategyRep::read(MPIUnpackBuffer& s)
{
  s >> strategyType >> graphicsFlag >> tabularDataFlag >> tabularDataFile
    >> outputPrecision >> resultsOutputFlag >> resultsOutputFile 
    >> iteratorServers >> iteratorScheduling >> methodPointer
    >> hybridMethodList >> hybridType //>> hybridProgThresh
    >> hybridGlobalMethodPointer >> hybridLocalMethodPointer >> hybridLSProb
    >> concurrentRandomJobs >> concurrentSeed >> concurrentParameterSets;
}


void DataStrategyRep::write(std::ostream& s) const
{
  s << strategyType << graphicsFlag << tabularDataFlag << tabularDataFile
    << outputPrecision << resultsOutputFlag << resultsOutputFile 
    << iteratorServers << iteratorScheduling << methodPointer
    << hybridMethodList << hybridType //<< hybridProgThresh
    << hybridGlobalMethodPointer << hybridLocalMethodPointer << hybridLSProb
    << concurrentRandomJobs << concurrentSeed << concurrentParameterSets;
}


DataStrategy::DataStrategy(): dataStratRep(new DataStrategyRep())
{
#ifdef REFCOUNT_DEBUG
  Cout << "DataStrategy::DataStrategy(), dataStratRep referenceCount = "
       << dataStratRep->referenceCount << endl;
#endif
}


DataStrategy::DataStrategy(const DataStrategy& data_strategy)
{
  // Increment new (no old to decrement)
  dataStratRep = data_strategy.dataStratRep;
  if (dataStratRep) // Check for an assignment of NULL
    dataStratRep->referenceCount++;

#ifdef REFCOUNT_DEBUG
  Cout << "DataStrategy::DataStrategy(DataStrategy&)" << endl;
  if (dataStratRep)
    Cout << "dataStratRep referenceCount = " << dataStratRep->referenceCount
	 << endl;
#endif
}


DataStrategy& DataStrategy::operator=(const DataStrategy& data_strategy)
{
  if (dataStratRep != data_strategy.dataStratRep) { // normal case: old != new
    // Decrement old
    if (dataStratRep) // Check for NULL
      if ( --dataStratRep->referenceCount == 0 ) 
	delete dataStratRep;
    // Assign and increment new
    dataStratRep = data_strategy.dataStratRep;
    if (dataStratRep) // Check for NULL
      dataStratRep->referenceCount++;
  }
  // else if assigning same rep, then do nothing since referenceCount
  // should already be correct

#ifdef REFCOUNT_DEBUG
  Cout << "DataStrategy::operator=(DataStrategy&)" << endl;
  if (dataStratRep)
    Cout << "dataStratRep referenceCount = " << dataStratRep->referenceCount
	 << endl;
#endif

  return *this;
}


DataStrategy::~DataStrategy()
{
  if (dataStratRep) { // Check for NULL
    --dataStratRep->referenceCount; // decrement
#ifdef REFCOUNT_DEBUG
    Cout << "dataStratRep referenceCount decremented to "
         << dataStratRep->referenceCount << endl;
#endif
    if (dataStratRep->referenceCount == 0) {
#ifdef REFCOUNT_DEBUG
      Cout << "deleting dataStratRep" << endl;
#endif
      delete dataStratRep;
    }
  }
}

} // namespace Dakota
