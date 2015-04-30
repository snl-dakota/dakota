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
// loadBalPair.cpp
//
// A little class useful in pairing up processors to do load balancing.
// Had to be separated from the rest of the load balancing code so it
// could be used in a template.
//
// Jonathan Eckstein
//

#include <acro_config.h>
#include <pebbl/loadBalPair.h>
