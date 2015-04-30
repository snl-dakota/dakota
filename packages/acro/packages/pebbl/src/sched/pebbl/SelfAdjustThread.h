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
 * \file SelfAdjustThread.h
 * \author Jonathan Eckstein
 *
 * Implements thread objects that try to adjust themselves to a given
 * timeslice allocation
 */

#ifndef pebbl_SelfAdjustThread_h
#define pebbl_SelfAdjustThread_h

#include <acro_config.h>
#include <utilib/DoubleVector.h>
#include <pebbl/ThreadObj.h>

namespace pebbl {

using utilib::DoubleVector;


//
// ThreadObj is an abstract class derived from the ThreadObj abstract class.
// It inherits most of its methods from ThreadObj.  However, the basic
// functionality of the thread is not performed by "exec()",
// but by "run(double* controlParameterP)".  The method 
// "exec()" first sets "controlParameter" to try to 
// acheive the desired run time "timeSlice", then calls 
// "run(controlParameterP)".  Using the assumption that time taken 
// by run(controlParameterP) method is related in an approximately linear 
// way to the value of "*controlParameterP", it tries to learn the correct 
// value of "controlParameter".
//
// The run method may alter the value of its argument to reflect the
// amount of work it actually did.  For example, suppose the argument was 
// 50 and run interprets this as meaning it should process 50 subproblems.
// If it runs out of work after 20 subproblems, it should change 
// *controlParameterP to 20.0.
//
// An additional feature is method that returns the total time so far consumed
// by the thread.
//

class SelfAdjustThread : virtual public ThreadObj
{
public:

  double quantum;

  RunStatus exec();

  double totalTime() { return overallTimeSum; }; // Total time used by thread.

  // Constructor.

  SelfAdjustThread(double quantum_        = .001,   // in seconds
		   int sampleSize_        = 10,     // Memory for adaptive alg.
		   double controlParam_   = 1.0,    // Initial control value
		   double maxControl_     = 1000.0, // Maximum value of above
		   double minControl_     = 1e-7,   // Minimum value of above
		   double maxAdjustRatio_ = 5,      // Max change in conrol
		   int Group_             = 0,      // For ThreadObj constr.
		   double bias_           = 0
#ifdef ACRO_HAVE_MPI
		   ,MPI_Comm comm_ = MPI_COMM_WORLD
#endif
		   );

protected:
  
  DoubleVector timeSample;      // Data from last 'inSample' calls.
  DoubleVector controlSample;
  double overallTimeSum;
  double timeSum;
  double controlSum;
  double lastControl;
  double lastQuantum;
  double maxControl;
  double minControl;
  double maxAdjustRatio;
  int inSample;
  int cursor;
  int sampleSize;
};

} // namespace pebbl

#endif
