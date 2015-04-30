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
// workerInHeap.cpp
//
// A little class used by the hub code in PEBBL.  Had to be moved to its
// own header file because it's used in a template.
//
// Jonathan Eckstein
//


#include <acro_config.h>
#ifdef ACRO_HAVE_MPI

#include <pebbl/parBranching.h>

using namespace std;

namespace pebbl {


int workerInHeap::compare(const workerInHeap& other) const
{
  return  global->lowerLoadWorker(w,other.w);
}


int workerInQHeap::compare(const workerInHeap& other) const
{
  return global->worseQualityWorker(w,other.w);
}


} // namespace pebbl


#endif
