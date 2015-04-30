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
 * \file loadBalPair.h
 * \author Jonathan Eckstein
 *
 * A little class useful in pairing up processors to do load balancing.
 * Had to be separated from the rest of the load balancing code so it
 * could be used in a template.
 */

#ifndef pebbl_loadBalPair_h
#define pebbl_loadBalPair_h

#include <acro_config.h>
#include <utilib/std_headers.h>

namespace pebbl {


class loadBalPair;

class loadBalPair
{
public:
  
  int donors;
  int receivers;

  void reset()
    {
      donors    = 0;
      receivers = 0;
    };
  
  loadBalPair() :
  donors(0),
  receivers(0)
    { };
  
  void operator+=(const loadBalPair& other)
    {
      donors    += other.donors;
      receivers += other.receivers;
    };

  void operator-=(const loadBalPair& other)
    {
      donors    -= other.donors;
      receivers -= other.receivers;
    };

};

} // namespace pebbl

inline std::ostream& operator<<(std::ostream& s,const pebbl::loadBalPair& pair)
{
  return s << "donors=" << pair.donors << " receivers=" << pair.receivers;
}

#endif
