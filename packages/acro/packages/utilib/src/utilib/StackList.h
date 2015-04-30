/*  _________________________________________________________________________
 *
 *  UTILIB: A utility library for developing portable C++ codes.
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README file in the top UTILIB directory.
 *  _________________________________________________________________________
 */

/**
 * \file StackList.h
 *
 * Defines the utilib::StackList class
 */

#ifndef utilib_StackList_h
#define utilib_StackList_h

#include <utilib_config.h>
#include <utilib/std_headers.h>
#include <utilib/LinkedList.h>

namespace utilib {


/**
 * A data type that defines a stack using a doubly-linked
 * list data structure.  This class is a simple derivation of 
 * LinkedList that sets the stack mode.
 */
template <class T>
class StackList : public LinkedList<T>
{
public:

  /// Empty constructor.
  StackList() : LinkedList<T>() {this->mode = this->stackLL;}

};

} // namespace utilib

#endif
