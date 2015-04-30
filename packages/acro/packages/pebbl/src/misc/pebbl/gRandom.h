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
 * \file gRandom.h
 * \author Jonathan Eckstein
 *
 * A global uniform random number -- coding convenience.
 */

#ifndef pebbl_gRandom_h
#define pebbl_gRandom_h

#include <acro_config.h>
#include <utilib/RNG.h>
#include <utilib/Uniform.h>
//#include <utilib/paramTable.h>

namespace pebbl {


utilib::RNG*    gRandomRNG();
extern utilib::Uniform gRandom;
extern size_t randomSeed;

// The following commented-out code is the functionality we want.
// Work around for gcc compiler is below

// void gRandomReSeed(int seed=(int) randomSeed(),int processorVariation=1);

void gRandomReSeed(int seed, int processorVariation=1);

void gRandomReSeed();

} // namespace pebbl

#endif
