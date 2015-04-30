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


#include <utilib/CommonIO.h>
#include <utilib/seconds.h>
#include <pebbl/Docking.h>

using namespace pebbl;


int main(int argc, char* argv[])
{
  return driver<serialDocking,parallelDocking>(argc,argv);
}
