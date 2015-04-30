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

#include <acro_config.h>
#include <utilib/seconds.h>
#include <pebbl/parallelLipshitzian.h>

using namespace pebbl;
using utilib::BasicArray;


// Note: this driver does not conform exactly to the standard pattern,
// and so does not use the standard "driver<B,PB>" template.


/// A simple quadratic problem
class FunctionClass
{
public:

 double operator()(BasicArray<double>& vec)
	{
	double ans=0.0;
	for (unsigned int i=0; i<vec.size(); i++)
	  ans += vec[i]*vec[i];
	return ans;
	}

  /// Lower bounds on the search domain
  BasicArray<double> lower;

  /// Upper bounds on the search domain
  BasicArray<double> upper;
};



int main(int argc, char* argv[])
{
try {
  ///
  /// Reset the UTILIB global timing information
  ///
  InitializeTiming();
  ///
  /// Setup the problem
  ///
  FunctionClass problem;
  problem.lower.resize(3);
  problem.upper.resize(3);
  problem.lower << 10.0;
  problem.upper << 11.0;
  double Lipshitz_constant = 400.0;
  ///
  /// If we're using MPI, then initialize the MPI data structures
  ///
#if defined(ACRO_HAVE_MPI)
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
     CommonIO::setIOFlush(1);
     ///
     /// Create the optimizer, initialize it with command line parameters
     /// and run it.
     ///
     parallelLipshitzian<FunctionClass> optimizer;
     if (optimizer.setup(argc,argv,problem)) {
	optimizer.Lipshitz_constant = Lipshitz_constant;
        optimizer.reset();
        optimizer.solve();
        }
     ///
     /// Clean up parallel I/O
     ///
     CommonIO::end();
     }
  else {
#endif
  ///
  /// Do serial optimization
  ///
     serialLipshitzian<FunctionClass> optimizer;
     if (optimizer.setup(argc,argv,problem)) {
	optimizer.Lipshitz_constant = Lipshitz_constant;
        optimizer.reset();
        optimizer.solve();
        }
#ifdef ACRO_HAVE_MPI
     }
  uMPI::done();
#endif
  }
///
/// Use a standard block of catch routines, which catches all 
/// STL exception types explicitly
///
STD_CATCH(;)

return 0;
}
