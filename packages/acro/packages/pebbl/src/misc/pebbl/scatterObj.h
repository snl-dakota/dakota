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
 * \file scatterObj.h
 * \author Jonathan Eckstein
 *
 * Logic for making random decisions as to whether to move work to other
 * processors/clusters.
 */

#ifndef pebbl_scatterObj_h
#define pebbl_scatterObj_h

#include <acro_config.h>
#include <utilib/std_headers.h>

namespace pebbl {


class scatterObj
{
public:

  double probability(double actual, double target) const;

  scatterObj() {}

  void reset(double minRatio_,
	     double maxRatio,
	     double minProb,
	     double targetProb_,
	     double maxProb);

  double minRatio;
  double maxRatio;
  double targetProb;
  double lowFac;
  double highFac;

  void write(std::ostream& s) const;

};

} // namespace pebbl


#endif


