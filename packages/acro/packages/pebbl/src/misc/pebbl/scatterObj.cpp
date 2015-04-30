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
// scatterObj.cpp
//
// Logic for making random decisions as to whether to move work to other
// processors/clusters.
//
// Jonathan Eckstein
//

#include <acro_config.h>
#include <pebbl/scatterObj.h>
#include <utilib/_math.h>


namespace pebbl {

  void scatterObj::reset(double minRatio_,
			 double maxRatio_,
			 double minProb,
			 double targetProb_,
			 double maxProb)
{
  minRatio = std::min(minRatio_,1.0);
  if (minRatio < 0)
    minRatio = 0;
  maxRatio = std::max(maxRatio_,1.0);

  targetProb  = targetProb_;

  if (minRatio < 1)
    lowFac = (targetProb - minProb)/(1 - minRatio);
  else
    lowFac = 0;
  if (maxRatio > 1)
    highFac = (maxProb - targetProb)/(maxRatio - 1);
  else
    highFac = 0;
}


double scatterObj::probability(double actual, double target) const
{
  if (target == 0)
    return targetProb;
  double ratio = actual/target;
  if (ratio < 1)
    return targetProb - (1 - std::max(ratio,minRatio))*lowFac;
  else   // (ratio >= 1)
    return targetProb + (std::min(ratio,maxRatio) - 1)*highFac;
}


void scatterObj::write(std::ostream& s) const
{
  s << minRatio << "<--(" << lowFac << ")--("
    << highFac << ")-->" << maxRatio;
}


} // namespace pebbl

