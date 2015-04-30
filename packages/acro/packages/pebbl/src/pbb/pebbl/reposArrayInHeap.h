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
 * \file reposArrayInHeap.h
 * \author Jonathan Eckstein
 *
 * A little class used to help manage the repository in parallel --
 * in particular, to merge multiple arrays of solutionIds 
 */

#ifndef pebbl_reposArrayInHeap_h
#define pebbl_reposArrayInHeap_h

#include <acro_config.h>
#include <utilib/std_headers.h>
#include <pebbl/branching.h>


namespace pebbl {


class reposArrayInHeap
{
public:

  reposArrayInHeap() :
    arrayP(NULL),
    cursor(0)
    { };

  void setup(BasicArray<solutionIdentifier>& ref)
    {
      arrayP = &ref;
    };

  void reset()
    {
      cursor = 0;
    };

  solutionIdentifier* advanceCursor()
    {
      return &((*arrayP)[cursor++]);
    };

  bool done() const
    {
      return cursor >= arrayP->size();
    };

  int compare(const reposArrayInHeap& other) const
    {
      bool iAmDone   = done();
      bool otherDone = other.done();
      if (iAmDone || otherDone)
	return (int) iAmDone - (int) otherDone;
      return (*arrayP)[cursor].compare((*other.arrayP)[other.cursor]);
    };

  // Dummies, at least for now
  void write(std::ostream& s) const {};
  void read(std::istream& s) {};

 protected:

  BasicArray<solutionIdentifier>* arrayP;
  size_type cursor;

};


} // namespace pebbl


// Don't know if we really need this...
// std::ostream& operator<<(std::ostream&, const pebbl::reposArrayInHeap& );


#endif
