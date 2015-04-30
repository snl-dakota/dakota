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
 * \file workerInHeap.h
 * \author Jonathan Eckstein
 *
 * A little class used by the hub code in PEBBL.  Had to be moved to its
 * own header file because it's used in a template.
 */

#ifndef pebbl_workerInHeap_h
#define pebbl_workerInHeap_h

#include <acro_config.h>
#include <utilib/std_headers.h>

namespace pebbl {


class parallelBranching;


class workerInHeap
{
public:
  int w,location;
  parallelBranching* global;

  virtual int compare(const workerInHeap& other) const;
  bool equivalent(workerInHeap& other) {return false;};
  void write(std::ostream& wy) const {};
  void read(std::istream& we) {};

  workerInHeap(){ w = 0; location = 0; global = NULL; };
};


// This class added later -- similar, but uses a different comparison 
// function.

class workerInQHeap : public workerInHeap
{
 public:
  int compare(const workerInHeap& other) const;

  workerInQHeap() { };
};


} // namespace pebbl


#endif
