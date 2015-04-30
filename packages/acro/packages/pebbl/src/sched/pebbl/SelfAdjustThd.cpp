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
// SelfAdjustThread.cpp
//
// Implements thread objects that try to adjust themselves to a given
// timeslice allocation
//
// Jonathan Eckstein
//

#include <acro_config.h>
#include <utilib/seconds.h>
#include <pebbl/fundamentals.h>
#include <pebbl/SelfAdjustThread.h>

namespace pebbl {


#define EPSILON 1e-16


SelfAdjustThread::SelfAdjustThread(double quantum_,
				   int sampleSize_,
				   double controlParam_,
				   double maxControl_,
				   double minControl_,
				   double maxAdjustRatio_,
				   int    Group_,
				   double bias_
#ifdef ACRO_HAVE_MPI
				   ,MPI_Comm comm_ 
#endif //ACRO_HAVE_MPI
				   ) :
				   
#ifdef ACRO_HAVE_MPI
				   ThreadObj(Group_,bias_,comm_),
#else
				   ThreadObj(Group_,bias_),
#endif
				   timeSample(sampleSize_),
				   controlSample(sampleSize_)
{
  quantum        = quantum_;
  lastControl    = controlParam_;
  maxControl     = maxControl_;
  minControl     = minControl_;
  maxAdjustRatio = maxAdjustRatio_;
  sampleSize     = sampleSize_;

  inSample    = 0;
  cursor      = 0;
  lastQuantum = quantum;
  timeSum     = 0;
  controlSum  = 0;

  overallTimeSum = 0;
}



ThreadObj::RunStatus SelfAdjustThread::exec()
{
  double currentControl;
  double adjustedLastControl = lastControl*quantum/lastQuantum;
  if (inSample > 0)
    {
      currentControl = (controlSum/timeSum)*quantum;
      double maxValue = std::min(adjustedLastControl*maxAdjustRatio,maxControl);
      if (currentControl > maxValue)
	currentControl = maxValue;
      else
	{
	  double minValue = std::max(adjustedLastControl/maxAdjustRatio,
				     minControl);
	  if (currentControl < minValue)
	    currentControl = minValue;
	}
      lastControl = currentControl;
    }
  else
    currentControl = lastControl;

  double time = -CPUSeconds();
  RunStatus returnCode = run(&currentControl); // May modify currentControl
  time           += CPUSeconds()+EPSILON;      // if it runs out of work etc.
  timeSum        += time;
  overallTimeSum += time;
  controlSum     += currentControl;
  lastQuantum     = quantum;

  if (inSample == sampleSize) 
    {
      timeSum -= timeSample[cursor];
      if (timeSum < 0)
	timeSum = 0;
      controlSum -= controlSample[cursor];
      if (controlSum < 0)
	controlSum = 0;
    }
  else
    inSample++;

  timeSample[cursor]      = time;
  controlSample[cursor++] = currentControl;
  if (cursor == sampleSize)
    cursor = 0;

  return returnCode;
}

} // namespace pebbl
